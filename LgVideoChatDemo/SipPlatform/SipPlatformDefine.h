

#ifndef _SIP_PLATFORM_DEFINE_H_
#define _SIP_PLATFORM_DEFINE_H_

#ifdef WIN32
#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

// Window XP ���� ������ ������ �Ʒ��� �ּ��� �����϶�.
//#define WINXP

#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <stdlib.h>
#endif

#define VC2008_VERSION	1500

#define strcasecmp	_stricmp
#define strncasecmp	_strnicmp
#define popen				_popen
#define pclose			_pclose
#define sleep(x)		Sleep(x*1000)

typedef unsigned __int64 uint64_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

typedef __int64 int64_t;
typedef int int32_t;
typedef __int16 int16_t;

#if _MSC_VER == VC2008_VERSION
typedef char int8_t;

#define snprintf _snprintf
#define atoll(x) _atoi64(x)
#endif

#define THREAD_API	DWORD WINAPI

#else

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define THREAD_API	void *
#define LPVOID			void *

#endif

#ifdef WIN32
#include <winsock2.h>
#else
#include <sys/time.h>
#endif

#if __WORDSIZE == 64
#ifndef __APPLE__
#define LINUX_64
#endif
#endif

#endif

#if defined LINUX_64
#define UNSIGNED_LONG_LONG_FORMAT	"%lu"
#define LONG_LONG_FORMAT	"%ld"
#else
#define UNSIGNED_LONG_LONG_FORMAT	"%llu"
#define LONG_LONG_FORMAT	"%lld"
#endif


