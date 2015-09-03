
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
#include "spdy_stream.h"
#include "spdy_bytes.h"
#include "spdy_control.h"

int spdy_stream_open (spdy_ctx * ctx, spdy_stream ** stream_out,
                      spdy_stream * assoc, int flags, int priority,
                      int cred_slot, size_t num_headers,
                      spdy_nv_pair * headers)
{
   spdy_stream * stream;
   int8_t frame_flags = 0;
   int res;

   *stream_out = 0;
  
   if (ctx->flags & SPDY_SESSION_CLOSED)
      return SPDY_E_SESSION_CLOSED;

   if (! (stream = (spdy_stream *) calloc (sizeof (spdy_stream), 1)))
      return SPDY_E_MEM;

   stream->ctx = ctx;
   stream->id = ctx->next_stream_id;
   stream->flags = SPDY_STREAM_SENT_ACK;

   if (assoc)
   {
      stream->assoc = assoc;
      ++ assoc->ref_count;
   }

   ctx->next_stream_id += 2;

   if (flags & SPDY_FLAG_FIN)
   {
      frame_flags |= SPDY_FLAG_FIN;
      stream->flags |= SPDY_STREAM_CLOSED_HERE;
   }

   if (flags & SPDY_FLAG_UNIDIRECTIONAL)
   {
      frame_flags |= SPDY_FLAG_UNIDIRECTIONAL;
      stream->flags |= SPDY_STREAM_CLOSED_REMOTE;
   }

   if ((res = spdy_emit_syn_stream
        (ctx, frame_flags, stream->id, assoc == NULL ? 0 : assoc->id,
             priority, cred_slot, num_headers, headers)) != SPDY_E_OK)
   {
      free (stream);
      return res;
   }

   if ((stream->flags & SPDY_STREAM_CLOSED_HERE) &&
         (stream->flags & SPDY_STREAM_CLOSED_REMOTE))
   {
      /* The stream is already closed */

      free (stream);
      return SPDY_E_OK;
   }

   spdy_link_stream (ctx, stream);
   *stream_out = stream;

   return SPDY_E_OK;
}

void spdy_stream_set_tag (spdy_stream * stream, void * tag)
{
   stream->tag = tag;
}

void * spdy_stream_get_tag (spdy_stream * stream)
{
   return stream->tag;
}

int spdy_stream_write_headers (spdy_stream * stream, size_t num_headers,
                               spdy_nv_pair * headers, int flags)
{
   spdy_ctx * ctx;
   int res;
   int8_t frame_flags = 0;
  
   ctx = stream->ctx;

   if (flags & (~ SPDY_FLAG_FIN)) /* only FLAG_FIN is allowed */
      return SPDY_E_PARAM;

   if (stream->flags & SPDY_STREAM_CLOSED_HERE)
      return SPDY_E_PARAM; /* stream is half-closed to us */

   if (flags & SPDY_FLAG_FIN)
      frame_flags |= SPDY_FLAG_FIN;

   if (! (stream->flags & SPDY_STREAM_SENT_ACK))
   {
      /* Haven't yet sent a SYN_REPLY for this stream */

      res = spdy_emit_syn_reply
         (ctx, frame_flags, stream->id, num_headers, headers);

      stream->flags |= SPDY_STREAM_SENT_ACK;
   }
   else
   {
      /* Already sent a SYN_REPLY - send an independent HEADERS frame. */

      res = spdy_emit_headers
         (ctx, frame_flags, stream->id, num_headers, headers);
   }

   if (res != SPDY_E_OK)
      return res;

   if (flags & SPDY_FLAG_FIN)
   {
      stream->flags |= SPDY_STREAM_CLOSED_HERE;

      if ((stream->flags & SPDY_STREAM_CLOSED_HERE) &&
            (stream->flags & SPDY_STREAM_CLOSED_REMOTE))
      {
         spdy_stream_delete (ctx, stream);
      }
   }

   return SPDY_E_OK;
}

void spdy_stream_write_data
   (spdy_stream * stream, const char * data, size_t total_size, int flags)
{
   spdy_ctx * ctx;
   size_t size;
   char header [8];

   ctx = stream->ctx;

   if (stream->flags & SPDY_STREAM_CLOSED_HERE)
      return;

   spdy_write_int31 (header, stream->id);

   if (total_size == 0)
   {
      header [4] = (flags & SPDY_FLAG_FIN) ? SPDY_FLAG_FIN : 0;
      spdy_write_int24 (header + 5, 0);

      ctx->config->emit (ctx, header, sizeof (header));
   }
   else
   {
      while (total_size > 0)
      {
         if (total_size > SPDY_INT24_MAX)
         {
            size = SPDY_INT24_MAX;
            header [4] = 0;
         }
         else
         {
            size = total_size;
            header [4] = (flags & SPDY_FLAG_FIN) ? SPDY_FLAG_FIN : 0;
         }

         spdy_write_int24 (header + 5, size);

         spdy_emitv (ctx, 2, header, sizeof(header),
                             data, size);

         total_size -= size;
         data += size;
      }
   }

   if (flags & SPDY_FLAG_FIN)
   {
      stream->flags |= SPDY_STREAM_CLOSED_HERE;

      if ((stream->flags & SPDY_STREAM_CLOSED_HERE) &&
            (stream->flags & SPDY_STREAM_CLOSED_REMOTE))
      {
         spdy_stream_delete (ctx, stream);
      }
   }
}

void spdy_stream_close (spdy_stream * stream, int status_code)
{
   spdy_ctx * ctx = stream->ctx;

   spdy_emit_rst_stream (ctx, 0, stream->id, status_code);

   spdy_stream_delete (ctx, stream);
}

spdy_bool spdy_stream_is_ours (spdy_stream * stream)
{
   return stream->ctx->config->is_server ?
           ((stream->id % 2) == 0)   /* even ID = initiated by the server */
         : ((stream->id % 2) != 0);  /* odd ID = initiated by the client */
}

spdy_bool spdy_stream_open_here (spdy_stream * stream)
{
   return (stream->flags & SPDY_STREAM_CLOSED_HERE) == 0;
}

spdy_bool spdy_stream_open_remote (spdy_stream * stream)
{
   return (stream->flags & SPDY_STREAM_CLOSED_REMOTE) == 0;
}

void spdy_link_stream (spdy_ctx * ctx, spdy_stream * stream)
{
   stream->next = ctx->stream_list;
   ctx->stream_list = stream;
}

spdy_stream * spdy_find_stream (spdy_ctx * ctx, int32_t stream_id)
{
   spdy_stream * stream = ctx->stream_list;

   while (stream && stream->id != stream_id)
      stream = stream->next;

   return stream;
}

void spdy_stream_delete (spdy_ctx * ctx, spdy_stream * stream)
{
   spdy_stream * list_stream = ctx->stream_list;

   if (!stream)
      return;

   if (! (stream->flags & SPDY_STREAM_GONE))
   {
      if (list_stream == stream)
      {
         ctx->stream_list = 0;
      }
      else
      {
         while (list_stream->next != stream)
            list_stream = list_stream->next;

         list_stream->next = stream->next;
      }
   }

   if (stream->ref_count > 0)
   {
      stream->flags |= SPDY_STREAM_GONE;
      return;
   }

   if (stream->assoc && (stream->assoc->flags & SPDY_STREAM_GONE))
   {
      if ((-- stream->assoc->ref_count) == 0)
         spdy_stream_delete (ctx, stream->assoc);

      stream->assoc = 0;
   }

   free (stream);
}

