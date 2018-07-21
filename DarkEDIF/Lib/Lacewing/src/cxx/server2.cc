
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

server lacewing::server_new (lacewing::pump pump)
{
   return (server) lw_server_new ((lw_pump) pump);
}

void lacewing::server_delete (lacewing::server server)
{
   lw_server_delete ((lw_server) server);
}

void _server::host (long port)
{
   lw_server_host ((lw_server) this, port);
}

void _server::host (lacewing::filter filter)
{
   lw_server_host_filter ((lw_server) this, (lw_filter) filter);
}

void _server::unhost  ()
{
   lw_server_unhost ((lw_server) this);
}

bool _server::hosting  ()
{
   return lw_server_hosting ((lw_server) this);
}

long _server::port  ()
{
   return lw_server_port ((lw_server) this);
}

bool _server::load_cert_file (const char * filename, const char * passphrase)
{
   return lw_server_load_cert_file ((lw_server) this, filename, passphrase);
}

bool _server::load_sys_cert (const char * store_name, const char * common_name,
                             const char * location)
{
   return lw_server_load_sys_cert ((lw_server) this, store_name,
                                   common_name, location);
}

bool _server::cert_loaded ()
{
   return lw_server_cert_loaded ((lw_server) this);
}

bool _server::can_npn ()
{
   return lw_server_can_npn ((lw_server) this);
}

void _server::add_npn (const char * protocol)
{
   return lw_server_add_npn ((lw_server) this, protocol);
}

size_t _server::num_clients ()
{
   return lw_server_num_clients ((lw_server) this);
}

server_client _server::client_first ()
{
   return (server_client) lw_server_client_first ((lw_server) this);
}

void _server::on_connect (_server::hook_connect hook)
{
   lw_server_on_connect ((lw_server) this, (lw_server_hook_connect) hook);
}

void _server::on_disconnect (_server::hook_disconnect hook)
{
   lw_server_on_disconnect ((lw_server) this, (lw_server_hook_disconnect) hook);
}

void _server::on_data (_server::hook_data hook)
{
   lw_server_on_data ((lw_server) this, (lw_server_hook_data) hook);
}

void _server::on_error (_server::hook_error hook)
{
   lw_server_on_error ((lw_server) this, (lw_server_hook_error) hook);
}

lacewing::address _server_client::address ()
{
   return (lacewing::address) lw_server_client_addr ((lw_server_client) this);
}

server_client _server_client::next ()
{
   return (server_client) lw_server_client_next ((lw_server_client) this);
}

const char * _server_client::npn ()
{
   return lw_server_client_npn ((lw_server_client) this);
}

void * _server::tag ()
{
   return lw_server_tag ((lw_server) this);
}

void _server::tag (void * tag)
{
   lw_server_set_tag ((lw_server) this, tag);
}

