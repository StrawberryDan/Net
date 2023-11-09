#pragma once


//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Core
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include "Strawberry/Core/IO/Error.hpp"
#include "Strawberry/Net/Endpoint.hpp"
#include "Strawberry/Core/Types/Result.hpp"
// Standard Library
#include <tuple>


#if defined(_WIN32)
#include <winsock2.h>
#endif


namespace Strawberry::Net::Socket
{
	class UDPClient
	{
	public:
		static Core::Result<UDPClient, Error> Create();
		static Core::Result<UDPClient, Error> CreateIPv4();
		static Core::Result<UDPClient, Error> CreateIPv6();


	public:
		UDPClient();
		UDPClient(const UDPClient& other) = delete;
		UDPClient(UDPClient&& other) noexcept;
		UDPClient& operator=(const UDPClient& other) = delete;
		UDPClient& operator=(UDPClient&& other) noexcept;
		~UDPClient();


		[[nodiscard]] bool  Poll() const;
		Core::Result<std::tuple<Core::Optional<Endpoint>, Core::IO::DynamicByteBuffer>, Core::IO::Error>
		Read();
		Core::Result<size_t, Core::IO::Error> Write(const Endpoint& endpoint,
											  const Core::IO::DynamicByteBuffer& bytes) const;


	private:
#if defined(__APPLE__) || defined(__linux__)
		int mSocket;
#elif defined(_WIN32)
		SOCKET mSocket;
#endif

		static constexpr size_t     BUFFER_SIZE = 25536;
		Core::IO::ByteBuffer<BUFFER_SIZE> mBuffer;
	};
} // namespace Strawberry::Net::Socket
