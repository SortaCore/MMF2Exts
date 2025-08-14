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

event lacewing::event_new ()
{
	return (event) lw_event_new ();
}

void lacewing::event_delete (lacewing::event event)
{
	lw_event_delete ((lw_event) event);
}

void _event::signal ()
{
	lw_event_signal ((lw_event) this);
}

void _event::unsignal ()
{
	lw_event_unsignal ((lw_event) this);
}

bool _event::signalled ()
{
	return lw_event_signalled ((lw_event) this);
}

bool _event::wait (long timeout)
{
	return lw_event_wait ((lw_event) this, timeout);
}

void * _event::tag ()
{
	return lw_event_tag ((lw_event) this);
}

void _event::tag (void * tag)
{
	lw_event_set_tag ((lw_event) this, tag);
}

