//======================================================================================================================
//	Includes
//----------------------------------------------------------------------------------------------------------------------
// Strawberry Net
#include "TCPListener.hpp"
// Strawberry Core

#include "API.hpp"
#include "Strawberry/Core/IO/Logging.hpp"
#include "Strawberry/Core/Markers.hpp"


#ifdef STRAWBERRY_TARGET_WINDOWS
#include <ws2tcpip.h>
#elifdef STRAWBERRY_TARGET_MAC
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
		if (endpoint.GetAddress()->IsIPv4())
		{
			return AF_INET;
		}
		if (endpoint.GetAddress()->IsIPv6())
		{
			return AF_INET6;
		}
		Core::Unreachable();
	}


	Core::Result<TCPListener, Error> TCPListener::Bind(const Endpoint& endpoint)
	{
		Core::Logging::Info("Opening TCP Listener at {}", endpoint.ToString());
		TCPListener listener;
		listener.mEndpoint = endpoint;

		int addressFamily = endpoint.GetAddress()->IsIPv4() ? AF_INET
		                  : endpoint.GetAddress()->IsIPv6() ? AF_INET6
		                  : Core::Unreachable<int>();

		// Create Socket.
		listener.mSocket = socket(addressFamily, SOCK_STREAM, IPPROTO_TCP);
		if (listener.mSocket == -1)
		{
			Core::Logging::Error("Failed to create socket! Error code: {}", API::GetError());
			return ErrorSocketCreation {};
		}


		// Get address info
		addrinfo  hints
			{
				.ai_flags = AI_ADDRCONFIG,
				.ai_family = addressFamily,
				.ai_socktype = SOCK_STREAM,
				.ai_protocol = IPPROTO_TCP
			};
		addrinfo* peerAddress = nullptr;
		auto	  addrResult  = getaddrinfo(
			endpoint.GetAddress()->AsString().c_str(),
			std::to_string(endpoint.GetPort()).c_str(),
			&hints, &peerAddress);
		if (addrResult != 0)
		{
			Core::Logging::Error("Failed to resolve the address: {}", endpoint.GetAddress()->AsString());
			freeaddrinfo(peerAddress);
			return ErrorAddressResolution {};
		}
		Core::AssertEQ(peerAddress->ai_family, addressFamily);


		// Bind Socket.
		int bindResult = bind(listener.mSocket, peerAddress->ai_addr, peerAddress->ai_addrlen);
		freeaddrinfo(peerAddress);
		if (bindResult == SOCKET_ERROR_CODE)
		{
			auto error = API::GetError();
			Core::Logging::Error("Failed to bind socket! Error code returned: {}", error);
			switch (error)
			{
			default: Core::Unreachable();
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


	TCPListener::TCPListener()
		: mSocket(-1) {}


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
		TCPSocket socket;

		sockaddr_storage peer{};
		socklen_t		 peerLen = sizeof(peer);

		socket.mSocket = accept(mSocket, reinterpret_cast<sockaddr*>(&peer), &peerLen);
#ifdef STRAWBERRY_TARGET_WINDOWS
		if (socket.mSocket == SOCKET_ERROR)
#elifdef STRAWBERRY_TARGET_MAC
			if (socket.mSocket == -1)
#endif
			{
				auto error = API::GetError();
				switch (error)
				{
				default: Core::Unreachable();
				}
			}


		if (peer.ss_family == AF_INET)
		{
			auto* sockaddr = reinterpret_cast<sockaddr_in*>(&peer);
			std::construct_at(&socket.mEndpoint, IPv4Address(Core::IO::ByteBuffer<4>(sockaddr->sin_addr)), sockaddr->sin_port);
		}
		else if (peer.ss_family == AF_INET6)
		{
			auto* sockaddr = reinterpret_cast<sockaddr_in6*>(&peer);
			std::construct_at(&socket.mEndpoint, IPv6Address(Core::IO::ByteBuffer<16>(sockaddr->sin6_addr)), sockaddr->sin6_port);
		}
		else
		{
			Core::Unreachable();
		}

#if STRAWBERRY_TARGET_WINDOWS
		DWORD keepAlive = 1;
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
		int keepAlive = 1;
#endif
		Core::AssertEQ(
			setsockopt(socket.mSocket, SOL_SOCKET, SO_KEEPALIVE, reinterpret_cast<const char*>(&keepAlive), sizeof(keepAlive)),
			0);

		return socket;
	}
}
