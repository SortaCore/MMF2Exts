
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef IPV6_V6ONLY
   #define IPV6_V6ONLY 27
#endif

#ifndef CERT_STORE_READONLY_FLAG
   #define CERT_STORE_READONLY_FLAG         0x00008000
#endif

#ifndef CERT_STORE_OPEN_EXISTING_FLAG
   #define CERT_STORE_OPEN_EXISTING_FLAG     0x00004000
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

typedef __time64_t (__stdcall * fn_mkgmtime64) (struct tm *);
fn_mkgmtime64 compat_mkgmtime64 ();

typedef BOOL (WINAPI * fn_GetFileSizeEx) (HANDLE, PLARGE_INTEGER);
fn_GetFileSizeEx compat_GetFileSizeEx ();




