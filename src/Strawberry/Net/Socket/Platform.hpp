#pragma once


// Define the type of the return value of socket functions.
#if STRAWBERRY_TARGET_WINDOWS
#define SOCKET_ERROR_CODE_TYPE decltype(SOCKET_ERROR)
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
#define SOCKET_ERROR_CODE_TYPE int
#endif


// Define macro for the value which socket function return on an error.
#if STRAWBERRY_TARGET_WINDOWS
#define SOCKET_ERROR_CODE SOCKET_ERROR
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
#define SOCKET_ERROR_CODE -1
#endif


// Define the platform dependant type used for socket option setting
#if STRAWBERRY_TARGET_WINDOWS
#define SOCKET_OPTION_TYPE DWORD
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
#define SOCKET_OPTION_TYPE int
#endif


// Cross-platform macro for the poll function
// argument structure type.
#if STRAWBERRY_TARGET_WINDOWS
#define SOCKET_POLL_FD_TYPE WSAPOLLFD
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
#define SOCKET_POLL_FD_TYPE pollfd
#endif


// Cross-platform macro for the poll function
#if STRAWBERRY_TARGET_WINDOWS
#define SOCKET_POLL_FUNCTION WSAPoll
#elif STRAWBERRY_TARGET_MAC || STRAWBERRY_TARGET_LINUX
#define SOCKET_POLL_FUNCTION poll
#endif
