
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
#include "spdy_nv_block.h"
#include "spdy_control.h"

int spdy_proc_syn_reply (spdy_ctx * ctx, int8_t flags, spdy_buffer * buffer)
{
   int32_t stream_id;
   spdy_stream * stream;
   size_t num_headers;
   spdy_nv_pair * headers;
   int res;

   if (buffer->size < 4)
      return SPDY_E_PROTOCOL;

   stream_id = spdy_read_int31 (buffer);

   if (!(stream = spdy_find_stream (ctx, stream_id)))
   {
      /* Unknown stream ID */

      spdy_emit_rst_stream (ctx, 0, stream_id, SPDY_STATUS_INVALID_STREAM);
      return spdy_soft_error (ctx);
   }

   if (!spdy_stream_is_ours (stream))
   {
      /* Received a SYN_REPLY for a stream we did not initiate */

      spdy_stream_close (stream, SPDY_STATUS_PROTOCOL_ERROR);
      return spdy_soft_error (ctx);
   }

   if (stream->flags & SPDY_STREAM_GOT_ACK)
   {
      /* Already received a SYN_REPLY for this stream */

      spdy_stream_close (stream, ctx->version == 2 ?
                            SPDY_STATUS_PROTOCOL_ERROR :
                            SPDY_STATUS_STREAM_IN_USE);

      return spdy_soft_error (ctx);
   }

   stream->flags |= SPDY_STREAM_GOT_ACK;

   if (ctx->version == 2)
   {
      if (buffer->size < 2)
         return SPDY_E_PROTOCOL;

      spdy_read_int16 (buffer); /* reserved; removed in draft 3 */
   }

   if ((res = spdy_read_nv_block
           (ctx, buffer, &headers, &num_headers)) != SPDY_E_OK)
   {
      free (stream);
      return res;
   }

   if (ctx->config->on_stream_create)
      ctx->config->on_stream_create (ctx, stream, num_headers, headers);

   spdy_nv_free (num_headers, headers);

   return SPDY_E_OK;
}

static int spdy_emit_syn_reply_draft3 (spdy_ctx * ctx, int8_t flags,
                                       int32_t stream_id, size_t num_headers,
                                       spdy_nv_pair * headers)
{
   int res;
   char * nv_buffer;
   size_t nv_size;
   char header [SPDY_CTRL_HEADER_SIZE + sizeof (stream_id)];

   if ((res = spdy_pack_nv_block
            (ctx, &nv_buffer, &nv_size, headers, num_headers) != SPDY_E_OK))
   {
      return res;
   }

   spdy_build_control_header (ctx, header, SYN_REPLY, flags,
         (sizeof (header) - SPDY_CTRL_HEADER_SIZE) + nv_size);

   spdy_write_int31 (header + SPDY_CTRL_HEADER_SIZE, stream_id);

   spdy_emitv (ctx, 2, header, sizeof(header),
                       nv_buffer, nv_size);

   free (nv_buffer);

   return SPDY_E_OK;
}

static int spdy_emit_syn_reply_draft2 (spdy_ctx * ctx, int8_t flags,
                                       int32_t stream_id, size_t num_headers,
                                       spdy_nv_pair * headers)
{
   int res;
   char * nv_buffer;
   size_t nv_size;
   char header [SPDY_CTRL_HEADER_SIZE + sizeof (stream_id) + 2];

   if ((res = spdy_pack_nv_block
            (ctx, &nv_buffer, &nv_size, headers, num_headers) != SPDY_E_OK))
   {
      return res;
   }

   spdy_build_control_header (ctx, header, SYN_REPLY, flags,
         (sizeof (header) - SPDY_CTRL_HEADER_SIZE) + nv_size);

   spdy_write_int31 (header + SPDY_CTRL_HEADER_SIZE, stream_id);

   spdy_write_int16
      (header + SPDY_CTRL_HEADER_SIZE + sizeof (stream_id), 0); /* reserved */
   
   spdy_emitv (ctx, 2, header, sizeof(header),
                       nv_buffer, nv_size);

   free (nv_buffer);

   return SPDY_E_OK;
}

int spdy_emit_syn_reply (spdy_ctx * ctx, int8_t flags, int32_t stream_id,
                         size_t num, spdy_nv_pair * headers)
{
   return ctx->version == 2 ?
             spdy_emit_syn_reply_draft2 (ctx, flags, stream_id, num, headers)
           : spdy_emit_syn_reply_draft3 (ctx, flags, stream_id, num, headers);
}

