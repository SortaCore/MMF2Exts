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

void lacewing::pump_delete (lacewing::pump pump)
{
	lw_pump_delete ((lw_pump) pump);
}

void _pump::add_user ()
{
	lw_pump_add_user ((lw_pump) this);
}

void _pump::remove_user ()
{
	lw_pump_remove_user ((lw_pump) this);
}

bool _pump::in_use ()
{
	return lw_pump_in_use ((lw_pump) this);
}

#ifdef _WIN32

 lw_pump_watch _pump::add (HANDLE handle, void * tag,
							lw_pump_callback callback)
 {
	return lw_pump_add ((lw_pump) this, handle, tag, callback);
 }

 void _pump::update_callbacks (lw_pump_watch watch, void * tag,
								lw_pump_callback callback)
 {
	lw_pump_update_callbacks ((lw_pump) this, watch, tag, callback);
 }

#else

 lw_pump_watch _pump::add (int fd, void * tag,
							lw_pump_callback on_read_ready,
							lw_pump_callback on_write_ready,
							bool edge_triggered)
 {
	return (lw_pump_watch) lw_pump_add
		((lw_pump) this, fd, tag, on_read_ready, on_write_ready, edge_triggered);
 }

 void _pump::update_callbacks (lw_pump_watch watch, void * tag,
								lw_pump_callback on_read_ready,
								lw_pump_callback on_write_ready,
								bool edge_triggered)
 {
	lw_pump_update_callbacks ((lw_pump) this, watch, tag, on_read_ready,
							  on_write_ready, edge_triggered);
 }

#endif

void _pump::remove (lw_pump_watch watch)
{
	lw_pump_remove ((lw_pump) this, watch);
}

void _pump::post (void * func, void * param)
{
	lw_pump_post ((lw_pump) this, func, param);
}

void _pump::post_remove (lw_pump_watch watch)
{
	lw_pump_post_remove ((lw_pump) this, watch);
}

void * _pump::tag ()
{
	return lw_pump_tag ((lw_pump) this);
}

void _pump::tag (void * tag)
{
	lw_pump_set_tag ((lw_pump) this, tag);
}

