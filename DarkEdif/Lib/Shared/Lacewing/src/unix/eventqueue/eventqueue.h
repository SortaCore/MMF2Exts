/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2013 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/
#ifdef __ANDROID__
#include "../android config.h"
#endif

#if defined(USE_EPOLL)

	#include <sys/epoll.h>

	#ifndef EPOLLRDHUP
	  #define EPOLLRDHUP 0x2000
	#endif

	/* epoll: lwp_eventqueue is an epoll fd, _event is an epoll_event
	*/
	typedef struct _lw_eventqueue * lwp_eventqueue;

	typedef struct _lw_eventqueue
	{
		int epollFD;
		int numFDsWatched;
	} _lw_eventqueue;
	typedef struct epoll_event lwp_eventqueue_event;

#elif defined(USE_KQUEUE)

	#include <sys/event.h>

	/* kqueue: lwp_eventqueue is a kqueue fd, _event is a kevent
	*/
	typedef int lwp_eventqueue;
	typedef struct kevent lwp_eventqueue_event;

#else

	/* select or other custom eventqueue implementation
	*/
	typedef struct _lwp_eventqueue * lwp_eventqueue;

	typedef struct _lwp_eventqueue_event
	{
	  lw_i8 flags;
	  void * tag;

	} lwp_eventqueue_event;

#endif

lwp_eventqueue lwp_eventqueue_new ();
void lwp_eventqueue_delete (lwp_eventqueue);


/* add: add a file descriptor to the eventqueue
 */
void lwp_eventqueue_add (lwp_eventqueue,
						 int fd,
						 lw_bool read,
						 lw_bool write,
						 lw_bool edge_triggered,
						 void * tag);


/* update: update an existing watched file descriptor
 */
void lwp_eventqueue_update (lwp_eventqueue,
							int fd,
							lw_bool was_reading, lw_bool read,
							lw_bool was_writing, lw_bool write,
							lw_bool was_edge_triggered, lw_bool edge_triggered,
							void * old_tag, void * new_tag);

/* drain: drain pending events from the queue
 */
int lwp_eventqueue_drain (lwp_eventqueue,
						  lw_bool block,
						  int max_events,
						  lwp_eventqueue_event * events);

lw_bool lwp_eventqueue_event_read_ready (lwp_eventqueue_event);
lw_bool lwp_eventqueue_event_write_ready (lwp_eventqueue_event);

void * lwp_eventqueue_event_tag (lwp_eventqueue_event);



