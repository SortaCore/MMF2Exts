/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/licenses/mit-license.php
*/

#include "../common.h"
#include "../../deps/multipart-parser/multipart_parser.h"
#include "../stream.h"

typedef struct _lwp_ws_client * lwp_ws_client;

struct _lw_ws_req_hdr
{
	char * name, * value;
	lw_ws_req_hdr * next;
};

struct _lw_ws_upload_hdr
{
	char * name, * value;
	lw_ws_upload_hdr * next;
};

struct _lw_ws_upload
{
	lw_ws_req request;

	lwp_nvhash disposition;

	lw_file autosave_file;
	char * autosave_filename;

	lw_list (struct _lw_ws_upload_hdr, headers);
};

lw_ws_upload lwp_ws_upload_new (lw_ws_req request);
void lwp_ws_upload_delete (lw_ws_upload);

#include "multipart.h"

#define lwp_session_id_length 32

struct _lw_ws_session
{
	char id [lwp_session_id_length * 2 + 1];

	lwp_nvhash data;
	UT_hash_handle hh;
};

struct _lw_ws
{
	lw_server socket, socket_secure;

	lw_pump pump;

	lw_timer timer;

	lw_ws_session sessions;

	lw_bool auto_finish;
	lw_bool websocket;

	// No request made timeout - ignored for websocket
	long timeout;

	lw_ws_hook_error		  		on_error;
	lw_ws_hook_get					on_get;
	lw_ws_hook_post					on_post;
	lw_ws_hook_head					on_head;
	lw_ws_hook_upload_start			on_upload_start;
	lw_ws_hook_upload_chunk			on_upload_chunk;
	lw_ws_hook_upload_done			on_upload_done;
	lw_ws_hook_upload_post			on_upload_post;
	lw_ws_hook_disconnect			on_disconnect;
	lw_ws_hook_websocket_message	on_websocket_message;

	void * tag;
};

struct _lw_ws_req_cookie
{
	char * name, * value, * attr;
	lw_bool changed;

	UT_hash_handle hh;
};

struct _lw_ws_req
{
	struct _lw_stream stream;

	void * tag;

	lw_ws ws;
	lwp_ws_client client;

	struct _lw_ws_req_cookie * cookies;


	/* Input */

	char version_major, version_minor;

	char method	 [16];
	char url		[4096];
	char hostname	[128];

	lw_list (struct _lw_ws_req_hdr, headers_in);
	lwp_nvhash get_items, post_items;

	/* The protocol implementation can use this for any intermediate
	* buffering, providing it contains the request body (if any) when
	* the handler is called.
	*/

	lwp_heapbuffer buffer;

	lw_bool parsed_post_data;


	/* Output */

	char status [64];

	lw_list (struct _lw_ws_req_hdr, headers_out);

	lw_bool responded;
};

lw_ws_req lwp_ws_req_new (lw_ws, lwp_ws_client, const lw_streamdef *);
void lwp_ws_req_delete (lw_ws_req);

#ifdef __cplusplus
extern "C"
#endif
void lwp_ws_req_clean (lw_ws_req);

void lwp_ws_req_set_cookie (lw_ws_req, size_t name_len, const char * name,
										size_t value_len, const char * value,
										size_t attr_len, const char * attr,
										lw_bool changed);

/* Request */

lw_bool lwp_ws_req_in_version (lw_ws_req, size_t len, const char * version);
lw_bool lwp_ws_req_in_method (lw_ws_req, size_t len, const char * method);

lw_bool lwp_ws_req_in_header (lw_ws_req, size_t name_len, const char * name,
										 size_t value_len, const char * value);

lw_bool lwp_ws_req_in_url (lw_ws_req, size_t len, const char * url);


/* Response */

void lwp_ws_req_before_handler (lw_ws_req);
void lwp_ws_req_after_handler (lw_ws_req);

void lwp_ws_req_call_hook (lw_ws_req);

void lwp_ws_req_respond (lw_ws_req);


struct _lwp_ws_client
{
	struct _lw_stream stream;

	void (* respond) (lwp_ws_client, lw_ws_req request);
	void (* tick) (lwp_ws_client);
	void (* cleanup) (lwp_ws_client);

	lw_bool secure;
	lw_bool websocket;

	lw_ws ws;
	lw_server_client socket;

	long timeout;
	// WebSocket: -1 or close code. WebSocket requires a close packet from both ends for a "clean" connection close
	lw_i16 local_close_code, remote_close_code;

	lwp_ws_multipart multipart;
};

#include "http/http.h"
