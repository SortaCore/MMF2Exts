
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
#include "spdy_control.h"

#define FLAG_SETTINGS_CLEAR_SETTINGS 1

#define FLAG_SETTINGS_PERSIST_VALUE 1
#define FLAG_SETTINGS_PERSISTED 2

int spdy_proc_settings (spdy_ctx * ctx, int8_t flags, spdy_buffer * buffer)
{
   int32_t num_entries, i = 0;
   int8_t entry_flags;
   int32_t last_entry_id = 0, entry_id, entry_value;
   size_t num_changed = 0;
   spdy_setting changed [SPDY_NUM_SETTINGS];

   if (buffer->size < 4)
      return SPDY_E_PROTOCOL;

   num_entries = spdy_read_int32 (buffer);

   if (buffer->size != (num_entries * 8))
      return SPDY_E_PROTOCOL;

   while (i < num_entries)
   {
      if (ctx->version == 2)
      {
         entry_id = spdy_read_int24 (buffer);
         entry_flags = spdy_read_int8 (buffer);
      }
      else
      {
         entry_flags = spdy_read_int8 (buffer);
         entry_id = spdy_read_int24 (buffer);
      }

      entry_value = spdy_read_int32 (buffer);

      /* Entries should be lowest ID to highest ID.  Multiple entries for the
       * same ID must be ignored.
       */

      if (entry_id <= last_entry_id)
      {
         ++ i;
         continue;
      }

      last_entry_id = entry_id;

      if (entry_id <= 0 || entry_id > SPDY_NUM_SETTINGS)
         return SPDY_E_PROTOCOL;

      changed [num_changed].id = entry_id;
      changed [num_changed].value = entry_value;

      changed [num_changed].persistant
         = entry_flags & FLAG_SETTINGS_PERSIST_VALUE;

      ++ num_changed;
      ++ i;
   }

   if (ctx->config->on_settings_changed)
   {
      ctx->config->on_settings_changed
         (ctx, flags & FLAG_SETTINGS_CLEAR_SETTINGS, num_changed, changed);
   }

   return SPDY_E_OK;
}

void spdy_emit_settings (spdy_ctx * ctx, int8_t flags, int returning_persisted,
                         int num, spdy_setting * settings)
{
   char header [SPDY_CTRL_HEADER_SIZE + 4];
   char entry [8];
   int i = 0;

   spdy_build_control_header (ctx, header, SETTINGS, flags, 4 + num * 8);
   spdy_write_int32 (header + SPDY_CTRL_HEADER_SIZE, num);

   ctx->config->emit (ctx, header, sizeof (header));

   while (i < num)
   {
      *entry = 0;
      
      if (returning_persisted)
         *entry |= FLAG_SETTINGS_PERSISTED;
      else if (settings [i].persistant && ctx->config->is_server)
         *entry |= FLAG_SETTINGS_PERSIST_VALUE;

      spdy_write_int24 (entry + 1, settings [i].id);
      spdy_write_int32 (entry + 4, settings [i].value);
      
      ctx->config->emit (ctx, entry, sizeof (entry));

      ++ i;
   }
}

void spdy_update_settings (spdy_ctx * ctx, int should_clear, int num_changed,
                           spdy_setting * changed)
{
   int8_t flags = 0;

   if (ctx->config->is_server && should_clear)
      flags |= FLAG_SETTINGS_CLEAR_SETTINGS;

   spdy_emit_settings (ctx, flags, 0, num_changed, changed);
}

