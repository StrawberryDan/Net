#pragma once


#include <cstdint>
#include <string>
#include <vector>


#include "Strawberry/Core/IO/ByteBuffer.hpp"
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include "Strawberry/Core/Types/Optional.hpp"

namespace Strawberry::Net
{
	class IPv4Address
	{
	public:
		// Static constructors
		static Core::Optional<IPv4Address> Parse(const std::string& data);


	public:
		// Constructors
		IPv4Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
		{
			mData[0] = a;
			mData[1] = b;
			mData[2] = c;
			mData[3] = d;
		}


		IPv4Address(const Core::IO::ByteBuffer<4> bytes)
			: mData(bytes)
		{}


		IPv4Address(const Core::IO::DynamicByteBuffer& bytes)
			: mData(bytes.AsStatic<4>())
		{}


		// Casting
		[[nodiscard]] Core::IO::ByteBuffer<4> AsBytes() const;
		[[nodiscard]] std::string       AsString() const;

	private:
		Core::IO::ByteBuffer<4> mData;
	};


	class IPv6Address
	{
	public:
		static Core::Optional<IPv6Address> Parse(const std::string& string);


	public:
		IPv6Address(const Core::IO::ByteBuffer<16>& bytes)
			: mData(bytes)
		{}


		IPv6Address(const Core::IO::DynamicByteBuffer& bytes)
			: mData(bytes.AsStatic<16>())
		{}


		// Casting
		[[nodiscard]] const Core::IO::ByteBuffer<16>& AsBytes() const;
		[[nodiscard]] std::string               AsString() const;

	private:
		Core::IO::ByteBuffer<16> mData;
	};


	class IPAddress
	{
	public:
		IPAddress(IPv4Address address);
		IPAddress(IPv6Address address);


		// Checking
		[[nodiscard]] inline bool IsIPv4() const { return std::holds_alternative<IPv4Address>(mPayload); }


		[[nodiscard]] inline bool IsIPv6() const { return std::holds_alternative<IPv6Address>(mPayload); }


		// Casting
		[[nodiscard]] Core::Optional<IPv4Address> AsIPv4() const;
		[[nodiscard]] Core::Optional<IPv6Address> AsIPv6() const;
		[[nodiscard]] Core::IO::DynamicByteBuffer AsBytes() const;
		[[nodiscard]] std::string           AsString() const;


	private:
		std::variant<IPv4Address, IPv6Address> mPayload;
	};
} // namespace Strawberry::Net
