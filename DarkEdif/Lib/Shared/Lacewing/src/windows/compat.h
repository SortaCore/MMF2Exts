/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef IPV6_V6ONLY
	#define IPV6_V6ONLY 27
#endif

#ifndef CERT_STORE_READONLY_FLAG
	#define CERT_STORE_READONLY_FLAG		 0x00008000
#endif

#ifndef CERT_STORE_OPEN_EXISTING_FLAG
	#define CERT_STORE_OPEN_EXISTING_FLAG	 0x00004000
#endif

#ifndef AI_V4MAPPED
	#define AI_V4MAPPED 0x00000800
#endif

#ifndef AI_ADDRCONFIG
	#define AI_ADDRCONFIG 0x00000400
#endif

typedef INT (WSAAPI * fn_getaddrinfo)
	(PCSTR, PCSTR, const struct addrinfo *, struct addrinfo **);

fn_getaddrinfo compat_getaddrinfo ();

typedef INT (WSAAPI * fn_freeaddrinfo) (struct addrinfo *);
fn_freeaddrinfo compat_freeaddrinfo ();

typedef INT (WSAAPI * fn_WSASendMsg) (SOCKET Handle,
	LPWSAMSG lpMsg,
	DWORD dwFlags,
	LPDWORD lpNumberOfBytesSent,
	LPWSAOVERLAPPED lpOverlapped,
	LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine);
fn_WSASendMsg compat_WSASendMsg ();

typedef __time64_t (__stdcall * fn_mkgmtime64) (struct tm *);
fn_mkgmtime64 compat_mkgmtime64 ();

typedef BOOL (WINAPI * fn_GetFileSizeEx) (HANDLE, PLARGE_INTEGER);
fn_GetFileSizeEx compat_GetFileSizeEx ();

typedef BOOL (WINAPI* fn_CancelIoEx)(HANDLE, LPOVERLAPPED);
fn_CancelIoEx compat_CancelIoEx ();

typedef HRESULT(WINAPI * fn_SetThreadDescription)(HANDLE, PCWSTR);
fn_SetThreadDescription compat_SetThreadDescription();

#if defined(_WIN32)
#if __cplusplus
extern "C"
#endif
// For Unicode support on Windows.
// Note: wchar_t in C files is not a built-in type; in C++, it's a keyword.
// Returns null or a wide-converted version of the U8 string passed. Free it with free(). Pass size -1 for null-terminated strings.
lw_import wchar_t * lw_char_to_wchar(const char * u8str, int size);
#endif



