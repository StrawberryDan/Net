#pragma once


#include "Strawberry/Core/Types/Variant.hpp"


namespace Strawberry::Net
{
	struct ErrorDNSResolution {};
	struct ErrorSocketCreation {};
	struct ErrorSocketBinding {};
	struct ErrorAddressResolution {};
	struct ErrorEstablishConnection {};
	struct ErrorSSLAllocation {};
	struct ErrorSSLHandshake {};
	struct ErrorParsingEndpoint {};
	struct ErrorParsingRTPPacket {};
	struct ErrorConnectionReset {};
	struct ErrorNoData {};
	struct ErrorProtocolError {};
	struct ErrorRefused {};
	struct ErrorOpenSSL {};
	struct ErrorSystem {};



	using Error = Core::Variant<
		ErrorDNSResolution,
		ErrorSocketCreation,
		ErrorSocketBinding,
		ErrorAddressResolution,
		ErrorEstablishConnection,
		ErrorSSLAllocation,
		ErrorSSLHandshake,
		ErrorParsingEndpoint,
		ErrorParsingRTPPacket,
		ErrorConnectionReset,
		ErrorNoData,
		ErrorProtocolError,
		ErrorRefused,
		ErrorOpenSSL,
		ErrorSystem>;
}
