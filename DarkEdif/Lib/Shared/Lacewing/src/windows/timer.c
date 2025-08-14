/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013, 2014 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"

static DWORD __stdcall timer_thread (lw_timer);

struct _lw_timer
{
	lw_pump pump;

	lw_thread timer_thread;

	HANDLE timer_handle;
	HANDLE shutdown_event;

	lw_bool started;

	lw_timer_hook_tick on_tick;

	void * tag;

	char * timer_name;
};

lw_timer lw_timer_new (lw_pump pump, const char * timer_name)
{
	lw_timer ctx = (lw_timer) calloc (sizeof (*ctx), 1);

	if (!ctx)
		return 0;

	ctx->pump = pump;

	ctx->shutdown_event = CreateEvent (0, TRUE, FALSE, 0);
	ctx->timer_handle = CreateWaitableTimer (0, FALSE, 0);

	ctx->timer_name = _strdup (timer_name);

	char threadName[128];
	lwp_snprintf (threadName, sizeof(threadName), "lw_thread for lw_timer \"%s\" (0x%" PRIXPTR ")", timer_name, (uintptr_t)ctx);

	ctx->timer_thread = lw_thread_new (threadName, (void *) timer_thread);
	lw_thread_start (ctx->timer_thread, ctx);

	return ctx;
}

void lw_timer_delete (lw_timer ctx)
{
	if (!ctx)
		return;

	SetEvent (ctx->shutdown_event);

	lw_timer_stop (ctx);

	lw_thread_join (ctx->timer_thread);
	lw_thread_delete (ctx->timer_thread);

	free (ctx->timer_name);

	free (ctx);
}

static void timer_completion (void * ptr)
{
	lw_timer ctx = (lw_timer) ptr;

	if (ctx->on_tick)
		ctx->on_tick (ctx);
}

DWORD __stdcall timer_thread (lw_timer ctx)
{
	HANDLE events [2] = { ctx->timer_handle, ctx->shutdown_event };

	for (;;)
	{
		int result = WaitForMultipleObjects (2, events, FALSE, INFINITE);

		if (result != WAIT_OBJECT_0)
		{
			if (result != WAIT_OBJECT_0 + 1)
				always_log ("lw_timer \"%s\" got wait result %d\n", ctx->timer_name, result);
			break;
		}

		lw_pump_post (ctx->pump, (void *) timer_completion, ctx);
	}
	return 0;
}

void lw_timer_start (lw_timer ctx, long interval)
{
	lw_timer_stop (ctx);

	LARGE_INTEGER due_time;
	due_time.QuadPart = 0LL - (interval * 1000LL * 10LL);

	if (!SetWaitableTimer (ctx->timer_handle, &due_time, interval, 0, 0, 0))
	{
		always_log("lw_timer \"%s\" couldn't set waitable timer: error %u\n",
			ctx->timer_name, GetLastError());
	}

	ctx->started = lw_true;
	lw_pump_add_user (ctx->pump);
}

void lw_timer_stop (lw_timer ctx)
{
	if (!lw_timer_started (ctx))
	  return;

	CancelWaitableTimer (ctx->timer_handle);

	ctx->started = lw_false;

	lw_pump_remove_user (ctx->pump);
}

lw_bool lw_timer_started (lw_timer ctx)
{
	return ctx->started;
}

void lw_timer_force_tick (lw_timer ctx)
{
	if (ctx->on_tick)
	  ctx->on_tick (ctx);
}

void lw_timer_set_tag (lw_timer ctx, void * tag)
{
	ctx->tag = tag;
}

void * lw_timer_tag (lw_timer ctx)
{
	return ctx->tag;
}

lwp_def_hook (timer, tick)
