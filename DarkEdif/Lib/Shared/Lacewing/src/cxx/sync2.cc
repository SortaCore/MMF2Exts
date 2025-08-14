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

lacewing::sync lacewing::sync_new ()
{
	return (lacewing::sync) lw_sync_new ();
}

void lacewing::sync_delete (lacewing::sync sync)
{
	lw_sync_delete ((lw_sync) sync);
}

_sync_lock::_sync_lock (lacewing::sync sync)
{
	this->sync = sync;

	lw_sync_lock ((lw_sync) sync);
}

_sync_lock::~_sync_lock ()
{
	lw_sync_release ((lw_sync) sync);
}
