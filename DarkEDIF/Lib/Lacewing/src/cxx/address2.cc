
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

address lacewing::address_new (lacewing::address address)
{
   return (lacewing::address) lw_addr_clone ((lw_addr) address);
}

address lacewing::address_new (const char * hostname, const char * service)
{
   return (address) lw_addr_new (hostname, service);
}

address lacewing::address_new (const char * hostname, long port)
{
   return (address) lw_addr_new_port (hostname, port);
}

address lacewing::address_new (const char * hostname, const char * service, long hints)
{
   return (address) lw_addr_new_hint (hostname, service, hints);
}

address lacewing::address_new (const char * hostname, long port, long hints)
{
   return (address) lw_addr_new_port_hint (hostname, port, hints);
}

void lacewing::address_delete (lacewing::address address)
{
   lw_addr_delete ((lw_addr) address);
}

long _address::port ()
{
   return lw_addr_port ((lw_addr) this);
}

void _address::port (long port)
{
   lw_addr_set_port ((lw_addr) this, port);
}

int _address::type ()
{
   return lw_addr_type ((lw_addr) this);
}

void _address::type (int type)
{
   lw_addr_set_type ((lw_addr) this, type);
}

bool _address::ipv6 ()
{
   return lw_addr_ipv6 ((lw_addr) this);
}

bool _address::ready ()
{
   return lw_addr_ready ((lw_addr) this);
}

error _address::resolve ()
{
   return (error) lw_addr_resolve ((lw_addr) this);
}

const char * _address::tostring ()
{
   return lw_addr_tostring ((lw_addr) this);
}

_address::operator const char * ()
{
   return tostring ();
}

bool _address::operator == (lacewing::address address)
{
   return lw_addr_equal ((lw_addr) this, (lw_addr) address);
}

bool _address::operator != (lacewing::address address)
{
   return !lw_addr_equal ((lw_addr) this, (lw_addr) address);
}

void * _address::tag ()
{
   return lw_addr_tag ((lw_addr) this);
}

void _address::tag (void * tag)
{
   lw_addr_set_tag ((lw_addr) this, tag);
}

