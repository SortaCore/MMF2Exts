/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef WINVER
	#define WINVER 0x0501
#endif

#ifndef _WIN32_WINNT
	#define _WIN32_WINNT 0x0501
#endif

#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <mswsock.h>

#define SECURITY_WIN32

#include <security.h>
#include <sspi.h>
#include <wincrypt.h>
#include <schannel.h>
#include <process.h>

#include "compat.h"

typedef SOCKET lwp_socket;

#define lwp_vsnprintf _vsnprintf
#define lwp_snprintf _snprintf
#define lwp_fmt_size "%zu"

#define lwp_last_error GetLastError()
#define lwp_last_socket_error WSAGetLastError()

#ifdef _MSC_VER
	#define ssize_t SSIZE_T
#endif

#define strcasecmp _stricmp

