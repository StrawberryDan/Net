#pragma once


//======================================================================================================================
//	Includes
//----------------------------------------------------------------------------------------------------------------------
// Strawberry Net
#include "Strawberry/Net/Endpoint.hpp"
// Core
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
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
		/// Create a dual-banded UDP Socket.
		static Core::Result<UDPSocket, Error> Create();
		/// Create an IPv6 UDP Socket.
		static Core::Result<UDPSocket, Error> CreateIPv4();
		/// Create an IPv4 UDP Socket.
		static Core::Result<UDPSocket, Error> CreateIPv6();

	public:
		// Prohibit copying, allow moving, close on exit.
		UDPSocket(const UDPSocket& other) = delete;
		UDPSocket(UDPSocket&& other) noexcept;
		UDPSocket& operator=(const UDPSocket& other) = delete;
		UDPSocket& operator=(UDPSocket&& other) noexcept;
		~UDPSocket();


		/// Bind this socket to a specific port.
		///
		/// Without this call, this socket will not be able to receive
		/// packets. Furthermore, without calling  Bind(), this socket will be
		/// assigned a random port when sending packets for the first time.
		Core::Result<void, Error> Bind(const Endpoint& endpoint) noexcept;


		/// Returns whether there is data waiting to be received on this socket.
		[[nodiscard]] bool                           Poll() const;
		/// Reads a packet of data from this socket.
		[[nodiscard]] Core::Result<UDPPacket, Error> Receive();
		/// Sends a message over this socket to the given endpoint.
		[[nodiscard]] Core::Result<void, Error>      Send(const Endpoint& endpoint, const Core::IO::DynamicByteBuffer& bytes) const;


private:
		/// Private default constructor for use in static methods of this class.
		UDPSocket();


		/// The handle to this socket.
		SocketHandle             mSocket;
		/// The endoint to which this socket is bound.
		/// Doesn't necessarily have a value until the first packet is sent.
		Core::Optional<Endpoint> mEndpoint;
		/// Boolean of whether this socket was created with IPv6 capacilities.
		/// True for both pure V6 and Dualband sockets.
		bool                     mIPv6;


		/// Size of the buffer in which to read packets to.
		/// Set to close to the maximum theoritical UDP packet size.
		static constexpr size_t           BUFFER_SIZE = 64 * 1024;
		/// Byte buffer for reading UDP packets into.
		Core::IO::DynamicByteBuffer mBuffer = Core::IO::DynamicByteBuffer::Zeroes(BUFFER_SIZE);
	};
} // namespace Strawberry::Net::Socket
