#pragma once


//======================================================================================================================
//		Includes
//----------------------------------------------------------------------------------------------------------------------
#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Net/Socket/Buffered.hpp"
// Strawberry Core
#include "Strawberry/Core/IO/Concepts.hpp"
#include "Strawberry/Net/Socket/TCPSocket.hpp"
#include "Strawberry/Net/Socket/TLSSocket.hpp"
#include "Strawberry/Net/Websocket/Message.hpp"
#include "Strawberry/Core/Sync/Mutex.hpp"
#include "Strawberry/Core/Types/NullType.hpp"
#include "Strawberry/Core/Types/Optional.hpp"
#include "Strawberry/Core/Types/Result.hpp"
// Standard Library
#include <cstdint>
#include <future>
#include <optional>
#include <string>
#include <thread>


namespace Strawberry::Net::Websocket
{
	template<typename S>
	class ClientBase
	{
	public:
		static constexpr size_t SOCKET_BUFFER_SIZE = 2<15;


	public:
		//======================================================================================================================
		//  Contruction/Destruction
		//----------------------------------------------------------------------------------------------------------------------
		ClientBase(const ClientBase&) = delete;
		ClientBase& operator=(const ClientBase&) = delete;


		ClientBase(ClientBase&& rhs) noexcept
			: mSocket(std::move(rhs.mSocket))
			, mError(std::move(rhs.mError))
		{}


		ClientBase& operator=(ClientBase&& rhs) noexcept
		{
			if (this != &rhs)
			{
				std::destroy_at(this);
				std::construct_at(this, std::move(rhs));
			}

			return *this;
		}

		~ClientBase();

		//======================================================================================================================
		//  Sending/Receiving Methods
		//----------------------------------------------------------------------------------------------------------------------
		Core::Result<Core::NullType, Error> SendMessage(const Message& message);

		Core::Result<Message, Error> ReadMessage();

		Core::Result<Message, Error> WaitMessage();


	protected:
		using Fragment = std::pair<bool, Message>;


	protected:
		//======================================================================================================================
		//  Implementation IO
		//----------------------------------------------------------------------------------------------------------------------
		// Receives a single Websocket Frame. This will consolidate sequences of broken up fragments.
		[[nodiscard]] Core::Result<Message, Error> ReceiveFrame();
		// Receives a single Websocket Fragment. This may return only parts of a whole websocket frame.
		[[nodiscard]] Core::Result<Fragment, Error> ReceiveFragment();
		// Sends
		[[nodiscard]] Core::Result<size_t, Error> TransmitFrame(const Message& frame);


		[[nodiscard]] static std::string GenerateNonce();
		[[nodiscard]] static uint8_t GetOpcodeMask(Message::Opcode opcode);
		[[nodiscard]] static Core::Optional<Message::Opcode> GetOpcodeFromByte(uint8_t byte);
		[[nodiscard]] static uint32_t GenerateMaskingKey();

		void Disconnect(int code = 1000);


	protected:
		ClientBase(Socket::Buffered<S> socket);


	protected:
		Socket::Buffered<S> mSocket;
		Core::Optional<Error>   mError;
	};


	class WSClient
		: public ClientBase<Socket::TCPSocket>
	{
	public:
		static Core::Result<WSClient, Error>
		Connect(const Endpoint& endpoint, const std::string& resource);


	protected:
		WSClient(Socket::Buffered<Socket::TCPSocket> socket);
	};


	class WSSClient
		: public ClientBase<Socket::TLSSocket>
	{
	public:
		static Core::Result<WSSClient, Error>
		Connect(const Endpoint& endpoint, const std::string& resource);


	protected:
		WSSClient(Socket::Buffered<Socket::TLSSocket> socket);
	};
} // namespace Strawberry::Net::Websocket

#include "Client.inl"
