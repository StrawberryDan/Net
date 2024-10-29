#pragma once

#if STRAWBERRY_TARGET_WINDOWS
#include <winsock2.h>
#elif STRAWBERRY_TARGET_MACOS || STRAWBERRY_TARGET_LINUX
    #include <sys/errno.h>
#endif


namespace Strawberry::Net::Socket
{

#ifdef STRAWBERRY_TARGET_WINDOWS
    using ErrorCode = int;
#elifdef STRAWBERRY_TARGET_MAC
    using ErrorCode = errno_t;
#endif


    namespace ErrorCodes {
        enum : ErrorCode
        {
#ifdef STRAWBERRY_TARGET_WINDOWS
            ConnectionReset = WSAECONNRESET,
            NoBufferSpace = WSAENOBUFS,
    #elifdef STRAWBERRY_TARGET_MAC
            ConnectionReset = ECONNRESET,
            NoBufferSpace = ENOBUFS,
    #endif
        };
    }



    class API
    {
        public:
            static void Initialise();
            static void Terminate();

            static bool IsInitialised();

            static ErrorCode GetError();

        private:
            static bool sIsInitialised;
    };
} // namespace Strawberry::Net::Socket
