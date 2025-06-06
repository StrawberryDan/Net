#pragma once


#include <future>
#include <random>


#include "Strawberry/Core/IO/Base64.hpp"
#include "Strawberry/Net/HTTP/HTTPClient.hpp"
#include "Strawberry/Core/IO/Endian.hpp"
#include "Strawberry/Core/Markers.hpp"


namespace Strawberry::Net::Websocket
{
    template<typename S>
    WebsocketClientBase<S>::~WebsocketClientBase()
    {
        if (mSocket)
        {
            Disconnect();
        }
    }


    template<typename S>
    Endpoint WebsocketClientBase<S>::GetEndpoint() const
    {
        return mSocket.Value().GetEndpoint();
    }


    template<typename S>
    Core::Result<void, Error> WebsocketClientBase<S>::SendMessage(const Message& message)
    {
        return TransmitFrame(message);
    }


    template<typename S>
    Core::Result<Message, Error> WebsocketClientBase<S>::ReadMessage()
    {
        return ReceiveFrame();
    }


    template<typename S>
    Core::Result<Message, Error> WebsocketClientBase<S>::WaitMessage()
    {
        while (true)
        {
            if (auto msg = ReadMessage(); msg.IsErr() && msg.Err().template IsType<ErrorNoData>())
            {
                std::this_thread::yield();
            }
            else
            {
                return msg;
            }
        }
    }


    template<typename S>
    std::string WebsocketClientBase<S>::GenerateNonce()
    {
        std::random_device          randomDevice;
        Core::IO::DynamicByteBuffer nonce(16);
        while (nonce.Size() < 16)
        {
            auto val = randomDevice();
            for (int i = 0; i < sizeof(val) && nonce.Size() < 16; i++)
            {
                nonce.Push(reinterpret_cast<uint8_t*>(&val)[i]);
            }
        }
        Core::Assert(nonce.Size() == 16);
        auto base64 = Core::IO::Base64::Encode(nonce);
        Core::Assert(base64.size() == 24);
        return base64;
    }


    template<typename S>
    uint8_t WebsocketClientBase<S>::GetOpcodeMask(Message::Opcode opcode)
    {
        switch (opcode)
        {
            case Message::Opcode::Continuation: return 0x0;
            case Message::Opcode::Text: return 0x1;
            case Message::Opcode::Binary: return 0x2;
            case Message::Opcode::Close: return 0x8;
            case Message::Opcode::Ping: return 0x9;
            case Message::Opcode::Pong: return 0xA;
            default: std::abort();
        }
    }


    template<typename S>
    Core::Optional<Message::Opcode> WebsocketClientBase<S>::GetOpcodeFromByte(uint8_t byte)
    {
        using Opcode = Message::Opcode;

        switch (byte)
        {
            case 0x0: return Opcode::Continuation;
            case 0x1: return Opcode::Text;
            case 0x2: return Opcode::Binary;
            case 0x8: return Opcode::Close;
            case 0x9: return Opcode::Ping;
            case 0xA: return Opcode::Pong;
            default: Core::DebugBreak();
                return {};
        }
    }


    template<typename S>
    uint32_t WebsocketClientBase<S>::GenerateMaskingKey()
    {
        std::random_device rd;
        uint32_t           key;
        for (int i = 0; i < sizeof(key); i++)
        {
            reinterpret_cast<uint8_t*>(&key)[i] = static_cast<uint8_t>(rd());
        }
        return key;
    }


    template<typename S>
    Core::Result<void, Error> WebsocketClientBase<S>::TransmitFrame(const Message& frame)
    {
        Core::IO::DynamicByteBuffer bytesToSend;

        uint8_t byte = 0b10000000 | GetOpcodeMask(frame.GetOpcode());
        bytesToSend.Push(byte);

        auto bytes = frame.AsBytes();
        if (bytes.size() <= 125)
        {
            byte = 0b10000000 | static_cast<uint8_t>(bytes.size());
            bytesToSend.Push(byte);
        }
        else if (bytes.size() <= std::numeric_limits<uint16_t>::max())
        {
            byte = 0b10000000 | 126;
            bytesToSend.Push(byte);
            bytesToSend.Push(Core::ToBigEndian(static_cast<uint16_t>(bytes.size())));
        }
        else if (bytes.size() <= std::numeric_limits<uint64_t>::max())
        {
            byte = 0b10000000 | 127;
            bytesToSend.Push(byte);
            bytesToSend.Push(Core::ToBigEndian(static_cast<uint64_t>(bytes.size())));
        }

        uint32_t maskingKey = Core::ToBigEndian(GenerateMaskingKey());
        bytesToSend.Push(maskingKey);

        for (int i = 0; i < bytes.size(); i++)
        {
            uint8_t mask = reinterpret_cast<uint8_t*>(&maskingKey)[i % sizeof(maskingKey)];
            bytesToSend.Push<uint8_t>(bytes[i] ^ mask);
        }

        return mSocket->Write(bytesToSend);
    }


    template<typename S>
    Core::Result<Message, Error> WebsocketClientBase<S>::ReceiveFrame()
    {
        auto fragResult = ReceiveFragment();

        if (fragResult)
        {
            auto [final, message] = fragResult.Unwrap();

            while (!final)
            {
                auto fragResultB = ReceiveFragment();
                if (fragResultB)
                {
                    auto [finalB, messageB] = fragResultB.Unwrap();
                    message.Append(messageB);
                    final = finalB;
                }
                else
                {
                    return fragResultB.Err();
                }
            }

            if (message.GetOpcode() == Message::Opcode::Close)
            {
                return ErrorConnectionReset {};
            }

            return std::move(message);
        }
        else
        {
            return fragResult.Err();
        }
    }


    template<typename S>
    Core::Result<typename WebsocketClientBase<S>::Fragment, Error> WebsocketClientBase<S>::ReceiveFragment()
    {
        using Opcode = Message::Opcode;

        if (!mSocket->Poll())
        {
            return ErrorNoData {};
        }

        bool   final;
        Opcode opcode;
        if (auto byte = mSocket->ReadAll(1).Map([](auto x) -> uint8_t
        {
            return x.template Into<uint8_t>();
        }))
        {
            final         = *byte & 0b10000000;
            auto opcodeIn = GetOpcodeFromByte(*byte & 0b00001111);
            if (opcodeIn)
            {
                opcode = opcodeIn.Unwrap();
            }
            else
            {
                Core::DebugBreak();
                Disconnect(1002);
                return ErrorProtocolError {};
            }
        }
        else
        {
            return byte.Err();
        }

        bool   masked;
        size_t size;
        if (auto byte = mSocket->ReadAll(1).template Map([](auto x)
        {
            return x.template Into<uint8_t>();
        }))
        {
            masked = *byte & 0b10000000;
            Core::Assert(!masked);
            uint8_t sizeByte = *byte & 0b01111111;
            if (sizeByte == 126)
            {
                size = Core::FromBigEndian(mSocket->ReadAll(sizeof(uint16_t)).Unwrap().template Into<uint16_t>());
            }
            else if (sizeByte == 127)
            {
                size = Core::FromBigEndian(mSocket->ReadAll(sizeof(uint64_t)).Unwrap().template Into<uint64_t>());
            }
            else
            {
                size = sizeByte;
            }
        }
        else
        {
            return byte.Err();
        }


        std::vector<uint8_t> payload;
        if (size > 0)
        {
            auto payloadRead = mSocket->ReadAll(size);
            if (payloadRead.IsErr()) return payloadRead.Err();
            payload = payloadRead.Unwrap().AsVector();
        }

        Core::Assert(payload.size() == size);
        return WebsocketClientBase<S>::Fragment(final, Message(opcode, payload));
    }


    template<typename S>
    void WebsocketClientBase<S>::Disconnect(int code)
    {
        Core::Assert(mSocket.HasValue());

        auto                        endianCode = Core::ToBigEndian<uint16_t>(code);
        Websocket::Message::Payload payload;
        payload.push_back(reinterpret_cast<uint8_t*>(&endianCode)[0]);
        payload.push_back(reinterpret_cast<uint8_t*>(&endianCode)[1]);
        auto sendResult = SendMessage(Message(Message::Opcode::Close, payload));
        if (sendResult == ErrorConnectionReset {})
        {
            return;
        }

        while (true)
        {
            auto msg = ReadMessage();
            if (!msg && msg.Err() == ErrorConnectionReset{} ||
                msg && msg.Unwrap().GetOpcode() == Message::Opcode::Close)
            {
                break;
            }
        }
    }


    template<typename S>
    WebsocketClientBase<S>::WebsocketClientBase(Socket::BufferedSocket<S> socket)
        : mSocket(std::move(socket))
    {
        mSocket->SetBufferCapacity(SOCKET_BUFFER_SIZE);
    }
} // namespace Strawberry::Net::Websocket
