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

address lacewing::address_new (lacewing::address address)
{
	return (lacewing::address) lw_addr_clone ((lw_addr) address);
}

address lacewing::address_new (const char * hostname, const char * service)
{
	return (address) lw_addr_new (hostname, service);
}

address lacewing::address_new (const char * hostname, lw_ui16 port)
{
	return (address) lw_addr_new_port (hostname, port);
}

address lacewing::address_new (const char * hostname, const char * service, int hints)
{
	return (address) lw_addr_new_hint (hostname, service, hints);
}

address lacewing::address_new (const char * hostname, lw_ui16 port, int hints)
{
	return (address) lw_addr_new_port_hint (hostname, port, hints);
}

void lacewing::address_delete (lacewing::address address)
{
	lw_addr_delete ((lw_addr) address);
}

lw_ui16 _address::port ()
{
	return lw_addr_port ((lw_addr) this);
}

void _address::port (lw_ui16 port)
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

in6_addr _address::toin6_addr()
{
	return lw_addr_toin6_addr ((lw_addr) this);
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

