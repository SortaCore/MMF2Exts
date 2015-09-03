
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
#include "../address.h"
#include "fdstream.h"

#define lw_client_flag_connecting  1
#define lw_client_flag_connected   2

struct _lw_client
{
   struct _lw_fdstream fdstream;

   lw_client_hook_connect     on_connect;
   lw_client_hook_disconnect  on_disconnect;
   lw_client_hook_data        on_data;
   lw_client_hook_error       on_error;

   char flags;

   lw_addr address;

   int socket;

   lw_pump pump;
   lw_pump_watch watch;
};

lw_client lw_client_new (lw_pump pump)
{
   lw_client ctx = calloc (sizeof (*ctx), 1);

   ctx->pump = pump;

   lwp_init ();

   lwp_fdstream_init (&ctx->fdstream, pump);

   return ctx;
}

void lw_client_delete (lw_client ctx)
{
   if (!ctx)
      return;

   lw_stream_close ((lw_stream) ctx, lw_true);

   free (ctx);
}

void lw_client_connect (lw_client ctx, const char * host, long port)
{
   lw_addr address = lw_addr_new_port (host, port);

   lw_client_connect_addr (ctx, address);
}

static void write_ready (void * tag)
{
   lw_client ctx = tag;

   assert (ctx->flags & lw_client_flag_connecting);

   int error;

   {  socklen_t error_len = sizeof (error);
      getsockopt (ctx->socket, SOL_SOCKET, SO_ERROR, &error, &error_len);
   }

   if (error != 0)
   {
      /* Failed to connect */

      ctx->flags &= ~ lw_client_flag_connecting;

      lw_error error = lw_error_new ();
      lw_error_addf (error, "Error connecting");

      if (ctx->on_error)
         ctx->on_error (ctx, error);

      lw_error_delete (error);

      return;
   }

   lw_fdstream_set_fd (&ctx->fdstream, ctx->socket, ctx->watch, lw_true);

   ctx->flags &= ~ lw_client_flag_connecting;

   if (ctx->on_connect)
      ctx->on_connect (ctx);

   if (ctx->on_data)
      lw_stream_read ((lw_stream) ctx, -1);
}

void lw_client_connect_addr (lw_client ctx, lw_addr address)
{
   if (lw_client_connected (ctx) || lw_client_connecting (ctx))
   {
      lw_error error = lw_error_new ();
      lw_error_addf (error, "Already connected to a server");

      if (ctx->on_error)
         ctx->on_error (ctx, error);

      lw_error_delete (error);

      return;
   }

   ctx->flags |= lw_client_flag_connecting;

   /* TODO : Resolve asynchronously? */

   {  lw_error error = lw_addr_resolve (address);

      if (error)
      {
         if (ctx->on_error)
            ctx->on_error (ctx, error);

         lw_error_delete (error);

         return;
      }
   }

   lw_addr_delete (ctx->address);
   ctx->address = lw_addr_clone (address);

   if ((ctx->socket = socket (lw_addr_ipv6 (address) ? AF_INET6 : AF_INET,
               SOCK_STREAM,
               IPPROTO_TCP)) == -1)
   {
      lw_error error = lw_error_new ();

      lw_error_add (error, errno);
      lw_error_addf (error, "Error creating socket");

      if (ctx->on_error)
         ctx->on_error (ctx, error);

      lw_error_delete (error);

      return;
   }

   if (!address->info)
   {
      lw_error error = lw_error_new ();
      lw_error_addf (error, "The provided Address object is not ready for use");

      if (ctx->on_error)
         ctx->on_error (ctx, error);

      lw_error_delete (error);

      return;
   }

   fcntl (ctx->socket, F_SETFL, fcntl (ctx->socket, F_GETFL, 0) | O_NONBLOCK);

   ctx->watch = lw_pump_add (ctx->pump, ctx->socket, ctx, 0, write_ready, lw_true);

   if (connect (ctx->socket, address->info->ai_addr,
            address->info->ai_addrlen) == -1)
   {
      if (errno == EINPROGRESS)
         return;

      ctx->flags &= ~ lw_client_flag_connecting;

      lw_error error = lw_error_new ();

      lw_error_add (error, errno);
      lw_error_addf (error, "The provided Address object is not ready for use");

      if (ctx->on_error)
         ctx->on_error (ctx, error);

      lw_error_delete (error);
   }
}

lw_bool lw_client_connected (lw_client ctx)
{
   return lw_fdstream_valid ((lw_fdstream) ctx);
}

lw_bool lw_client_connecting (lw_client ctx)
{
   return ctx->flags & lw_client_flag_connecting;
}

lw_addr lw_client_server_addr (lw_client ctx)
{
   return ctx->address;
}

static void on_stream_data (lw_stream stream, void * tag,
                            const char * buffer, size_t length)
{
   lw_client ctx = tag;

   ctx->on_data (ctx, buffer, length);
}

void lw_client_on_data (lw_client ctx, lw_client_hook_data on_data)
{
   ctx->on_data = on_data;

   if (on_data)
   {
      lw_stream_add_hook_data ((lw_stream) ctx, on_stream_data, ctx);
      lw_stream_read ((lw_stream) ctx, -1);
   }
   else
   {
      lw_stream_remove_hook_data ((lw_stream) ctx, on_stream_data, ctx);
   }
}

static void on_close (lw_stream stream, void * tag)
{
   lw_client ctx = tag;

   ctx->on_disconnect (ctx);
}

void lw_client_on_disconnect (lw_client ctx,
                              lw_client_hook_disconnect on_disconnect)
{
   ctx->on_disconnect = on_disconnect;

   if (on_disconnect)
      lw_stream_add_hook_close ((lw_stream) ctx, on_close, ctx);
   else
      lw_stream_remove_hook_close ((lw_stream) ctx, on_close, ctx);
}

lwp_def_hook (client, connect)
lwp_def_hook (client, error)

