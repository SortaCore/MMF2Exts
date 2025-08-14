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

thread lacewing::thread_new (const char * name, void * proc)
{
	return (thread) lw_thread_new (name, proc);
}

void lacewing::thread_delete (lacewing::thread thread)
{
	lw_thread_delete ((lw_thread) thread);
}

void _thread::start (void * param)
{
	lw_thread_start ((lw_thread) this, param);
}

bool _thread::started ()
{
	return lw_thread_started ((lw_thread) this);
}

void * _thread::join ()
{
	return lw_thread_join ((lw_thread) this);
}

void * _thread::tag ()
{
	return lw_thread_tag ((lw_thread) this);
}

void _thread::tag (void * tag)
{
	lw_thread_set_tag ((lw_thread) this, tag);
}

