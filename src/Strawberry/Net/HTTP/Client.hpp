#pragma once


#include "Request.hpp"
#include "Response.hpp"
#include "Strawberry/Net/Socket/TCPClient.hpp"
#include "Strawberry/Net/Socket/TLSClient.hpp"
#include "Strawberry/Core/Util/Strings.hpp"


namespace Strawberry::Net::HTTP
{
	template <typename S>
	class ClientBase
	{
	public:
		/// Sends an HTTP Request
		void     SendRequest(const Request& request);
		/// Waits for an HTTP Response
		Response Receive();


		/// Removes and returns the socket of an rvalue HTTP client.
		inline S IntoSocket()&& { return std::move(mSocket); }


	protected:
		/// Connects to the given endpoint over HTTP
		ClientBase(const Endpoint& endpoint);


	private:
		/// Reads a line of input until a newline character from the socket.
		std::string           ReadLine();
		/// Reads a chunked HTTP payload from the socket.
		Core::IO::DynamicByteBuffer ReadChunkedPayload();


	private:
		S mSocket;
	};


	class HTTPClient : public ClientBase<Socket::TCPClient>
	{
	public:
		explicit HTTPClient(const Net::Endpoint& endpoint);
	};


	class HTTPSClient : public ClientBase<Socket::TLSClient>
	{
	public:
		explicit HTTPSClient(const Net::Endpoint& endpoint);
	};
} // namespace Strawberry::Net::HTTP


#include "Client.inl"
