/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2014 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "common.h"

#ifdef _lacewing_debug

static void list_refs (char * buf, struct lwp_refcount * refcount)
{
	*buf = 0;

	for (int i = 0, printCount = 0; printCount < refcount->refcount; ++ i)
	{
		if (!refcount->refs [i])
			continue;

		if (++printCount > 1)
			strcat (buf, ", ");

		strcat (buf, refcount->refs [i]);
	}
}

#ifndef _WIN32
#include <sched.h> // for sched_yield, will be removed
#endif

lw_bool _lwp_retain (struct lwp_refcount * refcount, const char * name)
{
	// this ref counter is in use; wait and retry
#ifdef msvc_windows_atomic_workaround
	while (InterlockedExchange(&refcount->reflock, lw_true))
		Sleep(0);
#else
	if (refcount->refcount == 0)
		atomic_init(&refcount->reflock, lw_false);
	while (atomic_exchange(&refcount->reflock, lw_true))
	{
		// TODO: We should use a pthread mutex instead of telling kernel to reschedule.
		// Scheduler may report this thread is still most worthy of its CPU slice, and so return control to this thread,
		// while loop goes around again, and result is yielding is not much better than not even trying.
		sched_yield();
	}
#endif

	/* sanity check
	*/
	assert (refcount->refcount < MAX_REFS);

	if (refcount->enable_logging)
	{
		char refs [1024];
		list_refs (refs, refcount);

		lw_trace ("refcount: %s @ %p has %d refs (%s), now retaining (%s)",
					refcount->name,
					refcount,
					(int) refcount->refcount,
					refs,
					name);
	}

	++ refcount->refcount;

	for (int i = 0; i < MAX_REFS; ++ i)
	{
		if (!refcount->refs [i])
		{
			refcount->refs [i] = name;
			break;
		}
	}
#ifdef msvc_windows_atomic_workaround
	InterlockedExchange(&refcount->reflock, lw_false);
#else
	atomic_exchange(&refcount->reflock, lw_false);
#endif

	return lw_false;
}

lw_bool _lwp_release (struct lwp_refcount * refcount, const char * name)
{
	// this ref counter is in use; wait and retry
#ifdef msvc_windows_atomic_workaround
	while (InterlockedExchange(&refcount->reflock, lw_true))
		Sleep(0);
#else
	while (atomic_exchange(&refcount->reflock, lw_true))
		sched_yield();
#endif

	assert (refcount->refcount >= 1 && refcount->refcount < MAX_REFS);

	-- refcount->refcount;

	for (int i = 0; i < MAX_REFS; ++ i)
	{
		if (refcount->refs [i] &&
			!strcasecmp (refcount->refs [i], name))
		{
			refcount->refs [i] = NULL;
			break;
		}
	}

	if (refcount->enable_logging)
	{
		char refs [1024];
		list_refs (refs, refcount);

		lw_trace ("refcount: %s @ %p released by %s, now has %d refs (%s)",
					refcount->name,
					refcount,
					name,
					(int) refcount->refcount,
					refs);
	}

	if (refcount->refcount == 0)
	{
		// freeing, and can't set false after, so we'll do this in case of invalid memory
#ifdef msvc_windows_atomic_workaround
		InterlockedExchange(&refcount->reflock, lw_false);
#else
		atomic_exchange(&refcount->reflock, lw_false);
#endif
		if (refcount->on_dealloc)
			refcount->on_dealloc ((void *) refcount);
		else
			free (refcount);

		return lw_true;
	}

#ifdef msvc_windows_atomic_workaround
	InterlockedExchange(&refcount->reflock, lw_false);
#else
	atomic_exchange(&refcount->reflock, lw_false);
#endif
	return lw_false;
}

#endif

