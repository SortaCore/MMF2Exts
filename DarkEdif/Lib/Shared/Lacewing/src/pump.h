/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_pump_h
#define _lw_pump_h

struct _lw_pump
{
	const lw_pumpdef * def;

	long use_count;

	void * tag;
};

void lwp_pump_init (lw_pump ctx, const lw_pumpdef * def);

#endif


