// StrawberryNet
#include "Strawberry/Net/Endpoint.hpp"
#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include "Strawberry/Core/Markers.hpp"
#include "Strawberry/Net/Address.hpp"
#include <cstdint>
#include <sys/socket.h>
// OS-Level Networking Headers
#if STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
#include <netdb.h>
#elif STRAWBERRY_TARGET_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


namespace Strawberry::Net
{
	Core::Result<Endpoint, Error> Endpoint::Resolve(const std::string& hostname, uint16_t port)
	{
		addrinfo  hints{.ai_flags = AI_ALL | AI_ADDRCONFIG};
		addrinfo* peer		= nullptr;
		auto	  dnsResult = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &peer);
		if (dnsResult != 0)
		{
			return ErrorDNSResolution {};
		}

		Core::Optional<Endpoint> result;
		addrinfo*				 cursor = peer;
		while (cursor != nullptr)
		{
			if (cursor->ai_family == AF_INET)
			{
				auto		ipData = reinterpret_cast<sockaddr_in*>(cursor->ai_addr);
				IPv4Address addr(Core::IO::ByteBuffer<4>(ipData->sin_addr.s_addr));
				result = Endpoint(addr, port);
				break;
			}
			if (cursor->ai_family == AF_INET6)
			{
				auto		ipData = reinterpret_cast<sockaddr_in6*>(cursor->ai_addr);
				IPv6Address addr(Core::IO::ByteBuffer<16>(&ipData->sin6_addr));
				result = Endpoint(addr, port);
				break;
			}

			cursor = cursor->ai_next;
		}

		if (result)
		{
			result->mHostName = hostname;
			return result.Unwrap();
		}
		else
		{
			return ErrorDNSResolution {};
		}
	}


	Core::Result<Endpoint, Error> Endpoint::Resolve(const std::string& endpoint)
	{
		auto colonPos = endpoint.find(':');
		if (colonPos == std::string::npos) return ErrorParsingEndpoint {};

		std::string hostname = endpoint.substr(0, colonPos);
		std::string portstr	 = endpoint.substr(colonPos + 1, endpoint.size());

		uint16_t port;
		try
		{
			port = std::stoi(portstr);
		}
		catch (const std::exception& e)
		{
			return ErrorParsingEndpoint {};
		}

		return Resolve(hostname, port);
	}


	Endpoint Endpoint::LocalHostIPv4(uint16_t portNumber) noexcept
	{
		return Endpoint(IPv4Address::LocalHost(), portNumber);
	}


	Endpoint Endpoint::LocalHostIPv6(uint16_t portNumber) noexcept
	{
		return Endpoint(IPv6Address::LocalHost(), portNumber);
	}


	Endpoint::Endpoint(IPAddress address, uint16_t port)
		: mAddress(address)
		, mPort(port) {}


	std::string Endpoint::ToString() const noexcept
	{
		if (mHostName)
		{
			return fmt::format("{}:{}", *mHostName, mPort);
		}
		else
		{
			return fmt::format("{}:{}", mAddress.AsString(), mPort);
		}
	}


	sockaddr_storage Endpoint::GetPlatformRepresentation(bool mapIPv6) const noexcept
	{
		Core::IO::DynamicByteBuffer addressBytes =
			(mapIPv6 && GetAddress().IsIPv4())
				? IPv6Address::FromIPv4(GetAddress().AsIPv4().Unwrap()).AsBytes().ToDynamic()
				: GetAddress().AsBytes();

		sockaddr_storage result;
		memset(&result, 0, sizeof(result));

		if (GetAddress().IsIPv4() && !mapIPv6)
		{
			sockaddr_in* asIPv4 = reinterpret_cast<sockaddr_in*>(&result);
			asIPv4->sin_family = AF_INET;
			asIPv4->sin_port = htons(mPort);

			memcpy(&asIPv4->sin_addr.s_addr, addressBytes.Data(), 4);
		}
		else if (GetAddress().IsIPv6() || mapIPv6)
		{
			sockaddr_in6* asIPv6 = reinterpret_cast<sockaddr_in6*>(&result);
			asIPv6->sin6_family = AF_INET6;
			asIPv6->sin6_port = htons(mPort);

			memcpy(&asIPv6->sin6_addr.s6_addr, addressBytes.Data(), 16);
		}
		else Core::Unreachable();


		// Double check that the platform representation is correct using OS level functions
#if STRAWBERRY_DEBUG
		addrinfo hints
		{
			.ai_flags = AI_ALL | AI_ADDRCONFIG | (mapIPv6 ? AI_V4MAPPED : 0),
			.ai_family = (mapIPv6 || GetAddress().IsIPv6()) ? AF_INET6 : AF_INET,
		};
		addrinfo* addrinfo = nullptr;
		int getaddrinfoResult = getaddrinfo(GetAddress().AsString().c_str(),
											std::to_string(mPort).c_str(),
											&hints, &addrinfo);
		Core::Logging::Info("{}", addrinfo->ai_family);
		Core::AssertEQ(getaddrinfoResult, 0);
		Core::AssertImplication(GetAddress().IsIPv4() && !mapIPv6, addrinfo->ai_family == AF_INET,
								"Double check of IPv4 Platform Representation didn't match IP Family");
		Core::AssertImplication(GetAddress().IsIPv6() || mapIPv6, addrinfo->ai_family == AF_INET6,
								"Double check of IPv6 Platform Representation didn't match IP Family");

		Core::IO::DynamicByteBuffer addrinfoBytes;
		if (addrinfo->ai_family == AF_INET)
		{
			addrinfoBytes =
				Core::IO::DynamicByteBuffer(&reinterpret_cast<sockaddr_in*>(addrinfo->ai_addr)->sin_addr, 4);
		}
		else if (addrinfo->ai_family == AF_INET6)
		{
			addrinfoBytes =
				Core::IO::DynamicByteBuffer(&reinterpret_cast<sockaddr_in6*>(addrinfo->ai_addr)->sin6_addr, 16);
		}
		Core::AssertEQ(addressBytes, addrinfoBytes,
					   fmt::format(
								   "Double checking IP platform representation: address bytes did not match! (Real) {} != (Check) {}",
								   addressBytes.AsHexString(), addrinfoBytes.AsHexString()));
		freeaddrinfo(addrinfo);
#endif

		return result;
	}
} // namespace Strawberry::Net
