#pragma once
//======================================================================================================================
//	Includes
//----------------------------------------------------------------------------------------------------------------------
// Strawberry Net
#include "Strawberry/Net/Endpoint.hpp"
#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Net/Socket/TCPSocket.hpp"
// Strawberry Core
#include "Strawberry/Core/Types/Result.hpp"

#if STRAWBERRY_TARGET_WINDOWS
#include <winsock2.h>
#endif

//======================================================================================================================
//	Class Declaration
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Net::Socket
{
	class TCPListener
	{
	private:
#if STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
		using SocketHandle = int;
#elif STRAWBERRY_TARGET_WINDOWS
		using SocketHandle = SOCKET;
#endif

	public:
		static Core::Result<TCPListener, Error> Bind(const Endpoint& endpoint);


		TCPListener(const TCPListener&) = delete;
		TCPListener& operator= (const TCPListener&) = delete;
		TCPListener(TCPListener&& other);
		TCPListener& operator = (TCPListener&& other) noexcept;
		~TCPListener();


		Core::Optional<TCPSocket> Accept() const	noexcept;

	private:
		TCPListener();

	private:
		SocketHandle	mSocket;
		Endpoint		mEndpoint;
	};
}
