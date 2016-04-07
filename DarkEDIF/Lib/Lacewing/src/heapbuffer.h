
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

#ifndef _lw_heap_buffer_h
#define _lw_heap_buffer_h

typedef struct _lwp_heapbuffer
{
    size_t length, allocated, offset;
    char buffer [1];

} * lwp_heapbuffer;

lw_bool lwp_heapbuffer_add (lwp_heapbuffer *, const char * buffer, size_t length);
void lwp_heapbuffer_addf (lwp_heapbuffer *, const char * format, ...);

void lwp_heapbuffer_trim_left (lwp_heapbuffer *, size_t);
void lwp_heapbuffer_trim_right (lwp_heapbuffer *, size_t);

void lwp_heapbuffer_reset (lwp_heapbuffer *);
size_t lwp_heapbuffer_length (lwp_heapbuffer *);

char * lwp_heapbuffer_buffer (lwp_heapbuffer *);

void lwp_heapbuffer_free (lwp_heapbuffer *);

#endif

