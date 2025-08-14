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

eventpump lacewing::eventpump_new ()
{
	return (eventpump) lw_eventpump_new ();
}

error _eventpump::start_eventloop ()
{
	return (error) lw_eventpump_start_eventloop ((lw_eventpump) this);
}

error _eventpump::tick ()
{
	return (error) lw_eventpump_tick ((lw_eventpump) this);
}

error _eventpump::start_sleepy_ticking
	  (void (lw_callback * on_tick_needed) (eventpump))
{
	return (error) lw_eventpump_start_sleepy_ticking
	  ((lw_eventpump) this, (void (*) (lw_eventpump)) on_tick_needed);
}

void _eventpump::post_eventloop_exit ()
{
	lw_eventpump_post_eventloop_exit ((lw_eventpump) this);
}


