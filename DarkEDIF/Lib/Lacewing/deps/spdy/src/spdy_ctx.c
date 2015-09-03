
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
#include "spdy_stream.h"
#include "spdy_bytes.h"
#include "spdy_control.h"
#include "spdy_zlib.h"

#ifndef _WIN32
   #include <stdlib.h>
#else
   #include <malloc.h>
#endif

#include <stdarg.h>

spdy_ctx * spdy_ctx_new (const spdy_config * config, int version,
                         int num_persisted_settings,
                         spdy_setting * persisted_settings)
{
   spdy_ctx * ctx = (spdy_ctx *) calloc (sizeof (spdy_ctx), 1);

   if (!ctx)
      return 0;

   ctx->config = config;

   ctx->init_version = version ? version : 2;

   if (! (ctx->error_threshold = ctx->config->error_threshold))
      ctx->error_threshold = 10;

   ctx->next_stream_id = config->is_server ? 2 : 1;

   if ( (! config->is_server) && num_persisted_settings > 0)
   {
      spdy_emit_settings
         (ctx, 0, 1, num_persisted_settings, persisted_settings);
   }

   return ctx;
}

void spdy_ctx_delete (spdy_ctx * ctx)
{
   spdy_stream * cur_stream, * next_stream;

   if (!ctx)
      return;

   cur_stream = ctx->stream_list;
   ctx->stream_list = 0;

   while (cur_stream)
   {
      next_stream = cur_stream->next;
      free (cur_stream);
      cur_stream = next_stream;
   }

   free (ctx);
}

void spdy_ctx_set_tag (spdy_ctx * ctx, void * tag)
{
   ctx->tag = tag;
}

void * spdy_ctx_get_tag (spdy_ctx * ctx)
{
   return ctx->tag;
}

static int spdy_proc_data (spdy_ctx * ctx, spdy_buffer * buffer)
{
   int ret;
   int32_t stream_id;
   spdy_stream * stream;
   int16_t version;
   spdy_buffer frame;
   size_t data_size;

   while (buffer->size > 0)
   {
      if (! (ctx->flags & SPDY_GOT_HEADER))
      {
         if (buffer->size < 8)
            return SPDY_E_OK;

         ctx->flags |= SPDY_GOT_HEADER;

         if (*buffer->ptr & 0x80) /* check control bit */
         {
            version = spdy_read_int15 (buffer);

            if (ctx->flags & SPDY_GOT_VERSION)
            {
               if (version != ctx->version)
                  return SPDY_E_VERSION_MISMATCH;
            }
            else
            {
               if (version != 2 && version != 3)
                  return SPDY_E_VERSION_MISMATCH;

               if ((ret = spdy_set_version (ctx, version)) != SPDY_E_OK)
                  return ret;
            }

            ctx->frame.control.type = spdy_read_int16 (buffer);
            ctx->frame_flags = spdy_read_int8 (buffer);
         }
         else
         {
            ctx->flags |= SPDY_DATA_FRAME;

            stream_id = spdy_read_int31 (buffer);

            if (!(stream = spdy_find_stream (ctx, stream_id)))
            {
               spdy_emit_rst_stream
                  (ctx, 0, stream_id, SPDY_STATUS_INVALID_STREAM);

               if ((ret = spdy_soft_error (ctx)) != SPDY_E_OK)
                  return ret;
            }
            else if (stream->flags & SPDY_STREAM_CLOSED_REMOTE)
            {
               /* Stream is already half-closed from the sender */

               spdy_stream_close (stream, ctx->version == 2 ?
                                    SPDY_STATUS_PROTOCOL_ERROR :
                                    SPDY_STATUS_STREAM_ALREADY_CLOSED);
            }
            else
            {
               ctx->frame.data.stream = stream;
            }

            ctx->frame_flags = spdy_read_int8 (buffer);
         }

         ctx->frame_length = spdy_read_int24 (buffer);
      }

      ret = SPDY_E_OK;

      if (ctx->flags & SPDY_DATA_FRAME)
      {
         data_size = buffer->size < ctx->frame_length ?
                        buffer->size : ctx->frame_length;

         if (ctx->frame.data.stream && ctx->config->on_stream_data)
         {
            ctx->config->on_stream_data
               (ctx, ctx->frame.data.stream, buffer->ptr, data_size);
         }

         if (ctx->frame_flags & SPDY_FLAG_FIN)
         {
           stream->flags |= SPDY_STREAM_CLOSED_REMOTE;

           if ((stream->flags & SPDY_STREAM_CLOSED_HERE) &&
               (stream->flags & SPDY_STREAM_CLOSED_REMOTE))
             {

               if (ctx->frame.data.stream && ctx->config->on_stream_close)
               {
                 ctx->config->on_stream_close(ctx, ctx->frame.data.stream, 0);
               }

               spdy_stream_delete (ctx, stream);
             }
         }

         buffer->ptr += data_size;
         buffer->size -= data_size;

         if (ctx->frame_length > 0)
            return SPDY_E_OK; /* still more of this data frame to go */
      }
      else
      {
         if (buffer->size < ctx->frame_length)
            return SPDY_E_OK; /* not enough data */

         frame.ptr = buffer->ptr;
         frame.size = ctx->frame_length;

         switch (ctx->frame.control.type)
         {
         case SYN_STREAM:
            ret = spdy_proc_syn_stream
               (ctx, ctx->frame_flags, &frame);
            break;
         case SYN_REPLY:
            ret = spdy_proc_syn_reply
               (ctx, ctx->frame_flags, &frame);
            break;
         case RST_STREAM:
            ret = spdy_proc_rst_stream
               (ctx, ctx->frame_flags, &frame);
            break;
         case SETTINGS:
            ret = spdy_proc_settings
               (ctx, ctx->frame_flags, &frame);
            break;
         case NOOP:
            break; /* nothing to do; removed in draft 3 */
         case PING:
            ret = spdy_proc_ping
               (ctx, ctx->frame_flags, &frame);
            break;
         case GOAWAY:
            ret = spdy_proc_goaway
               (ctx, ctx->frame_flags, &frame);
            break;
         case HEADERS:
            ret = spdy_proc_headers
               (ctx, ctx->frame_flags, &frame);
            break;
         case WINDOW_UPDATE:
            ret = spdy_proc_window_update
               (ctx, ctx->frame_flags, &frame);
            break;
         case CREDENTIAL:
            ret = spdy_proc_credential
               (ctx, ctx->frame_flags, &frame);
            break;
         };

         if (ret == SPDY_E_OK && frame.size > 0)
            ret = SPDY_E_PROTOCOL; /* unprocessed data left over in the frame */
      }

      buffer->ptr += ctx->frame_length;
      buffer->size -= ctx->frame_length;

      /* clear flags for the next frame */
      ctx->flags &= ~ (SPDY_GOT_HEADER | SPDY_DATA_FRAME);

      if (ret != SPDY_E_OK)
         return ret;
   }

   /* Session closed and no streams left? */

   if ( (ctx->flags & SPDY_SESSION_CLOSED) && !ctx->stream_list)
      return SPDY_E_SESSION_CLOSED;
      
   return ret;
}

int spdy_data (spdy_ctx * ctx, const char * buffer, size_t * bytes)
{
   spdy_buffer sbuf;
   int res;

   sbuf.ptr = buffer;
   sbuf.size = *bytes;
  
   res = spdy_proc_data (ctx, &sbuf);

   *bytes -= sbuf.size;

   return res;
}

int spdy_set_version (spdy_ctx * ctx, int version)
{
   int res;

   if (ctx->flags & SPDY_GOT_VERSION)
      return SPDY_E_OK;

   ctx->version = version;
   ctx->flags |= SPDY_GOT_VERSION;

   /* Now we know which dictionary to use for deflate, init the zlib streams */

   if (inflateInit (&ctx->zlib_inflate) != Z_OK)
      return SPDY_E_INFLATE;

   if (deflateInit (&ctx->zlib_deflate, Z_BEST_SPEED) != Z_OK)
      return SPDY_E_DEFLATE;

   if ((res = spdy_set_deflate_dict (ctx, &ctx->zlib_deflate)) != SPDY_E_OK)
   {
      return res;
   }

   return SPDY_E_OK;
}

int spdy_active_version (spdy_ctx * ctx)
{
   return ctx->version;
}

int spdy_soft_error (spdy_ctx * ctx)
{
   if (ctx->config->error_threshold == -1)
      return SPDY_E_OK;

   if (ctx->error_threshold <= 0)
      return SPDY_E_THRESHOLD;

   -- ctx->error_threshold;

   return SPDY_E_OK;
}

void spdy_emitv (spdy_ctx * ctx, size_t num, ...)
{
   spdy_iovec * v;
   va_list list;
   size_t i = 0;
   char * emit_buffer;
   size_t emit_size;
  
   if (ctx->config->emitv)
   {
      /* emitv is available - build iovec list */

      v = (spdy_iovec *) alloca (sizeof (spdy_iovec *) * num);

      va_start (list, num);

      while (i < num)
      {
         v [i].iov_base = va_arg (list, void *);
         v [i].iov_len = va_arg (list, size_t);

         ++ i;
      }

      va_end (list);

      ctx->config->emitv (ctx, num, v);

      return;
   }

   /* emitv not implemented - pass each buffer to emit callback */

   va_start (list, num);

   while (i < num)
   {
      emit_buffer = va_arg (list, char *);
      emit_size = va_arg (list, size_t);

      ctx->config->emit (ctx, emit_buffer, emit_size);

      ++ i;
   }

   va_end (list);
}

