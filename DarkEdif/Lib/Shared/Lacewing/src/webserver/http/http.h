/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../../../deps/http-parser/http_parser.h"

typedef struct _lwp_ws_httpclient
{
	struct _lwp_ws_client client;

	lw_ws_req request; /* HTTP is one request at a time, so this is just reused */

	time_t last_activity;

	http_parser parser;

	lw_bool parsing_headers, signal_eof;

	char * cur_header_name;
	size_t cur_header_name_length;

} * lwp_ws_httpclient;

lwp_ws_client lwp_ws_httpclient_new
	(lw_ws, lw_server_client socket, lw_bool secure);

void lwp_ws_httpclient_delete (lw_ws, lwp_ws_httpclient);

extern const http_parser_settings parser_settings;

extern const lw_streamdef def_httpclient;
extern const lw_streamdef def_httprequest;

