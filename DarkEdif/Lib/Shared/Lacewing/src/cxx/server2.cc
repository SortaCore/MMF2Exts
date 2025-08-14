/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
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

bool _server::load_cert_file (const char * filename_certchain, const char* filename_privkey, const char * passphrase)
{
	return lw_server_load_cert_file ((lw_server) this, filename_certchain, filename_privkey, passphrase);
}

bool _server::load_sys_cert (const char * common_name, const char * location,
							 const char * store_name)
{
	return lw_server_load_sys_cert ((lw_server) this, common_name,
									location, store_name);
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

lw_ui16 _server::hole_punch (const char* remote_ip_and_port, lw_ui16 local_port)
{
	return lw_server_hole_punch ((lw_server) this, remote_ip_and_port, local_port);
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

lw_bool _server_client::is_websocket ()
{
	return lw_server_client_is_websocket ((lw_server_client) this);
}

void * _server::tag ()
{
	return lw_server_tag ((lw_server) this);
}

void _server::tag (void * tag)
{
	lw_server_set_tag ((lw_server) this, tag);
}

