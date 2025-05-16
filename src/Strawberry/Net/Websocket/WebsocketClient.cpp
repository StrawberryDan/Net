#include "Strawberry/Net/Websocket/WebsocketClient.hpp"


namespace Strawberry::Net::Websocket
{
    Core::Result<WSClient, Error> WSClient::Connect(const Endpoint& endpoint, const std::string& resource)
    {
        HTTP::HTTPClient handshaker(endpoint);
        HTTP::Request    upgradeRequest(HTTP::Verb::GET, resource);
        upgradeRequest.GetHeader().Add("Host", endpoint.GetHostname().Value());
        upgradeRequest.GetHeader().Add("Upgrade", "websocket");
        upgradeRequest.GetHeader().Add("Connection", "Upgrade");
        upgradeRequest.GetHeader().Add("Sec-WebSocket-Key", GenerateNonce());
        upgradeRequest.GetHeader().Add("Sec-WebSocket-Version", "13");
        handshaker.SendRequest(upgradeRequest);
        auto response = handshaker.Receive();
        if (response.GetStatus() != 101)
        {
            return Error::Refused;
        }


        WSClient client(std::move(handshaker).IntoSocket());

        return std::move(client);
    }


    WSClient::WSClient(Socket::BufferedSocket<Socket::TCPSocket> socket)
        : WebsocketClientBase(std::move(socket)) {}


    Core::Result<WSSClient, Error> WSSClient::Connect(const Endpoint& endpoint, const std::string& resource)
    {
        HTTP::HTTPSClient handshaker(endpoint);
        HTTP::Request     upgradeRequest(HTTP::Verb::GET, resource);
        upgradeRequest.GetHeader().Add("Host", endpoint.GetHostname().Value());
        upgradeRequest.GetHeader().Add("Upgrade", "websocket");
        upgradeRequest.GetHeader().Add("Connection", "Upgrade");
        upgradeRequest.GetHeader().Add("Sec-WebSocket-Key", GenerateNonce());
        upgradeRequest.GetHeader().Add("Sec-WebSocket-Version", "13");
        handshaker.SendRequest(upgradeRequest);
        auto response = handshaker.Receive();
        if (response.GetStatus() != 101)
        {
            return Error::Refused;
        }


        WSSClient client(std::move(handshaker).IntoSocket());
        return std::move(client);
    }


    WSSClient::WSSClient(Socket::BufferedSocket<Socket::TLSSocket> socket)
        : WebsocketClientBase(std::move(socket)) {}
} // namespace Strawberry::Net::Websocket
