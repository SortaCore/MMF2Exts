/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2013 James McLaughlin.
 * Copyright (C) 2012-2026 Darkwire Software.
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

	// This 32 is a hint to OS about number of FDs we'll monitor, not a hard limit
	queue->epollFD = epoll_create (32);
	queue->numFDsWatched = 0;

	// TODO: Phi note 23/12/2025: This queue fds and sync should be unnecessary.
	// Once stability has been ensured, consider dumping it.
	queue->sync = lw_sync_new();

	// create list_head, makes lists less strange
	_lwp_fd_monitoring first = {
		.fd = 0, .desc = 0, .lastUpdate = 0
	};
	list_push(lwp_fd_monitoring, queue->fds, &first);
	list_pop_front(lwp_fd_monitoring, queue->fds);
	return queue;
}

void lwp_eventqueue_delete (lwp_eventqueue queue)
{
	lw_sync_lock(queue->sync);
	if (queue->numFDsWatched > 0)
	{
		char log[512];
		int logAt = sprintf(log, "lwp_eventqueue_delete warning: had %i FDs left when closing eventqueue. Event pump must be deleted last.",
			queue->numFDsWatched);
		logAt += sprintf(log + logAt, " FDs still open: ");
		int i = 0;
		list_each(_lwp_fd_monitoring, queue->fds, e)
		{
			logAt += sprintf(log + logAt, "[%i] FD %i, desc \"%s\", last update \"%s\"; ",
				i++, e.fd, e.desc, e.lastUpdate);
		}
		log[logAt - 2] = 0;
		always_log("%s", logAt);
		assert(lw_false);
	}
	close (queue->epollFD);
	queue->epollFD = -1;
	lw_sync_release(queue->sync);

	free (queue);
}

void lwp_eventqueue_add (lwp_eventqueue queue,
						 int fd,
						 const char * desc,
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

	lw_sync_lock(queue->sync);

		epoll_ctl (queue->epollFD, EPOLL_CTL_ADD, fd, &event);

		++queue->numFDsWatched;
		_lwp_fd_monitoring item = {
			.fd = fd, .desc = strdup(desc), .lastUpdate = strdup("create")
		};
		list_push(_lwp_fd_monitoring, queue->fds, item);

	lw_sync_release(queue->sync);
}

void lwp_eventqueue_update (lwp_eventqueue queue,
							int fd, const char * updateReason,
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
		lw_sync_lock(queue->sync);
		lw_bool found = lw_false;
		list_each(_lwp_fd_monitoring, queue->fds, e)
		{
			if (e.fd == fd)
			{
				free(e.lastUpdate);
				e.lastUpdate = strdup(updateReason);
				found = lw_true;
				break;
			}
		}
		if (!found)
		{
			always_log("Couldn't update eventqueue fd %d, not found in list. Update reason \"%s\".",
				fd, updateReason);
			assert(lw_false);
		}

		res = epoll_ctl(queue->epollFD, EPOLL_CTL_MOD, fd, &event);
		if (res == -1)
			always_log("epoll_ctl mod for fd %d, epoll fd %d returned -1, err %d", fd, queue->epollFD, errno);
		else {
			lwp_trace("epoll_ctl mod for fd %d, epoll fd %d returned %i, err %d", fd, queue->epollFD, res, errno);
		}
		assert(res != -1 && "mod");
		lw_sync_release(queue->sync);
	}
	else // deleting
	{
		lw_sync_lock(queue->sync);
		// Pump already closed down - this should not happen!
		// It means the client/server closes after the pump is deleted. Should be before.
		if (queue->epollFD == -1)
			always_log("Error: Can't delete pump FD %d, main pump FD is already closed down.", fd);
		else
		{
			lw_bool valid = fcntl(fd, F_GETFD) != -1;
			list_each_elem(_lwp_fd_monitoring, queue->fds, e)
			{
				if (e->fd == fd)
				{
					if (valid)
					{
						always_log("Error in deleting a pump FD %d. It was still valid! Desc \"%s\", last update \"%s\".",
							fd, e->desc, e->lastUpdate);
						assert (lw_false);
					}
					list_elem_remove(e);
					break;
				}
			}

			// TODO: Phi 22nd Dec 2025: fd should've been closed by higher ups, and not ever valid here,
			// epoll will auto-remove closed FDs; retaining them via watch->fd is bad,
			// and should only be used for confirming which FDs are still alive
			// may be a race condition where FD is reused?
			assert(fcntl(fd, F_GETFD) == -1);
			if (lw_false)
			{
				always_log("epoll_ctl delete for fd %d, epoll fd %d...", fd, queue->epollFD);
				res = epoll_ctl(queue->epollFD, EPOLL_CTL_DEL, fd, &event);
				if (res == -1)
					always_log("epoll_ctl delete for fd %d, epoll fd %d returned -1, err %d", fd, queue->epollFD, errno);
				// assert(res != -1 && "del");
			}
		}
		--queue->numFDsWatched;
		lw_sync_release(queue->sync);
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

