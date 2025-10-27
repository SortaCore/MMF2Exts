/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"

static int on_message_begin (http_parser * parser)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) parser->data;

	lwp_ws_req_clean (ctx->request);

	return 0;
}

static int on_url (http_parser * parser, const char * url, size_t length)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) parser->data;

	if (!lwp_ws_req_in_url (ctx->request, length, url))
	{
		lwp_trace ("HTTP: Bad URL");
		return -1;
	}

	return 0;
}

static int on_header_field (http_parser * parser, const char * buffer,
							size_t length)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) parser->data;

	if (!ctx->request->version_major)
	{
		char version [16];

		lwp_snprintf (version, sizeof (version), "HTTP/%d.%d",
			(int) parser->http_major, (int) parser->http_minor);

		#ifdef _MSC_VER
			// HTTP version is always 2 digits, so this is always space for zero-terminated
			#pragma warning (suppress: 6053)
		#endif
		if (!lwp_ws_req_in_version (ctx->request, strlen (version), version))
		{
			lwp_trace ("HTTP: Bad version");
			return -1;
		}
	}

	/* Since we already ensure the parser receives entire lines while processing
	* headers, it's safe to just save the pointer and size from on_header_field
	* and use them in on_header_value
	*/

	ctx->cur_header_name = (char *) buffer;
	ctx->cur_header_name_length = length;

	return 0;
}

static int on_header_value (http_parser * parser, const char * value, size_t length)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) parser->data;

	if (!lwp_ws_req_in_header (ctx->request,
							  ctx->cur_header_name_length,
							  ctx->cur_header_name,
							  length, value))
	{
		lwp_trace ("HTTP: Bad header");
		return -1;
	}

	return 0;
}

static int on_headers_complete (http_parser * parser)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) parser->data;

	ctx->parsing_headers = lw_false;

	const char * method = http_method_str ((enum http_method) parser->method);

	if (!lwp_ws_req_in_method (ctx->request, strlen (method), method))
	{
		lwp_trace ("HTTP: Bad method");
		return -1;
	}

	const char * content_type = lw_ws_req_header (ctx->request, "content-type");

	lwp_trace ("Content-Type is %s", content_type);

	if (lwp_begins_with (content_type, "multipart"))
	{
		lwp_trace ("Creating Multipart...");

		if (! (ctx->client.multipart = lwp_ws_multipart_new
				(ctx->client.ws, ctx->request, content_type)))
		{
			return -1;
		}
	}

	return 0;
}

static int on_body (http_parser * parser, const char * buffer, size_t size)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) parser->data;

	if (!ctx->client.multipart)
	{
		/* Normal request body - just buffer it */

		lwp_heapbuffer_add (&ctx->request->buffer, buffer, size);
		return 0;
	}

	/* Multipart request body - hand it over to the multipart processor */

	if (lwp_ws_multipart_process (ctx->client.multipart, buffer, size) != size)
	{
		lwp_trace ("Error w/ multipart form data");

		lwp_ws_multipart_delete (ctx->client.multipart);
		ctx->client.multipart = 0;

		return -1;
	}

	if ( (!ctx->client.multipart) || ctx->client.multipart->done)
		ctx->signal_eof = lw_true;

	return 0;
}

static int on_message_complete (http_parser * parser)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) parser->data;

	if (!ctx->client.multipart)
		lwp_ws_req_call_hook (ctx->request);

	ctx->parsing_headers = lw_true;

	/* Since we're now transitioning back to parsing headers, our sink method
	* needs to start reading lines again (if it's currently reading the
	* body).  Pausing the parser returns control to the sink method.
	*/

	http_parser_pause (parser, 1);

	return 0;
}

const http_parser_settings parser_settings =
{
	on_message_begin,
	on_url,
	NULL /* on_status */,
	on_header_field,
	on_header_value,
	on_headers_complete,
	on_body,
	on_message_complete,
	NULL, /* on_chunk_header */
	NULL  /* on_chunk_complete */
};
