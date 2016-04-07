
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

#include "../common.h"
#include "fdstream.h"

struct _lw_file
{
   struct _lw_fdstream ctx;

   char name [lwp_max_path];
};

lw_file lw_file_new (lw_pump pump)
{
   lw_file ctx = (lw_file) malloc (sizeof (*ctx));

   if (!ctx)
      return 0;

   *ctx->name = 0;

   lwp_fdstream_init ((lw_fdstream) ctx, pump);
   
   return ctx;
}

lw_file lw_file_new_open (lw_pump pump, const char * filename, const char * mode)
{
   lw_file ctx = lw_file_new (pump);

   if (!ctx)
      return 0;

   lw_file_open (ctx, filename, mode);

   return ctx;
}

lw_bool lw_file_open (lw_file ctx, const char * filename, const char * mode)
{
   *ctx->name = 0;

   lw_fdstream_set_fd ((lw_fdstream) ctx, INVALID_HANDLE_VALUE, 0, lw_true);

   DWORD dwDesiredAccess, dwShareMode, dwCreationDisposition;

   switch (*mode ++)
   {
      case 'r':
         dwDesiredAccess = GENERIC_READ;
         dwCreationDisposition = OPEN_EXISTING;
         dwShareMode = FILE_SHARE_READ;
         break;

      case 'w':
         dwDesiredAccess = GENERIC_WRITE;
         dwCreationDisposition = CREATE_ALWAYS;
         dwShareMode = 0;
         break;

      case 'a':
         dwDesiredAccess = GENERIC_WRITE;
         dwCreationDisposition = OPEN_ALWAYS;
         dwShareMode = 0;
         break;

      default:
         return lw_false;
   };

   if (*mode == 'b')
      ++ mode;

   if (*mode == '+')
   {   
      dwDesiredAccess = GENERIC_READ | GENERIC_WRITE;
      ++ mode;
   }

   if (*mode == 'b')
      ++ mode;

   if (*mode == 'x')
   {
      if (dwDesiredAccess == GENERIC_READ)
         return lw_false;

      dwCreationDisposition = CREATE_NEW;
   }

   /* TODO: should be converting to UTF-16 and using *W functions? */

   HANDLE fd = CreateFileA (filename,
                            dwDesiredAccess,
                            dwShareMode,
                            0,
                            dwCreationDisposition,
                            FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                            0);

   if (fd == INVALID_HANDLE_VALUE)
      return lw_false;

   lw_fdstream_set_fd ((lw_fdstream) ctx, fd, 0, lw_true);

   if (lw_fdstream_valid ((lw_fdstream) ctx))
   {
      strcpy (ctx->name, filename);
      return lw_true;
   }

   return lw_false;
}

lw_bool lw_file_open_temp (lw_file ctx)
{
   char name [lwp_max_path];
   char random [8];
   size_t i = 0;

   lw_temp_path (name);
   lw_random (random, sizeof (random));

   while (i < sizeof (random))
   {
      sprintf (name + strlen (name), "%02x", random [i]);
      ++ i;
   }

   lwp_trace ("Opening temp file: %s", name);

   return lw_file_open (ctx, name, "wb");
}

const char * lw_file_name (lw_file ctx)
{
   return ctx->name;
}

