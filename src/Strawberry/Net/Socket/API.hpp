#pragma once


namespace Strawberry::Net::Socket
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
} // namespace Strawberry::Net::Socket
