#include "Strawberry/Net/Address.hpp"


#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/Markers.hpp"
#include <numeric>


#if defined(__APPLE__) || defined(__linux__)


#include <arpa/inet.h>
#include <netdb.h>


#elif defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


namespace Strawberry::Core::Net
{
	Optional<IPv4Address> IPv4Address::Parse(const std::string& data)
	{
		uint8_t buffer[sizeof(in_addr)] = {0};
		auto    result                  = inet_pton(AF_INET, data.data(), buffer);
		if (result == 1)
		{
			IO::ByteBuffer<4> bytes(buffer, 4);
			return IPv4Address(bytes);
		}
		else { return {}; }
	}


	IO::ByteBuffer<4> IPv4Address::AsBytes() const
	{
		return mData;
	}


	std::string IPv4Address::AsString() const
	{
		char buffer[INET_ADDRSTRLEN] = {0};
		auto result                  = inet_ntop(AF_INET, mData.Data(), buffer, INET_ADDRSTRLEN);
		Assert(result != nullptr);
		return {buffer};
	}

	Optional<IPv6Address> IPv6Address::Parse(const std::string& string)
	{
		uint8_t buffer[sizeof(in6_addr)] = {0};
		auto    result                   = inet_pton(AF_INET6, string.data(), buffer);
		if (result == 1)
		{
			IO::ByteBuffer<16> bytes(buffer, 16);
			return IPv6Address(bytes);
		}
		else { return {}; }
	}


	const IO::ByteBuffer<16>& IPv6Address::AsBytes() const
	{
		return mData;
	}


	std::string IPv6Address::AsString() const
	{
		char buffer[INET6_ADDRSTRLEN] = {0};
		auto result                   = inet_ntop(AF_INET6, mData.Data(), buffer, INET6_ADDRSTRLEN);
		Assert(result != nullptr);
		return {buffer};
	}


	IPAddress::IPAddress(IPv4Address address)
		: mPayload(address)
	{}


	IPAddress::IPAddress(IPv6Address address)
		: mPayload(address)
	{}

	Optional<IPv4Address> IPAddress::AsIPv4() const
	{
		if (IsIPv4()) return std::get<IPv4Address>(mPayload);
		else
			return {};
	}

	Optional<IPv6Address> IPAddress::AsIPv6() const
	{
		if (IsIPv6()) return std::get<IPv6Address>(mPayload);
		else
			return {};
	}


	IO::DynamicByteBuffer IPAddress::AsBytes() const
	{
		if (auto addr = AsIPv4()) { return IO::DynamicByteBuffer(addr->AsBytes()); }
		else if (auto addr = AsIPv6()) { return IO::DynamicByteBuffer(addr->AsBytes()); }
		else { Unreachable(); }
	}


	std::string IPAddress::AsString() const
	{
		if (auto addr = AsIPv4()) { return addr->AsString(); }
		else if (auto addr = AsIPv6()) { return addr->AsString(); }
		else { Unreachable(); }
	}
} // namespace Strawberry::Core::Net
