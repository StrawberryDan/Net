#include "Strawberry/Net/HTTP/Client.hpp"


namespace Strawberry::Net::HTTP
{
	HTTPClient::HTTPClient(const Endpoint& endpoint)
		: ClientBase<Socket::TCPClient>(endpoint)
	{}


	HTTPSClient::HTTPSClient(const Endpoint& endpoint)
		: ClientBase<Socket::TLSClient>(endpoint)
	{}
} // namespace Strawberry::Net::HTTP
