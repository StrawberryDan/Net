#pragma once


namespace Strawberry::Net
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
