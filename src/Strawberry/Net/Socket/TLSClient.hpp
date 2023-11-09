#pragma once


#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Core/Types/Optional.hpp"
#include "TCPClient.hpp"
#include <memory>
#include <openssl/ssl.h>
#include <string>


namespace Strawberry::Core::Net::Socket
{
	class TLSClient
	{
	public:
		static Result<TLSClient, Error> Connect(const Endpoint& endpoint);


	public:
		TLSClient();
		TLSClient(const TLSClient& other) = delete;
		TLSClient(TLSClient&& other) noexcept;
		TLSClient& operator=(const TLSClient& other) = delete;
		TLSClient& operator=(TLSClient&& other) noexcept;
		~TLSClient();


		[[nodiscard]] bool                       Poll() const;
		Result<IO::DynamicByteBuffer, IO::Error> Read(size_t length);
		Result<size_t, IO::Error>                Write(const IO::DynamicByteBuffer& bytes);


	private:
		TCPClient mTCP;
		SSL*      mSSL;
	};
} // namespace Strawberry::Core::Net::Socket
