#pragma once


#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Core/Types/Optional.hpp"
#include "TCPClient.hpp"
#include <memory>
#include <openssl/ssl.h>
#include <string>


namespace Strawberry::Net::Socket
{
	class TLSClient
	{
	public:
		static Core::Result<TLSClient, Error> Connect(const Endpoint& endpoint);


	public:
		TLSClient();
		TLSClient(const TLSClient& other) = delete;
		TLSClient(TLSClient&& other) noexcept;
		TLSClient& operator=(const TLSClient& other) = delete;
		TLSClient& operator=(TLSClient&& other) noexcept;
		~TLSClient();


		[[nodiscard]] bool                       Poll() const;
		Core::Result<Core::IO::DynamicByteBuffer, Core::IO::Error> Read(size_t length);
		Core::Result<size_t, Core::IO::Error>                Write(const Core::IO::DynamicByteBuffer& bytes);


	private:
		TCPClient mTCP;
		SSL*      mSSL;
	};
} // namespace Strawberry::Net::Socket
