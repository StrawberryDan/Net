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


namespace Strawberry::Core::Net::Socket
{
	class UDPClient
	{
	public:
		static Result<UDPClient, Error> Create();
		static Result<UDPClient, Error> CreateIPv4();
		static Result<UDPClient, Error> CreateIPv6();


	public:
		UDPClient();
		UDPClient(const UDPClient& other) = delete;
		UDPClient(UDPClient&& other) noexcept;
		UDPClient& operator=(const UDPClient& other) = delete;
		UDPClient& operator=(UDPClient&& other) noexcept;
		~UDPClient();


		[[nodiscard]] bool                                                     Poll() const;
		Result<std::tuple<Optional<Endpoint>, IO::DynamicByteBuffer>, IO::Error> Read();
		Result<size_t, IO::Error>                                              Write(const Endpoint& endpoint, const IO::DynamicByteBuffer& bytes) const;


	private:
#if defined(__APPLE__) || defined(__linux__)
		int mSocket;
#elif defined(_WIN32)
		SOCKET mSocket;
#endif

		static constexpr size_t     BUFFER_SIZE = 25536;
		IO::ByteBuffer<BUFFER_SIZE> mBuffer;
	};
} // namespace Strawberry::Core::Net::Socket
