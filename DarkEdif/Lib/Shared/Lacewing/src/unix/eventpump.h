/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../pump.h"
#include "eventqueue/eventqueue.h"

#define max_events  16

struct _lw_pump_watch
{
	lw_pump_callback on_read_ready, on_write_ready;
	lw_bool edge_triggered;

	int fd;
	void * tag;
};

struct _lw_eventpump
{
	struct _lw_pump pump;

	lwp_eventqueue queue;

	lw_sync sync_signals;

	int signalpipe_read, signalpipe_write;
	lw_list (void *, signalparams);
	int waiting_pipe_bytes; // protected by sync_signals

	#ifndef _lacewing_no_threads

	  /* for start_sleepy_ticking
		*/
	  struct
	  {
		 lw_thread thread;

		 int num_events;
		 lwp_eventqueue_event events [max_events];

		 lw_event resume_event;

	  } watcher;

	  void (lw_callback * on_tick_needed) (lw_eventpump);

	#endif
};

extern const lw_pumpdef def_eventpump;

/* epoll/kqueue/select specific
 */
int lwp_eventpump_create_queue ();


