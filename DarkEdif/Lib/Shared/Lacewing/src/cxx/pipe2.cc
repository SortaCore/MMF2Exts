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

lacewing::pipe lacewing::pipe_new ()
{
	return (lacewing::pipe) lw_pipe_new ((lw_pump) 0);
}

lacewing::pipe lacewing::pipe_new (lacewing::pump pump)
{
	return (lacewing::pipe) lw_pipe_new ((lw_pump) pump);
}

