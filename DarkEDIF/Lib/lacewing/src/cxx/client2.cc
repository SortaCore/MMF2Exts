
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

client lacewing::client_new (lacewing::pump pump)
{
   return (client) lw_client_new ((lw_pump) pump);
}

void _client::connect (const char * host, long port)
{
   lw_client_connect ((lw_client) this, host, port);
}

void _client::connect (address addr)
{
   lw_client_connect_addr ((lw_client) this, (lw_addr) addr);
}

bool _client::connected ()
{
   return lw_client_connected ((lw_client) this);
}

bool _client::connecting ()
{
   return lw_client_connecting ((lw_client) this);
}

address _client::server_address ()
{
   return (address) lw_client_server_addr ((lw_client) this);
}

void _client::on_connect (_client::hook_connect hook)
{
   lw_client_on_connect ((lw_client) this, (lw_client_hook_connect) hook);
}

void _client::on_disconnect (_client::hook_disconnect hook)
{
   lw_client_on_disconnect ((lw_client) this, (lw_client_hook_disconnect) hook);
}

void _client::on_data (_client::hook_data hook)
{
   lw_client_on_data ((lw_client) this, (lw_client_hook_data) hook);
}

void _client::on_error (_client::hook_error hook)
{
   lw_client_on_error ((lw_client) this, (lw_client_hook_error) hook);
}

