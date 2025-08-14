/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2013 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../../common.h"
#include "eventqueue.h"

int lwp_eventqueue_new ()
{
	return kqueue ();
}

void lwp_eventqueue_delete (lwp_eventqueue queue)
{
	close (queue);
}

void lwp_eventqueue_add (lwp_eventqueue queue,
						 int fd,
						 lw_bool read,
						 lw_bool write,
						 lw_bool edge_triggered,
						 void * tag)
{
	lwp_eventqueue_update (queue, fd,
						  lw_false, read,
						  lw_false, write,
						  lw_false, edge_triggered,
						  NULL, tag);
}

void lwp_eventqueue_update (lwp_eventqueue queue,
							int fd,
							lw_bool was_reading, lw_bool read,
							lw_bool was_writing, lw_bool write,
							lw_bool was_edge_triggered, lw_bool edge_triggered,
							void * old_tag, void * tag)
{
	struct kevent changes [2];
	int num_changes = 0;

	if (read)
	{
	  struct kevent * change = changes + (num_changes ++);

	  EV_SET (change, fd, EVFILT_READ,
				 EV_ADD | EV_ENABLE | EV_EOF |
				 (edge_triggered ? EV_CLEAR : 0), 0, 0, tag);
	}
	else
	{
	  if (was_reading)
	  {
		 /* Was reading; stop now
		  */
		 struct kevent * change = changes + (num_changes ++);

		 EV_SET (change, fd, EVFILT_READ, EV_DELETE, 0, 0, NULL);
	  }
	}

	if (write)
	{
	  struct kevent * change = changes + (num_changes ++);

	  EV_SET (change, fd, EVFILT_WRITE,
				 EV_ADD | EV_ENABLE | EV_EOF |
				 (edge_triggered ? EV_CLEAR : 0), 0, 0, tag);
	}
	else
	{
	  if (was_writing)
	  {
		 /* Was writing; stop now
		  */
		 struct kevent * change = changes + (num_changes ++);

		 EV_SET (change, fd, EVFILT_WRITE, EV_DELETE, 0, 0, NULL);
	  }
	}

	kevent (queue, changes, num_changes, 0, 0, 0);
}

int lwp_eventqueue_drain (lwp_eventqueue queue,
						  lw_bool block,
						  int max_events,
						  lwp_eventqueue_event * events)
{
	struct timespec zero = {};
	struct timespec * timeout = NULL;

	if (!block)
	  timeout = &zero;

	return kevent (queue, 0, 0, events, max_events, timeout);
}

lw_bool lwp_eventqueue_event_read_ready (lwp_eventqueue_event event)
{
	return event.filter == EVFILT_READ || (event.flags & EV_EOF);
}

lw_bool lwp_eventqueue_event_write_ready (lwp_eventqueue_event event)
{
	return event.filter == EVFILT_WRITE;
}

void * lwp_eventqueue_event_tag (lwp_eventqueue_event event)
{
	return event.udata;
}


