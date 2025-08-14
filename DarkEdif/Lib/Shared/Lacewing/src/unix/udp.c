/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"
#include "../address.h"

struct _lw_udp
{
	lwp_refcounted;
	lw_pump pump;

	lw_udp_hook_data on_data;
	lw_udp_hook_error on_error;

	lw_filter filter;
	lw_pump_watch pump_watch;

	int fd;

	long receives_posted;
	long writes_posted;

	void * tag;

	// True if socket is bound for sending to multiple IPv6 clients
	lw_bool is_ipv6_server;

	// For IPv6 hosters, multiple IPv6 outgoing addresses may be valid.
	// This will confuse clients who will ignore the messages from a different source.
	// We lock to one IPv6 address, ideally one with infinite lease, although a limited
	// lease should be kept active by activity.
	char public_fixed_ip6_addr_cmsg[CMSG_SPACE(sizeof(struct in6_pktinfo))];
};

static void read_ready (void * ptr)
{
	lw_udp ctx = (lw_udp)ptr;

	struct sockaddr_storage from;
	socklen_t from_size = sizeof (from);

	char buffer [lwp_default_buffer_size];

	lwp_retain(ctx, "udp read");

	lw_addr filter_addr = lw_filter_remote (ctx->filter);

	struct _lw_addr addr = {0};

	for (;;)
	{
		ssize_t bytes = recvfrom (ctx->fd, buffer, sizeof (buffer),
								0, (struct sockaddr *) &from, &from_size);

		if (bytes == -1)
			break;

		lwp_addr_set_sockaddr (&addr, (struct sockaddr *) &from);

		if (filter_addr && !lw_addr_equal(&addr, filter_addr))
		{
			free(addr.info->ai_addr);  // alloc'd by lwp_addr_set_sockaddr
			addr.info->ai_addr = NULL;
			free(addr.info);
			addr.info = NULL;
			break;
		}

		buffer [bytes] = 0;

		// There's a race where UDP is unhosted, and ctx->on_data() is still queued.
		// We can't unset on_data as the UDP is merely unhosted, not deleted.
		// However, the FD is now close()'d and invalid.
		// TODO: This check may not be necessary due to the shutdown() and manual dropping
		// of FD from epoll in the same commit on 17th July 2021, but since it's a cheap test,
		// we'll keep it.
		if (ctx->fd != -1 && ctx->on_data)
			ctx->on_data (ctx, &addr, buffer, (size_t)bytes);

		free(addr.info->ai_addr); // alloc'd by lwp_addr_set_sockaddr
		addr.info->ai_addr = NULL;
		free(addr.info);
		addr.info = NULL;
	}

	lwp_release(ctx, "udp read");
}

void lw_udp_host (lw_udp ctx, lw_ui16 port)
{
	lw_filter filter = lw_filter_new ();
	lw_filter_set_local_port (filter, port);

	lw_udp_host_filter (ctx, filter);

	lw_filter_delete (filter);
}

void lw_udp_host_addr (lw_udp ctx, lw_addr addr, lw_ui16 local_port)
{
	lw_filter filter = lw_filter_new ();
	lw_filter_set_remote (filter, addr);

	lw_filter_set_ipv6 (filter, lw_addr_ipv6 (addr));

	lw_udp_host_filter (ctx, filter);

	lw_filter_delete (filter);
}

void lw_udp_host_filter (lw_udp ctx, lw_filter filter)
{
	lw_udp_unhost (ctx);

	lw_error error = lw_error_new ();

	if ((ctx->fd = lwp_create_server_socket
			(filter, SOCK_DGRAM, IPPROTO_UDP, error)) == -1)
	{
		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);
		return;
	}

	lw_error_delete (error);

	lwp_make_nonblocking(ctx->fd);

	ctx->filter = lw_filter_clone (filter);

	/* Each IPv6 adapter uses temporary addresses for outgoing connections for privacy reasons,
	   as covered in RFC 4941.
	   If we serve public IPv6 addresses (no filter, or unspec/empty addr filter),
	   then we'll need a consistent outgoing IPv6 address,
	   or clients won't recognise the incoming messages as from this server. */
	const lw_addr remoteFilterAddr = lw_filter_remote(ctx->filter);
	ctx->is_ipv6_server = !remoteFilterAddr || !remoteFilterAddr->info ||
		(remoteFilterAddr->info->ai_addr->sa_family == AF_INET6 &&
			IN6_IS_ADDR_UNSPECIFIED(&((struct sockaddr_in6*)&remoteFilterAddr->info->ai_addr)->sin6_addr));

	// Couldn't find a matching IPv6; we'll have to let the OS pick a default
	if (ctx->is_ipv6_server && !lwp_set_ipv6pktinfo_cmsg(ctx->public_fixed_ip6_addr_cmsg))
	{
		lw_error error = lw_error_new();
		lw_error_addf(error, "Hosting will continue, but remote IPv6 clients may be unable to connect");
		lw_error_addf(error, "Error hosting UDP - couldn't find a stable global IPv6 address");
		if (ctx->on_error)
			ctx->on_error(ctx, error);
		lw_error_delete(error);
	}

	ctx->pump_watch = lw_pump_add (ctx->pump, ctx->fd, ctx, read_ready, 0, lw_true);
}

lw_bool lw_udp_hosting (lw_udp ctx)
{
	return ctx->fd != -1;
}

lw_ui16 lw_udp_port (lw_udp ctx)
{
	return lwp_socket_port (ctx->fd);
}

void lw_udp_unhost (lw_udp ctx)
{
	// pump_watch has an FD, used to cancel pending events, so we don't use close_socket until it's used
	if (ctx->fd != -1)
		shutdown(ctx->fd, SHUT_RDWR);

	lw_pump_remove(ctx->pump, ctx->pump_watch);
	ctx->pump_watch = NULL;

	lwp_close_socket(ctx->fd);
	ctx->fd = -1;

	lw_filter_delete (ctx->filter);
	ctx->filter = 0;

	ctx->is_ipv6_server = lw_false;
}

lw_udp lw_udp_new (lw_pump pump)
{
	lw_udp ctx = (lw_udp)calloc (sizeof (*ctx), 1);

	if (!ctx)
		return 0;

	lwp_init ();
	lwp_enable_refcount_logging(ctx, "udp");
	lwp_retain(ctx, "udp_new");

	ctx->pump = pump;
	ctx->fd = -1;
	ctx->is_ipv6_server = lw_false;

	return ctx;
}

void lw_udp_delete (lw_udp ctx)
{
	if (!ctx)
		return;

	lw_udp_unhost (ctx);

	// We should test if it's freed? But there's not really much the app can do to prevent it,
	// and the better behaviour is to let whatever's using it free it by itself.
	lwp_release(ctx, "udp_new"); // calls free (ctx)
}
#ifndef IN6_IS_ADDR_GLOBAL
#define IN6_IS_ADDR_GLOBAL(a) \
        ((((const uint32_t *) (a))[0] & htonl(0x70000000)) \
        == htonl (0x20000000))
#endif /* IS ADDR GLOBAL */

void lw_udp_send (lw_udp ctx, lw_addr addr, const char * data, size_t size)
{
	if (!lw_addr_ready (addr))
	{
		lw_error error = lw_error_new ();

		lw_error_addf (error, "The address object passed to send() wasn't ready");
		lw_error_addf (error, "Error sending");

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);

		return;
	}

	if (size == SIZE_MAX)
		size = strlen (data);

	if (sizeof(size) > 4)
		assert(size < 0xFFFFFFFF);

	if (!addr->info)
		return;

	lwp_retain(ctx, "udp write");
	++ctx->writes_posted;

	ssize_t res;
	// if a non-local IPv6 destination, then specify the outgoing IP we send from explicitly
	if (ctx->is_ipv6_server && lw_addr_ipv6(addr) &&
		IN6_IS_ADDR_GLOBAL(&((struct sockaddr_in6*)addr->info->ai_addr)->sin6_addr))
	{
		struct iovec iov = { .iov_base = (void *)data, .iov_len = size };

		struct msghdr msg;
		msg.msg_name = addr->info->ai_addr;
		msg.msg_namelen = addr->info->ai_addrlen;
		msg.msg_iov = &iov;
		msg.msg_iovlen = 1;
		msg.msg_control = ctx->public_fixed_ip6_addr_cmsg;
		msg.msg_controllen = CMSG_SPACE(sizeof(struct in6_pktinfo));
		msg.msg_flags = 0;
		res = sendmsg(ctx->fd, &msg, 0);
	}
	else
		res = sendto(ctx->fd, data, size, 0, addr->info->ai_addr,
			addr->info->ai_addrlen);

	// Ignore EAGAIN since we're sending UDP; if there's not outgoing room to send, just discard
	if (res == -1 && errno != EAGAIN)
	{
		lw_error error = lw_error_new ();

		lw_error_add (error, errno);
		lw_error_addf (error, "Error sending");

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);

		// fall through to lwp_release
	}
	lwp_release(ctx, "udp write");
}

void lw_udp_set_tag (lw_udp ctx, void * tag)
{
	ctx->tag = tag;
}

void * lw_udp_tag (lw_udp ctx)
{
	return ctx->tag;
}

lwp_def_hook (udp, error)
lwp_def_hook (udp, data)

