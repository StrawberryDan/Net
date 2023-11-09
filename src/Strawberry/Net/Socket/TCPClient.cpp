#include "Strawberry/Net/Socket/TCPClient.hpp"


#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/Markers.hpp"
#include "Strawberry/Core/Util/Stubs.hpp"


#if _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#elif __APPLE__ || __linux__


#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>


#endif // _WIN32


namespace Strawberry::Net::Socket
{
	Core::Result<TCPClient, Error> TCPClient::Connect(const Endpoint& endpoint)
	{
		TCPClient client;

		addrinfo hints{.ai_flags = AI_ADDRCONFIG, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP};
		if (endpoint.GetAddress()->IsIPv4()) hints.ai_family = AF_INET;
		else if (endpoint.GetAddress()->IsIPv6())
			hints.ai_family = AF_INET6;
		else
			Core::Unreachable();
		addrinfo* peerAddress = nullptr;
		auto      addrResult  = getaddrinfo(endpoint.GetAddress()->AsString().c_str(), std::to_string(endpoint.GetPort()).c_str(), &hints, &peerAddress);
		if (addrResult != 0)
		{
			freeaddrinfo(peerAddress);
			return Error::AddressResolution;
		}

		auto handle = socket(peerAddress->ai_family, peerAddress->ai_socktype, peerAddress->ai_protocol);
		if (handle == -1)
		{
			freeaddrinfo(peerAddress);
			return Error::SocketCreation;
		}

		auto connection = connect(handle, peerAddress->ai_addr, peerAddress->ai_addrlen);
		if (connection == -1)
		{
			freeaddrinfo(peerAddress);
			return Error::EstablishConnection;
		}

		freeaddrinfo(peerAddress);
		client.mSocket = handle;
		return client;
	}


	TCPClient::TCPClient()
		: mSocket(-1)
	{}


	TCPClient::TCPClient(TCPClient&& other) noexcept
		: mSocket(std::exchange(other.mSocket, -1))
	{}


	TCPClient& TCPClient::operator=(TCPClient&& other) noexcept
	{
		if (this != &other)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(other));
		}

		return *this;
	}


	TCPClient::~TCPClient()
	{
		if (mSocket != -1)
		{
#if defined(__APPLE__) || defined(__linux__)
			close(mSocket);
#elif defined(__WIN32)
			closesocket(mSocket);
#else
			Core::Unreachable();
#endif
		}
	}


	bool TCPClient::Poll() const
	{
#if defined(__APPLE__) || defined(__linux__)
		pollfd fds[] = {
			{mSocket, POLLIN, 0}
        };

		int pollResult = poll(fds, 1, 0);
		Core::Assert(pollResult >= 0);
		return static_cast<bool>(fds[0].revents & POLLIN);
#elif defined(__WIN32)
		WSAPOLLFD fds[] =
		{
				{mSocket, POLLIN, 0}
		};
		int pollResult = WSAPoll(fds, 1, 0);
		Core::Assert(pollResult >= 0);
		return static_cast<bool>(fds[0].revents & POLLIN);
#else
		Core::Unreachable();
#endif
	}


	Core::Result<Core::IO::DynamicByteBuffer, Core::IO::Error> TCPClient::Read(size_t length)
	{
		auto   buffer    = Core::IO::DynamicByteBuffer::Zeroes(length);
		size_t bytesRead = 0;

		while (bytesRead < length)
		{
			auto thisRead = recv(mSocket, reinterpret_cast<char*>(buffer.Data()) + bytesRead, length - bytesRead, 0);
			if (thisRead > 0) { bytesRead += thisRead; }
			else { Core::Unreachable(); }
		}

		Core::Assert(bytesRead == length);
		return buffer;
	}


	Core::Result<size_t, Core::IO::Error> TCPClient::Write(const Core::IO::DynamicByteBuffer& bytes)
	{
		size_t bytesSent = 0;

		while (bytesSent < bytes.Size())
		{
			auto thisSend = send(mSocket, reinterpret_cast<const char*>(bytes.Data()) + bytesSent, bytes.Size() - bytesSent, 0);
			if (thisSend > 0) { bytesSent += thisSend; }
			else { Core::Unreachable(); }
		}

		Core::Assert(bytesSent == bytes.Size());
		return bytesSent;
	}
} // namespace Strawberry::Net::Socket
