// platform detection

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)

#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS

#ifdef DLL_EXPORTS
	#define HERMOD_API __declspec( dllexport )
#else
	#define HERMOD_API __declspec( dllimport )
#endif

#undef min
#undef max

#ifndef NOMINMAX
# define NOMINMAX
#endif

#include <winsock2.h>
#pragma comment( lib, "Ws2_32.lib" )

#include <minwindef.h>

#elif PLATFORM == PLATFORM_MAC || 
PLATFORM == PLATFORM_UNIX

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

#endif