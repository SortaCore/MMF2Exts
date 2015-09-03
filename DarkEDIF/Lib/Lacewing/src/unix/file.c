
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

#include "../common.h"
#include "fdstream.h"

struct _lw_file
{
    struct _lw_fdstream fdstream;

    char name [lwp_max_path];
};

void lwp_file_init (lw_file ctx, lw_pump pump)
{
   *ctx->name = 0;

   lwp_fdstream_init (&ctx->fdstream, pump);
}

lw_file lw_file_new (lw_pump pump)
{
   lw_file ctx = malloc (sizeof (*ctx));
   lwp_file_init (ctx, pump);

   return ctx;
}

lw_file lw_file_new_open (lw_pump pump,
                          const char * filename,
                          const char * mode)
{
   lw_file ctx = lw_file_new (pump);

   if (!ctx)
      return 0;

   if (!lw_file_open (ctx, filename, mode))
   {
      lw_stream_delete ((lw_stream) ctx);
      return 0;
   }

   return ctx;
}

static int get_flags (const char * mode)
{
   /* Based on what FreeBSD does to convert the mode string for fopen(3) */

   int flags;

   switch (*mode ++)
   {
      case 'r':
         flags = O_RDONLY;
         break;

      case 'w':
         flags = O_WRONLY | O_CREAT | O_TRUNC;
         break;

      case 'a':
         flags = O_WRONLY | O_CREAT | O_APPEND;
         break;

      default:
         return -1;
   };

   if (*mode == 'b')
      ++ mode;

   if (*mode == '+')
   {   
      flags |= O_RDWR;
      ++ mode;
   }

   if (*mode == 'b')
      ++ mode;

   if (*mode == 'x')
   {
      if (flags & O_RDONLY)
         return -1;

      flags |= O_EXCL;
   }

   return flags;
}

lw_bool lw_file_open (lw_file ctx,
                      const char * filename,
                      const char * mode)
{
    lwp_trace ("%p : lw_file_open \"%s\", \"%s\"", ctx, filename, mode);

    *ctx->name = 0;

    int flags = get_flags (mode);

    if (flags == -1)
    {
        lwp_trace ("Error parsing mode");
        return lw_false;
    }

    int fd = open (filename, flags, S_IRWXU);

    if (fd == -1)
    {
        lwp_trace ("open() failed: %s", strerror (errno));
        return lw_false;
    }

    lw_fdstream_set_fd ((lw_fdstream) ctx, fd, 0, lw_true);

    if (lw_fdstream_valid ((lw_fdstream) ctx))
    {
        strcpy (ctx->name, filename);
        return lw_true;
    }

    lwp_trace ("valid() returned false");

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

