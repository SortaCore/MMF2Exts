
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
#include "spdy_stream.h"
#include "spdy_control.h"
#include "spdy_nv_block.h"

int spdy_proc_syn_stream (spdy_ctx * ctx, int8_t flags, spdy_buffer * buffer)
{
   int32_t stream_id, assoc_id;
   spdy_stream * stream;
   size_t header_count;
   spdy_nv_pair * headers;
   int res;

   if ((ctx->version == 2 && buffer->size < 10) || buffer->size < 12)
      return SPDY_E_PROTOCOL; /* not big enough to be a SYN_STREAM */

   if (ctx->flags & SPDY_SESSION_CLOSED)
   {
      /* Any SYN_STREAM frames received after a GOAWAY should be ignored */

      buffer->size = 0; /* pretend we read everything */
      return SPDY_E_OK;
   }
      
   stream_id = spdy_read_int31 (buffer);

   if (stream_id == 0 || stream_id <= ctx->last_remote_stream_id
         || (ctx->config->is_server && (stream_id % 2) == 0)
         || ((!ctx->config->is_server) && (stream_id % 2) != 0))
   {
      /* Stream ID not in sequence (soft error) */

      spdy_emit_rst_stream (ctx, 0, stream_id, SPDY_STATUS_INVALID_STREAM);
      return spdy_soft_error (ctx);
   }

   ctx->last_remote_stream_id = stream_id;

   if (! (stream = (spdy_stream *) malloc (sizeof (spdy_stream))))
      return SPDY_E_MEM;

   stream->ctx = ctx;
   stream->id = stream_id;
   stream->flags = 0;
   stream->ref_count = 1;  /* this proc counts as a ref */

   if (flags & SPDY_FLAG_FIN)
      stream->flags |= SPDY_STREAM_CLOSED_REMOTE;

   if (flags & SPDY_FLAG_UNIDIRECTIONAL)
      stream->flags |= SPDY_STREAM_CLOSED_HERE;

   if ((assoc_id = spdy_read_int31 (buffer)) == 0)
      stream->assoc = 0;
   else
   {
      if (! (stream->assoc = spdy_find_stream (ctx, assoc_id)))
      {
         spdy_emit_rst_stream
            (ctx, 0, stream->id, SPDY_STATUS_INVALID_STREAM);

         free (stream);

         return spdy_soft_error (ctx);
      }

      ++ stream->assoc->ref_count;
   }

   if (ctx->version == 2)
      stream->priority = (spdy_read_int8 (buffer) & 0xC0) >> 6;
   else
      stream->priority = (spdy_read_int8 (buffer) & 0xE0) >> 5;

   stream->cred_slot = spdy_read_int8 (buffer);
   
   if ((res = spdy_read_nv_block
         (ctx, buffer, &headers, &header_count)) != SPDY_E_OK)
   {
      free (stream);
      return res;
   }

   spdy_link_stream (ctx, stream);

   ctx->config->on_stream_create (ctx, stream, header_count, headers);

   spdy_nv_free (header_count, headers);

   -- stream->ref_count;

   if (stream->flags & SPDY_STREAM_GONE)
   {
      spdy_stream_delete (ctx, stream);

      return SPDY_E_OK;
   }

   /* Is the stream already closed at both ends? */

   if ((stream->flags & SPDY_STREAM_CLOSED_HERE)
       && (stream->flags & SPDY_STREAM_CLOSED_REMOTE))
   {
      ctx->config->on_stream_close (ctx, stream, 0);
      spdy_stream_delete (ctx, stream);

      return SPDY_E_OK;
   }

   return SPDY_E_OK;
}

int spdy_emit_syn_stream (spdy_ctx * ctx, int8_t flags, int32_t stream_id,
                          int32_t assoc_stream_id, int8_t priority, int8_t slot,
                          size_t num_headers, spdy_nv_pair * headers)
{
   int res;
   char * nv_buffer;
   size_t nv_size;
   char header [SPDY_CTRL_HEADER_SIZE + 10];

   if ((res = spdy_pack_nv_block
            (ctx, &nv_buffer, &nv_size, headers, num_headers) != SPDY_E_OK))
   {
      return res;
   }

   spdy_build_control_header
      (ctx, header, SYN_STREAM, flags, sizeof (header) - SPDY_CTRL_HEADER_SIZE + nv_size);

   spdy_write_int31 (header + SPDY_CTRL_HEADER_SIZE, stream_id);
   spdy_write_int31 (header + SPDY_CTRL_HEADER_SIZE + 4, assoc_stream_id);

   header [SPDY_CTRL_HEADER_SIZE + 8] = priority << 5;
   header [SPDY_CTRL_HEADER_SIZE + 9] = slot; /* present but not defined in v2 */

   spdy_emitv (ctx, 2, header, sizeof (header),
                       nv_buffer, nv_size);

   free (nv_buffer);

   return SPDY_E_OK;
}

