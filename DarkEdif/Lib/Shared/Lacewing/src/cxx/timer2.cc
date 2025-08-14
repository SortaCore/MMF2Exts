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

timer lacewing::timer_new (lacewing::pump pump, const char * timer_name)
{
	return (timer) lw_timer_new ((lw_pump) pump, timer_name);
}

void lacewing::timer_delete (lacewing::timer timer)
{
	lw_timer_delete ((lw_timer) timer);
}

void _timer::start (long msec)
{
	lw_timer_start ((lw_timer) this, msec);
}

void _timer::stop ()
{
	lw_timer_stop ((lw_timer) this);
}

bool _timer::started ()
{
	return lw_timer_started ((lw_timer) this);
}

void _timer::force_tick ()
{
	lw_timer_force_tick ((lw_timer) this);
}

void _timer::on_tick (_timer::hook_tick on_tick)
{
	lw_timer_on_tick ((lw_timer) this, (lw_timer_hook_tick) on_tick);
}

void * _timer::tag ()
{
	return lw_timer_tag ((lw_timer) this);
}

void _timer::tag (void * tag)
{
	lw_timer_set_tag ((lw_timer) this, tag);
}

