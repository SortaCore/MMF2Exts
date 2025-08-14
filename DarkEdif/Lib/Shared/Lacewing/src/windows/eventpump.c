/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"
#include "../pump.h"

extern const lw_pumpdef def_eventpump;

#define sig_exit_event_loop	  ((OVERLAPPED *) 1)
#define sig_end_watcher_thread	((OVERLAPPED *) 2)

struct _lw_pump_watch
{
	lw_pump_callback on_completion;
	void * tag;
};

struct _lw_eventpump
{
	struct _lw_pump pump;

	HANDLE completion_port;

	void (lw_callback * on_tick_needed) (lw_eventpump);

	struct
	{
		lw_thread thread;
		OVERLAPPED * overlapped;
		DWORD bytes_transferred;
		lw_pump_watch event;
		lw_event resume_event;
		int error;
	} watcher;

	size_t num_pending;
};

static lw_bool process (lw_eventpump ctx, OVERLAPPED * overlapped,
						unsigned int bytes_transferred, lw_pump_watch watch,
						int error)
{
	// when bytes_transferred is 0xFF, it indicates that watch points to a timer completion function,
	// and that overlapped is a lw_timer address.
	if (bytes_transferred == 0xFFFFFFFF)
	{
		/* See eventpump_post */
		((void * (*) (void *)) watch) (overlapped);

		return lw_true;
	}

	if (overlapped == sig_exit_event_loop)
		return lw_false;

	if (watch->on_completion)
		watch->on_completion(watch->tag, overlapped, bytes_transferred, error);

	return lw_true;
}

static void watcher (lw_eventpump ctx)
{
	for (;;)
	{
		ctx->watcher.error = 0;

		if (!GetQueuedCompletionStatus (ctx->completion_port,
									  &ctx->watcher.bytes_transferred,
									  (PULONG_PTR) &ctx->watcher.event,
									  &ctx->watcher.overlapped,
									  INFINITE))
		{
			if ((ctx->watcher.error = GetLastError ()) == WAIT_TIMEOUT)
				break;

			if (!ctx->watcher.overlapped)
				break;

			ctx->watcher.bytes_transferred = 0;
		}

		if (ctx->watcher.overlapped == sig_end_watcher_thread)
			break;

		ctx->on_tick_needed (ctx);

		lw_event_wait (ctx->watcher.resume_event, -1);
		lw_event_unsignal (ctx->watcher.resume_event);
	}
}

lw_eventpump lw_eventpump_new ()
{
	lwp_init ();

	lw_eventpump ctx = (lw_eventpump) calloc (sizeof (*ctx), 1);

	if (!ctx)
		return 0;

	ctx->watcher.event = NULL;
	ctx->watcher.thread = lw_thread_new ("watcher", (void *) watcher);
	ctx->watcher.resume_event = lw_event_new ();

	ctx->completion_port = CreateIoCompletionPort (INVALID_HANDLE_VALUE, 0, 4, 0);

	assert(ctx->completion_port);

	lwp_pump_init ((lw_pump) ctx, &def_eventpump);

	return ctx;
}

static void def_cleanup (lw_pump _ctx)
{
	lw_eventpump ctx = (lw_eventpump) _ctx;

	if (lw_thread_started (ctx->watcher.thread))
	{
		PostQueuedCompletionStatus (ctx->completion_port, 0, 0, sig_end_watcher_thread);

		lw_event_signal (ctx->watcher.resume_event);
		lw_thread_join (ctx->watcher.thread);
	}

	lw_thread_delete (ctx->watcher.thread);
	ctx->watcher.thread = NULL;

	lw_event_delete (ctx->watcher.resume_event);
	ctx->watcher.resume_event = NULL;
	ctx->watcher.event = NULL;

	CloseHandle (ctx->completion_port);
	ctx->completion_port = NULL;

	lwp_deinit();
}

lw_error lw_eventpump_tick (lw_eventpump ctx)
{
	if (ctx->on_tick_needed)
	{
		/* Process whatever the watcher thread dequeued before telling the caller to tick */

		process (ctx, ctx->watcher.overlapped,
					ctx->watcher.bytes_transferred,
					ctx->watcher.event,
					ctx->watcher.error);
	}

	OVERLAPPED * overlapped;
	DWORD bytes_transferred;

	lw_pump_watch watch;

	for (;;)
	{
		int error = 0;

		if (!GetQueuedCompletionStatus (ctx->completion_port,
										&bytes_transferred,
										(PULONG_PTR) &watch,
										&overlapped,
										0))
		{
			error = GetLastError ();

			if (error == WAIT_TIMEOUT)
				break;

			if (!overlapped)
				break;
		}

		process (ctx, overlapped, bytes_transferred, watch, error);
	}

	if (ctx->on_tick_needed)
		lw_event_signal (ctx->watcher.resume_event);

	return 0;
}

lw_error lw_eventpump_start_eventloop (lw_eventpump ctx)
{
	OVERLAPPED * overlapped;
	DWORD bytes_transferred;

	lw_pump_watch watch;

	lw_bool finished = lw_false;

	for (;;)
	{
		/* TODO : Use GetQueuedCompletionStatusEx where available */

		int error = 0;

		if (!GetQueuedCompletionStatus (ctx->completion_port,
										&bytes_transferred,
										(PULONG_PTR) &watch,
										&overlapped, finished ? 0 : INFINITE))
		{
			error = GetLastError();

			/* Are all pending operations completed?
			*/
			if (finished && error == WAIT_TIMEOUT)
				break;

			if (!overlapped)
				continue;
		}

		if (!process (ctx, overlapped, bytes_transferred, watch, error))
			finished = lw_true;
	}

	return 0;
}

void lw_eventpump_post_eventloop_exit (lw_eventpump ctx)
{
	PostQueuedCompletionStatus (ctx->completion_port, 0, 0, sig_exit_event_loop);
}

lw_error lw_eventpump_start_sleepy_ticking
	(lw_eventpump ctx, void (lw_callback * on_tick_needed) (lw_eventpump))
{
	ctx->on_tick_needed = on_tick_needed;
	lw_thread_start (ctx->watcher.thread, ctx);

	return 0;
}

static lw_pump_watch def_add (lw_pump _ctx, HANDLE handle,
							  void * tag, lw_pump_callback callback)
{
	lw_eventpump ctx = (lw_eventpump) _ctx;

	assert (callback != 0);

	lw_pump_watch watch = (lw_pump_watch) calloc (sizeof (*watch), 1);

	if (!watch)
		return 0;

	watch->on_completion = callback;
	watch->tag = tag;

	// If this fails; it does rarely happen as a race condition during a server shutdown
	if (CreateIoCompletionPort (handle, ctx->completion_port,
								(ULONG_PTR) watch, 0) == NULL)
	{
		always_log("CreateIoCompletionPort error %u.\n", GetLastError());
		free(watch);
		return 0;
	}

	return watch;
}

/* Note: the caller of lw_pump_remove is responsible for waiting for or
 * cancelling any pending overlapped operations first.  If a watch is freed
 * before any associated overlapped operations have completed, the behaviour
 * is undefined.
 */
static void def_remove (lw_pump _ctx, lw_pump_watch watch)
{
	lw_eventpump ctx = (lw_eventpump) _ctx;

	watch->on_completion = NULL;
	watch->tag = NULL;

	free (watch);
}

static void def_post (lw_pump _ctx, void * function, void * parameter)
{
	lw_eventpump ctx = (lw_eventpump) _ctx;

	PostQueuedCompletionStatus (ctx->completion_port, 0xFFFFFFFF,
								(ULONG_PTR) function,
								(OVERLAPPED *) parameter);
}

static void def_update_callbacks (lw_pump ctx,
								  lw_pump_watch watch,
								  void * tag,
								  lw_pump_callback on_completion)
{
	watch->tag = tag;
	watch->on_completion = on_completion;
}

const lw_pumpdef def_eventpump =
{
	def_add,
	def_update_callbacks,
	def_remove,
	def_post,
	def_cleanup
};
