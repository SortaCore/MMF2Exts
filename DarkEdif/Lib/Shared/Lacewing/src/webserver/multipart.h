/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

typedef struct _lwp_ws_multipart
{
	lw_ws ws;
	lw_ws_req request;

	struct _lwp_ws_multipart * parent, * child;

	multipart_parser * parser;

	lw_bool done;
	lw_bool called_handler;

	lwp_nvhash disposition;

	lw_bool parsing_headers;

	const char * cur_header_name;
	size_t cur_header_name_length;

	lw_list (struct _lw_ws_upload_hdr, headers);

	lw_ws_upload cur_upload;

	lw_ws_upload * uploads;
	size_t num_uploads;

} * lwp_ws_multipart;

lwp_ws_multipart lwp_ws_multipart_new
	(lw_ws, lw_ws_req, const char * content_type);

void lwp_ws_multipart_delete (lwp_ws_multipart);

size_t lwp_ws_multipart_process
	(lwp_ws_multipart, const char * buffer, size_t size);

void lwp_ws_multipart_call_hook (lwp_ws_multipart);

