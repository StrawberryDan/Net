#include "Strawberry/Net/HTTP/HTTPClient.hpp"


namespace Strawberry::Net::HTTP
{
    HTTPClient::HTTPClient(const Endpoint& endpoint)
        : HTTPClientBase<Socket::TCPSocket>(endpoint) {}


    HTTPSClient::HTTPSClient(const Endpoint& endpoint)
        : HTTPClientBase<Socket::TLSSocket>(endpoint) {}
} // namespace Strawberry::Net::HTTP
