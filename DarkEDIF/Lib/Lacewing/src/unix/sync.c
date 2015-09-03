
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.  All rights reserved.
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

struct _lw_sync
{
    pthread_mutex_t mutex;
    pthread_mutexattr_t attr;
};

lw_sync lw_sync_new ()
{
   lw_sync ctx = malloc (sizeof (*ctx));

   if (!ctx)
      return 0;

   pthread_mutexattr_init (&ctx->attr);
   pthread_mutexattr_settype (&ctx->attr, PTHREAD_MUTEX_RECURSIVE);

   pthread_mutex_init (&ctx->mutex, &ctx->attr);

   return ctx;
}

void lw_sync_delete (lw_sync ctx)
{
   pthread_mutex_destroy (&ctx->mutex);
   pthread_mutexattr_destroy (&ctx->attr);
}

void lw_sync_lock (lw_sync ctx)
{
    pthread_mutex_lock (&ctx->mutex);
}

void lw_sync_release (lw_sync ctx)
{
    pthread_mutex_unlock (&ctx->mutex);
}

