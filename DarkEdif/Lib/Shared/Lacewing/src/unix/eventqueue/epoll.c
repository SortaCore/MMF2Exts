
/* vim: set et ts=3 sw=3 sts=3 ft=c:
 *
 * Copyright (C) 2013 James McLaughlin.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "../../common.h"
#include "eventqueue.h"

lwp_eventqueue lwp_eventqueue_new ()
{
	lwp_eventqueue queue = (lwp_eventqueue)malloc(sizeof(_lw_eventqueue));
	queue->epollFD = epoll_create (32);
	queue->numFDsWatched = 0;
	return queue;
}

void lwp_eventqueue_delete (lwp_eventqueue queue)
{
	if (queue->numFDsWatched > 0)
		lw_trace("lwp_eventqueue_delete warning: had %i FDs left when closing eventqueue.", queue->numFDsWatched);
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
	struct epoll_event event = {};
	memset(&event, 0, sizeof(epoll_event));

	event.data.ptr = tag;

	event.events = (read ? EPOLLIN : 0) |
				  (write ? EPOLLOUT : 0) |
				  (edge_triggered ? EPOLLET : 0);
	lw_trace("lwp_eventqueue_add EPOLL_CTL_ADD: Queuing event with fd %d, read %d, write %d, edge %d, TAG %p.",
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
	struct epoll_event event = {};

	event.data.ptr = tag;

	int res;
	(void)res; // prevent unused warnings
	if (read || write)
	{
		event.events = (read ? EPOLLIN : 0) |
					   (write ? EPOLLOUT : 0) |
					   (edge_triggered ? EPOLLET : 0);
		res = epoll_ctl(queue->epollFD, EPOLL_CTL_MOD, fd, &event);
		assert(res != -1);
	}
	else
	{
		res = epoll_ctl(queue->epollFD, EPOLL_CTL_DEL, fd, &event);
		--queue->numFDsWatched;
		assert(res != -1);
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

