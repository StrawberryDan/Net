#include "Strawberry/Net/Endpoint.hpp"


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
        addrinfo* peer      = nullptr;
        auto      dnsResult = getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &peer);
        if (dnsResult != 0)
        {
            return ErrorDNSResolution {};
        }

        Core::Optional<Endpoint> result;
        addrinfo*                cursor = peer;
        while (cursor != nullptr)
        {
            if (cursor->ai_family == AF_INET)
            {
                auto        ipData = reinterpret_cast<sockaddr_in*>(cursor->ai_addr);
                IPv4Address addr(Core::IO::ByteBuffer<4>(ipData->sin_addr.s_addr));
                result = Endpoint(addr, port);
                break;
            }
            if (cursor->ai_family == AF_INET6)
            {
                auto        ipData = reinterpret_cast<sockaddr_in6*>(cursor->ai_addr);
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
        std::string portstr  = endpoint.substr(colonPos + 1, endpoint.size());

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


    Core::Result<Endpoint, Error> Endpoint::Parse(const std::string& endpoint)
    {
        auto colonPos = endpoint.find(':');
        if (colonPos == std::string::npos) return ErrorParsingEndpoint {};

        std::string hostname = endpoint.substr(0, colonPos);
        std::string portstr  = endpoint.substr(colonPos + 1, endpoint.size());

        uint16_t port;
        try
        {
            port = std::stoi(portstr);
        }
        catch (const std::exception& e)
        {
            return ErrorParsingEndpoint {};
        }

        return Endpoint(hostname, port);
    }


    Endpoint::Endpoint()
        : mHostName()
        , mAddress()
        , mPort(-1) {}


    Endpoint::Endpoint(IPAddress address, uint16_t port)
        : mAddress(address)
        , mPort(port) {}


    Endpoint::Endpoint(const std::string& hostname, uint16_t port)
        : mHostName(hostname)
        , mAddress(Resolve(hostname).IntoOptional().AndThen([](auto x)
        {
            return x.GetAddress();
        }))
        , mPort(port) {}
} // namespace Strawberry::Net
