
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

#ifndef SPDY_BUFFER_H
#define SPDY_BUFFER_H

/* for htons/htonl */

#ifdef _WIN32
   #include <winsock2.h>
#else
   #include <arpa/inet.h>
#endif

typedef struct _spdy_buffer
{
   const char * ptr;
   size_t size;

} spdy_buffer;

#define SPDY_INT24_MAX  ((size_t) 0xFFFFFF)

int8_t spdy_read_int8 (spdy_buffer *);
int16_t spdy_read_int15 (spdy_buffer *);
int16_t spdy_read_int16 (spdy_buffer *);
int32_t spdy_read_int24 (spdy_buffer *);
int32_t spdy_read_int31 (spdy_buffer *);
int32_t spdy_read_int32 (spdy_buffer *);

void spdy_write_int15 (char * ptr, int16_t);
void spdy_write_int16 (char * ptr, int16_t);
void spdy_write_int24 (char * ptr, int32_t);
void spdy_write_int31 (char * ptr, int32_t);
void spdy_write_int32 (char * ptr, int32_t);

void spdy_emit_int15 (spdy_ctx *, int16_t);
void spdy_emit_int16 (spdy_ctx *, int16_t);
void spdy_emit_int24 (spdy_ctx *, int32_t);
void spdy_emit_int31 (spdy_ctx *, int32_t);
void spdy_emit_int32 (spdy_ctx *, int32_t);

/* A bug in the initial draft 2 implementation means some of the 24-bit values
 * are incorrectly little-endian.
 */

int32_t spdy_read_int24_le (spdy_buffer *);
void spdy_write_int24_le (char * ptr, int32_t);
void spdy_emit_int24_le (spdy_ctx *, int32_t);

#endif

