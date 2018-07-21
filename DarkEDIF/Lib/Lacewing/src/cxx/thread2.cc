
/* vim: set et ts=3 sw=3 ft=cpp:
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

thread lacewing::thread_new (const char * name, void * proc)
{
   return (thread) lw_thread_new (name, proc);
}

void lacewing::thread_delete (lacewing::thread thread)
{
   lw_thread_delete ((lw_thread) thread);
}

void _thread::start (void * param)
{
   lw_thread_start ((lw_thread) this, param);
}

bool _thread::started ()
{
   return lw_thread_started ((lw_thread) this);
}

void * _thread::join ()
{
   return lw_thread_join ((lw_thread) this);
}

void * _thread::tag ()
{
   return lw_thread_tag ((lw_thread) this);
}

void _thread::tag (void * tag)
{
   lw_thread_set_tag ((lw_thread) this, tag);
}

