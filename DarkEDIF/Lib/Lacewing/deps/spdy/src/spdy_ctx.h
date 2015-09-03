
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

#ifndef SPDY_CONN_H
#define SPDY_CONN_H

#include "../include/spdy.h"

#include <zlib.h>

#include <stdlib.h>
#include <string.h>

#ifndef _MSC_VER
    #include <stdint.h>
#else
    typedef __int8 int8_t;
    typedef __int16 int16_t;
    typedef __int32 int32_t;
#endif

#define SPDY_GOT_HEADER      1
#define SPDY_DATA_FRAME      2
#define SPDY_GOT_VERSION     4
#define SPDY_SESSION_CLOSED  8

struct _spdy_ctx
{
   const spdy_config * config;

   void * tag;

   char flags;

   int init_version;
   int version;

   int error_threshold;

   int32_t last_remote_stream_id;
   int32_t next_stream_id;

   z_stream zlib_inflate;
   z_stream zlib_deflate;

   size_t frame_length;
   int8_t frame_flags;

   union
   {
      struct
      {
         int16_t type;

      } control;

      struct
      {
         spdy_stream * stream;

      } data;

   } frame;

   spdy_stream * stream_list;
};

int spdy_set_version (spdy_ctx * ctx, int version);
int spdy_soft_error (spdy_ctx *);
void spdy_emitv (spdy_ctx *, size_t num, ...);

#endif

