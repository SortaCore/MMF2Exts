/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/
#pragma once

#define _lacewing_internal

#ifdef _WIN32

	#if defined (_lacewing_vld)
		#include <vld.h>
	#endif

	#if defined(_DEBUG) && !defined(_lacewing_debug)
	  // #define _lacewing_debug
	#endif

	#ifndef _CRT_SECURE_NO_WARNINGS
	  #define _CRT_SECURE_NO_WARNINGS
	#endif

	#ifndef _CRT_NONSTDC_NO_WARNINGS
	  #define _CRT_NONSTDC_NO_WARNINGS
	#endif

	// These deprecation warnings are functionally useless
	#ifndef _WINSOCK_DEPRECATED_NO_WARNINGS
	  #define _WINSOCK_DEPRECATED_NO_WARNINGS
	#endif

	#ifdef HAVE_CONFIG_H
	  #include "../config.h"
	#endif
	#include <tchar.h>
	#include <inttypes.h>
#else

	#ifndef _GNU_SOURCE
	  #define _GNU_SOURCE
	#endif

#ifdef __ANDROID__
	#include "unix/android config.h"
#elif defined(__unix__)
	#include "unix/unix config.h"
#endif

#if defined(__APPLE__)
	#include "unix/ios config.h"
#else
	#include <sys/sendfile.h>
#endif

#endif

#ifdef _WIN32
	#ifndef _lacewing_static
	  #define lw_import __declspec(dllexport)
	#endif
#else
	#ifdef __GNUC__
	  #ifndef _lacewing_static
		 #define lw_import __attribute__((visibility("default")))
	  #endif
	#else
	  #define lw_import
	#endif
#endif

#ifdef __cplusplus
extern "C"
#endif
void always_log(const char* c, ...);

/* For convenience, some types (such as lw_client and lw_ws_req) are typedef-d
 * to lw_stream in lacewing.h instead of to their extended structure.  The
 * typedefs here are the internal ones mapping everything to their _real_ type,
 * so that the fields after the lw_stream ones may be accessed inside the
 * library.
 */

 typedef struct _lw_thread			* lw_thread;
 typedef struct _lw_addr			* lw_addr;
 typedef struct _lw_filter			* lw_filter;
 typedef struct _lw_pump			* lw_pump;
 typedef struct _lw_pump_watch		* lw_pump_watch;
 typedef struct _lw_eventpump		* lw_eventpump;
 typedef struct _lw_stream			* lw_stream;
 typedef struct _lw_fdstream		* lw_fdstream;
 typedef struct _lw_file			* lw_file;
 typedef struct _lw_timer			* lw_timer;
 typedef struct _lw_sync			* lw_sync;
 typedef struct _lw_event			* lw_event;
 typedef struct _lw_error			* lw_error;
 typedef struct _lw_client			* lw_client;
 typedef struct _lw_server			* lw_server;
 typedef struct _lw_server_client	* lw_server_client;
 typedef struct _lw_udp				* lw_udp;
 typedef struct _lw_flashpolicy		* lw_flashpolicy;
 typedef struct _lw_ws				* lw_ws;
 typedef struct _lw_ws_req			* lw_ws_req;
 typedef struct _lw_ws_req_hdr		* lw_ws_req_hdr;
 typedef struct _lw_ws_req_param	* lw_ws_req_param;
 typedef struct _lw_ws_req_cookie	* lw_ws_req_cookie;
 typedef struct _lw_ws_upload		* lw_ws_upload;
 typedef struct _lw_ws_upload_hdr	* lw_ws_upload_hdr;
 typedef struct _lw_ws_session		* lw_ws_session;
 typedef struct _lw_ws_sessionitem	* lw_ws_sessionitem;

#ifndef _lacewing_h
#include "../Lacewing.h"
#endif

#ifdef _MSC_VER
	#pragma warning(disable: 4200) /* zero-sized array in struct/union */
#endif

#include "list.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <time.h>
#include <ctype.h>


void lwp_init ();
void lwp_deinit ();

#ifdef _lacewing_debug
	#include "refcount-dbg.h"
#else
	#include "refcount.h"
#endif

#ifndef container_of
	#define container_of(p, type, v) \
		((type *)  (((char *) p) - offsetof(type, v)) )

#endif

#include "heapbuffer.h"

#include "../deps/uthash/uthash.h"
#include "nvhash.h"

#define lwp_max_path 512

#ifdef _WIN32
	#include "windows/common.h"
#else
	#include "unix/common.h"
#endif

#if defined(HAVE_MALLOC_H) || defined(_WIN32)
	#include <malloc.h>
#elif defined(HAVE_MALLOC_MALLOC_H)
	#include <malloc/malloc.h>
#endif

#if defined(_lacewing_debug) || defined(_lacewing_debug_output)
	#define lwp_trace lw_trace
#else
	#define lwp_trace(x, ...) (void)0
#endif

/* TODO : find the optimal value for this?  make adjustable? */

#define lwp_default_buffer_size (1024 * 64)

#define lwp_setsockopt(f,l,o,oname,olen) lwp_setsockopt2(f,l,o,#o,oname,olen)

#ifdef __cplusplus
extern "C" {
#endif
void lwp_make_nonblocking (lwp_socket socket);
void lwp_setsockopt2 (lwp_socket fd, int level, int option, const char * optionText, const char * value, socklen_t value_length);
void lwp_disable_ipv6_only (lwp_socket socket);

struct sockaddr_storage lwp_socket_addr (lwp_socket socket);

lw_ui16 lwp_socket_port (lwp_socket socket);

void lwp_close_socket (lwp_socket socket);

lw_bool lwp_urldecode (const char * in, size_t in_length,
						char * out, size_t out_length, lw_bool plus_spaces);

lw_bool lwp_begins_with (const char * string, const char * substring);

void lwp_copy_string (char * dest, const char * source, size_t size);

lw_bool lwp_find_char (const char ** str, size_t * len, char c);

ssize_t lwp_format (char ** output, const char * format, va_list args);

void lwp_to_lowercase (char * str);

extern const char * const lwp_weekdays [];
extern const char * const lwp_months [];

time_t lwp_parse_time (const char *);

lwp_socket lwp_create_server_socket (lw_filter, int type, int protocol, lw_error);

extern struct in6_addr lwp_ipv6_public_fixed_addr;
extern int lwp_ipv6_public_fixed_interface_index;
extern void lwp_trigger_public_address_hunt (lw_bool block);
extern lw_bool lwp_set_ipv6pktinfo_cmsg(void * cmsg);

#ifdef __cplusplus

	} /* extern "C" */

	using namespace lacewing;
	#include <new>

#endif

#define lwp_def_hook(c, hook) \
	void lw_##c##_on_##hook (lw_##c ctx, lw_##c##_hook_##hook hook)			\
	{	ctx->on_##hook = hook;												\
	}																		 \


