#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Net/Endpoint.hpp"
#include "Strawberry/Net/Socket/UDPSocket.hpp"
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
#include <chrono>
#include <cstdint>
#include <random>
#include <thread>


using namespace Strawberry;
using namespace Strawberry::Net;
using namespace Strawberry::Net::Socket;


Core::IO::DynamicByteBuffer CreateRandomMessage()
{
	static constexpr size_t MESSAGE_SIZE = 1024;
	static std::random_device rng;

	Core::IO::DynamicByteBuffer message =
	    Core::IO::DynamicByteBuffer::WithCapacity(MESSAGE_SIZE);
	for (int i = 0; i < MESSAGE_SIZE; i++)
	{
		message.Push<uint8_t>(rng());
	}
	return message;
}


void Wait()
{
	std::this_thread::sleep_for(std::chrono::seconds(1));
}


int main()
{
	// Open two sockets on localhost.
	// Bind them to different ports.
	// Send two random messages, one in each direction.
	// Succeeds if the messages are transmitted without error.


	static constexpr uint16_t portA = 65535 - 1002,
	                          portB = 65535 - 1003;


	const auto messageA = CreateRandomMessage(),
	           messageB = CreateRandomMessage();


	UDPSocket clientA = UDPSocket::Create().Unwrap();
	clientA.Bind(Endpoint::AnyIPv6(portA)).Unwrap();


	UDPSocket clientB = UDPSocket::CreateIPv4().Unwrap();
	clientB.Bind(Endpoint::AnyIPv4(portB)).Unwrap();


	Core::Logging::Trace("Sending message A");
	clientA.Send(Endpoint::LocalHostIPv4(portB), messageA).Unwrap();
	Wait();
	Core::Assert(clientB.Poll());
	Core::Logging::Trace("Receiving message A");
	auto receivedA = clientB.Receive().Unwrap();
	Core::AssertEQ(receivedA.contents, messageA);

	Core::Logging::Trace("Sending message B");
	clientB.Send(Endpoint::LocalHostIPv4(portA), messageB).Unwrap();
	Wait();
	Core::Assert(clientA.Poll());
	Core::Logging::Trace("Receiving message B");
	auto receivedB = clientA.Receive().Unwrap();
	Core::AssertEQ(receivedB.contents, messageB);
}
