// Strawberry Net
#include "Strawberry/Net/Socket/TCPSocket.hpp"
#include "Strawberry/Net/Socket/API.hpp"
#include "Strawberry/Net/Socket/Platform.hpp"
// Strawberry Core
#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/Markers.hpp"
#include <Strawberry/Core/IO/Logging.hpp>
#include <sys/poll.h>
// OS-Level Networking Headers
#if STRAWBERRY_TARGET_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif // STRAWBERRY_TARGET_WINDOWS


namespace Strawberry::Net::Socket
{
	Core::Result<TCPSocket, Error> TCPSocket::Connect(const Endpoint& endpoint)
	{
		Core::Logging::Info("Connecting TCP Socket to {}", endpoint.ToString());


		addrinfo hints{.ai_flags = AI_ADDRCONFIG, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP};
		hints.ai_family = endpoint.GetAddress().IsIPv4() ? AF_INET
			: endpoint.GetAddress().IsIPv6() ? AF_INET6
			: Core::Unreachable<int>();


		sockaddr_storage peer = endpoint.GetPlatformRepresentation();
		socklen_t peerLen = 0;
		switch (peer.ss_family)
		{
		case AF_INET:  peerLen = sizeof(sockaddr_in); break;
		case AF_INET6: peerLen = sizeof(sockaddr_in6); break;
		default: Core::Unreachable();
		}

		auto socketHandle = socket(peer.ss_family, SOCK_STREAM, IPPROTO_TCP);
		if (socketHandle == -1)
		{
			Core::Logging::Error("Failed to create TCP Socket for endpoint {}", endpoint.ToString());
			return ErrorSocketCreation {};
		}

		auto connection = connect(socketHandle, (const struct sockaddr*) &peer, peerLen);
		if (connection == -1)
		{
			Core::Logging::Error("Failed to estabilish TCP connection to endpoint {}", endpoint.ToString());
			return ErrorAddressResolution {};
		}

		TCPSocket tcpSocket(socketHandle, endpoint);

		SOCKET_OPTION_TYPE keepAlive = 1;
		SOCKET_ERROR_CODE_TYPE optResult =
			setsockopt(tcpSocket.mSocket, SOL_SOCKET, SO_KEEPALIVE,
					   reinterpret_cast<const char*>(&keepAlive), sizeof(keepAlive));
		Core::AssertEQ(optResult, 0);


		Core::Logging::Info("Connected TCP Socket ({}) to {}", tcpSocket.mSocket, endpoint.ToString());
		return tcpSocket;
	}


	TCPSocket::TCPSocket(SocketHandle socketHandle, Endpoint endpoint)
		: mSocket(socketHandle)
		, mEndpoint(std::move(endpoint)) {}


	TCPSocket::TCPSocket(TCPSocket&& other) noexcept
		: mSocket(std::exchange(other.mSocket, -1))
		, mEndpoint(std::move(other.mEndpoint)) {}


	TCPSocket& TCPSocket::operator=(TCPSocket&& other) noexcept
	{
		if (this != &other)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(other));
		}

		return *this;
	}


	TCPSocket::~TCPSocket()
	{
		if (mSocket != -1)
		{
			Core::Logging::Info("Closing socket ({})", mSocket);
			CLOSE_SOCKET_FUNCTION(mSocket);
		}
	}


	const Endpoint& TCPSocket::GetEndpoint() const noexcept
	{
		return mEndpoint;
	}


	bool TCPSocket::Poll() const
	{
		SOCKET_POLL_FD_TYPE fds[] = {
			{ mSocket, POLLIN, 0}
		};

		int pollResult = SOCKET_POLL_FUNCTION(fds, 1, 0);
		if (pollResult <= 0)
		{
			Core::Logging::Error("Error when polling TCP socket! Error code: {}", API::GetError());
			return false;
		}

		return static_cast<bool>(fds[0].revents & POLLIN);
	}


	StreamReadResult TCPSocket::Read(size_t length)
	{
		auto buffer = Core::IO::DynamicByteBuffer::Zeroes(length);

		auto recvResult = recv(mSocket, reinterpret_cast<char*>(buffer.Data()), buffer.Size(), 0);
		if (recvResult == SOCKET_ERROR_CODE)
		{
			switch (auto error = API::GetError())
			{
			default:
				Core::Logging::Error("Unhandled error code in TCPSocket::Read! Code: {}.", error);
				return ErrorUnknown{};
			}
		}

		Core::Assert(recvResult > 0);
		buffer.Resize(recvResult);
		return buffer;
	}


	StreamReadResult TCPSocket::ReadAll(size_t length)
	{
		auto buffer = Core::IO::DynamicByteBuffer::WithCapacity(length);

		while (buffer.Size() < length)
		{
			auto read = Read(length - buffer.Size());

			if (read.IsOk())
			{
				buffer.Push(read.Unwrap());
			}
			else
			{
				return read.Err();
			}
		}

		return buffer;
	}


	StreamWriteResult TCPSocket::Write(const Core::IO::DynamicByteBuffer& bytes)
	{
		size_t bytesSent = 0;

		while (bytesSent < bytes.Size())
		{
			auto sendResult = send(mSocket, reinterpret_cast<const char*>(bytes.Data()) + bytesSent, bytes.Size() - bytesSent, 0);
			if (sendResult > 0)
			{
				bytesSent += sendResult;
			}
			else
			{
				switch (auto error = API::GetError())
				{
				default:
					Core::Logging::Error("Unhandled error code in TCPSocket::Write! Code: {}.", error);
					return ErrorUnknown{};
				}
			}
		}

		Core::Assert(bytesSent == bytes.Size());
		return Core::Success;
	}
} // namespace Strawberry::Net::Socket
