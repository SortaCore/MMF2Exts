/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/licenses/mit-license.php
*/

#include "../common.h"

static void client_respond (lwp_ws_client, lw_ws_req request);
static void client_tick (lwp_ws_client);
static void client_cleanup (lwp_ws_client);

lwp_ws_client lwp_ws_httpclient_new (lw_ws ws, lw_server_client socket,
									 lw_bool secure)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) calloc (sizeof (*ctx), 1);

	if (!ctx)
	  return 0;

	ctx->client.ws = ws;
	ctx->client.socket = socket;
	ctx->client.websocket = lw_false;
	ctx->client.local_close_code = ctx->client.remote_close_code = -1;

	ctx->client.respond  = client_respond;
	ctx->client.tick	 = client_tick;
	ctx->client.cleanup  = client_cleanup;
	ctx->client.secure   = secure;

	lwp_stream_init ((lw_stream) ctx, &def_httpclient, 0);

	ctx->request = lwp_ws_req_new (ws, (lwp_ws_client) ctx, &def_httprequest);

	http_parser_init (&ctx->parser, HTTP_REQUEST);
	ctx->parser.data = ctx;

	ctx->parsing_headers = lw_true;
	ctx->signal_eof = lw_false;

	lw_stream_write_stream
	  ((lw_stream) socket, (lw_stream) ctx->request, SIZE_MAX, lw_false);

	lw_stream_begin_queue ((lw_stream) ctx->request);

	/* When the retry mode is more_data and we can't sink everything, our sink
	* method will be called again as soon as more data arrives.
	*/

	lw_stream_retry ((lw_stream) ctx, lw_stream_retry_more_data);

	return (lwp_ws_client) ctx;
}

void client_cleanup (lwp_ws_client client)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) client;

	/* Only call the disconnect handler for requests that have not yet been
	* completed (responded == false)
	*/

	if (!ctx->request->responded || ctx->client.websocket)
	{
	  if (ctx->client.ws->on_disconnect)
		 ctx->client.ws->on_disconnect (ctx->client.ws, ctx->request);
	}

	lwp_ws_req_delete (ctx->request);
}



/*
 * Stream implementation
 */

size_t lw_webserver_sink_websocket(lw_ws webserver, lwp_ws_httpclient client, const char* buffer, int size);
static size_t def_sink_data (lw_stream stream, const char * buffer, size_t size)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) stream;

	lwp_trace ("HTTP got " lwp_fmt_size " bytes", size);

	size_t processed = 0;

	/* TODO: A naughty client could keep the connection open by sending 1 byte
	* every 5 seconds.
	*/

	ctx->last_activity = time (0);

	if (ctx->client.websocket)
		return lw_webserver_sink_websocket(ctx->client.ws, ctx, buffer, (int)size);

	for (;;)
	{
	  if (!ctx->request->responded)
	  {
		 /* The application hasn't yet called Finish() for the last request,
		  * so no more data can be processed.
		  */

		 return processed;
	  }

	  /* Already processed everything in the last loop iteration? */

	  if (processed == size)
		 return processed;

	  /* When parsing headers, we provide the HTTP parser with complete lines
		* to avoid getting any fragmented data.
		*
		* When partial lines are received, we can take advantage of the
		* natural buffering provided by ws_httpclient being a stream (by
		* returning < size bytes from sink and calling stream_retry later).
		*/

	  /* TODO: max line length */

	  if (ctx->parsing_headers)
	  {
		 for (size_t i = processed; i < size; ++ i)
		 {
			char b = buffer [i];

			if (b == '\r' || b == '\n')
			{
				/* Reached the end of a line */

				size_t to_parse = (i + 1) - processed;

				size_t parsed = http_parser_execute (&ctx->parser,
													&parser_settings,
													buffer + processed,
													to_parse);

				processed += parsed;

				if (ctx->parser.http_errno == HPE_PAUSED)
				{
				  /* Paused by onMessageEnd - this has no significance, since
					* we're already parsing headers.
					*/

				  http_parser_pause (&ctx->parser, 0);
				  continue;
				}
				else if (parsed != to_parse || ctx->parser.upgrade)
				{
				  lwp_trace ("HTTP error (headers), closing socket...");

				  lw_stream_close ((lw_stream) ctx->client.socket, lw_true);
				  return size;
				}

				if (!ctx->parsing_headers)
				  break;  /* headers finished */
			}
		 }

		 /* Reached the end of the buffer - are we still parsing headers? */

		 if (ctx->parsing_headers)
			return processed;
		 else
		 {
			if (processed == size)
				return processed;
		 }
	  }

	  /* Parsing the body.
		*
		* Note that we have no idea if the buffer we have here contains only
		* body data - it might well contain a fragment of the next request,
		* which we would want to pass to the HTTP parser line by line.  For
		* this reason, the parser will always be paused in on_message_complete
		* so that control is returned here.
		*/

	  size_t to_parse = size - processed;

	  size_t parsed = http_parser_execute (&ctx->parser, &parser_settings,
											buffer + processed,
											to_parse);

	  processed += parsed;

	  if (ctx->parser.http_errno == HPE_PAUSED)
	  {
		 /* on_message_complete always pauses the parser so that control
		  * is returned here, so that we can transition back to parsing
		  * headers.
		  */

		 http_parser_pause (&ctx->parser, 0);
	  }
	  else if (ctx->parser.upgrade == 1)
	  {
		  if (ctx->client.websocket)
		  {
			  if (parsed != to_parse)
				  continue;
		  }

		  lwp_trace("HTTP error (upgrade), closing socket...");

		  lw_stream_close((lw_stream)ctx->client.socket, lw_true);
		  return size;
	  }
	  else if (parsed != to_parse)
	  {
		 lwp_trace ("HTTP error (body), closing socket...");

		 lw_stream_close ((lw_stream) ctx->client.socket, lw_true);
		 return size;
	  }

	  if (ctx->signal_eof)
	  {
		 http_parser_execute (&ctx->parser, &parser_settings, 0, 0);
		 ctx->signal_eof = lw_false;
	  }

	  return processed;
	}
}

const lw_streamdef def_httpclient =
{
	def_sink_data,
	0, /* sink_stream */
	0, /* retry */
	0, /* is_transparent */
	0, /* close */
	0, /* bytes_left */
	0, /* read */
	0  /* cleanup */
};

static lw_bool def_is_transparent (lw_stream stream)
{
	return lw_true;
}

const lw_streamdef def_httprequest =
{
	0, /* sink_data */
	0, /* sink_stream */
	0, /* retry */
	def_is_transparent,
	0, /* close */
	0, /* bytes_left */
	0, /* read */
	0  /* cleanup */
};

void client_respond (lwp_ws_client client, lw_ws_req request)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) client;

	/* The request parameter is redundant here, because HTTP only ever has one
	* request object per client (unlike now-deprecated SPDY).  Sanity check...
	*/

	assert (request == ctx->request);

	ctx->last_activity = time (0);

	/* TODO: Eliminate the use of this buffer (use stream queueing instead) */

	lwp_heapbuffer_reset (&request->buffer);

	lwp_heapbuffer_addf (&request->buffer, "HTTP/%d.%d %s",
						(int) request->version_major,
						(int) request->version_minor,
						request->status);

	list_each (struct _lw_ws_req_hdr, request->headers_out, header)
	{
	  lwp_heapbuffer_addf (&request->buffer, "\r\n%s: %s",
							header.name, header.value);
	}

	for (lw_ws_req_cookie cookie = request->cookies; cookie;
		 cookie = (lw_ws_req_cookie) cookie->hh.next)
	{
	  if (!cookie->changed)
		 continue;

	  lwp_heapbuffer_addf (&request->buffer, "\r\nset-cookie: %s=%s",
							 cookie->name, cookie->value);

	  if (*cookie->attr)
		 lwp_heapbuffer_addf (&request->buffer, "; %s", cookie->attr);
	}

	lwp_heapbuffer_addf (&request->buffer, "\r\ncontent-length: " lwp_fmt_size "\r\n\r\n",
							lw_stream_queued ((lw_stream) ctx->request));

	lw_fdstream_cork ((lw_fdstream) ctx->client.socket);

	char * head_buffer = lwp_heapbuffer_buffer (&request->buffer);
	size_t head_length = lwp_heapbuffer_length (&request->buffer);

	lw_stream_end_queue_hb ((lw_stream) ctx->request, 1,
							(const char **) &head_buffer, &head_length);

	lw_stream_begin_queue ((lw_stream) ctx->request);

	lwp_heapbuffer_reset (&request->buffer);

	lw_fdstream_uncork ((lw_fdstream) ctx->client.socket);

	if (!http_should_keep_alive (&ctx->parser) && !ctx->client.websocket)
	  lw_stream_close ((lw_stream) ctx->client.socket, lw_false);

	request->responded = lw_true;

	/* If any data was queued while waiting to respond to this request, we'll
	* be able to process it now.
	*/

	lw_stream_retry ((lw_stream) ctx, lw_stream_retry_now);
}

void client_tick (lwp_ws_client client)
{
	lwp_ws_httpclient ctx = (lwp_ws_httpclient) client;

	if (ctx->client.websocket)
	{
		if (ctx->client.ws->timeout != 0 &&
			(time(0) - ctx->last_activity) > ctx->client.ws->timeout)
		{
			lwp_trace("Force-closing WebSocket connection due to no acknowledgement of close packet (%s)",
				lw_addr_tostring(lw_server_client_addr(ctx->client.socket)));

			lw_stream_close((lw_stream)ctx->client.socket, lw_true);
		}
		return;
	}

	if (ctx->request->responded
		 && (time(0) - ctx->last_activity) > ctx->client.ws->timeout)
	{
	  lwp_trace ("Dropping HTTP connection due to inactivity (%s)",
			lw_addr_tostring (lw_server_client_addr (ctx->client.socket)));

	  lw_stream_close ((lw_stream) ctx->client.socket, lw_true);
	}
}

