#include "Strawberry/Net/Socket/TCPSocket.hpp"
#include <Strawberry/Core/IO/Logging.hpp>

#include "API.hpp"
#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/Markers.hpp"
#include "Strawberry/Core/Util/Strings.hpp"


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
        TCPSocket tcpSocket;

        addrinfo hints{.ai_flags = AI_ADDRCONFIG, .ai_socktype = SOCK_STREAM, .ai_protocol = IPPROTO_TCP};
        hints.ai_family = endpoint.GetAddress()->IsIPv4() ? AF_INET : endpoint.GetAddress()->IsIPv6() ? AF_INET6 : Core::Unreachable<int>();

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
        tcpSocket.mSocket   = handle;
        tcpSocket.mEndpoint = endpoint;
        return tcpSocket;
    }


    TCPSocket::TCPSocket()
        : mSocket(-1) {}


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
#if STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
			close(mSocket);
#elif STRAWBERRY_TARGET_WINDOWS
            closesocket(mSocket);
#else
			Core::Unreachable();
#endif
        }
    }


    const Endpoint& TCPSocket::GetEndpoint() const noexcept
    {
        return mEndpoint;
    }


    bool TCPSocket::Poll() const
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


    StreamReadResult TCPSocket::Read(size_t length)
    {
        auto buffer = Core::IO::DynamicByteBuffer::Zeroes(length);

        auto recvResult = recv(mSocket, reinterpret_cast<char*>(buffer.Data()), buffer.Size(), 0);
        if (recvResult == -1)
        {
            auto error = API::GetError();
            switch (error)
            {
                case ErrorCodes::ConnectionReset: return Error::ConnectionReset;
                default: Core::Logging::Error("Unknown error on TCP recv: {}", error);
                    Core::Unreachable();
            }
        }


        if (recvResult == 0) return Error::ConnectionReset;
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
                    case ErrorCodes::ConnectionReset: return Error::ConnectionReset;
                    default: Core::Logging::Error("Unknown error on TCP send: {}", error);
                        Core::Unreachable();
                }
            }
        }

        Core::Assert(bytesSent == bytes.Size());
        return {};
    }
} // namespace Strawberry::Net::Socket
