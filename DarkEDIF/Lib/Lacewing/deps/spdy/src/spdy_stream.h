
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

#ifndef SPDY_STREAM_H
#define SPDY_STREAM_H

#define SPDY_STREAM_SENT_ACK        1   /* sent a SYN_REPLY? */
#define SPDY_STREAM_GOT_ACK         2   /* received a SYN_REPLY? */
#define SPDY_STREAM_CLOSED_HERE     4   /* half-closed on our end? */
#define SPDY_STREAM_CLOSED_REMOTE   8   /* half-closed on their end? */
#define SPDY_STREAM_GONE            16  /* fully closed but not yet freed */


struct _spdy_stream
{
   spdy_ctx * ctx;

   void * tag;

   int32_t id;

   struct _spdy_stream * next;
   struct _spdy_stream * assoc;

   int ref_count;

   char flags;
   char priority;
   char cred_slot;
};

void spdy_link_stream (spdy_ctx *, spdy_stream *);
spdy_stream * spdy_find_stream (spdy_ctx *, int32_t stream_id);

void spdy_stream_delete (spdy_ctx * ctx, spdy_stream * stream);
void spdy_stream_free (spdy_ctx * ctx, spdy_stream * stream);

#endif


