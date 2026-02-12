// Strawberry Net
#include "Strawberry/Net/Socket/UDPSocket.hpp"
#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Net/Socket/API.hpp"
#include "Strawberry/Net/Socket/Platform.hpp"
// Strawberry Core
#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/Markers.hpp"
#include <Strawberry/Core/IO/Logging.hpp>
#include <cerrno>
#include <netinet/in.h>
// OS-Level Networking Headers
#if STRAWBERRY_TARGET_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>
#endif // STRAWBERRY_TARGET_WINDOWS
// Standard Library
#include <cstdint>
#include <thread>


namespace Strawberry::Net::Socket
{
	Core::Result<UDPSocket, Error> UDPSocket::Create()
	{
		auto handle = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
		if (handle == -1)
		{
			Core::Logging::Error("Failed to create dual band UDP socket!");
			return ErrorSocketCreation {};
		}

		int		ipv6Only = 0;
		auto	optSetResult = setsockopt(
			handle, IPPROTO_IPV6, IPV6_V6ONLY,
			reinterpret_cast<const char*>(&ipv6Only),
			sizeof(ipv6Only));
		Core::Assert(optSetResult == 0);

		UDPSocket	client;
		client.mSocket = handle;
		return client;
	}


	Core::Result<UDPSocket, Error> UDPSocket::CreateIPv4()
	{
		auto handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (handle == -1)
		{
			Core::Logging::Error("Failed to create IPv4 UDP socket!");
			return ErrorSocketCreation {};
		}

		UDPSocket client;
		client.mSocket = handle;
		return client;
	}


	Core::Result<UDPSocket, Error> UDPSocket::CreateIPv6()
	{
		auto handle = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
		if (handle == -1)
		{
			Core::Logging::Error("Failed to create IPv6 UDP socket!");
			return ErrorSocketCreation {};
		}

		UDPSocket client;
		client.mSocket = handle;
		return client;
	}


	UDPSocket::UDPSocket()
		: mSocket(-1) {}


	UDPSocket::UDPSocket(UDPSocket&& other) noexcept
		: mSocket(std::exchange(other.mSocket, -1))
		, mPort(std::move(other.mPort)) {}


	UDPSocket& UDPSocket::operator=(UDPSocket&& other) noexcept
	{
		if (this != &other)
		{
			std::destroy_at(this);
			mSocket = std::exchange(other.mSocket, -1);
		}

		return *this;
	}


	UDPSocket::~UDPSocket()
	{
		if (mSocket != -1)
		{
			CLOSE_SOCKET_FUNCTION(mSocket);
		}
	}


	bool UDPSocket::Poll() const
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

	Core::Result<void, Error> UDPSocket::Bind(uint16_t portNumber) noexcept
	{
		Core::Logging::Info("Binding UDP Socket ({}) to port {}", mSocket, portNumber);


		addrinfo hints
		{
			.ai_flags = AI_ADDRCONFIG,
			.ai_family = AF_UNSPEC,
			.ai_socktype = SOCK_DGRAM,
			.ai_protocol = IPPROTO_UDP
		};
		addrinfo* addressInfo = nullptr;
		SOCKET_ERROR_CODE_TYPE error = 0;


		error = getaddrinfo("::", std::to_string(portNumber).c_str(), &hints, &addressInfo);
		if (error != 0)
		{
			freeaddrinfo(addressInfo);
			Core::Logging::Error("Failed to get address info for [::] at port {}", portNumber);
			return ErrorSystem{};
		}


		error = bind(mSocket, addressInfo->ai_addr, addressInfo->ai_addrlen);
		if (error == SOCKET_ERROR_CODE)
		{
			Core::Logging::Error("Failed to bind UDP socket to port {}", portNumber);
			switch (auto error = API::GetError())
			{
			default:
				Core::Logging::Error("Unhandled error code in UDPSocket::Bind(). Code: {}.", error);
				return ErrorUnknown{};
			}
		}

		freeaddrinfo(addressInfo);

		mPort = portNumber;
		return Core::Success;
	}

	Core::Result<UDPPacket, Error> UDPSocket::Receive()
	{
		// Cannot receive packets if we have not bound ourselves to a port.
		Core::Logging::ErrorIf(!mPort.HasValue(), "Attempted to receive a packet on an unbound UDP port!");
		Core::Assert(mPort.HasValue());

		sockaddr_storage peer{};
		socklen_t		 peerLen   = 0;
		auto			 bytesRead = recvfrom(mSocket,
											  reinterpret_cast<char*>(mBuffer.Data()),
											  mBuffer.Size(),
											  0,
											  reinterpret_cast<sockaddr*>(&peer),
											  &peerLen);

		if (bytesRead >= 0)
		{
			Core::Optional<Endpoint> endpoint;
			if (peer.ss_family == AF_INET)
			{
				auto* sockaddr = reinterpret_cast<sockaddr_in*>(&peer);
				endpoint.Emplace(IPv4Address(Core::IO::ByteBuffer<4>(sockaddr->sin_addr)), sockaddr->sin_port);
			}
			else if (peer.ss_family == AF_INET6)
			{
				auto* sockaddr = reinterpret_cast<sockaddr_in6*>(&peer);
				endpoint.Emplace(IPv6Address(Core::IO::ByteBuffer<16>(sockaddr->sin6_addr)), sockaddr->sin6_port);
			}
			else
			{
				Core::Unreachable();
			}

			return UDPPacket{
				.endpoint = std::move(endpoint),
				.contents = Core::IO::DynamicByteBuffer(mBuffer.Data(), bytesRead)
			};
		}
		else switch (auto error = API::GetError())
		{
		default:
			Core::Logging::Error("Unhandled error code when calling recvfrom in UDPSocket::Receive. Code: {}.", error);
			return ErrorUnknown{};
		}
	}


	Core::Result<void, Error> UDPSocket::Send(const Endpoint& endpoint, const Core::IO::DynamicByteBuffer& bytes) const
	{
		addrinfo  hints{.ai_flags = AI_ADDRCONFIG, .ai_socktype = SOCK_DGRAM, .ai_protocol = IPPROTO_UDP};
		addrinfo* peer	 = nullptr;
		auto	  result = getaddrinfo(endpoint.GetAddress()->AsString().c_str(), std::to_string(endpoint.GetPort()).c_str(), &hints, &peer);
		Core::Assert(result == 0);


		while (true)
		{
			auto sendResult = sendto(mSocket, reinterpret_cast<const char*>(bytes.Data()), bytes.Size(), 0, peer->ai_addr, peer->ai_addrlen);
			if (sendResult <= 0)
			{
				switch (auto error = API::GetError())
				{
				default:
					Core::Logging::Error("Unhandled error code when calling sendto in UDPSocket::Send. Code: {}.", error);
					return ErrorUnknown{};
				}
			}


			Core::AssertEQ(sendResult, bytes.Size());
			break;
		}

		freeaddrinfo(peer);
		return Core::Success;
	}
} // namespace Strawberry::Net::Socket
