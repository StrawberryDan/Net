#pragma once
// Strawberry Net
#include "Address.hpp"
#include "Error.hpp"
// Strawberry Core
#include "Strawberry/Core/Types/Result.hpp"


#if STRAWBERRY_TARGET_WINDOWS
#include <ws2def.h>
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUS
#include <sys/socket.h>
#endif

namespace Strawberry::Net
{
	class Endpoint
	{
	public:
		/// Resolves endpoints from a hostname and a port seperatly.
		static Core::Result<Endpoint, Error> Resolve(const std::string& hostname, uint16_t port);
		/// Parses strings of the form [hostname]:[port] and resolves IP
		static Core::Result<Endpoint, Error> Resolve(const std::string& endpoint);
		/// Static shorthands for creating local host endpoints
		static Endpoint LocalHostIPv4(uint16_t portNumber) noexcept;
		static Endpoint LocalHostIPv6(uint16_t portNumber) noexcept;

	public:
		Endpoint(IPAddress address, uint16_t port);


		[[nodiscard]] inline const Core::Optional<std::string>& GetHostname() const
		{
			return mHostName;
		}


		[[nodiscard]] IPAddress GetAddress() const
		{
			return mAddress;
		}


		[[nodiscard]] inline uint16_t GetPort() const
		{
			return mPort;
		}


		[[nodiscard]] std::string ToString() const noexcept;


		/// Returns this endpoint in the platform's binary format.
		[[nodiscard]] sockaddr_storage GetPlatformRepresentation(bool mapIPv6 = true) const noexcept;


	private:
		Core::Optional<std::string> mHostName;
		IPAddress                   mAddress;
		uint16_t                    mPort;
	};
} // namespace Strawberry::Net
