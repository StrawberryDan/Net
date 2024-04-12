#include "Strawberry/Net/HTTP/Client.hpp"


namespace Strawberry::Net::HTTP
{
	HTTPClient::HTTPClient(const Endpoint& endpoint)
		: ClientBase<Socket::TCPSocket>(endpoint)
	{}


	HTTPSClient::HTTPSClient(const Endpoint& endpoint)
		: ClientBase<Socket::TLSSocket>(endpoint)
	{}
} // namespace Strawberry::Net::HTTP
