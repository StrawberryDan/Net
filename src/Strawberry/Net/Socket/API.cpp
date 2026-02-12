// Strawberry dnet
#include "Strawberry/Net/Socket/API.hpp"
// Windows includes
#if STRAWBERRY_TARGET_WINDOWS
// Strawberry Core
#include "Strawberry/Core/IO/Logging.hpp"
#include "Strawberry/Core/Assert.hpp"
// Win32
#include <winsock2.h>
#endif


namespace Strawberry::Net::Socket
{
	std::atomic<bool> API::sIsInitialised = false;


	void API::Initialise()
	{
		if (!IsInitialised())
		{
#if STRAWBERRY_TARGET_WINDOWS
			WSAData wsaData;
			auto	err = WSAStartup(MAKEWORD(2, 2), &wsaData);
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


	int API::GetError()
	{
#if STRAWBERRY_TARGET_WINDOWS
		return WSAGetLastError();
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
		return errno;
#endif
	}
} // namespace Strawberry::Net::Socket
