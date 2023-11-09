#include "Strawberry/Net/Socket/UDPClient.hpp"


#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/Markers.hpp"
#include "Strawberry/Core/Util/Stubs.hpp"


#if _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#elif __APPLE__ || __linux__


#include <fcntl.h>
#include <netdb.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>


#endif // _WIN32


namespace Strawberry::Core::Net::Socket
{
	Result<UDPClient, Error> UDPClient::Create()
	{
		auto handle = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
		if (handle == -1) { return Error::SocketCreation; }

		int  ipv6Only     = 0;
		auto optSetResult = setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&ipv6Only), sizeof(ipv6Only));
		Assert(optSetResult == 0);

		UDPClient client;
		client.mSocket = handle;
		return client;
	}


	Result<UDPClient, Error> UDPClient::CreateIPv4()
	{
		auto handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
		if (handle == -1) { return Error::SocketCreation; }

		UDPClient client;
		client.mSocket = handle;
		return client;
	}


	Result<UDPClient, Error> UDPClient::CreateIPv6()
	{
		auto handle = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
		if (handle == -1) { return Error::SocketCreation; }

		UDPClient client;
		client.mSocket = handle;
		return client;
	}


	UDPClient::UDPClient()
		: mSocket(-1)
	{}


	UDPClient::UDPClient(UDPClient&& other) noexcept
		: mSocket(std::exchange(other.mSocket, -1))
	{}


	UDPClient& UDPClient::operator=(UDPClient&& other) noexcept
	{
		if (this != &other)
		{
			std::destroy_at(this);
			mSocket = std::exchange(other.mSocket, -1);
		}

		return *this;
	}


	UDPClient::~UDPClient()
	{
		if (mSocket != -1)
		{
#if defined(__APPLE__) || defined(__linux__)
			close(mSocket);
#elif defined(_WIN32)
			closesocket(mSocket);
#else
			Unreachable();
#endif
		}
	}


	bool UDPClient::Poll() const
	{
#if defined(__APPLE__) || defined(__linux__)
		pollfd fds[] = {
			{mSocket, POLLIN, 0}
        };

		int pollResult = poll(fds, 1, 0);
		Assert(pollResult >= 0);
		return static_cast<bool>(fds[0].revents & POLLIN);
#elif defined(__WIN32)
		WSAPOLLFD fds[] =
				{
						{mSocket, POLLIN, 0}
				};
		int pollResult = WSAPoll(fds, 1, 0);
		Assert(pollResult >= 0);
		return static_cast<bool>(fds[0].revents & POLLIN);
#else
		Unreachable();
#endif
	}

	Result<std::tuple<Optional<Endpoint>, IO::DynamicByteBuffer>, IO::Error> UDPClient::Read()
	{
		sockaddr_storage peer{};
		socklen_t        peerLen = 0;
		auto bytesRead           = recvfrom(mSocket, reinterpret_cast<char*>(mBuffer.Data()), mBuffer.Size(), 0, reinterpret_cast<sockaddr*>(&peer), &peerLen);

		if (bytesRead >= 0)
		{
			Optional<Endpoint> endpoint;
			if (peer.ss_family == AF_INET)
			{
				auto* sockaddr = reinterpret_cast<sockaddr_in*>(&peer);
				endpoint.Emplace(IPv4Address(IO::ByteBuffer<4>(sockaddr->sin_addr)), sockaddr->sin_port);
			}
			else if (peer.ss_family == AF_INET6)
			{
				auto* sockaddr = reinterpret_cast<sockaddr_in6*>(&peer);
				endpoint.Emplace(IPv6Address(IO::ByteBuffer<16>(sockaddr->sin6_addr)), sockaddr->sin6_port);
			}

			return std::make_tuple(endpoint, IO::DynamicByteBuffer(mBuffer.Data(), bytesRead));
		}
		else { Unreachable(); }
	}


	Result<size_t, IO::Error> UDPClient::Write(const Endpoint& endpoint, const IO::DynamicByteBuffer& bytes) const
	{
		addrinfo  hints{.ai_flags = AI_ADDRCONFIG, .ai_socktype = SOCK_DGRAM, .ai_protocol = IPPROTO_UDP};
		addrinfo* peer   = nullptr;
		auto      result = getaddrinfo(endpoint.GetAddress()->AsString().c_str(), std::to_string(endpoint.GetPort()).c_str(), &hints, &peer);
		Core::Assert(result == 0);


		auto bytesSent = sendto(mSocket, reinterpret_cast<const char*>(bytes.Data()), bytes.Size(), 0, peer->ai_addr, peer->ai_addrlen);
		freeaddrinfo(peer);
		if (bytesSent >= 0) { return bytesSent; }
		else { Unreachable(); }
	}
} // namespace Strawberry::Core::Net::Socket
