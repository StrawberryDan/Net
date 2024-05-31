#include "Strawberry/Net/Socket/API.hpp"


#include "Strawberry/Core/IO/Logging.hpp"


#include "Strawberry/Core/Assert.hpp"
#if STRAWBERRY_TARGET_WINDOWS
#include <winsock2.h>
#endif


namespace Strawberry::Net::Socket
{
    bool API::sIsInitialised = false;


    void API::Initialise()
    {
        if (!IsInitialised())
        {
#if STRAWBERRY_TARGET_WINDOWS
            WSAData wsaData;
            auto    err = WSAStartup(MAKEWORD(2, 2), &wsaData);
            Core::Assert(err == 0);
#endif
            sIsInitialised = true;
        }
    }


    void API::Terminate()
    {
        if (IsInitialised())
        {
#if STRAWBERRY_TARGET_WINDOWS
            WSACleanup();
#endif
            sIsInitialised = false;
        }
    }


    bool API::IsInitialised()
    {
        return sIsInitialised;
    }
} // namespace Strawberry::Net::Socket
