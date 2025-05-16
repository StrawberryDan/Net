#include "Strawberry/Net/Socket/UDPSocket.hpp"

#include <thread>
#include <Strawberry/Core/IO/Logging.hpp>

#include "API.hpp"
#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/Markers.hpp"
#include "Strawberry/Core/Util/Strings.hpp"


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


namespace Strawberry::Net::Socket
{
    Core::Result<UDPSocket, Error> UDPSocket::Create()
    {
        auto handle = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
        if (handle == -1)
        {
            return ErrorSocketCreation {};
        }

        int  ipv6Only     = 0;
        auto optSetResult = setsockopt(handle, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<const char*>(&ipv6Only), sizeof(ipv6Only));
        Core::Assert(optSetResult == 0);

        UDPSocket client;
        client.mSocket = handle;
        return client;
    }


    Core::Result<UDPSocket, Error> UDPSocket::CreateIPv4()
    {
        auto handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (handle == -1)
        {
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
            return ErrorSocketCreation {};
        }

        UDPSocket client;
        client.mSocket = handle;
        return client;
    }


    UDPSocket::UDPSocket()
        : mSocket(-1) {}


    UDPSocket::UDPSocket(UDPSocket&& other) noexcept
        : mSocket(std::exchange(other.mSocket, -1)) {}


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
#if STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
			close(mSocket);
#elif STRAWBERRY_TARGET_WINDOWS
            closesocket(mSocket);
#else
			Core::Unreachable();
#endif
        }
    }


    bool UDPSocket::Poll() const
    {
#if STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
		pollfd fds[] = {
			{mSocket, POLLIN, 0}
        };

		int pollResult = poll(fds, 1, 0);
		Core::Assert(pollResult >= 0);
		return static_cast<bool>(fds[0].revents & POLLIN);
#elif STRAWBERRY_TARGET_WINDOWS
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


    Core::Result<UDPPacket, Error> UDPSocket::Receive()
    {
        sockaddr_storage peer{};
        socklen_t        peerLen   = 0;
        auto             bytesRead = recvfrom(mSocket,
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
        else
        {
            Core::Unreachable();
        }
    }


    Core::Result<void, Core::IO::Error> UDPSocket::Send(const Endpoint& endpoint, const Core::IO::DynamicByteBuffer& bytes) const
    {
        addrinfo  hints{.ai_flags = AI_ADDRCONFIG, .ai_socktype = SOCK_DGRAM, .ai_protocol = IPPROTO_UDP};
        addrinfo* peer   = nullptr;
        auto      result = getaddrinfo(endpoint.GetAddress()->AsString().c_str(), std::to_string(endpoint.GetPort()).c_str(), &hints, &peer);
        Core::Assert(result == 0);


        while (true)
        {
            auto sendResult = sendto(mSocket, reinterpret_cast<const char*>(bytes.Data()), bytes.Size(), 0, peer->ai_addr, peer->ai_addrlen);
            if (sendResult <= 0)
            {
                auto error = API::GetError();
                switch (error)
                {
                    // If socket buffer is full, give it a chance to empty and try again
                    case ErrorCodes::NoBufferSpace:
                        std::this_thread::yield();
                        continue;
                    default:
                        Core::Logging::Error("Unknown error on UDP sendto: {}", error);
                        Core::Unreachable();
                }
            }


            Core::AssertEQ(sendResult, bytes.Size());
            break;
        }

        freeaddrinfo(peer);
        return Core::Success;
    }
} // namespace Strawberry::Net::Socket
