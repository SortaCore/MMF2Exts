/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"

static void * MSVCRT (const char * fn)
{
	static HINSTANCE DLL = 0;

	if (!DLL)
		DLL = LoadLibraryA ("msvcrt.dll");

	return DLL ? (void *) GetProcAddress (DLL, fn) : 0;
}

static void * WS2_32 (const char * fn)
{
	static HINSTANCE DLL = 0;

	if (!DLL)
		DLL = LoadLibraryA ("ws2_32.dll");

	return DLL ? (void *) GetProcAddress (DLL, fn) : 0;
}

static void * KERNEL32 (const char * fn)
{
	static HINSTANCE DLL = 0;

	if (!DLL)
		DLL = LoadLibraryA ("kernel32.dll");

	return DLL ? (void *) GetProcAddress (DLL, fn) : 0;
}

fn_getaddrinfo compat_getaddrinfo ()
{
	static fn_getaddrinfo fn = 0;

	return fn ? fn : (fn = (fn_getaddrinfo) WS2_32 ("getaddrinfo"));
}

fn_freeaddrinfo compat_freeaddrinfo ()
{
	static fn_freeaddrinfo fn = 0;

	return fn ? fn : (fn = (fn_freeaddrinfo) WS2_32 ("freeaddrinfo"));
}

fn_WSASendMsg compat_WSASendMsg()
{
	static fn_WSASendMsg fn = 0;

	/* Note that SIO_GET_EXTENSION_FUNCTION_POINTER for WSASendMsg is not necessary in Vista+:
	   https://learn.microsoft.com/en-us/windows/win32/winsock/provider-specific-extension-mechanism-2
	   WSASendMsg is only available in Vista+ anyway, so we will look up directly. */
	return fn ? fn : (fn = (fn_WSASendMsg) WS2_32 ("WSASendMsg"));
}

fn_mkgmtime64 compat_mkgmtime64 ()
{
	static fn_mkgmtime64 fn = 0;

	return fn ? fn : (fn = (fn_mkgmtime64) MSVCRT ("_mkgmtime64"));
}

fn_GetFileSizeEx compat_GetFileSizeEx ()
{
	static fn_GetFileSizeEx fn = 0;

	return fn ? fn : (fn = (fn_GetFileSizeEx) KERNEL32 ("GetFileSizeEx"));
}

fn_CancelIoEx compat_CancelIoEx ()
{
	static fn_CancelIoEx fn = 0;

	return fn ? fn : (fn = (fn_CancelIoEx) KERNEL32 ("CancelIoEx"));
}

fn_SetThreadDescription compat_SetThreadDescription ()
{
	static fn_SetThreadDescription fn = 0;

	return fn ? fn : (fn = (fn_SetThreadDescription) KERNEL32 ("SetThreadDescription"));
}

#if defined(_WIN32)

// Returns null or a wide-converted version of the U8 string passed. Free it with free(). Pass size -1 for null-terminated strings.
lw_import wchar_t * lw_char_to_wchar(const char * u8str, int size)
{
	int length = MultiByteToWideChar(CP_UTF8, 0, u8str, size, NULL, 0);
	if (length > 0)
	{
		length += 10;
		wchar_t * u8Wide = (wchar_t *)malloc(length * sizeof(wchar_t));
		if (u8Wide)
		{
			length = MultiByteToWideChar(CP_UTF8, 0, u8str, size, u8Wide, length);
			if (length > 0)
			{
				// If size does not include the null byte, the converted result won't either.
				#pragma warning (suppress: 6386)
				u8Wide[length] = L'\0';
				return u8Wide;
			}
			free(u8Wide);
		}
	}
	return NULL;
}
#endif
