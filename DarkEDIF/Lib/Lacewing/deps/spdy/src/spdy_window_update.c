
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

int spdy_proc_window_update (spdy_ctx * ctx, int8_t flags, spdy_buffer * buffer)
{
   spdy_stream * stream;
   int32_t delta_window_size;

   if (buffer->size != 8)
      return SPDY_E_PROTOCOL;

   if (!(stream = spdy_find_stream (ctx, spdy_read_int31 (buffer))))
   {
      /* Unknown stream ID */

      return spdy_soft_error (ctx);
   }

   delta_window_size = spdy_read_int31 (buffer);

   /* TODO */

   return SPDY_E_OK;
}

int spdy_emit_window_update (spdy_ctx * ctx, int8_t flags,
                             int32_t stream_id, int32_t delta_window_size)
{
   char message [SPDY_CTRL_HEADER_SIZE
                    + sizeof (stream_id)
                    + sizeof (delta_window_size)];

   spdy_build_control_header (ctx, message, WINDOW_UPDATE, flags,
                              sizeof (message) - SPDY_CTRL_HEADER_SIZE);

   spdy_write_int31 (message + SPDY_CTRL_HEADER_SIZE, stream_id);
   spdy_write_int31 (message + SPDY_CTRL_HEADER_SIZE + 4, delta_window_size);

   ctx->config->emit (ctx, message, sizeof (message));

   return SPDY_E_OK;
}


