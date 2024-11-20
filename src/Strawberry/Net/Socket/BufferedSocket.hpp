#pragma once


//======================================================================================================================
//	Includes
//======================================================================================================================
#include "Strawberry/Core/Collection/CircularBuffer.hpp"
#include "Strawberry/Net/Socket/Types.hpp"
#include "Strawberry/Net/Endpoint.hpp"
// Standard Library
#include <cstdint>
#include <thread>


namespace Strawberry::Net::Socket
{
    template<typename T>
    class BufferedSocket {};


    template<typename S> requires (std::same_as<S, class TCPSocket> || std::same_as<S, class TLSSocket>)
    class BufferedSocket<S>
    {
        public:
            BufferedSocket(S socket, size_t bufferSize)
                : mSocket(std::move(socket))
                , mBuffer(bufferSize) {}


            bool Poll() const
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


            StreamReadResult Read(size_t size)
            {
                Core::IO::DynamicByteBuffer bytes = Core::IO::DynamicByteBuffer::WithCapacity(size);

                while (bytes.Size() < size)
                {
                    while (!mBuffer.Empty() && bytes.Size() < size)
                    {
                        bytes.Push(mBuffer.Pop().Unwrap());
                    }

                    if (auto refillResult = RefillBuffer(); !refillResult)
                    {
                        switch (refillResult.Err())
                        {
                            case Error::ConnectionReset: return refillResult.Err();
                            case Error::NoData: continue;
                            default: Core::Unreachable();
                        }
                    }

                    if (mBuffer.Empty())
                    {
                        return bytes;
                    }
                }

                return bytes;
            }


            StreamReadResult ReadAll(size_t size)
            {
                Core::IO::DynamicByteBuffer bytes = Core::IO::DynamicByteBuffer::WithCapacity(size);

                while (bytes.Size() < size)
                {
                    while (!mBuffer.Empty() && bytes.Size() < size)
                    {
                        bytes.Push(mBuffer.Pop().Unwrap());
                    }

                    if (auto refillResult = RefillBuffer(); !refillResult)
                    {
                        switch (refillResult.Err())
                        {
                            case Error::ConnectionReset: return refillResult.Err();
                            case Error::NoData: continue;
                            default: Core::Unreachable();
                        }
                    }

                    if (mBuffer.Empty())
                    {
                        std::this_thread::yield();
                    }
                }

                return bytes;
            }


            StreamWriteResult Write(Core::IO::DynamicByteBuffer bytes)
            {
                return mSocket.Write(bytes);
            }


            void Resize(size_t newSize)
            {
                mBuffer.Resize(newSize);
            }


            size_t BufferSize() const
            {
                return mBuffer.Capacity();
            }


            Endpoint GetEndpoint() const
            {
                return mSocket.GetEndpoint();
            }


            S TakeSocket() &&
            {
                return std::move(mSocket);
            }

        protected:
            Core::Result<void, Error> RefillBuffer()
            {
                if (!mSocket.Poll())
                {
                    return Error::NoData;
                }

                if (BufferSpaceAvailable() > 0)
                {
                    if (auto readResult = mSocket.Read(BufferSpaceAvailable()))
                    {
                        for (auto byte: readResult.Unwrap())
                        {
                            mBuffer.Push(byte);
                        }
                    }
                    else
                    {
                        return readResult.Err();
                    }
                }

                return Core::Success;
            }


            size_t BufferSpaceAvailable() const
            {
                return mBuffer.Capacity() - mBuffer.Size();
            }

        private:
            S                                         mSocket;
            Core::Collection::CircularBuffer<uint8_t> mBuffer;
    };


    template<typename S>
    BufferedSocket(S, size_t) -> BufferedSocket<S>;
}
