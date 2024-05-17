#pragma once


#include <future>
#include <random>


#include "Strawberry/Core/IO/Base64.hpp"
#include "Strawberry/Core/IO/Concepts.hpp"
#include "Strawberry/Net/HTTP/Client.hpp"
#include "Strawberry/Core/IO/Endian.hpp"
#include "Strawberry/Core/Markers.hpp"


namespace Strawberry::Net::Websocket
{
	template <typename S>
	ClientBase<S>::~ClientBase()
	{
		Disconnect();
	}


	template <typename S>
	Core::Result<Core::NullType, Error> ClientBase<S>::SendMessage(const Message& message)
	{
		auto result = TransmitFrame(message);
		if (!result)
		{
			return result.Err();
		}
		else
		{
			return Core::NullType();
		}
	}


	template <typename S>
	Core::Result<Message, Error> ClientBase<S>::ReadMessage()
	{
		return ReceiveFrame();
	}


	template <typename S>
	Core::Result<Message, Error> ClientBase<S>::WaitMessage()
	{
		while (true)
		{
			if (auto msg = ReadMessage(); msg.IsErr() && msg.Err() == Error::NoData)
			{
				std::this_thread::yield();
				continue;
			}
			else
			{
				return msg;
			}
		}
	}


	template <typename S>
	std::string ClientBase<S>::GenerateNonce()
	{
		std::random_device    randomDevice;
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


	template <typename S>
	uint8_t ClientBase<S>::GetOpcodeMask(Message::Opcode opcode)
	{
		switch (opcode)
		{
			case Message::Opcode::Continuation:
				return 0x0;
			case Message::Opcode::Text:
				return 0x1;
			case Message::Opcode::Binary:
				return 0x2;
			case Message::Opcode::Close:
				return 0x8;
			case Message::Opcode::Ping:
				return 0x9;
			case Message::Opcode::Pong:
				return 0xA;
			default:
				std::abort();
		}
	}


	template <typename S>
	Core::Optional<Message::Opcode> ClientBase<S>::GetOpcodeFromByte(uint8_t byte)
	{
		using Opcode = Message::Opcode;

		switch (byte)
		{
			case 0x0:
				return Opcode::Continuation;
			case 0x1:
				return Opcode::Text;
			case 0x2:
				return Opcode::Binary;
			case 0x8:
				return Opcode::Close;
			case 0x9:
				return Opcode::Ping;
			case 0xA:
				return Opcode::Pong;
			default:
				Core::DebugBreak();
				return {};
		}
	}


	template <typename S>
	uint32_t ClientBase<S>::GenerateMaskingKey()
	{
		std::random_device rd;
		uint32_t           key;
		for (int i = 0; i < sizeof(key); i++)
		{
			reinterpret_cast<uint8_t*>(&key)[i] = static_cast<uint8_t>(rd());
		}
		return key;
	}


	template <typename S>
	Core::Result<size_t, Error>
	ClientBase<S>::TransmitFrame(const Message& frame)
	{
		if (mError == Error::ConnectionReset)
		{
			return Error::ConnectionReset;
		}

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

		auto sendResult = mSocket.Write(bytesToSend);
		if (sendResult)
		{
			return sendResult.Unwrap();
		}
		else
			switch (sendResult.Err())
			{
				default:
					Core::Unreachable();
			}
	}


	template <typename S>
	Core::Result<Message, Error> ClientBase<S>::ReceiveFrame()
	{
		if (mError == Error::ConnectionReset)
		{
			return *mError;
		}

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
					return Core::Result<Message, Error>::Err(fragResultB.Err());
				}
			}

			if (message.GetOpcode() == Message::Opcode::Close)
			{
				mError = Error::ConnectionReset;
			}

			return std::move(message);
		}
		else
		{
			return fragResult.Err();
		}
	}


	template <typename S>
	Core::Result<typename ClientBase<S>::Fragment, Error>
	ClientBase<S>::ReceiveFragment()
	{
		using Opcode = Message::Opcode;

		if (!mSocket.Poll())
		{
			return Error::NoData;
		}

		bool   final;
		Opcode opcode;
		if (auto byte = mSocket.Read(1).template Map([](auto x) -> uint8_t { return x.template Into<uint8_t>(); }))
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
				return Error::ProtocolError;
			}
		}
		else
			switch (byte.Err())
			{
				case Error::ConnectionReset:
					return Error::ConnectionReset;
				default:
					Core::Unreachable();
			}

		bool   masked;
		size_t size;
		if (auto byte = mSocket.Read(1).template Map([](auto x) { return x.template Into<uint8_t>(); }))
		{
			masked = *byte & 0b10000000;
			Core::Assert(!masked);
			uint8_t sizeByte = *byte & 0b01111111;
			if (sizeByte == 126)
			{
				size = Core::FromBigEndian(mSocket.Read(sizeof(uint16_t)).Unwrap().template Into<uint16_t>());
			}
			else if (sizeByte == 127)
			{
				size = Core::FromBigEndian(mSocket.Read(sizeof(uint64_t)).Unwrap().template Into<uint64_t>());
			}
			else
			{
				size = sizeByte;
			}
		}
		else
		{
			Core::Unreachable();
		}


		std::vector<uint8_t> payload;
		if (size > 0)
		{
			payload = mSocket.Read(size).Unwrap().AsVector();
		}

		Core::Assert(payload.size() == size);
		return ClientBase<S>::Fragment(final, Message(opcode, payload));
	}


	template <typename S>
	void ClientBase<S>::Disconnect(int code)
	{
		if (mError == Error::ConnectionReset) return;

		auto endianCode = Core::ToBigEndian<uint16_t>(code);
		Websocket::Message::Payload payload;
		payload.push_back(reinterpret_cast<uint8_t*>(&endianCode)[0]);
		payload.push_back(reinterpret_cast<uint8_t*>(&endianCode)[1]);
		SendMessage(Message(Message::Opcode::Close, payload)).Unwrap();
		while (true)
		{
			auto msg = ReadMessage();
			if (!msg && msg.Err() == Error::ConnectionReset ||
				msg && msg.Unwrap().GetOpcode() == Message::Opcode::Close)
			{
				break;
			}
		}

		mError = Error::ConnectionReset;
	}
} // namespace Strawberry::Net::Websocket
