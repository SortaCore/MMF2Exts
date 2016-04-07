
/* vim: set et ts=3 sw=3 ft=cpp:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.  All rights reserved.
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

filter lacewing::filter_new ()
{
   return (filter) lw_filter_new ();
}

void lacewing::filter_delete (lacewing::filter filter)
{
   lw_filter_delete ((lw_filter) filter);
}

void _filter::local (address addr)
{
   lw_filter_set_local ((lw_filter) this, (lw_addr) addr);
}

void _filter::remote (address addr)
{
   lw_filter_set_remote ((lw_filter) this, (lw_addr) addr);
}

address _filter::local ()
{
   return (address) lw_filter_local ((lw_filter) this);
}

address _filter::remote ()
{
   return (address) lw_filter_remote ((lw_filter) this);
}

long _filter::local_port ()
{
   return lw_filter_local_port ((lw_filter) this);
}

void _filter::local_port (long port)
{
   lw_filter_set_local_port ((lw_filter) this, port);
}

long _filter::remote_port ()
{
   return lw_filter_remote_port ((lw_filter) this);
}

void _filter::remote_port (long port)
{
   lw_filter_set_remote_port ((lw_filter) this, port);
}

bool _filter::reuse ()
{
   return lw_filter_reuse ((lw_filter) this);
}

void _filter::reuse (bool reuse)
{
   lw_filter_set_reuse ((lw_filter) this, reuse);
}

bool _filter::ipv6 ()
{
   return lw_filter_ipv6 ((lw_filter) this);
}

void _filter::ipv6 (bool ipv6)
{
   lw_filter_set_ipv6 ((lw_filter) this, ipv6);
}

void * _filter::tag ()
{
   return lw_filter_tag ((lw_filter) this);
}

void _filter::tag (void * tag)
{
   lw_filter_set_tag ((lw_filter) this, tag);
}

