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
