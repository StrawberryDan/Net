#pragma once


//======================================================================================================================
//	Includes
//======================================================================================================================
#include "Strawberry/Net/Socket/Types.hpp"
#include "Strawberry/Net/Endpoint.hpp"
// Standard Library
#include <cstdint>
#include <deque>
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
                : mCapacity(bufferSize)
                , mSocket(std::move(socket))
            {}


            BufferedSocket(const BufferedSocket&) = delete;
            BufferedSocket& operator=(const BufferedSocket&) = delete;

            BufferedSocket(BufferedSocket&&) noexcept = default;
            BufferedSocket& operator=(BufferedSocket&& buffered) = delete;


            bool Poll() const
            {
                return !mBuffer.empty() || mSocket.Poll();
            }


            StreamReadResult Read(size_t size)
            {
                Core::IO::DynamicByteBuffer bytes = Core::IO::DynamicByteBuffer::WithCapacity(size);

                while (bytes.Size() < size)
                {
                    if (mBuffer.empty())
                    {
                        if (auto refillResult = RefillBuffer(); !refillResult)
                        {
                            if (refillResult.Err().template IsType<ErrorNoData>()) continue;
                            return refillResult.Err();
                        }
                    }


                    while (!mBuffer.empty() && bytes.Size() < size)
                    {
                        bytes.Push(mBuffer.front());
                        mBuffer.pop_front();
                    }

                    if (!Poll())
                    {
                        break;
                    }
                }

                return bytes;
            }


            StreamReadResult ReadAll(size_t size)
            {
                Core::IO::DynamicByteBuffer bytes = Core::IO::DynamicByteBuffer::WithCapacity(size);

                while (bytes.Size() < size)
                {
                    if (mBuffer.empty())
                    {
                        if (auto refillResult = RefillBuffer(); !refillResult)
                        {
                            if (refillResult.Err().template IsType<ErrorNoData>()) continue;
                            return refillResult.Err();
                        }
                    }


                    while (!mBuffer.empty() && bytes.Size() < size)
                    {
                        bytes.Push(mBuffer.front());
                        mBuffer.pop_front();
                    }


                    if (bytes.Size() < size && !Poll())
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


            void SetBufferCapacity(size_t newSize)
            {
                mCapacity = newSize;
            }


            [[nodiscard]] size_t GetBufferCapacity() const
            {
                return mCapacity;
            }


            [[nodiscard]] Endpoint GetEndpoint() const
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
                    return ErrorNoData {};
                }

                const size_t bufferSpaceAvailable = mCapacity - mBuffer.size();
                if (bufferSpaceAvailable > 0)
                {
                    if (auto readResult = mSocket.Read(bufferSpaceAvailable))
                    {
                        for (auto byte: readResult.Unwrap())
                        {
                            mBuffer.emplace_back(byte);
                        }
                    }
                    else
                    {
                        return readResult.Err();
                    }
                }

                return Core::Success;
            }

        private:
            size_t              mCapacity;
            S                   mSocket;
            std::deque<uint8_t> mBuffer;
    };


    template<typename S>
    BufferedSocket(S, size_t) -> BufferedSocket<S>;
}
