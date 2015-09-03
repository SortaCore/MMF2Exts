
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.  All rights reserved.
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

#include "spdy_ctx.h"
#include "spdy_bytes.h"
#include "spdy_control.h"
#include "spdy_stream.h"

int spdy_proc_goaway (spdy_ctx * ctx, int8_t flags, spdy_buffer * buffer)
{
   spdy_stream * stream;
   spdy_stream * next_stream, * prev_stream;
   int32_t last_good_stream_id;
   spdy_stream * last_good_stream;
   int32_t status_code;

   if (buffer->size < 4)
      return SPDY_E_PROTOCOL;

   last_good_stream_id = spdy_read_int31 (buffer);

   if (ctx->version == 2)
      status_code = SPDY_STATUS_CANCEL;
   else
   {
      if (buffer->size < 4)
         return SPDY_E_PROTOCOL;

      switch (spdy_read_int32 (buffer))
      {
      case SPDY_GOAWAY_OK:
         status_code = SPDY_STATUS_CANCEL;
         break;

      case SPDY_GOAWAY_PROTOCOL_ERROR:
         status_code = SPDY_STATUS_PROTOCOL_ERROR;
         break;

      case SPDY_GOAWAY_INTERNAL_ERROR:
         status_code = SPDY_STATUS_INTERNAL_ERROR;
         break;
      };
   }

   if (ctx->flags & SPDY_SESSION_CLOSED)
      return SPDY_E_OK;  /* session already closed; ignore this frame */

   if (! (last_good_stream = spdy_find_stream (ctx, last_good_stream_id)))
   {
      /* Unknown stream ID */

      spdy_emit_rst_stream (ctx, 0, last_good_stream_id, status_code);

      return spdy_soft_error (ctx);
   }

   ctx->flags |= SPDY_SESSION_CLOSED;

   stream = ctx->stream_list;
   prev_stream = 0;

   /* Remove any streams older than last_good_stream_id from the list */

   while (stream)
   {
      next_stream = stream->next;

      if (stream->id < last_good_stream_id)
      {
         if (prev_stream)
            prev_stream->next = stream->next;
         else
            ctx->stream_list = stream->next;

         if (ctx->config->on_stream_close)
            ctx->config->on_stream_close (ctx, stream, SPDY_STATUS_CANCEL);

         spdy_stream_delete (ctx, stream);
      }
      else
      {
         prev_stream = stream;
      }

      stream = next_stream;
   }

   return SPDY_E_OK;
}

void spdy_emit_goaway (spdy_ctx * ctx, int8_t flags,
                       int32_t last_good_stream_id)
{
   char message [SPDY_CTRL_HEADER_SIZE + 4];

   spdy_build_control_header (ctx, message, GOAWAY, flags, 4);
   spdy_write_int31 (message + SPDY_CTRL_HEADER_SIZE, last_good_stream_id);

   ctx->config->emit (ctx, message, sizeof (message));
}



