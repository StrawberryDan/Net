#pragma once


namespace Strawberry::Core::Net
{
	enum class Error
	{
		DNSResolution,
		SocketCreation,
		AddressResolution,
		EstablishConnection,
		SSLAllocation,
		SSLHandshake,
		ParsingEndpoint,
		ParsingRTPPacket,
	};
}