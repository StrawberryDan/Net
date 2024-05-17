#include "Strawberry/Net/Websocket/Client.hpp"


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
		if (response.GetStatus() != 101) { return Error::Refused; }


		WSClient client(std::move(handshaker).IntoSocket());

		return std::move(client);
	}


	WSClient::WSClient(Socket::Buffered<Socket::TCPSocket> socket)
		: ClientBase(std::move(socket))
	{}


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
		if (response.GetStatus() != 101) { return Error::Refused; }


		WSSClient client(std::move(handshaker).IntoSocket());
		return std::move(client);
	}


	WSSClient::WSSClient(Socket::Buffered<Socket::TLSSocket> socket)
		: ClientBase(std::move(socket))
	{}
} // namespace Strawberry::Net::Websocket
