#pragma once


//======================================================================================================================
//	Includes
//======================================================================================================================
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include "Strawberry/Core/IO/Error.hpp"
#include "Strawberry/Net/Endpoint.hpp"
#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Core/Types/Result.hpp"
#include "Strawberry/Net/Socket/Types.hpp"
// Standard Library
#include <string>
// Platform Specific
#if STRAWBERRY_TARGET_WINDOWS
#include <winsock2.h>
#endif


namespace Strawberry::Net::Socket
{
    class TCPSocket
    {
        friend class TLSSocket;
        friend class TCPListener;

        private:
#if STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
		using SocketHandle = int;
#elif STRAWBERRY_TARGET_WINDOWS
            using SocketHandle = SOCKET;
#endif

        public:
            static Core::Result<TCPSocket, Error> Connect(const Endpoint& endpoint);

        public:
            TCPSocket();
            TCPSocket(const TCPSocket& other) = delete;
            TCPSocket(TCPSocket&& other) noexcept;
            TCPSocket& operator=(const TCPSocket& other) = delete;
            TCPSocket& operator=(TCPSocket&& other) noexcept;
            ~TCPSocket();


            const Endpoint&    GetEndpoint() const noexcept;
            [[nodiscard]] bool Poll() const;
            StreamReadResult   Read(size_t length);
            StreamReadResult   ReadAll(size_t length);
            StreamWriteResult  Write(const Core::IO::DynamicByteBuffer& bytes);

        private:
            SocketHandle mSocket;
            Endpoint     mEndpoint;
    };
} // namespace Strawberry::Net::Socket
