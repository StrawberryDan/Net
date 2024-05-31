#pragma once


//======================================================================================================================
//	Includes
//======================================================================================================================
#include "Strawberry/Net/Error.hpp"
#include "Strawberry/Core/Types/Result.hpp"
#include "Strawberry/Core/IO/DynamicByteBuffer.hpp"
// Standard Library
#include <cstdint>


namespace Strawberry::Net::Socket
{
    using StreamReadResult  = Core::Result<Core::IO::DynamicByteBuffer, Error>;
    using StreamWriteResult = Core::Result<void, Error>;
}
