
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
#include "spdy_nv_block.h"
#include "spdy_zlib.h"

static int spdy_inflate (spdy_ctx * ctx, z_stream * stream, Bytef * dest,
                         size_t size)
{
   int res;

   stream->next_out = dest;
   stream->avail_out = size;

   res = inflate (stream, Z_SYNC_FLUSH);
   
   if (res == Z_NEED_DICT)
   {
      if ((res = spdy_set_inflate_dict (ctx, stream)) != SPDY_E_OK)
         return Z_NEED_DICT;

      return spdy_inflate (ctx, stream, dest, size);
   }

   if (res == Z_OK && stream->avail_out > 0)
      return Z_DATA_ERROR;
   
   return res;
}

int spdy_read_nv_block (spdy_ctx * ctx, spdy_buffer * buffer,
                        spdy_nv_pair ** pairs, size_t * count)
{
   int16_t pair_count_16, name_len_16, value_len_16;
   int32_t pair_count_32, name_len_32, value_len_32;
   z_stream * stream;
   spdy_nv_pair * pair;
   int i = 0;
   int res = Z_OK;

   *pairs = 0;

   stream = &ctx->zlib_inflate;

   stream->next_in = (Bytef *) buffer->ptr;
   stream->avail_in = buffer->size;

   if (ctx->version == 2)
   {
      if ((res = spdy_inflate
               (ctx, stream, (Bytef *) &pair_count_16, 2)) != Z_OK)
      {
         goto e_inflate;
      }

      *count = htons (pair_count_16);
   }
   else
   {
      if ((res = spdy_inflate
               (ctx, stream, (Bytef *) &pair_count_32, 4)) != Z_OK)
      {
         goto e_inflate;
      }

      *count = htonl (pair_count_32);
   }

   if (! (*pairs = calloc (sizeof (spdy_nv_pair) * (*count), 1)))
      goto e_mem;

   while (i < *count)
   {
      pair = & (*pairs) [i];

      /* Name length */

      if (ctx->version == 2)
      {
         if ((res = spdy_inflate
                  (ctx, stream, (Bytef *) &name_len_16, 2)) != Z_OK)
         {
            goto e_inflate;
         }

         pair->name_len = ntohs (name_len_16);
      }
      else
      {
         if ((res = spdy_inflate
                  (ctx, stream, (Bytef *) &name_len_32, 4)) != Z_OK)
         {
            goto e_inflate;
         }

         pair->name_len = ntohl (name_len_32);
      }

      /* Name */

      if (! (pair->name = (char *) malloc (pair->name_len + 1)))
         goto e_mem;

      if ((res = spdy_inflate
               (ctx, stream, (Bytef *) pair->name, pair->name_len)) != Z_OK)
      {
         goto e_inflate;
      }

      pair->name [pair->name_len] = 0;
      
      /* Value length */

      if (ctx->version == 2)
      {
         if ((res = spdy_inflate
                  (ctx, stream, (Bytef *) &value_len_16, 2)) != Z_OK)
         {
            goto e_inflate;
         }

         pair->value_len = ntohs (value_len_16);
      }
      else
      {
         if ((res = spdy_inflate
                  (ctx, stream, (Bytef *) &value_len_32, 4)) != Z_OK)
         {
            goto e_inflate;
         }

         pair->value_len = ntohl (value_len_32);
      }

      /* Value */

      if (! (pair->value = (char *) malloc (pair->value_len + 1)))
         goto e_mem;

      if ((res = spdy_inflate
               (ctx, stream, (Bytef *) pair->value, pair->value_len)) != Z_OK)
      {
         goto e_inflate;
      }
      
      pair->value [pair->value_len] = 0;

      
      ++ i;
   }

   if (stream->avail_in > 0)
      goto e_inflate;

   buffer->ptr += buffer->size;
   buffer->size = 0;

   return SPDY_E_OK;

e_inflate:

   spdy_nv_free (*count, *pairs);

   return res == Z_MEM_ERROR ?
            SPDY_E_MEM : SPDY_E_INFLATE;

e_mem:

   spdy_nv_free (*count, *pairs);
   return SPDY_E_MEM;
}

void spdy_nv_free (size_t count, spdy_nv_pair * pairs)
{
   spdy_nv_pair * pair;
   size_t i = 0;

   if (!pairs)
      return;

   while (i < count)
   {
      pair = &pairs [i];

      free (pair->name);
      free (pair->value);
      
      ++ i;
   }

   free (pairs);
}

int spdy_pack_nv_block (spdy_ctx * ctx, char ** buffer, size_t * deflate_size,
                        spdy_nv_pair * pairs, size_t count)
{
   z_stream * stream;
   spdy_nv_pair * pair;
   size_t i = 0;
   size_t orig_size;
   size_t deflate_max;
   char * p;
   int res = Z_OK;

   if (!(ctx->flags & SPDY_GOT_VERSION))
     {
       if ((res = spdy_set_version (ctx, ctx->init_version)) != SPDY_E_OK)
         return res;
     }
 
   stream = &ctx->zlib_deflate;

   orig_size = ctx->version == 2 ? 2 : 4; /* size of the pair count */

   while (i < count)
   {
      orig_size += (ctx->version == 2 ? 4 : 8) /* size of the lengths */
                   + pairs [i].name_len + pairs [i].value_len;

      ++ i;
   }

   deflate_max = deflateBound (stream, orig_size);

   if (! (*buffer = (char *) malloc (deflate_max + orig_size)))
      return SPDY_E_MEM;

   i = 0;
   p = *buffer + deflate_max;

   if (ctx->version == 2)
   {
      spdy_write_int16 (p, count);
      p += sizeof (int16_t);

      while (i < count)
      {
         pair = &pairs[i ++];

         spdy_write_int16 (p, pair->name_len);
         p += sizeof (int16_t);

         memcpy (p, pair->name, pair->name_len);
         p += pair->name_len;

         spdy_write_int16 (p, pair->value_len);
         p += sizeof (int16_t);

         memcpy (p, pair->value, pair->value_len);
         p += pair->value_len;
      }
   }
   else
   {
      spdy_write_int32 (p, count);
      p += sizeof (int32_t);

      while (i < count)
      {
         pair = &pairs[i ++];

         spdy_write_int32 (p, pair->name_len);
         p += sizeof (int32_t);

         memcpy (p, pair->name, pair->name_len);
         p += pair->name_len;

         spdy_write_int32 (p, pair->value_len);
         p += sizeof (int32_t);

         memcpy (p, pair->value, pair->value_len);
         p += pair->value_len;
      }
   }

   stream->next_in = (Bytef *) *buffer + deflate_max;
   stream->avail_in = orig_size;

   stream->next_out = (Bytef *) *buffer;
   stream->avail_out = deflate_max;

   if ((res = deflate (stream, Z_SYNC_FLUSH)) != Z_OK || stream->avail_in > 0)
   {
      free (*buffer);

      return res == Z_MEM_ERROR ?
               SPDY_E_MEM : SPDY_E_DEFLATE;
   }

   *deflate_size = deflate_max - stream->avail_out;

   return SPDY_E_OK;
}


