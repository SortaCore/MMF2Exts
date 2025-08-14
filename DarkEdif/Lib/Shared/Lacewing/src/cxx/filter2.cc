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

