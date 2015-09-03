
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

int spdy_proc_ping (spdy_ctx * ctx, int8_t flags, spdy_buffer * buffer)
{
   int32_t id;

   if (buffer->size != 4)
      return SPDY_E_PROTOCOL; /* ping frames are always 4 bytes long */

   id = spdy_read_int32 (buffer);

   if (ctx->config->is_server)
   {   if ((id % 2) == 0) /* even ID sent by a client should be ignored */
         return SPDY_E_OK;
   }
   else
   {   if ((id % 2) != 0) /* odd ID sent by the server should be ignored */
         return SPDY_E_OK;
   }

   spdy_emit_ping (ctx, 0, id);

   return SPDY_E_OK;
}

void spdy_emit_ping (spdy_ctx * ctx, int8_t flags, int32_t id)
{
   char message [SPDY_CTRL_HEADER_SIZE + sizeof (id)];

   spdy_build_control_header (ctx, message, PING, flags, sizeof (id));
   spdy_write_int32 (message + SPDY_CTRL_HEADER_SIZE, id);

   ctx->config->emit (ctx, message, sizeof (message));
}

