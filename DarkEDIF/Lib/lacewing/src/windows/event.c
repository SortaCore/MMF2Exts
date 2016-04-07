/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin et al.  All rights reserved.
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

#include "../common.h"

struct _lw_event
{
   WSAEVENT event;

   void * tag;
};

lw_event lw_event_new ()
{
   lwp_init ();

   lw_event ctx = (lw_event) calloc (sizeof (*ctx), 1);

   if (!ctx)
      return 0;

   ctx->event = WSACreateEvent ();

   return ctx;
}

void lw_event_delete (lw_event ctx)
{
   if (!ctx)
      return;

   WSACloseEvent (ctx->event);

   free (ctx);
}

lw_bool lw_event_signalled (lw_event ctx)
{
   return WSAWaitForMultipleEvents
   (
      1,
      &ctx->event,
      lw_true,
      0,
      lw_false

   ) == WAIT_OBJECT_0;
}

void lw_event_signal (lw_event ctx)
{
   WSASetEvent (ctx->event);
}

void lw_event_unsignal (lw_event ctx)
{
   WSAResetEvent (ctx->event);
}

lw_bool lw_event_wait (lw_event ctx, long timeout)
{
   return WSAWaitForMultipleEvents
   (
      1,
      &ctx->event,
      lw_true,
      timeout == -1 ? INFINITE : timeout,
      lw_false

   ) == WAIT_OBJECT_0;
}

void lw_event_set_tag (lw_event ctx, void * tag)
{
   ctx->tag = tag;
}

void * lw_event_tag (lw_event ctx)
{
   return ctx->tag;
}

