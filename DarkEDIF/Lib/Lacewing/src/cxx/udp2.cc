
/* vim: set et ts=4 sw=4 ft=cpp:
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

udp lacewing::udp_new (lacewing::pump pump)
{
   return (udp) lw_udp_new ((lw_pump) pump);
}

void lacewing::udp_delete (lacewing::udp udp)
{
   lw_udp_delete ((lw_udp) udp);
}

void _udp::host (long port)
{
   lw_udp_host ((lw_udp) this, port);
}

void _udp::host (lacewing::filter filter)
{
   lw_udp_host_filter ((lw_udp) this, (lw_filter) filter);
}

void _udp::host (lacewing::address address)
{
   lw_udp_host_addr ((lw_udp) this, (lw_addr) address);
}

bool _udp::hosting ()
{
   return lw_udp_hosting ((lw_udp) this);
}

void _udp::unhost ()
{
   lw_udp_unhost ((lw_udp) this);
}

long _udp::port ()
{
   return lw_udp_port ((lw_udp) this);
}

void _udp::send (lacewing::address address, const char * data, size_t size)
{
   lw_udp_send ((lw_udp) this, (lw_addr) address, data, size);
}

void _udp::on_data (_udp::hook_data hook)
{
   lw_udp_on_data ((lw_udp) this, (lw_udp_hook_data) hook);
}

void _udp::on_error (_udp::hook_error hook)
{
   lw_udp_on_error ((lw_udp) this, (lw_udp_hook_error) hook);
}

void * _udp::tag ()
{
   return lw_udp_tag ((lw_udp) this);
}

void _udp::tag (void * tag)
{
   lw_udp_set_tag ((lw_udp) this, tag);
}

