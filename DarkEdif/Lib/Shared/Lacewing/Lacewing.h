/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lacewing_h
#define _lacewing_h

#include <stdarg.h>
#include <stdio.h>

#ifndef _MSC_VER

	#ifndef __STDC_FORMAT_MACROS
		#define __STDC_FORMAT_MACROS
	#endif

	#include <inttypes.h>

	typedef   int64_t	lw_i64;
	typedef  uint64_t	lw_ui64;
	typedef  intptr_t	lw_iptr;
	typedef uintptr_t	lw_uiptr;
	typedef   int32_t	lw_i32;
	typedef  uint32_t	lw_ui32;
	typedef   int16_t	lw_i16;
	typedef  uint16_t	lw_ui16;
	typedef	   int8_t	lw_i8;
	typedef	  uint8_t	lw_ui8;

	#define lw_PRId64 PRId64
	#define lw_PRIu64 PRIu64

#else

	#ifdef _WIN64
		typedef 		 __int64  lw_iptr;
		typedef unsigned __int64  lw_uiptr;
	#else
		typedef 		 __int32  lw_iptr;
		typedef unsigned __int32  lw_uiptr;
	#endif

	typedef 		 __int64  lw_i64;
	typedef unsigned __int64  lw_ui64;
	typedef 		 __int32  lw_i32;
	typedef unsigned __int32  lw_ui32;
	typedef 		 __int16  lw_i16;
	typedef unsigned __int16  lw_ui16;
	typedef 		  __int8  lw_i8;
	typedef unsigned  __int8  lw_ui8;

	#define lw_PRId64 "I64d"
	#define lw_PRIu64 "I64u"

#endif

#ifndef _WIN32
	#ifndef lw_callback
		#define lw_callback
	#endif

	#ifndef lw_import
		#define lw_import
	#endif
	#include <netinet/in.h>
#else

	/* For the definition of HANDLE and OVERLAPPED (used by lw_pump) */
	#ifndef _INC_WINDOWS
		#include <winsock2.h>
		#include <windows.h>
	#endif

	#define lw_callback __cdecl
	#include <in6addr.h>

#endif

#ifndef lw_import
	#ifdef _lacewing_static
		#define lw_import
	#else
		#define lw_import __declspec (dllimport)
	#endif
#endif

#ifdef __cplusplus

#include <atomic>
#include <vector>
#include <memory>
#include <string>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <cctype>
#include <cstring>

// std::string_view requires C++17
#if (__cplusplus < 201703L && _MSVC_LANG < 201703L) || (defined(__clang__) && !__has_include(<string_view>))
	#error C++17 std::string_view not available, check what C++ standard your project is using
#endif
#include <string_view>
using namespace std::string_view_literals;

#endif // __cplusplus
#define LacewingFatalErrorMsgBox() LacewingFatalErrorMsgBox2(__FUNCTION__, __FILE__, __LINE__)
void LacewingFatalErrorMsgBox2(const char * const func, const char * const file, const int line);

typedef lw_i8 lw_bool;

#define  lw_true ((lw_bool) 1)
#define lw_false ((lw_bool) 0)

#ifdef _WIN32
	typedef HANDLE lw_fd;
#else
	typedef int lw_fd;
#endif

#if (!defined(_lacewing_internal))

	/* The ugly underscore prefixes are only necessary because we have to
	 * compile the library as C++ for MSVC compatibility.
	 */

	typedef struct _lw_thread			*  lw_thread;
	typedef struct _lw_addr				*  lw_addr;
	typedef struct _lw_filter			*  lw_filter;
	typedef struct _lw_pump				*  lw_pump;
	typedef struct _lw_pump_watch		*  lw_pump_watch;
	typedef struct _lw_eventpump		*  lw_eventpump;
	typedef struct _lw_stream			*  lw_stream;
	typedef struct _lw_fdstream			*  lw_fdstream;
	typedef struct _lw_file				*  lw_file;
	typedef struct _lw_timer			*  lw_timer;
	typedef struct _lw_sync				*  lw_sync;
	typedef struct _lw_event			*  lw_event;
	typedef struct _lw_error			*  lw_error;
	typedef struct _lw_client			*  lw_client;
	typedef struct _lw_server			*  lw_server;
	typedef struct _lw_server_client	*  lw_server_client;
	typedef struct _lw_udp				*  lw_udp;
	typedef struct _lw_flashpolicy		*  lw_flashpolicy;
	typedef struct _lw_ws				*  lw_ws;
	typedef struct _lw_ws_req			*  lw_ws_req;
	typedef struct _lw_ws_req_hdr		*  lw_ws_req_hdr;
	typedef struct _lw_ws_req_param		*  lw_ws_req_param;
	typedef struct _lw_ws_req_cookie	*  lw_ws_req_cookie;
	typedef struct _lw_ws_upload		*  lw_ws_upload;
	typedef struct _lw_ws_upload_hdr	*  lw_ws_upload_hdr;
	typedef struct _lw_ws_session		*  lw_ws_session;
	typedef struct _lw_ws_sessionitem	*  lw_ws_sessionitem;

#endif

#ifdef __cplusplus
 extern "C"
 {
#endif

	lw_import	const char *  lw_version			();
	lw_import		  lw_i64  lw_file_last_modified	(const char * filename);
	lw_import		 lw_bool  lw_file_exists		(const char * filename);
	lw_import		  size_t  lw_file_size			(const char * filename);
	lw_import		 lw_bool  lw_path_exists		(const char * filename);
	lw_import			void  lw_temp_path			(char * buffer);
	lw_import	const char *  lw_guess_mimetype		(const char * filename);
	lw_import			void  lw_md5				(char * output, const char * input, size_t length);
	lw_import			void  lw_md5_hex			(char * output, const char * input, size_t length);
	lw_import			void  lw_sha1				(char * output, const char * input, size_t length);
	lw_import			void  lw_sha1_hex			(char * output, const char * input, size_t length);
#if defined(_MSC_VER) || (defined(__unix__) && !defined(__ANDROID__))
	#define __printflike(a,b) /* no op */
#endif
	lw_import			void  lw_trace				(const char * format, ...) __printflike(1, 2);
	lw_import			void  lw_dump				(const char * buffer, size_t size);
	lw_import		 lw_bool  lw_random				(char * buffer, size_t size);
	lw_import		  size_t  lw_min_size_t			(size_t a, size_t b);

/* Thread */

	lw_import	   lw_thread  lw_thread_new		(const char * name, void * proc);
	lw_import			void  lw_thread_delete	(lw_thread);
	lw_import			void  lw_thread_start	(lw_thread, void * parameter);
	lw_import		 lw_bool  lw_thread_started	(lw_thread);
	lw_import		  void *  lw_thread_join	(lw_thread);
	lw_import		  void *  lw_thread_tag		(lw_thread);
	lw_import			void  lw_thread_set_tag	(lw_thread, void *);

/* Address */

	lw_import			lw_addr  lw_addr_new			(const char * hostname, const char * service);
	lw_import			lw_addr  lw_addr_new_port		(const char * hostname, lw_ui16 port);
	lw_import			lw_addr  lw_addr_new_hint		(const char * hostname, const char * service, int hints);
	lw_import			lw_addr  lw_addr_new_port_hint	(const char * hostname, lw_ui16 port, int hints);
	lw_import			lw_addr  lw_addr_clone			(lw_addr);
	lw_import			   void  lw_addr_delete			(lw_addr);
	lw_import			lw_ui16  lw_addr_port			(lw_addr);
	lw_import			   void  lw_addr_set_port		(lw_addr, long port);
	lw_import				int  lw_addr_type			(lw_addr);
	lw_import			   void  lw_addr_set_type		(lw_addr, int);
	lw_import			lw_bool  lw_addr_ready			(lw_addr);
	lw_import		   lw_error  lw_addr_resolve		(lw_addr);
	lw_import			lw_bool  lw_addr_ipv6			(lw_addr);
	lw_import			lw_bool  lw_addr_equal			(lw_addr, lw_addr);
	lw_import	   const char *  lw_addr_tostring		(lw_addr);
	lw_import   struct in6_addr  lw_addr_toin6_addr		(lw_addr);
	lw_import			 void *  lw_addr_tag			(lw_addr);
	lw_import			   void  lw_addr_set_tag		(lw_addr, void *);

	#define lw_addr_type_tcp		1
	#define lw_addr_type_udp		2
	#define lw_addr_hint_ipv6		4

/* Filter */

	lw_import	lw_filter  lw_filter_new			 ();
	lw_import		 void  lw_filter_delete			 (lw_filter);
	lw_import	lw_filter  lw_filter_clone			 (lw_filter);
	lw_import	  lw_addr  lw_filter_remote			 (lw_filter);
	lw_import		 void  lw_filter_set_remote		 (lw_filter, lw_addr);
	lw_import	  lw_addr  lw_filter_local			 (lw_filter);
	lw_import		 void  lw_filter_set_local		 (lw_filter, lw_addr);
	lw_import		 long  lw_filter_local_port		 (lw_filter);
	lw_import		 void  lw_filter_set_local_port	 (lw_filter, long port);
	lw_import		 long  lw_filter_remote_port	 (lw_filter);
	lw_import		 void  lw_filter_set_remote_port (lw_filter, long port);
	lw_import	  lw_bool  lw_filter_reuse			 (lw_filter);
	lw_import		 void  lw_filter_set_reuse		 (lw_filter, lw_bool);
	lw_import	  lw_bool  lw_filter_ipv6			 (lw_filter);
	lw_import		 void  lw_filter_set_ipv6		 (lw_filter, lw_bool);
	lw_import	   void *  lw_filter_tag			 (lw_filter);
	lw_import		 void  lw_filter_set_tag		 (lw_filter, void *);

/* Pump */

	lw_import		void  lw_pump_delete		(lw_pump);
	lw_import		void  lw_pump_add_user		(lw_pump);
	lw_import		void  lw_pump_remove_user	(lw_pump);
	lw_import	 lw_bool  lw_pump_in_use		(lw_pump);
	lw_import		void  lw_pump_remove		(lw_pump, lw_pump_watch);
	lw_import		void  lw_pump_post_remove	(lw_pump, lw_pump_watch);
	lw_import		void  lw_pump_post			(lw_pump, void * fn, void * param);
	lw_import	  void *  lw_pump_tag			(lw_pump);
	lw_import		void  lw_pump_set_tag		(lw_pump, void *);

	#ifdef _WIN32

	typedef void (lw_callback * lw_pump_callback)
		(void * tag, OVERLAPPED *, unsigned long bytes, int error);

	lw_import lw_pump_watch lw_pump_add (lw_pump, HANDLE, void * tag,
										 lw_pump_callback);

	lw_import void lw_pump_update_callbacks (lw_pump, lw_pump_watch,
											 void * tag, lw_pump_callback);
	#else

	typedef void (lw_callback * lw_pump_callback) (void * tag);

	lw_import lw_pump_watch lw_pump_add (lw_pump, int fd, void * tag,
										 lw_pump_callback on_read_ready,
										 lw_pump_callback on_write_ready,
										 lw_bool edge_triggered);

	lw_import void lw_pump_update_callbacks (lw_pump, lw_pump_watch, void * tag,
											 lw_pump_callback on_read_ready,
											 lw_pump_callback on_write_ready,
											 lw_bool edge_triggered);
	#endif

	/* For pump implementors */

	typedef struct lw_pumpdef
	{
		#ifdef _WIN32

		lw_pump_watch (* add)	  (lw_pump, HANDLE, void * tag, lw_pump_callback);
		void (* update_callbacks) (lw_pump, lw_pump_watch,
								   void * tag, lw_pump_callback);
		#else // !_WIN32

		lw_pump_watch (* add) (lw_pump, int FD, void * tag,
							   lw_pump_callback on_read_ready,
							   lw_pump_callback on_write_ready,
							   lw_bool edge_triggered);
		void (* update_callbacks) (lw_pump, lw_pump_watch, void * tag,
								   lw_pump_callback on_read_ready,
								   lw_pump_callback on_write_ready,
								   lw_bool edge_triggered);

		#endif // _WIN32

		void (* remove)	 (lw_pump, lw_pump_watch);
		void (* post)	 (lw_pump, void * fn, void * param);
		void (* cleanup) (lw_pump);

		size_t tail_size;

	} lw_pumpdef;

	lw_import 			 lw_pump  lw_pump_new (const lw_pumpdef *);
	lw_import const lw_pumpdef *  lw_pump_get_def (lw_pump);

	lw_import 		void * lw_pump_tail  (lw_pump);
	lw_import lw_pump lw_pump_from_tail  (void *);

	/* EventPump */

	lw_import	lw_eventpump  lw_eventpump_new					();
	lw_import		lw_error  lw_eventpump_tick					(lw_eventpump);
	lw_import		lw_error  lw_eventpump_start_eventloop		(lw_eventpump);
	lw_import		lw_error  lw_eventpump_start_sleepy_ticking	(lw_eventpump, void (lw_callback * on_tick_needed) (lw_eventpump));
	lw_import			void  lw_eventpump_post_eventloop_exit	(lw_eventpump);

	/* Stream */

	lw_import		void  lw_stream_delete					(lw_stream);
	lw_import	  size_t  lw_stream_bytes_left				(lw_stream);
	lw_import		void  lw_stream_read					(lw_stream, size_t bytes);
	lw_import		void  lw_stream_begin_queue				(lw_stream);
	lw_import	  size_t  lw_stream_queued					(lw_stream);
	lw_import		void  lw_stream_end_queue				(lw_stream);
	lw_import		void  lw_stream_end_queue_hb			(lw_stream, int num_head_buffers, const char ** buffers, size_t * lengths);
	lw_import		void  lw_stream_write					(lw_stream, const char * buffer, size_t length);
	lw_import		void  lw_stream_write_text				(lw_stream, const char * buffer);
	lw_import		void  lw_stream_writef					(lw_stream, const char * format, ...);
	lw_import		void  lw_stream_writev					(lw_stream, const char * format, va_list);
	lw_import		void  lw_stream_write_stream			(lw_stream, lw_stream src, size_t size, lw_bool delete_when_finished);
	lw_import		void  lw_stream_write_file				(lw_stream, const char * filename);
	lw_import		void  lw_stream_retry					(lw_stream, int when);
	lw_import		void  lw_stream_add_filter_upstream		(lw_stream, lw_stream filter, lw_bool delete_with_stream, lw_bool close_together);
	lw_import		void  lw_stream_add_filter_downstream	(lw_stream, lw_stream filter, lw_bool delete_with_stream, lw_bool close_together);
	lw_import	 lw_bool  lw_stream_close					(lw_stream, lw_bool immediate);
	lw_import	  void *  lw_stream_tag						(lw_stream);
	lw_import		void  lw_stream_set_tag					(lw_stream, void *);
	lw_import	 lw_pump  lw_stream_pump					(lw_stream);

	#define lw_stream_retry_now  1
	#define lw_stream_retry_never  2
	#define lw_stream_retry_more_data  3

	typedef void (lw_callback * lw_stream_hook_data)
		(lw_stream, void * tag, const char * buffer, size_t length);

	lw_import void lw_stream_add_hook_data (lw_stream, lw_stream_hook_data, void * tag);
	lw_import void lw_stream_remove_hook_data (lw_stream, lw_stream_hook_data, void * tag);

	typedef void (lw_callback * lw_stream_hook_close) (lw_stream, void * tag);

	lw_import void lw_stream_add_hook_close (lw_stream, lw_stream_hook_close, void * tag);
	lw_import void lw_stream_remove_hook_close (lw_stream, lw_stream_hook_close, void * tag);

	/* For stream implementors */

	typedef struct lw_streamdef
	{
		  size_t  (* sink_data)		 (lw_stream, const char * buffer, size_t size);
		  lw_i64  (* sink_stream)	 (lw_stream, lw_stream source, size_t size);
			void  (* retry)			 (lw_stream, int when);
		 lw_bool  (* is_transparent) (lw_stream);
		 lw_bool  (* close) 		 (lw_stream, lw_bool immediate);
		  size_t  (* bytes_left)	 (lw_stream);
			void  (* read)			 (lw_stream, size_t bytes);
			void  (* cleanup)		 (lw_stream);
		  size_t  tail_size;
	} lw_streamdef;

	lw_import			lw_stream	 lw_stream_new		 (const lw_streamdef *, lw_pump);
	lw_import  const lw_streamdef *  lw_stream_get_def	 (lw_stream);
	lw_import				 void *  lw_stream_tail		 (lw_stream);
	lw_import			  lw_stream  lw_stream_from_tail (void *);
	lw_import				   void  lw_stream_data		 (lw_stream, const char * buffer, size_t size);

	/* FDStream */

	lw_import  lw_fdstream  lw_fdstream_new		(lw_pump);
	lw_import		  void  lw_fdstream_set_fd	(lw_fdstream, lw_fd fd, lw_pump_watch watch, lw_bool auto_close, lw_bool is_socket);
	lw_import		  void  lw_fdstream_cork	(lw_fdstream);
	lw_import		  void  lw_fdstream_uncork	(lw_fdstream);
	lw_import		  void  lw_fdstream_nagle	(lw_fdstream, lw_bool nagle);
	lw_import	   lw_bool  lw_fdstream_valid	(lw_fdstream);
	lw_import		  long  lw_fdstream_get_fd_debug (lw_fdstream);

	/* File */

	lw_import lw_file lw_file_new (lw_pump);
	lw_import lw_file lw_file_new_open
		(lw_pump, const char * filename, const char * mode);

	lw_import lw_bool lw_file_open
		(lw_file, const char * filename, const char * mode);

	lw_import		 lw_bool  lw_file_open_temp		(lw_file);
	lw_import   const char *  lw_file_name			(lw_file);

	/* Pipe */

	lw_import	   lw_stream  lw_pipe_new			(lw_pump);

	/* Timer */

	lw_import		lw_timer  lw_timer_new			(lw_pump, const char * timer_name);
	lw_import			void  lw_timer_delete		(lw_timer);
	lw_import			void  lw_timer_start		(lw_timer, long milliseconds);
	lw_import		 lw_bool  lw_timer_started		(lw_timer);
	lw_import			void  lw_timer_stop			(lw_timer);
	lw_import			void  lw_timer_force_tick	(lw_timer);
	lw_import		  void *  lw_timer_tag			(lw_timer);
	lw_import			void  lw_timer_set_tag		(lw_timer, void *);

	typedef void (lw_callback * lw_timer_hook_tick) (lw_timer);
	lw_import void lw_timer_on_tick (lw_timer, lw_timer_hook_tick);

	/* Sync */

	lw_import	 lw_sync  lw_sync_new		 ();
	lw_import		void  lw_sync_delete	 (lw_sync);
	lw_import		void  lw_sync_lock		 (lw_sync);
	lw_import		void  lw_sync_release	 (lw_sync);

	/* Event */

	lw_import	lw_event  lw_event_new		 ();
	lw_import		void  lw_event_delete	 (lw_event);
	lw_import		void  lw_event_signal	 (lw_event);
	lw_import		void  lw_event_unsignal	 (lw_event);
	lw_import	 lw_bool  lw_event_signalled (lw_event);
	lw_import	 lw_bool  lw_event_wait		 (lw_event, long milliseconds);
	lw_import	  void *  lw_event_tag		 (lw_event);
	lw_import		void  lw_event_set_tag	 (lw_event, void *);

	/* Error */

	lw_import		lw_error  lw_error_new		();
	lw_import			void  lw_error_delete	(lw_error);
	lw_import			void  lw_error_add		(lw_error, int);
	lw_import			void  lw_error_addf		(lw_error, const char * format, ...);
	lw_import			void  lw_error_addv		(lw_error, const char * format, va_list);
	lw_import		  size_t  lw_error_size		(lw_error);
	lw_import	const char *  lw_error_tostring	(lw_error);
	lw_import		lw_error  lw_error_clone	(lw_error);
	lw_import		  void *  lw_error_tag		(lw_error);
	lw_import			void  lw_error_set_tag	(lw_error, void *);

	/* Client
	 *
	 * Note: lw_client derives from lw_stream, so all of the stream functions are
	 * applicable. To delete a lw_client, use lw_stream_delete.
	 */

	lw_import	   lw_client  lw_client_new					(lw_pump);
	lw_import			void  lw_client_connect				(lw_client, const char * host, lw_ui16 port);
	lw_import			void  lw_client_connect_addr		(lw_client, lw_addr);
	lw_import			void  lw_client_connect_secure		(lw_client, const char * host, lw_ui16 port);
	lw_import			void  lw_client_connect_addr_secure	(lw_client, lw_addr);
	lw_import			void  lw_client_set_local_port		(lw_client, lw_ui16 localport);
	lw_import			void  lw_client_disconnect			(lw_client);
	lw_import		 lw_bool  lw_client_connected			(lw_client);
	lw_import		 lw_bool  lw_client_connecting			(lw_client);
	lw_import		 lw_addr  lw_client_server_addr			(lw_client);

	typedef void (lw_callback * lw_client_hook_connect) (lw_client);
	lw_import void lw_client_on_connect (lw_client, lw_client_hook_connect);

	typedef void (lw_callback * lw_client_hook_disconnect) (lw_client);
	lw_import void lw_client_on_disconnect (lw_client, lw_client_hook_disconnect);

	typedef void (lw_callback * lw_client_hook_data) (lw_client, const char * buffer, size_t size);
	lw_import void lw_client_on_data (lw_client, lw_client_hook_data);

	typedef void (lw_callback * lw_client_hook_error) (lw_client, lw_error);
	lw_import void lw_client_on_error (lw_client, lw_client_hook_error);

	/* Server */

	lw_import		   lw_server  lw_server_new				(lw_pump);
	lw_import				void  lw_server_delete			(lw_server);
	lw_import				void  lw_server_host			(lw_server, long port);
	lw_import				void  lw_server_host_filter		(lw_server, lw_filter);
	lw_import				void  lw_server_unhost			(lw_server);
	lw_import			 lw_bool  lw_server_hosting			(lw_server);
	lw_import				 int  lw_server_port			(lw_server);
	lw_import			 lw_bool  lw_server_load_cert_file	(lw_server, const char * filename_certchain, const char * filename_privkey, const char * passphrase);
	lw_import			 lw_bool  lw_server_load_sys_cert	(lw_server, const char * common_name, const char * location, const char * store_name);
	lw_import			 lw_bool  lw_server_cert_loaded		(lw_server);
	lw_import			  time_t  lw_server_cert_expiry_time(lw_server);
	lw_import			 lw_bool  lw_server_can_npn			(lw_server);
	lw_import				void  lw_server_add_npn			(lw_server, const char * protocol);
	lw_import		const char *  lw_server_client_npn		(lw_server_client);
	lw_import			 lw_bool  lw_server_client_is_websocket (lw_server_client);
	lw_import			 lw_addr  lw_server_client_addr		(lw_server_client);
	lw_import			  size_t  lw_server_num_clients		(lw_server);
	lw_import	lw_server_client  lw_server_client_first	(lw_server);
	lw_import	lw_server_client  lw_server_client_next		(lw_server_client);
	lw_import			  void *  lw_server_tag				(lw_server);
	lw_import				void  lw_server_set_tag			(lw_server, void *);
	lw_import			 lw_ui16  lw_server_hole_punch		(lw_server, const char * remote_ip_and_port, lw_ui16 local_port);

	typedef void (lw_callback * lw_server_hook_connect) (lw_server, lw_server_client);
	lw_import void lw_server_on_connect (lw_server, lw_server_hook_connect);

	typedef void (lw_callback * lw_server_hook_disconnect) (lw_server, lw_server_client);
	lw_import void lw_server_on_disconnect (lw_server, lw_server_hook_disconnect);

	typedef void (lw_callback * lw_server_hook_data) (lw_server, lw_server_client, const char * buffer, size_t size);
	lw_import void lw_server_on_data (lw_server, lw_server_hook_data);

	typedef void (lw_callback * lw_server_hook_error) (lw_server, lw_error);
	lw_import void lw_server_on_error (lw_server, lw_server_hook_error);

	/* UDP */

	lw_import	  lw_udp  lw_udp_new		 (lw_pump);
	lw_import		void  lw_udp_delete		 (lw_udp);
	lw_import		void  lw_udp_host		 (lw_udp, lw_ui16 port);
	lw_import		void  lw_udp_host_filter (lw_udp, lw_filter);
	lw_import		void  lw_udp_host_addr	 (lw_udp, lw_addr, lw_ui16 local_port);
	lw_import	 lw_bool  lw_udp_hosting	 (lw_udp);
	lw_import		void  lw_udp_unhost		 (lw_udp);
	lw_import	 lw_ui16  lw_udp_port		 (lw_udp);
	lw_import		void  lw_udp_send		 (lw_udp, lw_addr, const char * buffer, size_t size);
	lw_import	  void *  lw_udp_tag		 (lw_udp);
	lw_import		void  lw_udp_set_tag	 (lw_udp, void *);

	typedef void (lw_callback * lw_udp_hook_data)(lw_udp, lw_addr, const char * buffer, size_t size);
	lw_import void lw_udp_on_data (lw_udp, lw_udp_hook_data);

	typedef void (lw_callback * lw_udp_hook_error) (lw_udp, lw_error);
	lw_import void lw_udp_on_error (lw_udp, lw_udp_hook_error);

	/* FlashPolicy */

	lw_import  lw_flashpolicy  lw_flashpolicy_new			(lw_pump);
	lw_import			 void  lw_flashpolicy_delete		(lw_flashpolicy);
	lw_import			 void  lw_flashpolicy_host			(lw_flashpolicy, const char * filename);
	lw_import			 void  lw_flashpolicy_host_filter	(lw_flashpolicy, const char * filename, lw_filter);
	lw_import			 void  lw_flashpolicy_unhost		(lw_flashpolicy);
	lw_import		  lw_bool  lw_flashpolicy_hosting		(lw_flashpolicy);
	lw_import		   void *  lw_flashpolicy_tag			(lw_flashpolicy);
	lw_import			 void  lw_flashpolicy_set_tag		(lw_flashpolicy, void *);

	typedef void (lw_callback * lw_flashpolicy_hook_error) (lw_flashpolicy, lw_error);
	lw_import void lw_flashpolicy_on_error (lw_flashpolicy, lw_flashpolicy_hook_error);

	/* Webserver */

	lw_import			   lw_ws  lw_ws_new						(lw_pump);
	lw_import				void  lw_ws_delete					(lw_ws);
	lw_import				void  lw_ws_host					(lw_ws, long port);
	lw_import				void  lw_ws_host_secure				(lw_ws, long port);
	lw_import				void  lw_ws_host_filter				(lw_ws, lw_filter);
	lw_import				void  lw_ws_host_secure_filter		(lw_ws, lw_filter);
	lw_import				void  lw_ws_unhost					(lw_ws);
	lw_import				void  lw_ws_unhost_secure			(lw_ws);
	lw_import			 lw_bool  lw_ws_hosting					(lw_ws);
	lw_import			 lw_bool  lw_ws_hosting_secure			(lw_ws);
	lw_import				 int  lw_ws_port					(lw_ws);
	lw_import				 int  lw_ws_port_secure				(lw_ws);
	lw_import			 lw_bool  lw_ws_load_cert_file			(lw_ws, const char * filename_certchain, const char * filename_privkey, const char * passphrase);
	lw_import			 lw_bool  lw_ws_load_sys_cert			(lw_ws, const char * common_name, const char * location, const char * store_name);
	lw_import			 lw_bool  lw_ws_cert_loaded				(lw_ws);
	lw_import			  time_t  lw_ws_cert_expiry_time		(lw_ws);
	lw_import				void  lw_ws_session_close			(lw_ws, const char * id);
	lw_import				void  lw_ws_enable_manual_finish	(lw_ws);
	lw_import				long  lw_ws_idle_timeout			(lw_ws);
	lw_import				void  lw_ws_set_idle_timeout		(lw_ws, long seconds);
	lw_import			  void *  lw_ws_tag						(lw_ws);
	lw_import				void  lw_ws_set_tag					(lw_ws, void * tag);
	lw_import			 lw_addr  lw_ws_req_addr				(lw_ws_req);
	lw_import			 lw_bool  lw_ws_req_secure				(lw_ws_req);
	lw_import			 lw_bool  lw_ws_req_websocket			(lw_ws_req);
	lw_import		const char *  lw_ws_req_url					(lw_ws_req);
	lw_import		const char *  lw_ws_req_hostname			(lw_ws_req);
	lw_import				void  lw_ws_req_disconnect			(lw_ws_req, unsigned int websocket_reason_code);
	lw_import				void  lw_ws_req_set_redirect		(lw_ws_req, const char * url);
	lw_import				void  lw_ws_req_status				(lw_ws_req, long code, const char * message);
	lw_import				void  lw_ws_req_set_mimetype		(lw_ws_req, const char * mimetype);
	lw_import				void  lw_ws_req_set_mimetype_ex		(lw_ws_req, const char * mimetype, const char * charset);
	lw_import				void  lw_ws_req_guess_mimetype		(lw_ws_req, const char * filename);
	lw_import				void  lw_ws_req_finish				(lw_ws_req);
	lw_import			  lw_i64  lw_ws_req_last_modified		(lw_ws_req);
	lw_import				void  lw_ws_req_set_last_modified	(lw_ws_req, lw_i64);
	lw_import				void  lw_ws_req_set_unmodified		(lw_ws_req);
	lw_import				void  lw_ws_req_set_header			(lw_ws_req, const char * name, const char * value);
	lw_import				void  lw_ws_req_add_header			(lw_ws_req, const char * name, const char * value);
	lw_import		const char *  lw_ws_req_header				(lw_ws_req, const char * name);
	lw_import	   lw_ws_req_hdr  lw_ws_req_hdr_first			(lw_ws_req);
	lw_import		const char *  lw_ws_req_hdr_name			(lw_ws_req_hdr);
	lw_import		const char *  lw_ws_req_hdr_value			(lw_ws_req_hdr);
	lw_import	   lw_ws_req_hdr  lw_ws_req_hdr_next			(lw_ws_req_hdr);
	lw_import	 lw_ws_req_param  lw_ws_req_GET_first			(lw_ws_req);
	lw_import	 lw_ws_req_param  lw_ws_req_POST_first			(lw_ws_req);
	lw_import		const char *  lw_ws_req_param_name			(lw_ws_req_param);
	lw_import		const char *  lw_ws_req_param_value			(lw_ws_req_param);
	lw_import	 lw_ws_req_param  lw_ws_req_param_next			(lw_ws_req_param);
	lw_import	lw_ws_req_cookie  lw_ws_req_cookie_first		(lw_ws_req);
	lw_import		const char *  lw_ws_req_cookie_name			(lw_ws_req_cookie);
	lw_import		const char *  lw_ws_req_cookie_value		(lw_ws_req_cookie);
	lw_import	lw_ws_req_cookie  lw_ws_req_cookie_next			(lw_ws_req_cookie);
	lw_import				void  lw_ws_req_set_cookie			(lw_ws_req, const char * name, const char * value);
	lw_import				void  lw_ws_req_set_cookie_attr		(lw_ws_req, const char * name, const char * value, const char * attributes);
	lw_import		const char *  lw_ws_req_get_cookie			(lw_ws_req, const char * name);
	lw_import		const char *  lw_ws_req_session_id			(lw_ws_req);
	lw_import				void  lw_ws_req_session_write		(lw_ws_req, const char * name, const char * value);
	lw_import		const char *  lw_ws_req_session_read		(lw_ws_req, const char * name);
	lw_import				void  lw_ws_req_session_close		(lw_ws_req);
	lw_import  lw_ws_sessionitem  lw_ws_req_session_first		(lw_ws_req);
	lw_import		const char *  lw_ws_sessionitem_name		(lw_ws_sessionitem);
	lw_import		const char *  lw_ws_sessionitem_value		(lw_ws_sessionitem);
	lw_import  lw_ws_sessionitem  lw_ws_sessionitem_next		(lw_ws_sessionitem);
	lw_import		const char *  lw_ws_req_GET					(lw_ws_req, const char * name);
	lw_import		const char *  lw_ws_req_POST				(lw_ws_req, const char * name);
	lw_import		const char *  lw_ws_req_body				(lw_ws_req);
	lw_import				void  lw_ws_req_disable_cache		(lw_ws_req);
	lw_import				long  lw_ws_req_idle_timeout		(lw_ws_req);
	lw_import				void  lw_ws_req_set_idle_timeout	(lw_ws_req, long seconds);
/*  lw_import				void  lw_ws_req_enable_dl_resuming	(lw_ws_req);
	lw_import			  lw_i64  lw_ws_req_reqrange_begin		(lw_ws_req);
	lw_import			  lw_i64  lw_ws_req_reqrange_end		(lw_ws_req);
	lw_import				void  lw_ws_req_set_outgoing_range	(lw_ws_req, lw_i64 begin, lw_i64 end);*/
	lw_import		const char *  lw_ws_upload_form_el_name		(lw_ws_upload);
	lw_import		const char *  lw_ws_upload_filename			(lw_ws_upload);
	lw_import		const char *  lw_ws_upload_header			(lw_ws_upload, const char * name);
	lw_import				void  lw_ws_upload_set_autosave		(lw_ws_upload);
	lw_import		const char *  lw_ws_upload_autosave_fname	(lw_ws_upload);
	lw_import	lw_ws_upload_hdr  lw_ws_upload_hdr_first		(lw_ws_upload);
	lw_import		const char *  lw_ws_upload_hdr_name			(lw_ws_upload_hdr);
	lw_import		const char *  lw_ws_upload_hdr_value		(lw_ws_upload_hdr);
	lw_import	lw_ws_upload_hdr  lw_ws_upload_hdr_next			(lw_ws_upload_hdr);

	typedef void (lw_callback * lw_ws_hook_get) (lw_ws, lw_ws_req);
	lw_import void lw_ws_on_get (lw_ws, lw_ws_hook_get);

	typedef void (lw_callback * lw_ws_hook_post) (lw_ws, lw_ws_req);
	lw_import void lw_ws_on_post (lw_ws, lw_ws_hook_post);

	typedef void (lw_callback * lw_ws_hook_head) (lw_ws, lw_ws_req);
	lw_import void lw_ws_on_head (lw_ws, lw_ws_hook_head);

	typedef void (lw_callback * lw_ws_hook_error) (lw_ws, lw_error);
	lw_import void lw_ws_on_error (lw_ws, lw_ws_hook_error);

	typedef void (lw_callback * lw_ws_hook_disconnect) (lw_ws, lw_ws_req);
	lw_import void lw_ws_on_disconnect (lw_ws, lw_ws_hook_disconnect);

	typedef void (lw_callback * lw_ws_hook_upload_start) (lw_ws, lw_ws_req, lw_ws_upload);
	lw_import void lw_ws_on_upload_start (lw_ws, lw_ws_hook_upload_start);

	typedef void (lw_callback * lw_ws_hook_upload_chunk) (lw_ws, lw_ws_req, lw_ws_upload, const char * buffer, size_t size);
	lw_import void lw_ws_on_upload_chunk (lw_ws, lw_ws_hook_upload_chunk);

	typedef void (lw_callback * lw_ws_hook_upload_done) (lw_ws, lw_ws_req, lw_ws_upload);
	lw_import void lw_ws_on_upload_done (lw_ws, lw_ws_hook_upload_done);

	typedef void (lw_callback * lw_ws_hook_upload_post) (lw_ws, lw_ws_req, lw_ws_upload uploads [], size_t num_uploads);
	lw_import void lw_ws_on_upload_post (lw_ws, lw_ws_hook_upload_post);

	typedef void (lw_callback * lw_ws_hook_websocket_message) (lw_ws, lw_ws_req, const char * buffer, size_t size);
	lw_import void lw_ws_on_websocket_message (lw_ws, lw_ws_hook_websocket_message);

	void * lw_malloc_or_exit (const size_t size);
	void * lw_calloc_or_exit (const size_t count, const size_t size);
	void * lw_realloc_or_exit (void * origptr, size_t newsize);

#ifdef __cplusplus
} /* extern "C" */

#ifdef _lacewing_wrap_cxx
	#define lw_class_wraps(c) struct _lw_##c _inst
#else
	#define lw_class_wraps(c)
#endif
#ifdef _MSC_VER
	#define lw_sprintf_s sprintf_s
#else
	#define lw_sprintf_s sprintf
#endif // _MSC_VER

// Every Unicode library decomposes into 4-byte chars, probably for the x86 nativeness, and
// for one code unit per code point.
// For platform compatibility, we use a third-party library; this one is actually used in Julia.
#include "deps/utf8proc.h"

// Converts a IPv4-mapped-IPv6 address to IPv4, stripping ports.
// If the address is IPv4 or unmapped IPv6, copies it without the port.
void lw_addr_prettystring(const char * input, char * const output, size_t outputSize);

// Compares if two strings match, returns true if so. Does a size check, then does flat buffer comparison;
// make sure if you're passing UTF-8, both args are valid, normalized UTF-8 strings.
// Does not validate or check strings' content.
bool lw_sv_cmp(const std::string_view first, const std::string_view second);

// Compares if two strings match, returns true if so. Case insensitive. Does a size check.
// Expects both are valid UTF-8 strings, non-destructively simplified.
// See lw_sv_cmp().
bool lw_u8str_icmp(const std::string_view first, const std::string_view second);

// Validates a UTF-8 std::string as having valid UTF-8 codepoints.
// Does not ensure strings are normalized. Empty strings return true.
bool lw_u8str_validate(const std::string_view toValidate);

// Normalizes the passed std::string to its least-bytes equivalent (using NFC), and returns true.
// Empty = true. Handles invalid UTF-8 strings by returning false.
bool lw_u8str_normalize(std::string & input);

/** Returns a NFC/NKFC, case-folded, stripped-down version of the passed string.
	Used for easier searching, and to prevent similar names as an exploit.
	Handles invalid UTF-8 string by returning blank.
	@param destructive If true, converts to lowercase, and lumps some things together with UTF8PROC lumping.
					   Use false to check if two strings (after the simplifying) differ by case alone.
	@param extralumping Replaces lookalike characters (e.g. 0 and O to o). */
std::string lw_u8str_simplify(const std::string_view first, bool destructive = true, bool extralumping = true);

/** Removes whitespace, control, and strange code points from both beginning and end of string,
	and returns the result. Stricter on the beginning. Ignores the middle of the string.
	Handles invalid UTF-8 strings by returning blank.
	@param abortIfTrimNeeded If abort is specified, at the first code point needed to be trimmed,
							 the function aborts and returns an empty string_view instead.
	@remarks The front of the string must be letters, numbers, punctuation in Unicode category.
			 The end of the string is like the start, but also allows marks and symbols.
			 Both control and whitespace category will always be removed. */
std::string_view lw_u8str_trim(std::string_view toTrim, bool abortOnTrimNeeded = false);

#if defined(_WIN32)
// For Unicode support on Windows.
// Returns null or a wide-converted version of the U8 string passed. Free it with free(). Pass size -1 for null-terminated strings.
extern "C" lw_import wchar_t * lw_char_to_wchar(const char * u8str, int size);
#endif

namespace lacewing
{

/** error **/

typedef struct _error * error;

struct _error
{
	lw_class_wraps (error);

	lw_import void add (const char * format, ...);
	lw_import void add (int);
	lw_import void add (const char * format, va_list);

	lw_import size_t size ();

	lw_import const char * tostring ();
	lw_import operator const char * ();

	lw_import error clone ();

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import error error_new ();
void error_delete (error);

/** event **/

typedef struct _event * event;

struct _event
{
	lw_class_wraps (event);

	lw_import void signal ();
	lw_import void unsignal ();

	lw_import bool signalled ();

	lw_import bool wait (long timeout = -1);

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import event event_new ();
void event_delete (event);


/** pump **/

typedef struct _pump * pump;

struct _pump
{
	lw_class_wraps (pump);

	lw_import void add_user ();
	lw_import void remove_user ();

	lw_import bool in_use ();

	#ifdef _WIN32

		lw_import lw_pump_watch add (HANDLE, void * tag, lw_pump_callback);
		lw_import void update_callbacks (lw_pump_watch, void * tag, lw_pump_callback);

	#else

		lw_import lw_pump_watch add (int fd, void * tag,
									 lw_pump_callback on_read_ready,
									 lw_pump_callback on_write_ready = 0,
									 bool edge_triggered = true);

		lw_import void update_callbacks (lw_pump_watch, void * tag,
										 lw_pump_callback on_read_ready,
										 lw_pump_callback on_write_ready = 0,
										 bool edge_triggered = true);

	#endif

	void remove (lw_pump_watch);
	void post_remove (lw_pump_watch);

	void post (void * proc, void * parameter = 0);

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import pump pump_new ();
lw_import void pump_delete (pump);


/** eventpump **/

typedef struct _eventpump * eventpump;

struct _eventpump : public _pump
{
	lw_class_wraps (eventpump);

	lw_import error start_eventloop ();
	lw_import error tick ();

	lw_import error start_sleepy_ticking
		(void (lw_callback * on_tick_needed) (eventpump));

	lw_import void post_eventloop_exit ();
};

lw_import eventpump eventpump_new ();


/** thread **/

typedef struct _thread * thread;

struct _thread
{
	lw_class_wraps (_thread);

	lw_import void start (void * param = 0);
	lw_import bool started ();

	lw_import void * join ();

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import thread thread_new (const char * name, void * proc);
lw_import void thread_delete (thread);


/** timer **/

typedef struct _timer * timer;

struct _timer
{
	lw_class_wraps (timer);

	lw_import void start	(long msec);
	lw_import void stop	 ();
	lw_import bool started  ();

	lw_import void force_tick ();

	typedef void (lw_callback * hook_tick) (timer);
	lw_import void on_tick (hook_tick);

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import timer timer_new (pump, const char *);
lw_import void timer_delete (timer);


/** sync **/

typedef struct _sync * sync;
 typedef struct _sync_lock * sync_lock;

struct _sync
{
	lw_class_wraps (sync);
};

struct _sync_lock
{
	lw_import _sync_lock (lacewing::sync);
	lw_import ~ _sync_lock ();

protected:

	lacewing::sync sync;
};

lw_import sync sync_new ();
lw_import void sync_delete (sync);


/** stream **/

typedef struct _stream * stream;

struct _stream
{
	lw_class_wraps (stream);

	typedef void (lw_callback * hook_data)
		(stream, void * tag, const char * buffer, size_t size);

	 lw_import void add_hook_data (hook_data, void * tag = 0);
	 lw_import void remove_hook_data (hook_data, void * tag = 0);

	typedef void (lw_callback * hook_close) (stream, void * tag);

	 lw_import void add_hook_close (hook_close, void * tag);
	 lw_import void remove_hook_close (hook_close, void * tag);

	lw_import size_t bytes_left (); /* if -1, read() does nothing */
	lw_import void read (size_t bytes = -1); /* -1 = until EOF */

	lw_import void begin_queue ();
	lw_import size_t queued ();

	/* When end_queue is called, one or more head buffers may optionally be
	* written _before_ the queued data.  This is used for e.g. including HTTP
	* headers before the (already buffered) response body.
	*/

	lw_import void end_queue ();

	lw_import void end_queue
		(int head_buffers, const char ** buffers, size_t * lengths);

	lw_import void write (const char * buffer, size_t size = -1);

	lw_import void writef (const char * format, ...);

	lw_import void write
		(stream, size_t size = -1, bool delete_when_finished = false);

	lw_import void write_file (const char * filename);

	lw_import void add_filter_upstream
		(stream, bool delete_with_stream = false, bool close_together = false);

	lw_import void add_filter_downstream
		(stream, bool delete_with_stream = false, bool close_together = false);

	lw_import bool close (bool immediate = false);

	lw_import lacewing::pump pump ();

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import stream stream_new (const lw_streamdef *, pump);
lw_import void stream_delete (stream);


/** pipe **/

typedef struct _pipe * pipe;

struct _pipe : public _stream
{
	lw_class_wraps (pipe);
};

lw_import pipe pipe_new ();
lw_import pipe pipe_new (pump);


/** fdstream **/

typedef struct _fdstream * fdstream;

struct _fdstream : public _stream
{
	lw_class_wraps (fdstream);

	lw_import void set_fd
		(lw_fd, lw_pump_watch watch = 0, bool auto_close = false, bool is_socket = false);

	lw_import bool valid ();

	lw_import void cork ();
	lw_import void uncork ();

	lw_import void nagle (bool);

};

lw_import fdstream fdstream_new (pump);


/** file **/

typedef struct _file * file;

struct _file : public _fdstream
{
	lw_class_wraps (file);

	lw_import bool open (const char * filename, const char * mode = "rb");
	lw_import bool open_temp ();

	lw_import const char * name ();
};

lw_import file file_new (pump);
lw_import file file_new (pump, const char * filename, const char * mode = "rb");


/** address **/

typedef struct _address * address;

struct _address
{
	lw_class_wraps (address);

	lw_import lw_ui16 port ();
	lw_import void port (lw_ui16);

	lw_import int type ();
	lw_import void type (int);

	lw_import bool ipv6 ();

	lw_import bool ready ();
	lw_import error resolve ();

	lw_import in6_addr toin6_addr ();
	lw_import const char * tostring ();
	lw_import operator const char *  ();

	lw_import bool operator == (address);
	lw_import bool operator != (address);

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import address address_new (address);
lw_import address address_new (const char * hostname, const char * service);
lw_import address address_new (const char * hostname, lw_ui16 port);
lw_import address address_new (const char * hostname, const char * service, int hints);
lw_import address address_new (const char * hostname, lw_ui16 port, int hints);

lw_import void address_delete (address);


/** filter **/

typedef struct _filter * filter;

struct _filter
{
	lw_class_wraps (filter);

	lw_import void local (address);
	lw_import void remote (address);

	lw_import address local ();
	lw_import address remote ();

	lw_import void local_port (long port);
	lw_import long local_port ();

	lw_import void remote_port (long port);
	lw_import long remote_port ();

	lw_import void reuse (bool enabled);
	lw_import bool reuse ();

	lw_import void ipv6 (bool enabled);
	lw_import bool ipv6 ();

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import filter filter_new ();
lw_import void filter_delete (filter);


/** client **/

typedef struct _client * client;

struct _client : public _fdstream
{
	lw_class_wraps (client);

	lw_import void connect (const char * host, lw_ui16 remote_port);
	lw_import void connect (address);
	lw_import void setlocalport (lw_ui16 port);

	lw_import bool connected ();
	lw_import bool connecting ();

	lw_import address server_address ();

	typedef void (lw_callback * hook_connect) (client);
	typedef void (lw_callback * hook_disconnect) (client);

	typedef void (lw_callback * hook_data)
		(client, const char * buffer, size_t size);

	typedef void (lw_callback * hook_error) (client, error);

	lw_import void on_connect	 (hook_connect);
	lw_import void on_disconnect (hook_disconnect);
	lw_import void on_data		 (hook_data);
	lw_import void on_error		 (hook_error);
};

lw_import client client_new (pump);


/** server **/

typedef struct _server * server;
 typedef struct _server_client * server_client;

struct _server
{
	lw_class_wraps (server);

	lw_import void host	(long port);
	lw_import void host	(filter);

	lw_import void unhost  ();
	lw_import bool hosting ();
	lw_import long port	();

	lw_import bool load_cert_file
		(const char * filename_certchain, const char * filename_privkey, const char * passphrase = "");

	lw_import bool load_sys_cert
		(const char * common_name, const char * location = "CurrentUser",
			const char * store_name = "My");

	lw_import bool cert_loaded ();

	lw_import bool can_npn ();
	lw_import void add_npn (const char *);

	lw_import size_t num_clients ();
	lw_import server_client client_first ();

	lw_import lw_ui16 hole_punch (const char * ip_and_remote_port, lw_ui16 local_port);

	typedef void (lw_callback * hook_connect) (server, server_client);
	typedef void (lw_callback * hook_disconnect) (server, server_client);

	typedef void (lw_callback * hook_data)
	  (server, server_client, const char * buffer, size_t size);

	typedef void (lw_callback * hook_error) (server, error);

	lw_import void on_connect	 (hook_connect);
	lw_import void on_disconnect  (hook_disconnect);
	lw_import void on_data		(hook_data);
	lw_import void on_error		(hook_error);

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import server server_new (pump);
lw_import void server_delete (server);

struct _server_client : public _fdstream
{
	lw_class_wraps (server_client);

	lw_import lacewing::address address ();

	lw_import server_client next ();

	lw_import const char * npn ();

	lw_import lw_bool is_websocket ();
};


/** udp **/

typedef struct _udp * udp;

struct _udp
{
	lw_class_wraps (udp);

	lw_import void host (lw_ui16 port);
	lw_import void host (filter);
	lw_import void host (address, lw_ui16 local_port = 0);

	lw_import bool hosting ();
	lw_import void unhost ();

	lw_import lw_ui16 port ();

	lw_import void send (address, const char * data, size_t size = -1);

	typedef void (lw_callback * hook_data)
		(udp, address, char * buffer, size_t size);

	typedef void (lw_callback * hook_error) (udp, error);

	lw_import void on_data	(hook_data);
	lw_import void on_error  (hook_error);

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import udp udp_new (pump);
lw_import void udp_delete (udp);


/** webserver **/

typedef struct _webserver						* webserver;
typedef struct _webserver_request				* webserver_request;
typedef struct _webserver_request_header		* webserver_request_header;
typedef struct _webserver_request_cookie		* webserver_request_cookie;
typedef struct _webserver_request_param		 * webserver_request_param;
typedef struct _webserver_sessionitem			* webserver_sessionitem;
typedef struct _webserver_upload				* webserver_upload;
typedef struct _webserver_upload_header		 * webserver_upload_header;

struct _webserver
{
	lw_class_wraps (webserver);

	lw_import void host			(long port = 80);
	lw_import void host_secure	(long port = 443);

	lw_import void host (filter);
	lw_import void host_secure (filter);

	lw_import void unhost ();
	lw_import void unhost_secure ();

	lw_import bool hosting ();
	lw_import bool hosting_secure ();

	lw_import int port ();
	lw_import int port_secure ();

	lw_import bool load_cert_file
		(const char * filename_certchain, const char* filename_privkey, const char * passphrase = "");

	lw_import bool load_sys_cert
		(const char* common_name, const char* location = "CurrentUser",
			const char* store_name = "My");

	lw_import bool cert_loaded ();
	lw_import time_t cert_expiry_time ();

	lw_import void enable_manual_finish ();

	lw_import long idle_timeout ();
	lw_import void idle_timeout (long sec);

	lw_import void session_close (const char * id);

	typedef void (lw_callback * hook_get) (webserver, webserver_request);
	typedef void (lw_callback * hook_post) (webserver, webserver_request);
	typedef void (lw_callback * hook_head) (webserver, webserver_request);
	typedef void (lw_callback * hook_disconnect) (webserver, webserver_request);
	typedef void (lw_callback * hook_error) (webserver, error);

	typedef void (lw_callback * hook_upload_start)
		(webserver, webserver_request, webserver_upload);

	typedef void (lw_callback * hook_upload_chunk)
		(webserver, webserver_request, webserver_upload, const char * buffer, size_t size);

	typedef void (lw_callback * hook_upload_done)
		(webserver, webserver_request, webserver_upload);

	typedef void (lw_callback * hook_upload_post)
		(webserver, webserver_request, webserver_upload uploads[], size_t num_uploads);

	typedef void (lw_callback* hook_websocketmessage)
		(webserver, webserver_request, const char * buffer, size_t size);

	lw_import void on_get			(hook_get);
	lw_import void on_upload_start	(hook_upload_start);
	lw_import void on_upload_chunk	(hook_upload_chunk);
	lw_import void on_upload_done	(hook_upload_done);
	lw_import void on_upload_post	(hook_upload_post);
	lw_import void on_post			(hook_post);
	lw_import void on_head			(hook_head);
	lw_import void on_disconnect	(hook_disconnect);
	lw_import void on_error			(hook_error);
	lw_import void on_websocket_message (hook_websocketmessage);

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import webserver webserver_new (pump);
lw_import void webserver_delete (webserver);

struct _webserver_request : public _stream
{
	lw_class_wraps (ws_req);
	lw_import ~ _webserver_request ();

	lw_import lacewing::address address ();

	lw_import bool secure ();
	lw_import bool websocket ();

	lw_import const char * url ();
	lw_import const char * hostname ();

	lw_import void disconnect ();

	lw_import void redirect (const char * url);
	lw_import void status (long code, const char * message);

	lw_import void set_mimetype (const char * mimetype,
		 const char * charset = "UTF-8");

	lw_import void guess_mimetype (const char * filename);

	lw_import void finish ();

	lw_import long idle_timeout ();
	lw_import void idle_timeout (long sec);

	lw_import lw_i64 last_modified  ();
	lw_import void	last_modified  (lw_i64 time);
	lw_import void	set_unmodified ();

	lw_import void disable_cache ();

	lw_import void enable_download_resuming ();


	/** Headers **/

	lw_import webserver_request_header header_first ();

	lw_import const char * header (const char * name);
	lw_import void header (const char * name, const char * value);

	lw_import void add_header (const char * name, const char * value);


	/** Cookies **/

	lw_import webserver_request_cookie cookie_first ();

	lw_import const char * cookie (const char * name);

	lw_import void cookie
		(const char * name, const char * value);

	lw_import void cookie
		(const char * name, const char * value, const char * attr);


	/** Sessions **/

	lw_import webserver_sessionitem session_first ();

	lw_import const char * session ();
	lw_import const char * session (const char * key);
	lw_import void		 session (const char * key, const char * value);

	lw_import void  session_close ();


	/** GET/POST data **/

	lw_import webserver_request_param GET ();
	lw_import webserver_request_param POST ();

	lw_import const char * GET  (const char * name);
	lw_import const char * POST (const char * name);

	lw_import const char * body ();
};

struct _webserver_request_header
{
	lw_class_wraps (ws_req_hdr);

	lw_import const char * name ();
	lw_import const char * value ();

	lw_import webserver_request_header next ();
};

struct _webserver_request_cookie
{
	lw_class_wraps (ws_req_cookie);

	lw_import const char * name ();
	lw_import const char * value ();

	lw_import webserver_request_cookie next ();
};

struct _webserver_sessionitem
{
	lw_class_wraps (ws_sessionitem);

	lw_import const char * name ();
	lw_import const char * value ();

	lw_import webserver_sessionitem next ();
};

struct _webserver_request_param
{
	lw_class_wraps (ws_req_param);

	lw_import const char * name ();
	lw_import const char * value ();
	lw_import const char * content_type ();

	lw_import webserver_request_param next ();
};

struct _webserver_upload
{
	lw_class_wraps (ws_upload);

	lw_import const char * form_element_name ();
	lw_import const char * filename ();
	lw_import void		   set_autosave ();
	lw_import const char * autosave_filename ();

	lw_import const char * header (const char * name);

	lw_import webserver_upload_header header_first ();
};

struct _webserver_upload_header
{
	lw_class_wraps (ws_upload_hdr);

	lw_import const char * name ();
	lw_import const char * value ();

	lw_import webserver_upload_header next ();
};


/** flashpolicy **/

typedef struct _flashpolicy * flashpolicy;

struct _flashpolicy
{
	lw_class_wraps (flashpolicy);

	lw_import void host (const char * filename);
	lw_import void host (const char * filename, filter);

	lw_import void unhost ();

	lw_import bool hosting ();

	typedef void (lw_callback * hook_error) (flashpolicy, error);
	lw_import void on_error (hook_error);

	lw_import void tag (void *);
	lw_import void * tag ();
};

lw_import flashpolicy flashpolicy_new (pump);
lw_import void flashpolicy_delete (flashpolicy);

//#pragma region Phi stuff
// NOTE: if you edit this due to new liblacewing release, note:
// _flashpolicy::on_error requires flash policy definition to be extracted.
// Otherwise the flashpolicy can't be accessed for the C++ Flash Policy extension covered in flashpolicy2.cc.
// lw_flashpolicy_new needs the server tag to be set to ctx, or on_error fails.

// lacewing::_address has no lacewing::_address comparisons.
// So if you're checking two lacewing::address == lacewing::address, do * on the first one.
// This will expose the equality operator in the lacewing::_address class.

// lacewing::fd_stream::set_fd uses WSADuplicateSocket() to check if the handle passed is a socket.
// Normally, this is used to pass access to a socket to a second process, so this prepares the socket for copying.
// The duplicated information is not used for a new socket, though. It's just discarded.
//
// The missing second socket causes the client OS to not fully disconnect the socket, so on disconnect,
// • you will normally get four FIN/ACK exchanges. If you encounter this bug, you will get two FIN/ACK.
// • you get CLOSE_WAIT state until something OS-side does a timeout, or on process close.
//
// On process close, the socket is force-closed with a TCP RST (Reset) message.
//
// To avoid this, just modify the function (and C++ mirror) to pass is_socket parameter.
// That way the "is socket" hack isn't needed.

// You should also run a check on _lw_addr used on stack. This is normally followed by lw_addr_set_sockaddr.
// That's fine, but that function runs malloc on the _lw_addr, which leads to a memory leak when the stack
// variable is freed. Run lw_addr_cleanup() on the stack address at the _lw_addr scope exits to compensate for it.
//
// lw_client is_connecting is now set to false properly if getaddrinfo fails.

// Make sure you also add UDP keep-alive. Routers close connection otherwise.
// Make sure you also fix the timer issue in unix.


// Maximum size of a UDP datagram is 65535, minus IP(v4/v6) header, minus Lacewing Relay UDP header,
// with some extra margin, due to WebSocket psuedo.
// It's probably fragmented beyond 1400 bytes anyway, due to Ethernet MTU.
// UDP exceeding this size will be silently dropped.
const static lw_ui16 relay_max_udp_payload = UINT16_MAX - 40 - 20;

struct readlock;
struct writelock;
struct readwritelock
{
	friend readlock;
	friend writelock;
	readwritelock();
	~readwritelock() noexcept(false);


	// Debug breakpoint if writelock is not held by current thread.
	bool checkHoldsWrite(bool excIfNot = true) const;
	// Debug breakpoint if readlock is not held by current thread.
	bool checkHoldsRead(bool excIfNot = true) const;

#ifdef _DEBUG
#define lw_rwlock_debugParamNames const char * file, const char * func, int line
#define lw_rwlock_debugParamDefs __FILE__, __FUNCTION__, __LINE__

	[[nodiscard]]
	lacewing::readlock createReadLock(const char * file, const char * func, int line);
	[[nodiscard]]
	lacewing::writelock createWriteLock(const char * file, const char * func, int line);

protected:
	void openReadLock(readlock &rl, lw_rwlock_debugParamNames);
	void openWriteLock(writelock &wl, lw_rwlock_debugParamNames);
	void closeReadLock(readlock & rl, lw_rwlock_debugParamNames);
	void closeWriteLock(writelock & wl, lw_rwlock_debugParamNames);

	[[nodiscard]]
	lacewing::writelock upgradeReadLock(readlock &rl, lw_rwlock_debugParamNames);
	[[nodiscard]]
	lacewing::readlock downgradeWriteLock(writelock &wl, lw_rwlock_debugParamNames);
	void upgradeReadLock(readlock &rl, writelock &wl, lw_rwlock_debugParamNames);
	void downgradeWriteLock(writelock &wl, readlock &rl, lw_rwlock_debugParamNames);

#else
	lacewing::readlock createReadLock();
	lacewing::writelock createWriteLock();
#endif

protected:
	void openReadLock(readlock & rl);
	void openWriteLock(writelock & wl);
	void closeReadLock(readlock & rl);
	void closeWriteLock(writelock & wl);
	lacewing::writelock upgradeReadLock(readlock &rl);
	lacewing::readlock downgradeWriteLock(writelock &wl);
	void upgradeReadLock(readlock &rl, writelock &wl);
	void downgradeWriteLock(writelock &wl, readlock &rl);

private:

	// Would use std::shared_mutex, or std::shared_timed_mutex, but iOS doesn't support it until 10.0,
	// so we'll roll our own.
	std::shared_timed_mutex lock;
	//std::condition_variable read, write;
	::std::atomic<size_t> readers, writers, read_waiters, write_waiters;
	mutable ::std::atomic<bool> metaLock = false;

	struct holder {
#ifdef _DEBUG
		const char * file = "";
		const char * func = "";
		int line = -1;
#endif
		bool isWrite = false;
		std::thread::id threadID;
	};
	std::vector<holder> holders;
};
struct readlock {
	friend readwritelock;
	friend writelock;
	bool isEnabled() const;

#ifdef _DEBUG
	readlock(readwritelock &lock, lw_rwlock_debugParamNames);
	void unlockDebug(lw_rwlock_debugParamNames);
	void relockDebug(lw_rwlock_debugParamNames);
#ifdef LW_ESCALATION
	lacewing::writelock upgrade(lw_rwlock_debugParamNames);
	void upgrade(lw_rwlock_debugParamNames, lacewing::writelock &wl);
#endif
	// If writelock is previously opened on this thread, make sure it's not closed when this read lock is closed.
	bool supercededByWriter = false;
	const char * writerOpenFile = nullptr;
	int writerOpenLine = 0;

#else // !_DEBUG
	readlock(readwritelock &lock);
	void unlock();
	void relock();
#ifdef LW_ESCALATION
	lacewing::writelock upgrade();
	void upgrade(lacewing::writelock &wl);
#endif // LW_ESCALATION

#endif // !_DEBUG
	~readlock();
	readlock(readlock&) = delete;
protected:
	readwritelock & lock;
	std::shared_lock<decltype(readwritelock::lock)> locker;
	bool locked = true;
};

struct writelock {
	friend readwritelock;
	friend readlock;
	bool isEnabled() const;

#ifdef _DEBUG
	writelock(readwritelock &lock, const char * file, const char * func, int line);
	void unlockDebug(lw_rwlock_debugParamNames);
	void relockDebug(lw_rwlock_debugParamNames);
#if LW_ESCALATION
	lacewing::readlock downgrade(lw_rwlock_debugParamNames);
	void downgrade(lw_rwlock_debugParamNames, lacewing::readlock &wl);
#endif // LW_ESCALATION
#else // !_DEBUG
	writelock(readwritelock &lock);
	void unlock();
	void relock();
#if LW_ESCALATION
	lacewing::readlock downgrade();
	void downgrade(lacewing::readlock &rl);
#endif
#endif
	~writelock();
	writelock(writelock&) = delete;
protected:
	readwritelock & lock;
	std::unique_lock<decltype(readwritelock::lock)> locker;
	bool locked = true;
};

#ifdef _DEBUG
#define lw_unlock() unlockDebug(lw_rwlock_debugParamDefs)
#define lw_relock() relockDebug(lw_rwlock_debugParamDefs)
#define createReadLock() createReadLock(lw_rwlock_debugParamDefs)
#define createWriteLock() createWriteLock(lw_rwlock_debugParamDefs)
#define lw_upgrade() upgrade(lw_rwlock_debugParamDefs)
#define lw_downgrade() downgrade(lw_rwlock_debugParamDefs)
#define lw_upgrade_to(x) upgrade(lw_rwlock_debugParamDefs, x)
#define lw_downgrade_to(x) downgrade(lw_rwlock_debugParamDefs, x)
#else
#define lw_unlock() unlock()
#define lw_relock() relock()
#define lw_upgrade() upgrade()
#define lw_downgrade() downgrade()
#define lw_upgrade_to(x) upgrade(x)
#define lw_downgrade_to(x) downgrade(x)
#endif


struct relayclientinternal;
struct relayclient
{
public:
	const static int buildnum = 105;

	void * internaltag = nullptr, *tag = nullptr;

	relayclient(pump);
	~relayclient();

	void connect(const char * host, lw_ui16 port = 6121);
	void connect(lacewing::address);
	// For pinholing connections, set before connect()
	void setlocalport(lw_ui16 port);

	bool connecting();
	bool connected();

	void disconnect();

	lacewing::address serveraddress();

	lw_ui16 id() const;

	void name(std::string_view name);
	std::string name() const;

	std::string welcomemessage() const;

	void listchannels();
	size_t channelcount() const;

	struct channellisting
	{
		friend relayclient;
		friend relayclientinternal;
	private:
		// internaltag contains relayclientinternal
		void * internaltag = nullptr, *tag = nullptr;

		lw_ui16 _peercount = 0xFFFF;
		std::string _name, _namesimplified;

	public:
		lw_ui16 peercount() const;
		std::string name() const;
		std::string namesimplified() const;
	};

	size_t channellistingcount() const;
	const std::vector<std::shared_ptr<channellisting>> & getchannellisting() const;

	void join(std::string_view channelName, bool hidden = false, bool autoclose = false);

	void sendserver(lw_ui8 subchannel, std::string_view data, lw_ui8 type = 0) const;
	void blastserver(lw_ui8 subchannel, std::string_view data, lw_ui8 type = 0) const;

	struct channel;
	const std::vector<std::shared_ptr<channel>> & getchannels() const;

	mutable lacewing::readwritelock lock;

	struct channel
	{
		struct peer;

		friend relayclient;
		friend relayclientinternal;
		friend peer;
		mutable readwritelock lock;
	protected:
		relayclientinternal &client;

		lw_ui16 _id = 0xFFFF;
		std::string _name, _namesimplified;
		bool _ischannelmaster = false;
		std::atomic<bool> _readonly = false;
		std::vector<std::shared_ptr<relayclient::channel::peer>> peers;

		// Searches for the first peer by id number, returns null if not found
		std::shared_ptr<relayclient::channel::peer> findpeerbyid(lw_ui16 id);

		/** Adds a new peer.
		 * @param peerid ID number for the peer.
		 * @param flags  The flags of the peer connect/channel join message.
		 *				 0x1 = master. Other flags are not accepted.
		 * @param name   The peer name. Must not be null or blank. */
		std::shared_ptr<relayclient::channel::peer> addnewpeer(lw_ui16 peerid, lw_ui8 flags, std::string_view name);

	public:
		channel(relayclientinternal &_client) noexcept;
		~channel() noexcept;

		void * internaltag = nullptr, *tag = nullptr;

		bool ischannelmaster() const;
		std::string name() const;
		std::string namesimplified() const;

		void send(lw_ui8 subchannel, std::string_view data, lw_ui8 type = 0) const;
		void blast(lw_ui8 subchannel, std::string_view data, lw_ui8 type = 0) const;

		struct peer
		{
			friend relayclient;
			friend relayclientinternal;
			friend relayclient::channel;
			mutable readwritelock lock;
		protected:
			relayclient::channel &channel;

			lw_ui16 _id = 0xFFFF;
			std::string _name, _namesimplified, _prevname;

			bool _ischannelmaster = false;
			bool _readonly = false;

		public:
			peer(relayclient::channel &_channel, lw_ui16 id, lw_ui8 flags, std::string_view name) noexcept;
			~peer() noexcept;

			lw_ui16 id() const;
			bool ischannelmaster() const;

			void send(lw_ui8 subchannel, std::string_view data, lw_ui8 type = 0) const;
			void blast(lw_ui8 subchannel, std::string_view data, lw_ui8 type = 0) const;

			std::string name() const;
			std::string namesimplified() const;
			std::string prevname() const;

			bool readonly() const;
		};

		int peercount() const;
		lw_ui16 id() const;

		void leave() const;
		// Indicates (but does not say for definite!) whether a writelock allows changes.
		// Another thread may obtain writelock between the same thread reading this and getting its own writelock
		bool readonly() const;
		const std::vector<std::shared_ptr<lacewing::relayclient::channel::peer>> & getpeers() const;
	};

	// int channelcount() const;
	// relayclient::channel * firstchannel() const;

	typedef void(*handler_connect)				(lacewing::relayclient &client);
	typedef void(*handler_connectiondenied)		(lacewing::relayclient &client, std::string_view denyreason);
	typedef void(*handler_disconnect)			(lacewing::relayclient &client);

	typedef void(*handler_message_server)
		(lacewing::relayclient &client, bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant);

	typedef void(*handler_message_channel)
		(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,
			std::shared_ptr<lacewing::relayclient::channel::peer> peer,
			bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant);

	typedef void(*handler_message_peer)
		(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel,
			std::shared_ptr<lacewing::relayclient::channel::peer> peer,
			bool blasted, lw_ui8 subchannel, std::string_view message, lw_ui8 variant);

	typedef void(*handler_message_serverchannel)
		(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel, bool blasted,
			 lw_ui8 subchannel, std::string_view message, lw_ui8 variant);

	typedef void(*handler_error)				(lacewing::relayclient &client, lacewing::error);
	typedef void(*handler_channel_join)			(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target);
	typedef void(*handler_channel_joindenied)	(lacewing::relayclient &client, std::string_view channelname, std::string_view denyreason);
	typedef void(*handler_channel_leave)		(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target);
	typedef void(*handler_channel_leavedenied)	(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> target, std::string_view denyreason);
	typedef void(*handler_name_set)				(lacewing::relayclient &client);
	typedef void(*handler_name_changed)			(lacewing::relayclient &client, std::string_view oldname);
	typedef void(*handler_name_denied)			(lacewing::relayclient &client, std::string_view name, std::string_view denyreason);
	typedef void(*handler_peer_connect)			(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel, std::shared_ptr<lacewing::relayclient::channel::peer> peer);
	typedef void(*handler_peer_disconnect)		(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel, std::shared_ptr<lacewing::relayclient::channel::peer> peer);
	typedef void(*handler_peer_changename)		(lacewing::relayclient &client, std::shared_ptr<lacewing::relayclient::channel> channel, std::shared_ptr<lacewing::relayclient::channel::peer> peer, std::string oldname);
	typedef void(*handler_channellistreceived)	(lacewing::relayclient &client);

	void onconnect(handler_connect);
	void onconnectiondenied(handler_connectiondenied);
	void ondisconnect(handler_disconnect);
	void onmessage_server(handler_message_server);
	void onmessage_channel(handler_message_channel);
	void onmessage_peer(handler_message_peer);
	void onmessage_serverchannel(handler_message_serverchannel);
	void onerror(handler_error);
	void onchannel_join(handler_channel_join);
	void onchannel_joindenied(handler_channel_joindenied);
	void onchannel_leave(handler_channel_leave);
	void onchannel_leavedenied(handler_channel_leavedenied);
	void onname_set(handler_name_set);
	void onname_changed(handler_name_changed);
	void onname_denied(handler_name_denied);
	void onpeer_connect(handler_peer_connect);
	void onpeer_disconnect(handler_peer_disconnect);
	void onpeer_changename(handler_peer_changename);
	void onchannellistreceived(handler_channellistreceived);
};

} // ~namespace lacewing
#include "FrameReader.h"
#include "MessageReader.h"
namespace lacewing {

// List of code points, code point ranges, and categories, tied to utf8proc.
struct codepointsallowlist {
	// Shorthand for "all three lists are empty", saying there is no filter
	bool allAllowed = true;
	std::string list;

	std::vector<std::int32_t> codePointCategories;
	std::vector<std::int32_t> specificCodePoints;
	std::vector<std::pair<std::int32_t, std::int32_t>> codePointRanges;

	// Updates the allowlisted Unicode code points in this struct, returns error or blank
	std::string setcodepointsallowedlist(std::string codePointList);
	// -1 if the string passed matches the allow list, otherwise index of failure.
	int checkcodepointsallowed(const std::string_view toTest, int * const rejectedUTF32CodePoint = NULL) const;
};
struct relayserverinternal;
struct relayserver
{
	static const int buildnum = 39;

	void * internaltag, * tag = nullptr;

	lacewing::server socket;
	lacewing::webserver websocket;
	lacewing::udp udp;
	lacewing::flashpolicy flash;
	bool hole_punch_used = false;

	relayserver(pump) noexcept;
	~relayserver() noexcept;

	void host(lw_ui16 port = 6121);
	void host(lacewing::filter &filter);
	void host_websocket(lw_ui16 portNonSecure = 80, lw_ui16 portSecure = 443);
	void host_websocket(lacewing::filter& filterNonSecure, lacewing::filter& filterSecure);
	void unhost();
	// This works with clients of regular server, so you should use this instead of websocket->unhost/unhost_secure
	void unhost_websocket(bool insecure, bool secure);
	void hole_punch(const char* ip, lw_ui16 local_port);

	bool hosting();
	lw_ui16 port();

	void setchannellisting(bool enabled);
	void setwelcomemessage(std::string_view message);
	std::string getwelcomemessage();

	struct client;

	struct channel
	{
		friend relayserverinternal;
		friend relayserver;
		friend relayserver::client;

		void * tag = nullptr;

		lw_ui16 id();

		std::shared_ptr<client> channelmaster() const;

		// Reads a copy of the channel name. Automatically read-locks the channel.
		std::string name() const;
		// Reads a simplified copy of the channel name. Automatically read-locks the channel.
		std::string nameSimplified() const;
		// Sets the channel name.
		void name(std::string_view str);

		bool hidden() const;
		bool autocloseenabled() const;
		bool readonly() const;

		// Throw all clients off this channel, sending Leave Request Success.
		void close();

		void send(lw_ui8 subchannel, std::string_view message, lw_ui8 variant = 0);
		void blast(lw_ui8 subchannel, std::string_view message, lw_ui8 variant = 0);

		size_t clientcount() const;

		~channel() noexcept;

		// Called when server wants to add one. Also invoked by liblacewing itself.
		// Sends relevant join/leave response messages.

		//void addclient(std::shared_ptr<relayserver::client> newClient);
		//void removeclient(std::shared_ptr<relayserver::client> newClient);

		std::vector<std::shared_ptr<lacewing::relayserver::client>> & getclients();

		// Sets channel master, will notify users if ok.
		void setchannelmaster(std::shared_ptr<client> client);

		mutable lacewing::readwritelock lock;

		// Internal use only. Must be public for std::make_shared.
		channel(relayserverinternal &_server, std::string_view _name) noexcept;

	protected:
		std::atomic<bool> _readonly = false;
		// Channel Close handler may be run during srvint->channel_removeclient() or ch->channel_close()
		bool closehandlerrun = false;
		relayserverinternal &server;

		std::vector<std::shared_ptr<relayserver::client>> clients;

		std::string _name, _namesimplified;
		lw_ui16 _id = 0xFFFF;
		bool _hidden = true;
		bool _autoclose = false;
		// TODO: should be weak_ptr?
		std::shared_ptr<client> _channelmaster;

		std::shared_ptr<client> readpeer(messagereader &r);

		void PeerToChannel(relayserver &server_, std::shared_ptr<relayserver::client> client,
			bool blasted, lw_ui8 subchannel, lw_ui8 variant, std::string_view message);
	};

	size_t channelcount() const;
	std::vector<std::shared_ptr<lacewing::relayserver::client>> & getclients();
	std::vector<std::shared_ptr<lacewing::relayserver::channel>> & getchannels();
	void channel_addclient(std::shared_ptr<relayserver::channel> channel, std::shared_ptr<relayserver::client> client);
	void channel_removeclient(std::shared_ptr<relayserver::channel> channel, std::shared_ptr<relayserver::client> client);

	struct client
	{
		friend relayserverinternal;
		friend relayserver;
		friend relayserver::channel;

		enum class clientimpl
		{
			// Can be Relay or old versions of Blue
			Unknown,
			// Bluewing, Windows. Assume ANSI.
			Windows,
			Flash,
			// Bluewing, Windows UTF-8 aware.
			Windows_Unicode,
			Android,
			iOS,
			Macintosh,
			HTML5,
			UWP
			// Edit relayserverinternal::client::getimplementation if you add more lines
		};

		mutable lacewing::readwritelock lock;

		void * tag = nullptr;

		lw_ui16 id();

		std::string_view getaddress() const;
		in6_addr getaddressasint() const;
		const char * getimplementation() const;
		clientimpl getimplementationvalue() const;
		std::vector<std::shared_ptr<lacewing::relayserver::channel>> & getchannels();

		void disconnect(std::shared_ptr<relayserver::client> cli = nullptr, int websocket_exit_code = 0);

		void send(lw_ui8 subchannel, std::string_view data, lw_ui8 variant = 0);
		void blast(lw_ui8 subchannel, std::string_view data, lw_ui8 variant = 0);

		std::string name() const;
		std::string nameSimplified() const;
		void name(std::string_view);

		size_t channelcount() const;
		// Set when connection is approved, before that, returns 0
		lw_i64 getconnecttime() const;

		bool readonly() const;
		bool istrusted() const;

		// Internal use only!
		client(relayserverinternal &server, lacewing::server_client socket) noexcept;
		~client() noexcept;
	protected:
		lacewing::server_client socket = nullptr;
		relayserverinternal &server;
		// Can't use socket->address, as when server_client is free'd it is no longer valid
		// Since there's a logical use for looking up address during closing, we'll keep a copy.
		std::string address;
		in6_addr addressInt = {};
		// Time the Relay connection was approved - zero timepoint if not yet approved
		::std::chrono::high_resolution_clock::time_point connectRequestApprovedTime;
		::std::chrono::steady_clock::time_point lasttcpmessagetime;
		::std::chrono::steady_clock::time_point lastudpmessagetime; // UDP problem where unused connections are dropped by router, so must keep these separate
		::std::chrono::steady_clock::time_point lastchannelorpeermessagetime; // For clients that go idle
		framereader reader;
		std::vector<std::shared_ptr<channel>> channels;
		std::string _name, _namesimplified, _prevname;
		// Indicates if this socket has closed, or is expected to close.
		std::atomic<bool> _readonly = false;

		clientimpl clientImpl = clientimpl::Unknown;

		std::string clientImplStr;

		bool pseudoUDP = true; // Is UDP not supported (e.g. HTML5, UWP JS) so "faked" by receiver

		// Got opening null byte, indicating not a HTTP client.
		bool gotfirstbyte = false;
		// After TCP connect approval, Lacewing connect message request was received, and server has said OK to it
		bool connectRequestApproved = false;
		// Client has only ever used valid Lacewing messages; e.g. valid UTF-8, no missing elements in messages.
		// Does not indicate all messages succeed. When false, client is kicked very shortly after.
		bool trustedClient = true;
		// Has a TCP ping request been sent by server, and was replied to.
		// If false, next ping timer tick will consider a failed ping and kick the client, so it is true by default.
		bool pongedOnTCP = true;
		// Has a UDP message received, confirming its UDP address. Implies psuedoUDP is false.
		bool lockedUDPAddress = false;

		lacewing::address udpaddress;

		lw_ui16 _id = 0xFFFF;

		void PeerToPeer(relayserver &server, std::shared_ptr<relayserver::channel> viachannel, std::shared_ptr<relayserver::client> receivingclient,
			bool blasted, lw_ui8 subchannel, lw_ui8 variant, std::string_view message);

		// Checks if name can be set to given name, by this client.
		// Checks whether name is valid, and whether name is in use already.
		bool checkname(std::string_view name);

		// Reads channel from client's channel list; fails if not joined.
		std::shared_ptr<relayserver::channel> readchannel(messagereader &reader);
	};

	size_t clientcount() const;
	//client * firstclient();

	// Creates channel and adds to server list, accepts no master for the channel.
	// Expects you have already checked channel with that name does not exist.
	std::shared_ptr<relayserver::channel> createchannel(std::string_view channelName, std::shared_ptr<lacewing::relayserver::client> master, bool hidden, bool autoclose);

	// handles unhost/host, welcome message change, handler change
	mutable lacewing::readwritelock lock_meta;
	// handles channel list modifications - only to the underlying vector, not to requests like leave or close requests
	mutable lacewing::readwritelock lock_channellist;
	// handles client list modifications - only to the underlying vector, not to requests like disconnect requests
	mutable lacewing::readwritelock lock_clientlist;
	// handles UDP?
	mutable lacewing::readwritelock lock_udp;

	typedef void(*handler_connect)		(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client);
	typedef void(*handler_disconnect)	(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client);
	typedef void(*handler_error)		(lacewing::relayserver &server, lacewing::error);

	typedef void(*handler_message_server)
		(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client, bool blasted, lw_ui8 subchannel,
			std::string_view data, lw_ui8 variant);

	typedef void(*handler_message_channel)
		(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client, std::shared_ptr<lacewing::relayserver::channel> channel,
			bool blasted, lw_ui8 subchannel, std::string_view data, lw_ui8 variant);

	typedef void(*handler_message_peer)
		(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client, std::shared_ptr<lacewing::relayserver::channel> channel,
			std::shared_ptr<lacewing::relayserver::client> targetclient, bool blasted,
			lw_ui8 subchannel, std::string_view data, lw_ui8 variant);

	typedef void(*handler_channel_join)
		(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client, std::shared_ptr<lacewing::relayserver::channel> channel,
			bool hidden, bool autoclose);

	typedef void(*handler_channel_leave)
		(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client, std::shared_ptr<lacewing::relayserver::channel> channel);

	typedef bool(*handler_channel_close)
		(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::channel> channel);

	typedef void(*handler_nameset)
		(lacewing::relayserver &server, std::shared_ptr<lacewing::relayserver::client> client, std::string_view requestedname);

	// Plain MS value. Note that 0 or negatives are not usable values.
	void setinactivitytimer(long milliSeconds);

	// Used in setcodepointsallowedlist() only.
	enum class codepointsallowlistindex : int {
		ClientNames = 0,
		ChannelNames,
		// Messages sent to clients includes peer messages, channel messages and server -> client/channel
		MessagesSentToClients,
		// Server messages are only checked if server is processing client -> server messages.
		// Handler will not be called at all if message contains denied characters; the client is
		// instead marked as untrusted and an error created.
		MessagesSentToServer,
	};

	// Updates the allowlisted Unicode code points used in text messages, channel names and peer names.
	std::string setcodepointsallowedlist(codepointsallowlistindex type, std::string codePointList);
	// -1 if the string passed matches the allow list, otherwise the char index
	int checkcodepointsallowed(relayserver::codepointsallowlistindex type, std::string_view toTest, int * rejectedUTF32CodePoint = nullptr) const;

	void onconnect(handler_connect);
	void ondisconnect(handler_disconnect);
	void onerror(handler_error);
	void onmessage_server(handler_message_server);
	void onmessage_channel(handler_message_channel);
	void onmessage_peer(handler_message_peer);
	void onchannel_join(handler_channel_join);
	void onchannel_leave(handler_channel_leave);
	void onchannel_close(handler_channel_close);
	void onnameset(handler_nameset);

	void connect_response(std::shared_ptr<lacewing::relayserver::client> client,
		std::string_view denyReason);
	void joinchannel_response(std::shared_ptr<lacewing::relayserver::channel> channel,
		std::shared_ptr<lacewing::relayserver::client> client, std::string_view denyReason);
	void channelmessage_permit(
		std::shared_ptr<lacewing::relayserver::client> sendingclient, std::shared_ptr<lacewing::relayserver::channel> channel,
		bool blasted, lw_ui8 subchannel, std::string_view data, lw_ui8 variant, bool accept);
	void clientmessage_permit(std::shared_ptr<lacewing::relayserver::client> sendingclient, std::shared_ptr<lacewing::relayserver::channel> channel,
		std::shared_ptr<lacewing::relayserver::client> receivingclient,
		bool blasted, lw_ui8 subchannel, std::string_view data, lw_ui8 variant, bool accept);
	// The ability to prevent a client from leaving a channel seems pointless; they can always pull the plug.
	void leavechannel_response(std::shared_ptr<lacewing::relayserver::channel> channel,
		std::shared_ptr<lacewing::relayserver::client> client, std::string_view denyReason);
	// Closing a channel can't be prevented, but the peer list and such needn't be instantly updated.
	// If it's delayed, this should be run to permit it.
	void closechannel_finish(std::shared_ptr<lacewing::relayserver::channel> channel);
	void nameset_response(std::shared_ptr<lacewing::relayserver::client> client,
		std::string_view newClientName, std::string_view denyReason);
};
//#pragma endregion

}

#endif /* defined (__cplusplus) */
#endif /* _lacewing_h */

