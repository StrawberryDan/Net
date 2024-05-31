#include <utility>

#include "Strawberry/Net/Websocket/Message.hpp"


#include "Strawberry/Core/Assert.hpp"
#include "Strawberry/Core/IO/Endian.hpp"


namespace Strawberry::Net::Websocket
{
    Message::Message(Message::Opcode opcode, Payload payload)
        : mOpcode(opcode)
        , mPayload(std::move(payload)) {}


    Message::Message(const std::string& string)
        : mOpcode(Opcode::Text)
        , mPayload(string.data(), string.data() + string.size()) {}


    Message::Message(const nlohmann::json& json)
        : Message(nlohmann::to_string(json)) {}


    Message::Message(std::vector<uint8_t> bytes)
        : mOpcode(Opcode::Binary)
        , mPayload(std::move(bytes)) {}


    std::string Message::AsString() const
    {
        return {mPayload.data(), mPayload.data() + mPayload.size()};
    }


    Core::Result<nlohmann::json, std::string> Message::AsJSON() const
    {
        nlohmann::json json;
        switch (mOpcode)
        {
            case Opcode::Text:
            case Opcode::Binary:
            {
                try
                {
                    json = nlohmann::json::parse(mPayload.begin(), mPayload.end());
                }
                catch (std::exception& e)
                {
                    return std::string("Parse Error");
                }
                return Core::Result<nlohmann::json, std::string>::Ok(std::forward<nlohmann::json>(json));
            }

            default: Core::DebugBreak();
                return std::string("Invalid Message Type");
        }
    }


    uint16_t Message::GetCloseStatusCode() const
    {
        uint16_t s = static_cast<uint16_t>(mPayload[0]) << 0 | static_cast<uint16_t>(mPayload[1]) << 8;
        s          = Core::FromBigEndian(s);
        return s;
    }


    void Message::Append(const Message& other)
    {
        mPayload.insert(mPayload.end(), other.mPayload.begin(), other.mPayload.end());
    }
} // namespace Strawberry::Net::Websocket
