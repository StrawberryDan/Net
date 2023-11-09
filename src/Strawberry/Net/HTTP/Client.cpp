#include "Strawberry/Net/HTTP/Client.hpp"


namespace Strawberry::Core::Net::HTTP
{
	HTTPClient::HTTPClient(const Core::Net::Endpoint& endpoint)
		: ClientBase<Socket::TCPClient>(endpoint)
	{}


	HTTPSClient::HTTPSClient(const Core::Net::Endpoint& endpoint)
		: ClientBase<Socket::TLSClient>(endpoint)
	{}
} // namespace Strawberry::Core::Net::HTTP
