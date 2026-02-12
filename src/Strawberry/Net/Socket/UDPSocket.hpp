#pragma once


//======================================================================================================================
//	Includes
//----------------------------------------------------------------------------------------------------------------------
// Strawberry Net
#include "Strawberry/Net/Endpoint.hpp"
// Core
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include "Strawberry/Core/IO/Error.hpp"
#include "Strawberry/Core/Types/Result.hpp"


#if STRAWBERRY_TARGET_WINDOWS
#include <winsock2.h>
#endif


namespace Strawberry::Net::Socket
{
	struct UDPPacket
	{
		Core::Optional<Endpoint>	endpoint;
		Core::IO::DynamicByteBuffer contents;
	};


	class UDPSocket
	{
	private:
#if STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
		using SocketHandle = int;
#elif STRAWBERRY_TARGET_WINDOWS
		using SocketHandle = SOCKET;
#endif

	public:
		static Core::Result<UDPSocket, Error> Create();
		static Core::Result<UDPSocket, Error> CreateIPv4();
		static Core::Result<UDPSocket, Error> CreateIPv6();

	public:
		UDPSocket(const UDPSocket& other) = delete;
		UDPSocket(UDPSocket&& other) noexcept;
		UDPSocket& operator=(const UDPSocket& other) = delete;
		UDPSocket& operator=(UDPSocket&& other) noexcept;
		~UDPSocket();


		Core::Result<void, Error> Bind(uint16_t portNumber) noexcept;


		[[nodiscard]] bool								  Poll() const;
		[[nodiscard]] Core::Result<UDPPacket, Error>	  Receive();
		[[nodiscard]] Core::Result<void, Error> Send(const Endpoint& endpoint, const Core::IO::DynamicByteBuffer& bytes) const;

private:
		UDPSocket();

		
		SocketHandle             mSocket;
		Core::Optional<uint16_t> mPort;
		bool                     mIPv6;

		static constexpr size_t           BUFFER_SIZE = 25536;
		Core::IO::ByteBuffer<BUFFER_SIZE> mBuffer;
	};
} // namespace Strawberry::Net::Socket
