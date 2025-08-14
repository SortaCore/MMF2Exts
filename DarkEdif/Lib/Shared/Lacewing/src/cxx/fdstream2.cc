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

fdstream lacewing::fdstream_new (lacewing::pump pump)
{
	return (fdstream) lw_fdstream_new ((lw_pump) pump);
}

void _fdstream::set_fd (lw_fd fd, lw_pump_watch watch, bool auto_close, bool is_socket)
{
	lw_fdstream_set_fd ((lw_fdstream) this, fd, watch, auto_close, is_socket);
}

bool _fdstream::valid ()
{
	return lw_fdstream_valid ((lw_fdstream) this);
}

void _fdstream::cork ()
{
	lw_fdstream_cork ((lw_fdstream) this);
}

void _fdstream::uncork ()
{
	lw_fdstream_uncork ((lw_fdstream) this);
}

void _fdstream::nagle (bool enabled)
{
	lw_fdstream_nagle ((lw_fdstream) this, enabled);
}


