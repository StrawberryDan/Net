#pragma once


//======================================================================================================================
//	Includes
//======================================================================================================================
#include "Strawberry/Core/Collection/CircularBuffer.hpp"
#include "Strawberry/Net/Socket/Types.hpp"
// Standard Library
#include <cstdint>
#include <thread>


namespace Strawberry::Net::Socket
{
	template <typename S>
    class Buffered
	{
	public:
		Buffered(S socket, size_t bufferSize);


		bool              Poll() const;
		StreamReadResult  Read(size_t size);
		StreamReadResult  ReadAll(size_t size);
		StreamWriteResult Write(Core::IO::DynamicByteBuffer bytes);


		void Resize(size_t newSize);
		size_t BufferSize() const;


		S TakeSocket() &&;


	protected:
		Core::Optional<Error> RefillBuffer();
		size_t BufferSpaceAvailable() const;


	private:
		S                                         mSocket;
		Core::Collection::CircularBuffer<uint8_t> mBuffer;
	};

	template <typename S>
	Buffered(S, size_t) -> Buffered<S>;


	template<typename S>
	Buffered<S>::Buffered(S socket, size_t bufferSize)
			: mSocket(std::move(socket))
			, mBuffer(bufferSize)
	{}


	template<typename S>
	bool Buffered<S>::Poll() const
	{
		if (mBuffer.Empty())
		{
			return mSocket.Poll();
		}
		else
		{
			return true;
		}
	}


	template<typename S>
	StreamReadResult Buffered<S>::Read(size_t size)
	{
		Core::IO::DynamicByteBuffer bytes = Core::IO::DynamicByteBuffer::WithCapacity(size);

		while (bytes.Size() < size)
		{
			while (!mBuffer.Empty() && bytes.Size() < size)
			{
				bytes.Push(mBuffer.Pop().Unwrap());
			}

			auto refillError = RefillBuffer();
			if (refillError)
			{
				switch (refillError.Value())
				{
					case Error::ConnectionReset:
						return refillError.Value();
					default:
						Core::Unreachable();
				}
			}

			if (mBuffer.Empty())
			{
				return bytes;
			}
		}

		return bytes;
	}


	template<typename S>
	StreamReadResult Buffered<S>::ReadAll(size_t size)
	{
		Core::IO::DynamicByteBuffer bytes = Core::IO::DynamicByteBuffer::WithCapacity(size);

		while (bytes.Size() < size)
		{
			while (!mBuffer.Empty() && bytes.Size() < size)
			{
				bytes.Push(mBuffer.Pop().Unwrap());
			}

			Core::Assert(!RefillBuffer().HasValue());

			if (mBuffer.Empty())
			{
				std::this_thread::yield();
			}
		}

		return bytes;
	}


	template<typename S>
	StreamWriteResult Buffered<S>::Write(Core::IO::DynamicByteBuffer bytes)
	{
		return mSocket.Write(bytes);
	}


	template<typename S>
	void Buffered<S>::Resize(size_t newSize)
	{
		mBuffer.Resize(newSize);
	}


	template<typename S>
	size_t Buffered<S>::BufferSize() const
	{
		return mBuffer.Capacity();
	}


	template<typename S>
	S Buffered<S>::TakeSocket()&&
	{
		return std::move(mSocket);
	}


	template<typename S>
	Core::Optional<Error> Buffered<S>::RefillBuffer()
	{
		if (!mSocket.Poll())
		{
			return Core::NullOpt;
		}

		if (BufferSpaceAvailable() > 0)
		{
			if (auto readResult = mSocket.Read(BufferSpaceAvailable()))
			{
				for (auto byte : readResult.Unwrap())
				{
					mBuffer.Push(byte);
				}
			}
			else
			{
				return readResult.Err();
			}
		}

		return Core::NullOpt;
	}


	template<typename S>
	size_t Buffered<S>::BufferSpaceAvailable() const
	{
		return mBuffer.Capacity() - mBuffer.Size();
	}
}
