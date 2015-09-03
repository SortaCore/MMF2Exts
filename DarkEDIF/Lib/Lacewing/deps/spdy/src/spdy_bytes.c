
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

#include <assert.h>

int8_t spdy_read_int8 (spdy_buffer * buffer)
{
   assert (buffer->size >= 1);
   -- buffer->size;

   return *buffer->ptr ++;
}

int16_t spdy_read_int15 (spdy_buffer * buffer)
{
   int16_t value;

   assert (buffer->size >= 2);
   buffer->size -= 2;

   value = *(int16_t *) buffer->ptr;
   *(int8_t *) &value &= 0x7F;

   buffer->ptr += 2;

   return ntohs (value);
}

int16_t spdy_read_int16 (spdy_buffer * buffer)
{
   int16_t value;

   assert (buffer->size >= 2);
   buffer->size -= 2;

   value = ntohs (*(int16_t *) buffer->ptr);
   buffer->ptr += 2;
   return value;
}

int32_t spdy_read_int24 (spdy_buffer * buffer)
{
   int32_t value;

   assert (buffer->size >= 3);
   buffer->size -= 3;

   ((char *) &value) [0] = 0;
   ((char *) &value) [1] = buffer->ptr [0];
   ((char *) &value) [2] = buffer->ptr [1];
   ((char *) &value) [3] = buffer->ptr [2];

   buffer->ptr += 3;

   return ntohl (value);
}

int32_t spdy_read_int24_le (spdy_buffer * buffer)
{
   int32_t value;

   assert (buffer->size >= 3);
   buffer->size -= 3;

   ((char *) &value) [0] = buffer->ptr [2];
   ((char *) &value) [1] = buffer->ptr [1];
   ((char *) &value) [2] = buffer->ptr [0];
   ((char *) &value) [3] = 0;

   buffer->ptr += 3;

   return ntohl (value);
}

int32_t spdy_read_int31 (spdy_buffer * buffer)
{
   int32_t value;

   assert (buffer->size >= 4);
   buffer->size -= 4;

   value = *(int32_t *) buffer->ptr;
   *(int8_t *) &value &= 0x7F;

   buffer->ptr += 4;

   return ntohl (value);
}

int32_t spdy_read_int32 (spdy_buffer * buffer)
{
   int32_t value;

   assert (buffer->size >= 4);
   buffer->size -= 4;

   value = ntohl (*(int32_t *) buffer->ptr);
   buffer->ptr += 4;
   return value;
}

void spdy_write_int15 (char * ptr, int16_t value)
{
   value = htons (value);

   *(int8_t *) &value &= 0x7F;
   *(int16_t *) ptr = value;
}

void spdy_write_int16 (char * ptr, int16_t value)
{
   *(int16_t *) ptr = htons (value);
}

void spdy_write_int24 (char * ptr, int32_t value)
{
   value = htonl (value);

   ptr [0] = ((char *) &value) [1];
   ptr [1] = ((char *) &value) [2];
   ptr [2] = ((char *) &value) [3];
}

void spdy_write_int24_le (char * ptr, int32_t value)
{
   value = htonl (value);

   ptr [0] = ((char *) &value) [2];
   ptr [1] = ((char *) &value) [1];
   ptr [2] = ((char *) &value) [0];
}

void spdy_write_int31 (char * ptr, int32_t value)
{
   value = htonl (value);
   *(int8_t *) &value &= 0x7F;
   *(int32_t *) ptr = value;
}

void spdy_write_int32 (char * ptr, int32_t value)
{
   *(int32_t *) ptr = htonl (value);
}

void spdy_emit_int15 (spdy_ctx * ctx, int16_t value)
{
   spdy_write_int15 ((char *) &value, value);
   ctx->config->emit (ctx, (char *) &value, 2);
}

void spdy_emit_int16 (spdy_ctx * ctx, int16_t value)
{
   spdy_write_int16 ((char *) &value, value);
   ctx->config->emit (ctx, (char *) &value, 2);
}

void spdy_emit_int24 (spdy_ctx * ctx, int32_t value)
{
   spdy_write_int24 ((char *) &value, value);
   ctx->config->emit (ctx, (char *) &value, 3);
}

void spdy_emit_int24_le (spdy_ctx * ctx, int32_t value)
{
   spdy_write_int24_le ((char *) &value, value);
   ctx->config->emit (ctx, (char *) &value, 3);
}

void spdy_emit_int31 (spdy_ctx * ctx, int32_t value)
{
   spdy_write_int31 ((char *) &value, value);
   ctx->config->emit (ctx, (char *) &value, 4);
}

void spdy_emit_int32 (spdy_ctx * ctx, int32_t value)
{
   spdy_write_int32 ((char *) &value, value);
   ctx->config->emit (ctx, (char *) &value, 4);
}



