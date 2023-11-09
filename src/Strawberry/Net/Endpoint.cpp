#include "Strawberry/Net/Endpoint.hpp"


#if defined(__APPLE__) || defined(__linux__)


#include <netdb.h>


#elif defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


namespace Strawberry::Core::Net
{
	Result<Endpoint, Error> Endpoint::Resolve(const std::string& hostname, uint16_t port)
	{
		addrinfo  hints{.ai_flags = AI_ALL | AI_ADDRCONFIG};
		addrinfo* peer      = nullptr;
		auto      dnsResult = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &peer);
		if (dnsResult != 0) { return Error::DNSResolution; }

		Optional<Endpoint> result;
		addrinfo*        cursor = peer;
		while (cursor != nullptr)
		{
			if (cursor->ai_family == AF_INET)
			{
				auto        ipData = reinterpret_cast<sockaddr_in*>(cursor->ai_addr);
				IPv4Address addr(IO::ByteBuffer<4>(ipData->sin_addr.s_addr));
				result = Endpoint(addr, port);
			}
			else if (cursor->ai_family == AF_INET6)
			{
				auto        ipData = reinterpret_cast<sockaddr_in6*>(cursor->ai_addr);
				IPv6Address addr(IO::ByteBuffer<16>(&ipData->sin6_addr));
				result = Endpoint(addr, port);
			}

			cursor = cursor->ai_next;
		}

		if (result)
		{
			result->mHostName = hostname;
			return *result;
		}
		else { return Error::DNSResolution; }
	}


	Result<Endpoint, Error> Endpoint::Resolve(const std::string& endpoint)
	{
		auto colonPos = endpoint.find(':');
		if (colonPos == std::string::npos) return Error::ParsingEndpoint;

		std::string hostname = endpoint.substr(0, colonPos);
		std::string portstr  = endpoint.substr(colonPos + 1, endpoint.size());

		uint16_t port;
		try
		{
			port = std::stoi(portstr);
		} catch (const std::exception& e)
		{
			return Error::ParsingEndpoint;
		}

		return Resolve(hostname, port);
	}


	Result<Endpoint, Error> Endpoint::Parse(const std::string& endpoint)
	{
		auto colonPos = endpoint.find(':');
		if (colonPos == std::string::npos) return Error::ParsingEndpoint;

		std::string hostname = endpoint.substr(0, colonPos);
		std::string portstr  = endpoint.substr(colonPos + 1, endpoint.size());

		uint16_t port;
		try
		{
			port = std::stoi(portstr);
		} catch (const std::exception& e)
		{
			return Error::ParsingEndpoint;
		}

		return Endpoint(hostname, port);
	}


	Endpoint::Endpoint(IPAddress address, uint16_t port)
		: mAddress(address)
		, mPort(port)
	{}


	Endpoint::Endpoint(const std::string& hostname, uint16_t port)
		: mHostName(hostname)
		  , mAddress(Resolve(hostname).IntoOptional().AndThen([](auto x) { return x.GetAddress(); }))
		, mPort(port)
	{}
} // namespace Strawberry::Core::Net
