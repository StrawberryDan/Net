// Strawberry dnet
#include "Strawberry/Net/Address.hpp"
// Strawberry Core
#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/Markers.hpp"
// OS-Level Networkng Headers
#if STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
#include <arpa/inet.h>
#include <netdb.h>
#elif STRAWBERRY_TARGET_WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#endif


namespace Strawberry::Net
{
	Core::Optional<IPv4Address> IPv4Address::Parse(const std::string& data)
	{
		uint8_t buffer[sizeof(in_addr)] = {0};
		auto	result					= inet_pton(AF_INET, data.data(), buffer);
		if (result == 1)
		{
			Core::IO::ByteBuffer<4> bytes(buffer, 4);
			return IPv4Address(bytes);
		}
		else
		{
			return {};
		}
	}


	IPv4Address IPv4Address::LocalHost() noexcept
	{
		return IPv4Address(127, 0, 0, 1);
	}


	Core::IO::ByteBuffer<4> IPv4Address::AsBytes() const
	{
		return mData;
	}


	std::string IPv4Address::AsString() const
	{
		char buffer[INET_ADDRSTRLEN] = {0};
		auto result					 = inet_ntop(AF_INET, mData.Data(), buffer, INET_ADDRSTRLEN);
		Core::Assert(result != nullptr);
		return {buffer};
	}


	Core::Optional<IPv6Address> IPv6Address::Parse(const std::string& string)
	{
		uint8_t buffer[sizeof(in6_addr)] = {0};
		auto	result					 = inet_pton(AF_INET6, string.data(), buffer);
		if (result == 1)
		{
			Core::IO::ByteBuffer<16> bytes(buffer, 16);
			return IPv6Address(bytes);
		}
		else
		{
			return {};
		}
	}


	const Core::IO::ByteBuffer<16>& IPv6Address::AsBytes() const
	{
		return mData;
	}


	std::string IPv6Address::AsString() const
	{
		char buffer[INET6_ADDRSTRLEN] = {0};
		auto result					  = inet_ntop(AF_INET6, mData.Data(), buffer, INET6_ADDRSTRLEN);
		Core::Assert(result != nullptr);
		return {buffer};
	}


	IPAddress::IPAddress(IPv4Address address)
		: mPayload(address) {}


	IPAddress::IPAddress(IPv6Address address)
		: mPayload(address) {}


	Core::Optional<IPv4Address> IPAddress::AsIPv4() const
	{
		if (IsIPv4()) return mPayload.Ref<IPv4Address>();

		return {};
	}


	Core::Optional<IPv6Address> IPAddress::AsIPv6() const
	{
		if (IsIPv6()) return mPayload.Ref<IPv6Address>();

		return {};
	}


	Core::IO::DynamicByteBuffer IPAddress::AsBytes() const
	{
		if (auto addr = AsIPv4())
		{
			return Core::IO::DynamicByteBuffer(addr->AsBytes());
		}
		else if (auto addr = AsIPv6())
		{
			return Core::IO::DynamicByteBuffer(addr->AsBytes());
		}
		else
		{
			Core::Unreachable();
		}
	}


	std::string IPAddress::AsString() const
	{
		if (auto addr = AsIPv4())
		{
			return addr->AsString();
		}
		else if (auto addr = AsIPv6())
		{
			return addr->AsString();
		}
		else
		{
			Core::Unreachable();
		}
	}
} // namespace Strawberry::Net
