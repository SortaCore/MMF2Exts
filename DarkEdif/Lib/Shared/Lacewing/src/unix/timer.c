/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.
 * Copyright (C) 2012-2022 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/licenses/mit-license.php
*/

#include "../common.h"
#include "eventpump.h"

struct _lw_timer
{
	lw_pump pump;
	lw_pump_watch pump_watch;

	lw_timer_hook_tick on_tick;

	void * tag;

	lw_bool started;

	#ifdef _lacewing_use_timerfd
	  int fd;
	#endif

	lw_event stop_event;
	long interval;

	lw_thread timer_thread;
};

static void timer_tick (lw_timer ctx)
{
	if (ctx->on_tick)
		ctx->on_tick (ctx);

	#ifdef _lacewing_use_timerfd
		lw_i64 expirations;
		ssize_t s = read (ctx->fd, &expirations, sizeof (lw_i64));
		(void)s;
		assert(s > 0);
		// TODO: why was expirations read here? Is there an abort?
	#endif
}

static void timer_thread (void * ptr)
{
	lw_timer ctx = (lw_timer)ptr;

	for (;;)
	{
		lw_event_wait (ctx->stop_event, ctx->interval);

		if (lw_event_signalled (ctx->stop_event))
			break;

		lw_pump_post (ctx->pump, (void *)timer_tick, ctx);
	}
}

lw_timer lw_timer_new (lw_pump pump)
{
	lw_timer ctx = (lw_timer)calloc (sizeof (*ctx), 1);

	if (!ctx)
		return 0;

	ctx->pump = pump;
	ctx->timer_thread = lw_thread_new ("timer_thread", (void *)timer_thread);
	ctx->stop_event = lw_event_new ();

	#ifdef _lacewing_use_timerfd
		ctx->fd = timerfd_create (CLOCK_MONOTONIC, TFD_NONBLOCK);
		ctx->pump_watch = lw_pump_add (ctx->pump, ctx->fd, ctx, (lw_pump_callback) timer_tick, 0, lw_true);
	#endif

	return ctx;
}

void lw_timer_delete (lw_timer ctx)
{
	lw_timer_stop (ctx);
	lw_event_delete (ctx->stop_event);

	#ifdef _lacewing_use_timerfd
		close (ctx->fd);
		lw_pump_remove(ctx->pump, ctx->pump_watch);
	#endif

	lw_thread_delete(ctx->timer_thread);

	free (ctx);
}

void lw_timer_start (lw_timer ctx, long interval)
{
	lw_timer_stop (ctx);

	ctx->started = lw_true;
	ctx->interval = interval;

	lw_pump_add_user (ctx->pump);

	#ifdef USE_KQUEUE

	  if (ctx->pump->def == &def_eventpump)
	  {
		 struct kevent event;

		 EV_SET (&event, (uintptr_t) ctx, EVFILT_TIMER,
					EV_ADD | EV_ENABLE | EV_CLEAR, 0, interval, ctx);

		 if (kevent (((lw_eventpump) ctx->pump)->queue,
						&event, 1, 0, 0, 0) == -1)
		 {
			lwp_trace ("Timer: Failed to add timer to kqueue: %s",
							strerror (errno));

			return;
		 }
	  }
	  else
	  {
		 ctx->interval = interval;
		 lw_thread_start (ctx->timer_thread, ctx);
	  }

	#else
	  #ifdef _lacewing_use_timerfd

			struct itimerspec spec;

			spec.it_value.tv_sec = spec.it_interval.tv_sec  = interval / 1000;
			spec.it_value.tv_nsec = spec.it_interval.tv_nsec = (interval % 1000) * 1000000;

			timerfd_settime (ctx->fd, 0, &spec, 0);

	  #else
			ctx->interval = interval;
			lw_thread_start (ctx->timer_thread, ctx);
	  #endif
	#endif
}

void lw_timer_stop (lw_timer ctx)
{
	if (!lw_timer_started (ctx))
	  return;

	/* TODO: What if a tick has been posted and this gets destructed? */

	#ifndef _lacewing_use_timerfd

	  lw_event_signal (ctx->stop_event);
	  lw_thread_join (ctx->timer_thread);
	  lw_event_unsignal (ctx->stop_event);

	#endif

	#ifdef USE_KQUEUE

	  if (ctx->pump->def == &def_eventpump)
	  {
		 struct kevent event;

		 EV_SET (&event, (uintptr_t) ctx, EVFILT_TIMER, EV_DELETE, 0, 0, ctx);

		 if (kevent (((lw_eventpump) ctx->pump)->queue,
						&event, 1, 0, 0, 0) == -1)
		 {
			lwp_trace ("Timer: Failed to remove timer from kqueue: %s",
							strerror (errno));

			return;
		 }
	  }
	  else
	  {
		 /* TODO */
	  }

	#else
		#ifdef _lacewing_use_timerfd
			struct itimerspec spec = {0};
			timerfd_settime (ctx->fd, 0, &spec, 0);
		#endif
	#endif

	ctx->started = lw_false;
	lw_pump_remove_user (ctx->pump);
}

void lw_timer_force_tick (lw_timer ctx)
{
	if (ctx->on_tick)
	  ctx->on_tick (ctx);
}

lw_bool lw_timer_started (lw_timer ctx)
{
	return ctx->started;
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

