#pragma once
// Strawberry Net
#include "Address.hpp"
#include "Error.hpp"
// Strawberry Core
#include "Strawberry/Core/Types/Result.hpp"


namespace Strawberry::Net
{
	class Endpoint
	{
	public:
		/// Resolves endpoints from a hostname and a port seperatly.
		static Core::Result<Endpoint, Error> Resolve(const std::string& hostname, uint16_t port);
		/// Parses strings of the form [hostname]:[port] and resolves IP
		static Core::Result<Endpoint, Error> Resolve(const std::string& endpoint);
		/// Parses strings of the form [hostname]:[port] without resolving IP
		static Core::Result<Endpoint, Error> Parse(const std::string& endpoint);
		/// Static shorthands for creating local host endpoints
		static Endpoint LocalHostIPv4(uint16_t portNumber) noexcept;
		static Endpoint LocalHostIPv6(uint16_t portNumber) noexcept;

	public:
		Endpoint();
		Endpoint(const std::string& hostname, uint16_t port);
		Endpoint(IPAddress address, uint16_t port);


		[[nodiscard]] inline const Core::Optional<std::string>& GetHostname() const
		{
			return mHostName;
		}


		[[nodiscard]] inline Core::Optional<IPAddress> GetAddress() const
		{
			return mAddress;
		}


		[[nodiscard]] inline uint16_t GetPort() const
		{
			return mPort;
		}


		[[nodiscard]] std::string ToString() const noexcept;


	private:
		Core::Optional<std::string> mHostName;
		Core::Optional<IPAddress>	mAddress;
		uint16_t					mPort;
	};
} // namespace Strawberry::Net
