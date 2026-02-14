//======================================================================================================================
//	Includes
//----------------------------------------------------------------------------------------------------------------------
// Strawberry Net
#include "Strawberry/Net/Socket/TCPListener.hpp"
#include "Strawberry/Core/IO/ByteBuffer.hpp"
#include "Strawberry/Net/Address.hpp"
#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Net/Socket/Platform.hpp"
// Strawberry Core
#include "API.hpp"
#include "Strawberry/Core/IO/Logging.hpp"
#include "Strawberry/Core/Markers.hpp"
// Standard Library
#include <cerrno>
// Platform specific networking headers
#if STRAWBERRY_TARGET_WINDOWS
#include <ws2tcpip.h>
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#endif


//======================================================================================================================
//	Method Definitions
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Net::Socket
{
	static int GetAddressFamily(const Endpoint& endpoint)
	{
		if (endpoint.GetAddress().IsIPv4())
		{
			return AF_INET;
		}
		if (endpoint.GetAddress().IsIPv6())
		{
			return AF_INET6;
		}
		Core::Unreachable();
	}


	Core::Result<TCPListener, Error> TCPListener::Bind(const Endpoint& endpoint)
	{
		Core::Logging::Info("Opening TCP Listener at {}", endpoint.ToString());

		int addressFamily = endpoint.GetAddress().IsIPv4() ? AF_INET
		                  : endpoint.GetAddress().IsIPv6() ? AF_INET6
		                  : Core::Unreachable<int>();

		// Create Socket.
		SocketHandle socketHandle = socket(addressFamily, SOCK_STREAM, IPPROTO_TCP);
		if (socketHandle == -1)
		{
			Core::Logging::Error("Failed to create socket! Error code: {}", API::GetError());
			return ErrorSocketCreation {};
		}

		// Construct listener object.
		TCPListener listener(socketHandle, endpoint);

		sockaddr_storage peer = endpoint.GetPlatformRepresentation();
		socklen_t peerLen = endpoint.GetAddress().IsIPv6() ? sizeof(sockaddr_in6) : sizeof(sockaddr_in);

		// Bind Socket.
		int bindResult = bind(listener.mSocket, (const sockaddr*) &peer, peerLen);
		if (bindResult == SOCKET_ERROR_CODE)
		{
			switch (auto error = API::GetError())
			{
			case SOCKET_ERROR_TYPE_CODE(EAFNOSUPPORT):
				Core::Logging::Error("Failed to bind TCPListener to {} because the address was not supported!", endpoint.ToString());
				return ErrorIPAddressFamily{};
			case SOCKET_ERROR_TYPE_CODE(EADDRINUSE):
				Core::Logging::Error("Failed to bind TCPListener to {} because the address was in use!", endpoint.ToString());
				return ErrorAddressInUse{};
			default:
				Core::Logging::Error("Failed to bind TCPListener to endpoint {}. Error code: {}.", endpoint.ToString(), error);
			}
		}


		// Put socket into listen mode.
		int listenResult = listen(listener.mSocket, SOMAXCONN);
		if (listenResult == SOCKET_ERROR_CODE)
		{
			auto error = API::GetError();
			Core::Logging::Error("Failed to set socket into listen mode! Error code: {}", error);
			switch (error)
			{
			default: Core::Unreachable();
			}
		}
		Core::AssertEQ(listenResult, 0);

		SOCKET_OPTION_TYPE keepAlive = 1;
		setsockopt(listener.mSocket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(keepAlive), sizeof(keepAlive));

		return std::move(listener);
	}


	TCPListener::TCPListener(SocketHandle handle, Endpoint endpoint)
		: mSocket(handle)
		, mEndpoint(std::move(endpoint)) {}


	TCPListener::TCPListener(TCPListener&& other)
		: mSocket(std::exchange(other.mSocket, -1))
		, mEndpoint(std::move(other.mEndpoint)) {}


	TCPListener& TCPListener::operator=(TCPListener&& other) noexcept
	{
		if (this != &other)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(other));
		}

		return *this;
	}


	TCPListener::~TCPListener()
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


	Core::Optional<TCPSocket> TCPListener::Accept() const noexcept
	{
		sockaddr_storage peer{};
		socklen_t		 peerLen = sizeof(peer);

		SocketHandle socketHandle = accept(mSocket, reinterpret_cast<sockaddr*>(&peer), &peerLen);
		if (socketHandle == SOCKET_ERROR_CODE)
		{
			auto error = API::GetError();
			switch (error)
			{
			default: Core::Unreachable();
			}
		}


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
		else Core::Unreachable();

		SOCKET_OPTION_TYPE keepAlive = 1;
		Core::AssertEQ(
			setsockopt(socketHandle, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&keepAlive), sizeof(keepAlive)),
			0);

		return TCPSocket(socketHandle, endpoint.Unwrap());
	}
}
