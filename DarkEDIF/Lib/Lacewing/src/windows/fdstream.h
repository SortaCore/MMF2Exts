
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

#ifndef _lw_fdstream_h
#define _lw_fdstream_h

#include "../stream.h"

typedef struct _fdstream_overlapped
{
    OVERLAPPED overlapped;

    char type;

    char data [1];

} * fdstream_overlapped;

struct _lw_fdstream
{
   struct _lw_stream stream;

   struct _fdstream_overlapped read_overlapped;
   struct _fdstream_overlapped transmitfile_overlapped;

   lw_fdstream transmit_file_from,
               transmit_file_to;

   char buffer [lwp_default_buffer_size];

   HANDLE fd;

   lw_pump_watch watch;

   size_t size;
   size_t reading_size;

   LARGE_INTEGER offset;

   char flags;

   /* The number of pending writes.  May not be the same as
    * list_length(pending_writes) because transmitfile counts as a pending
    * write too, in both the source and dest stream.
    */
   int num_pending_writes;

   list (fdstream_overlapped, pending_writes);

   lw_fdstream transmitfile_from, transmitfile_to;
};

#define lwp_fdstream_flag_read_pending     1
#define lwp_fdstream_flag_nagle            2
#define lwp_fdstream_flag_is_socket        4
#define lwp_fdstream_flag_close_asap       8  /* FD close pending on write? */
#define lwp_fdstream_flag_auto_close       16

void lwp_fdstream_init (lw_fdstream, lw_pump);

#endif


