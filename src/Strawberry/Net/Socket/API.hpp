#pragma once


namespace Strawberry::Core::Net::Socket
{
	class API
	{
    public:
        static void Initialise();
        static void Terminate();

        static bool IsInitialised();

    private:
        static bool sIsInitialised;
	};
} // namespace Strawberry::Core::Net::Socket