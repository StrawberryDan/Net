#pragma once


#include <string>


#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include "Strawberry/Core/IO/Error.hpp"
#include "Strawberry/Net/Endpoint.hpp"
#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Core/Types/Result.hpp"


#if STRAWBERRY_TARGET_WINDOWS
#include <winsock2.h>
#endif


namespace Strawberry::Net::Socket
{
	class TCPSocket
	{
		friend class TLSSocket;


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


		const Endpoint&                                            GetEndpoint() const noexcept;
		[[nodiscard]] bool                                         Poll() const;
		Core::Result<Core::IO::DynamicByteBuffer, Core::IO::Error> Read(size_t length);
		Core::Result<size_t, Core::IO::Error>                      Write(const Core::IO::DynamicByteBuffer& bytes);


	private:
		SocketHandle mSocket;
		Endpoint     mEndpoint;
	};
} // namespace Strawberry::Net::Socket
