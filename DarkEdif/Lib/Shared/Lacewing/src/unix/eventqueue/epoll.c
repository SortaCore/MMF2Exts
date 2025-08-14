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

lwp_eventqueue lwp_eventqueue_new ()
{
	lwp_eventqueue queue = (lwp_eventqueue)malloc(sizeof(_lw_eventqueue));
	if (!queue)
		return NULL;

	queue->epollFD = epoll_create (32);
	queue->numFDsWatched = 0;
	return queue;
}

void lwp_eventqueue_delete (lwp_eventqueue queue)
{
	if (queue->numFDsWatched > 0)
		always_log ("lwp_eventqueue_delete warning: had %i FDs left when closing eventqueue.", queue->numFDsWatched);
	close (queue->epollFD);
	free (queue);
}

void lwp_eventqueue_add (lwp_eventqueue queue,
						 int fd,
						 lw_bool read,
						 lw_bool write,
						 lw_bool edge_triggered,
						 void * tag)
{
	struct epoll_event event = {0};

	event.data.ptr = tag;

	event.events = (read != 0 ? EPOLLIN : 0u) |
				  (write != 0 ? EPOLLOUT : 0u) |
				  (edge_triggered != 0 ? EPOLLET : 0u);
	lwp_trace ("lwp_eventqueue_add EPOLL_CTL_ADD: Queuing event with fd %d, read %d, write %d, edge %d, TAG %p.",
		fd, read ? 1 : 0, write ? 1 : 0, edge_triggered ? 1 : 0, tag);

	epoll_ctl (queue->epollFD, EPOLL_CTL_ADD, fd, &event);
	++queue->numFDsWatched;
}

void lwp_eventqueue_update (lwp_eventqueue queue,
							int fd,
							lw_bool was_reading, lw_bool read,
							lw_bool was_writing, lw_bool write,
							lw_bool was_edge_triggered, lw_bool edge_triggered,
							void * old_tag, void * tag)
{
	struct epoll_event event = {0};

	event.data.ptr = tag;

	int res;
	(void)res; // prevent unused warnings
	if (read || write)
	{
		event.events = (read ? EPOLLIN : 0u) |
					   (write ? EPOLLOUT : 0u) |
					   (edge_triggered ? EPOLLET : 0u);
		res = epoll_ctl(queue->epollFD, EPOLL_CTL_MOD, fd, &event);
		if (res == -1)
			always_log("epoll_ctl mod for fd %d, epoll fd %d returned -1, err %d", fd, queue->epollFD, errno);
		else {
			lwp_trace("epoll_ctl mod for fd %d, epoll fd %d returned %i, err %d", fd, queue->epollFD, res, errno);
		}
		assert(res != -1 && "mod");
	}
	else // deleting
	{
		// Pump already closed down - this should not happen!
		// It means the client/server closes after the pump is deleted. Should be before.
		if (queue->epollFD == -1)
			always_log("Error: Can't delete pump FD %d, main pump FD is already closed down.", fd);
		else
		{
			res = epoll_ctl(queue->epollFD, EPOLL_CTL_DEL, fd, &event);
			if (res == -1)
				always_log ("epoll_ctl delete for fd %d, epoll fd %d returned -1, err %d", fd, queue->epollFD, errno);
			// assert(res != -1 && "del");
		}
		--queue->numFDsWatched;
	}
}

int lwp_eventqueue_drain (lwp_eventqueue queue,
						  lw_bool block,
						  int max_events,
						  lwp_eventqueue_event * events)
{
	return epoll_wait (queue->epollFD, events, max_events, block ? -1 : 0);
}

lw_bool lwp_eventqueue_event_read_ready (lwp_eventqueue_event event)
{
	return (event.events & (EPOLLIN | EPOLLHUP | EPOLLRDHUP)) != 0;
}

lw_bool lwp_eventqueue_event_write_ready (lwp_eventqueue_event event)
{
	return (event.events & EPOLLOUT) != 0;
}

void * lwp_eventqueue_event_tag (lwp_eventqueue_event event)
{
	return event.data.ptr;
}

