/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"

client lacewing::client_new (lacewing::pump pump)
{
	return (client) lw_client_new ((lw_pump) pump);
}

void _client::connect (const char * host, lw_ui16 port)
{
	lw_client_connect ((lw_client) this, host, port);
}

void _client::connect (address addr)
{
	lw_client_connect_addr ((lw_client) this, (lw_addr) addr);
}

void _client::setlocalport (lw_ui16 port)
{
	lw_client_set_local_port ((lw_client)this, port);
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

