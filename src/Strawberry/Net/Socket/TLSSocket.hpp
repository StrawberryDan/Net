#pragma once

//======================================================================================================================
//	Includes
//======================================================================================================================
#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Net/Socket/TCPSocket.hpp"
#include "Strawberry/Core/Types/Optional.hpp"
// Open SSL
#include <openssl/ssl.h>
// Standard Library
#include <memory>
#include <string>


//======================================================================================================================
//	Includes
//======================================================================================================================
namespace Strawberry::Net::Socket
{
	class TLSSocket
	{
	public:
		static Core::Result<TLSSocket, Error> Connect(const Endpoint& endpoint);

	public:
		TLSSocket(const TLSSocket& other) = delete;
		TLSSocket(TLSSocket&& other) noexcept;
		TLSSocket& operator=(const TLSSocket& other) = delete;
		TLSSocket& operator=(TLSSocket&& other) noexcept;
		~TLSSocket();


		Endpoint GetEndpoint() const;


		[[nodiscard]] bool Poll() const;
		StreamReadResult   Read(size_t length);
		StreamReadResult   ReadAll(size_t length);
		StreamWriteResult  Write(const Core::IO::DynamicByteBuffer& bytes);

	private:
		TLSSocket(TCPSocket socket, SSL* ssl, Endpoint endpoint);


		TCPSocket mTCP;
		SSL*	  mSSL;
		Endpoint  mEndpoint;
	};
} // namespace Strawberry::Net::Socket
