#pragma once


#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Core/Types/Optional.hpp"
#include "TCPSocket.hpp"
#include <memory>
#include <openssl/ssl.h>
#include <string>


namespace Strawberry::Net::Socket
{
	class TLSSocket
	{
	public:
		static Core::Result<TLSSocket, Error> Connect(const Endpoint& endpoint);


	public:
		TLSSocket();
		TLSSocket(const TLSSocket& other) = delete;
		TLSSocket(TLSSocket&& other) noexcept;
		TLSSocket& operator=(const TLSSocket& other) = delete;
		TLSSocket& operator=(TLSSocket&& other) noexcept;
		~TLSSocket();


		[[nodiscard]] bool                       Poll() const;
		Core::Result<Core::IO::DynamicByteBuffer, Core::IO::Error> Read(size_t length);
		Core::Result<size_t, Core::IO::Error>                Write(const Core::IO::DynamicByteBuffer& bytes);


	private:
		TCPSocket mTCP;
		SSL*      mSSL;
	};
} // namespace Strawberry::Net::Socket
