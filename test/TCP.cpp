#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include "Strawberry/Net/Address.hpp"
#include "Strawberry/Net/Endpoint.hpp"
#include "Strawberry/Net/Socket/TCPListener.hpp"
#include "Strawberry/Net/Socket/TCPSocket.hpp"
#include <chrono>
#include <random>
#include <thread>

using namespace Strawberry;
using namespace Net;


int main()
{
	static constexpr size_t MESSAGE_SIZE = 32 * 1024; // 32 Kilobytes
	std::random_device rng;


	// Test with both IPv4 and IPv6 endpoints
	std::array endpoints
	{
		Endpoint(IPv4Address::LocalHost(), 65535 - 1000),
		Endpoint(IPv6Address::LocalHost(), 65535 - 1001)
	};


	for (auto endpoint : endpoints)
	{
		// Create listener.
		Core::Logging::Trace("Opening listener!");
		auto listener = Socket::TCPListener::Bind(endpoint).Unwrap();
		// Create client
		Core::Logging::Trace("Opening client");
		auto client = Socket::TCPSocket::Connect(endpoint).Unwrap();

		// Accept client from listener.
		auto listenerSocket = listener.Accept().Unwrap();
		Core::Logging::Trace("Accepted client");

		// Generate random message.
		Core::IO::DynamicByteBuffer message;
		for (int i = 0; i < MESSAGE_SIZE; i++)
		{
			message.Push<uint8_t>(rng());
		}

		// Send message to server
		Core::Logging::Trace("Sending message from client");
		client.Write(message).Unwrap();

		// Wait for a second.
		std::this_thread::sleep_for(std::chrono::seconds(1));

		// Read message from listenerSocket.
		Core::Logging::Trace("Reading message");
		Core::Assert(listenerSocket.Poll());
		auto receivedMessage = listenerSocket.ReadAll(MESSAGE_SIZE).Unwrap();
		Core::AssertEQ(receivedMessage, message);

		// Generate fresh message
		message.Clear();
		for (int i = 0; i < MESSAGE_SIZE; i++)
		{
			message.Push<uint8_t>(rng());
		}
		listenerSocket.Write(message).Unwrap();

		// Wait for a second
		std::this_thread::sleep_for(std::chrono::seconds(1));

		// Read new message from server
		Core::Assert(client.Poll());
		receivedMessage = client.ReadAll(MESSAGE_SIZE).Unwrap();
		Core::AssertEQ(receivedMessage, message);
	}
}
