#pragma once


#include <cstdint>
#include <string>
#include <vector>


#include "Strawberry/Core/IO/ByteBuffer.hpp"
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include "Strawberry/Core/Types/Optional.hpp"

namespace Strawberry::Core::Net
{
	class IPv4Address
	{
	public:
		// Static constructors
		static Optional<IPv4Address> Parse(const std::string& data);


	public:
		// Constructors
		IPv4Address(const IO::ByteBuffer<4> bytes)
			: mData(bytes)
		{}


		IPv4Address(const IO::DynamicByteBuffer& bytes)
			: mData(bytes.AsStatic<4>())
		{}


		// Casting
		[[nodiscard]] IO::ByteBuffer<4> AsBytes() const;
		[[nodiscard]] std::string       AsString() const;

	private:
		IO::ByteBuffer<4> mData;
	};


	class IPv6Address
	{
	public:
		static Optional<IPv6Address> Parse(const std::string& string);


	public:
		IPv6Address(const IO::ByteBuffer<16>& bytes)
			: mData(bytes)
		{}


		IPv6Address(const IO::DynamicByteBuffer& bytes)
			: mData(bytes.AsStatic<16>())
		{}


		// Casting
		[[nodiscard]] const IO::ByteBuffer<16>& AsBytes() const;
		[[nodiscard]] std::string               AsString() const;

	private:
		IO::ByteBuffer<16> mData;
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
		[[nodiscard]] Optional<IPv4Address> AsIPv4() const;
		[[nodiscard]] Optional<IPv6Address> AsIPv6() const;
		[[nodiscard]] IO::DynamicByteBuffer AsBytes() const;
		[[nodiscard]] std::string           AsString() const;


	private:
		std::variant<IPv4Address, IPv6Address> mPayload;
	};
} // namespace Strawberry::Core::Net