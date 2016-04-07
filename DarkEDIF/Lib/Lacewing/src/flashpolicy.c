
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin.  All rights reserved.
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

#include "common.h"

struct _lw_flashpolicy
{
   lw_server server;

   char * buffer;
   size_t size;

   lw_flashpolicy_hook_error on_error;

   void * tag;
};

static void on_data (lw_server server, lw_server_client client,
                     const char * buffer, size_t size)
{
   lw_flashpolicy ctx = (lw_flashpolicy) lw_server_tag (server);

   for (size_t i = 0; i < size; ++ i)
   {
      if (!buffer [i])
      {
         lw_stream_write ((lw_stream) client, ctx->buffer, ctx->size);
         lw_stream_write ((lw_stream) client, "\0", 1);

         return;
      }
   }
}

static void on_error (lw_server server, lw_error error)
{
   lw_flashpolicy ctx = (lw_flashpolicy) lw_server_tag (server);

   lw_error_addf (error, "Socket error");

   if (ctx->on_error)
      ctx->on_error (ctx, error);
}

lw_flashpolicy lw_flashpolicy_new (lw_pump pump)
{
   lw_flashpolicy ctx = (lw_flashpolicy) calloc (sizeof (*ctx), 1);

   ctx->server = lw_server_new (pump);

   lw_server_on_error (ctx->server, on_error);
   lw_server_on_data (ctx->server, on_data);

   return ctx;
}

void lw_flashpolicy_delete (lw_flashpolicy ctx)
{
   if (!ctx)
      return;

   lw_server_delete (ctx->server);

   free (ctx->buffer);
   free (ctx);
}

void lw_flashpolicy_host (lw_flashpolicy ctx, const char * filename)
{
   lw_filter filter = lw_filter_new ();
   lw_flashpolicy_host_filter (ctx, filename, filter);
   lw_filter_delete (filter);
}

void lw_flashpolicy_host_filter (lw_flashpolicy ctx, const char * filename,
                                 lw_filter filter)
{
   lw_flashpolicy_unhost (ctx);

   lw_filter_set_local_port (filter, 843);
    
   FILE * file = fopen (filename, "r");

   if (!file)
   {
      lw_error error = lw_error_new ();

      lw_error_add (error, lwp_last_socket_error);
      lw_error_addf (error, "Error opening file: %s", filename);

      if (ctx->on_error)
         ctx->on_error (ctx, error);

      return;
   }

   fseek (file, 0, SEEK_END);

   ctx->size = ftell (file);
   ctx->buffer = (char *) malloc (ctx->size);

   fseek (file, 0, SEEK_SET);

   size_t bytes = fread (ctx->buffer, 1, ctx->size, file);

   if (bytes != ctx->size)
   {
      ctx->size = bytes;

      if (ferror (file))
      {
         lw_error error = lw_error_new ();

         lw_error_add (error, lwp_last_error);
         lw_error_addf (error, "Error reading file: %s", filename);

         if (ctx->on_error)
            ctx->on_error (ctx, error);

         free (ctx->buffer);
         ctx->buffer = 0;

         fclose (file);

         return;
      }
   }

   fclose (file);

   lw_server_host_filter (ctx->server, filter);
}

void lw_flashpolicy_unhost (lw_flashpolicy ctx)
{
   lw_server_unhost (ctx->server);

   free (ctx->buffer);
   ctx->buffer = 0;
}

lw_bool lw_flashpolicy_hosting (lw_flashpolicy ctx)
{
   return lw_server_hosting (ctx->server);
}

void lw_flashpolicy_set_tag (lw_flashpolicy ctx, void * tag)
{
   ctx->tag = tag;
}

void * lw_flashpolicy_tag (lw_flashpolicy ctx)
{
   return ctx->tag;
}

lwp_def_hook (flashpolicy, error);

