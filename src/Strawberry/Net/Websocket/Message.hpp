#pragma once


#include <cstdint>
#include <string>
#include <variant>
#include <vector>


#include "Strawberry/Core/Types/Result.hpp"
#include "nlohmann/json.hpp"


namespace Strawberry::Core::Net::Websocket
{
	class Message
	{
	public:
		enum class Opcode;
		using Payload = std::vector<uint8_t>;

	public:
		explicit Message(Opcode opcode, Payload payload = {});
		explicit Message(const std::string& string);
		explicit Message(const nlohmann::json& json);
		explicit Message(std::vector<uint8_t> bytes);


		[[nodiscard]] inline Opcode GetOpcode() const { return mOpcode; }


		[[nodiscard]] inline std::vector<uint8_t> AsBytes() const { return mPayload; }


		[[nodiscard]] std::string                         AsString() const;
		[[nodiscard]] Result<nlohmann::json, std::string> AsJSON() const;
		[[nodiscard]] uint16_t                            GetCloseStatusCode() const;


		void Append(const Message& other);


	private:
		Opcode  mOpcode;
		Payload mPayload;
	};


	enum class Message::Opcode
	{
		Continuation,
		Text,
		Binary,
		Close,
		Ping,
		Pong,
	};
} // namespace Strawberry::Core::Net::Websocket