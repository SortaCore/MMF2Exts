/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
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

void _udp::host (lw_ui16 port)
{
	lw_udp_host ((lw_udp) this, port);
}

void _udp::host (lacewing::filter filter)
{
	lw_udp_host_filter ((lw_udp) this, (lw_filter) filter);
}

void _udp::host (lacewing::address address, lw_ui16 local_port)
{
	lw_udp_host_addr ((lw_udp) this, (lw_addr) address, local_port);
}

bool _udp::hosting ()
{
	return lw_udp_hosting ((lw_udp) this);
}

void _udp::unhost ()
{
	lw_udp_unhost ((lw_udp) this);
}

lw_ui16 _udp::port ()
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

