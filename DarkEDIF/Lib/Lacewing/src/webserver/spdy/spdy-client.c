
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin.  All rights reserved.
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

#include "../common.h"

static void client_respond (lwp_ws_client, lw_ws_req request);
static void client_tick (lwp_ws_client);
static void client_cleanup (lwp_ws_client);

lwp_ws_client lwp_ws_spdyclient_new (lw_ws ws, lw_server_client socket,
                                     lw_bool secure, int version)
{
   lwp_ws_spdyclient ctx = calloc (sizeof (*ctx), 1);

   if (!ctx)
      return 0;

   ctx->client.ws = ws;
   ctx->client.socket = socket;

   ctx->client.respond = client_respond;
   ctx->client.tick = client_tick;
   ctx->client.cleanup = client_cleanup;

   lwp_stream_init ((lw_stream) ctx, &def_spdyclient, 0);

   ctx->spdy = spdy_ctx_new (&lwp_ws_spdy_config, version, 0, 0);
   spdy_ctx_set_tag (ctx->spdy, ctx);

   lwp_trace ("SPDY context created @ %p", ctx->spdy);

   /* When the retry mode is Retry_MoreData and Put can't consume everything,
    * Put will be called again as soon as more data arrives.
    */

   lw_stream_retry ((lw_stream) ctx, lw_stream_retry_more_data);

   return (lwp_ws_client) ctx;
}

void client_cleanup (lwp_ws_client client)
{
   lwp_ws_spdyclient ctx = (lwp_ws_spdyclient) client;

   list_each (ctx->requests, request)
   {
      if (ctx->client.ws->on_disconnect)
         ctx->client.ws->on_disconnect (ctx->client.ws, request);

      lwp_ws_req_delete (request);
   }

   list_clear (ctx->requests);

   spdy_ctx_delete (ctx->spdy);
}

static size_t def_spdyclient_sink_data (lw_stream stream,
                                        const char * buffer, size_t size)
{
   lwp_ws_spdyclient ctx = (lwp_ws_spdyclient) stream;

   /* lw_dump (buffer, size); */

   int res = spdy_data (ctx->spdy, buffer, &size);

   lwp_trace ("SPDY processed " lwp_fmt_size " bytes", size);

   if (res != SPDY_E_OK)
   {
      lwp_trace ("SPDY error: %s", spdy_error_string (res));

      lw_stream_close ((lw_stream) ctx->client.socket, lw_false);

      return size;
   }

   return size;
}

const lw_streamdef def_spdyclient =
{
   .sink_data = def_spdyclient_sink_data
};

void client_respond (lwp_ws_client client, lw_ws_req request)
{
   lwp_ws_spdyclient ctx = (lwp_ws_spdyclient) client;

   spdy_stream * stream = (spdy_stream *) request->tag;

   assert (!spdy_stream_open_remote (stream));

   lw_fdstream_cork ((lw_fdstream) ctx->client.socket);

   spdy_nv_pair * headers = alloca
      (sizeof (spdy_nv_pair) * (list_length (request->headers_out) + 3));

   int n = 0;

   /* version header */

   char version [16];

   sprintf (version, "HTTP/%d.%d",
         (int) request->version_major, (int) request->version_minor);

   {  spdy_nv_pair * pair = &headers [n ++];

      pair->name_len = strlen (pair->name = (char *)
            (spdy_active_version (ctx->spdy) == 2 ? "version" : ":version"));

      pair->value_len = strlen (pair->value = version);
   }

   /* status header */

   {  spdy_nv_pair * pair = &headers [n ++];

      pair->name_len = strlen (pair->name = (char *)
            (spdy_active_version (ctx->spdy) == 2 ? "status" : ":status"));

      pair->value_len = strlen (pair->value = request->status);
   }

   /* content-length header */

   size_t length = lw_stream_queued ((lw_stream) request);

   char length_str [24];
   sprintf (length_str, lwp_fmt_size, length);

   {  spdy_nv_pair * pair = &headers [n ++];

      pair->name_len = strlen (pair->name = (char *) "content-length");
      pair->value_len = strlen (pair->value = length_str);
   }

   list_each (request->headers_out, header)
   {
      spdy_nv_pair * pair = &headers [n ++];

      pair->name = header.name;
      pair->name_len = strlen (header.name);

      pair->value = header.value;
      pair->value_len = strlen (header.value);
   }

   if (length > 0)
   {
      int res = spdy_stream_write_headers (stream, n, headers, 0);

      if (res != SPDY_E_OK)
      {
         lwp_trace ("Error writing headers: %s", spdy_error_string (res));
      }

   }
   else
   {
      /* Writing headers with SPDY_FLAG_FIN when the stream is already closed
       * to the remote will cause the stream to be deleted.
       */

      spdy_stream_write_headers (stream, n, headers, SPDY_FLAG_FIN);
      stream = 0;
   }

   lwp_trace ("SPDY: %d headers -> stream @ %p, content length " lwp_fmt_size,
         n, stream, length);

   lw_stream_write_stream ((lw_stream) ctx->client.socket,
                           (lw_stream) request,
                           length,
                           lw_false);

   lw_stream_end_queue ((lw_stream) request);
   lw_stream_begin_queue ((lw_stream) request);

   if (length > 0)
   {
      spdy_stream_write_data (stream, 0, 0, SPDY_FLAG_FIN);
      stream = 0;
   }

   lw_fdstream_uncork ((lw_fdstream) socket);

   lwp_ws_req_delete (request);
}

void client_tick (lwp_ws_client client)
{


}

