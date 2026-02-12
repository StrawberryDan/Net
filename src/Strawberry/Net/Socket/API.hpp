#pragma once
// Standard library
#include <atomic>
#if STRAWBERRY_TARGET_WINDOWS
#include <winsock2.h>
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
#include <cerrno>
#endif


namespace Strawberry::Net::Socket
{
	class API
	{
	public:
		static void Initialise();
		static void Terminate();

		static bool IsInitialised();

		static int GetError();

	private:
		static std::atomic<bool> sIsInitialised;
	};
} // namespace Strawberry::Net::Socket
