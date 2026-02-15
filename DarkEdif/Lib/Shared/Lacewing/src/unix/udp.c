/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.
 * Copyright (C) 2012-2026 Darkwire Software.
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
	int icmpfd;
	int icmpv6fd;

	long receives_posted;
	long writes_posted;

	void * tag;
};

static void read_ready (void * ptr)
{
	lw_udp ctx = (lw_udp)ptr;

	struct sockaddr_storage from;
	socklen_t from_size = sizeof (from);

	char buffer [lwp_default_buffer_size];
	char cmsgbuf[CMSG_SPACE(sizeof(struct in6_pktinfo)) + CMSG_SPACE(sizeof(struct in_pktinfo))];
	struct cmsghdr* const cmsg = (struct cmsghdr*)cmsgbuf;
	struct iovec iov = { .iov_base = buffer, .iov_len = sizeof(buffer) };
	struct msghdr msg = {
		.msg_control = cmsgbuf,
		.msg_controllen = sizeof(cmsgbuf),
		.msg_name = &from,
		.msg_namelen = from_size,
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_flags = 0,
	};

	lwp_retain(ctx, "udp read");

	struct _lw_addr remote_addr = { 0 }, local_addr = { 0 };
	lw_ui32 ifidx;

	for (;;)
	{
		ssize_t bytes = recvmsg(ctx->fd, &msg, MSG_NOSIGNAL);

		if (bytes == -1)
		{
			// Ignore, we already processed the messages
			if (errno == EAGAIN || errno == EWOULDBLOCK)
				break;
			lw_log_if_debug("Error in recvmsg: %d. read_ready ignored.\n", errno);
			break;
		}

		// Success but nothing - 0 bytes of data in UDP datagram?
		if (bytes == 0)
		{
			lw_log_if_debug("Warning from recvmsg: Got 0 bytes. read_ready ignored.\n");
			break;
		}

		// Does not match expected incoming filter, discard it
		if (!lw_filter_check_remote_addr(ctx->filter, &remote_addr))
		{
			lw_log_if_debug("Dropping incoming UDP packet from unexpected remote address \"%s\".\n",
				lw_addr_tostring(&remote_addr, lw_addr_tostring_flag_box_ipv6));
			lwp_addr_cleanup(&remote_addr);
			memset(&remote_addr, 0, sizeof(remote_addr));
			continue;
		}

		struct cmsghdr* hdr = (struct cmsghdr*)cmsg;
		
		// IP4 pkt info given for IP6 incoming address, or vice versa; skip to next
		if ((from.ss_family == AF_INET6) != (hdr->cmsg_level == IPPROTO_IPV6))
		{
			hdr = CMSG_NXTHDR(&msg, hdr);
		}
		if (cmsg->cmsg_type == IPV6_PKTINFO)
		{
			assert(remote_addr.info->ai_addr->sa_family == AF_INET6);
			struct in6_pktinfo* const recvLocalAddr = (struct in6_pktinfo*)CMSG_DATA(cmsg);
			((struct sockaddr_in6*)&from)->sin6_family = AF_INET6;
			((struct sockaddr_in6*)&from)->sin6_addr = recvLocalAddr->ipi6_addr;
			lwp_addr_set_sockaddr(&local_addr, (struct sockaddr*)&from);
			ifidx = recvLocalAddr->ipi6_ifindex;
		}
		else if (((struct cmsghdr*)cmsg)->cmsg_type == IP_PKTINFO)
		{
			assert(remote_addr.info->ai_addr->sa_family == AF_INET);
			struct in_pktinfo* const recvLocalAddr = (struct in_pktinfo*)CMSG_DATA(cmsg);
			((struct sockaddr_in*)&from)->sin_family = AF_INET;
			((struct sockaddr_in*)&from)->sin_addr.s_addr = recvLocalAddr->ipi_addr.s_addr;
			lwp_addr_set_sockaddr(&local_addr, (struct sockaddr*)&from);
			ifidx = (unsigned int)recvLocalAddr->ipi_ifindex;
		}
		else
		{
			ifidx = 0; // guess
			assert(!"Invalid cmsg type");
		}

		buffer [bytes] = 0;

		// There's a race where UDP is unhosted, and ctx->on_data() is still queued.
		// We can't unset on_data as the UDP is merely unhosted, not deleted.
		// However, the FD is now close()'d and invalid.
		// TODO: This check may not be necessary due to the shutdown() and manual dropping
		// of FD from epoll in the same commit on 17th July 2021, but since it's a cheap test,
		// we'll keep it.
		if (ctx->fd != -1 && ctx->on_data)
			ctx->on_data(ctx, ifidx ? &local_addr : NULL, ifidx, &remote_addr, buffer, (size_t)bytes);

		lwp_addr_cleanup(&local_addr);
		memset(&local_addr, 0, sizeof(local_addr));

		lwp_addr_cleanup(&remote_addr);
		memset(&remote_addr, 0, sizeof(remote_addr));
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
	lw_bool madeipv6;

	if ((ctx->fd = lwp_create_server_socket
			(filter, SOCK_DGRAM, IPPROTO_UDP, &madeipv6, error)) == -1)
	{
		lw_error_addf(error, "Creating UDP port");
		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);
		return;
	}
	if (madeipv6 && (ctx->icmpv6fd = lwp_create_server_socket
		(filter, SOCK_RAW, IPPROTO_ICMPV6, NULL, error)) == -1)
	{
		lw_error_addf(error, "Creating ICMPv6 port");
		if (ctx->on_error)
			ctx->on_error (ctx, error);
		// non-fatal, we don't need ICMP
	}
	if ((ctx->icmpfd = lwp_create_server_socket
		(filter, SOCK_RAW, IPPROTO_ICMP, NULL, error)) == -1)
	{
		lw_error_addf(error, "Creating ICMP port");
		if (ctx->on_error)
			ctx->on_error (ctx, error);
		// non-fatal, we don't need ICMP
	}

	lw_error_delete (error);

	lwp_make_nonblocking(ctx->fd);
	if (ctx->icmpfd != -1)
		lwp_make_nonblocking(ctx->icmpfd);
	if (ctx->icmpv6fd != -1)
		lwp_make_nonblocking(ctx->icmpv6fd);

	ctx->filter = lw_filter_clone (filter);

	ctx->pump_watch = lw_pump_add (ctx->pump, ctx->fd, "udp host", ctx, read_ready, 0, lw_true);
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

	lw_pump_remove(ctx->pump, ctx->pump_watch, "udp unhost");
	ctx->pump_watch = NULL;

	lwp_close_socket(ctx->fd);
	lwp_close_socket(ctx->icmpfd);
	lwp_close_socket(ctx->icmpv6fd);
	ctx->icmpfd = ctx->icmpv6fd = ctx->fd = -1;

	lw_filter_delete (ctx->filter);
	ctx->filter = 0;
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
	ctx->icmpfd = ctx->icmpv6fd = ctx->fd = -1;

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

extern lw_bool lw_in_wsl;
extern const struct in6_addr in6addr_loopback_wsl;

void lw_udp_send (lw_udp ctx, lw_addr from, lw_ui32 ifidx, lw_addr to, const char * data, size_t size)
{
	if (!to || (!lw_addr_ready(to)) || !to->info)
	{
		lw_error error = lw_error_new ();

		lw_error_addf (error, "The remote address passed to send() wasn't ready");
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

	if (!to->info)
		return;

	lwp_retain(ctx, "udp write");
	++ctx->writes_posted;

	// TODO: Double-check this isn't some sort of IPv6/IPv4 mixup with outgoing local send address
	// If running under WSL, localhost sendmsg is borked for wsl receiver -> host Windows, but sendto routes properly.
	if (from && lw_in_wsl &&
		// Address stored in network order, check 127.x.x.x at big end
		((from->info->ai_addr->sa_family == AF_INET &&
			((lw_ui8 *)&(((struct sockaddr_in*)from->info->ai_addr)->sin_addr))[3] == 127) ||
		// WSL host connecting via localhost: it is likely padded IPv4 of [::ffff:127.0.0.1]
		!memcmp(&((struct sockaddr_in6*)from->info->ai_addr)->sin6_addr, &in6addr_loopback_wsl, sizeof(struct in6_addr))))
	{
		from = NULL;
	}

	// Unlike Windows, Linux copies the data passed to sendmsg in non-blocking IO
	ssize_t res;
	if (from)
	{
		assert(*(lw_i32 *)&ifidx >= 0); // negative is invalid
		struct iovec iov = { .iov_base = (void*)data, .iov_len = size };
		char cmsgdata[CMSG_SPACE(sizeof(struct in6_pktinfo))];
		struct msghdr msg = {
			.msg_name = to->info->ai_addr,
			.msg_namelen = to->info->ai_addrlen,
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_control = cmsgdata,
			.msg_flags = 0
		};
		struct cmsghdr* const cmsg = (struct cmsghdr*)cmsgdata;
		if (from->info->ai_family == AF_INET)
		{
			msg.msg_controllen = CMSG_SPACE(sizeof(struct in_pktinfo));
			cmsg->cmsg_level = IPPROTO_IP;
			cmsg->cmsg_type = IP_PKTINFO;
			cmsg->cmsg_len = CMSG_LEN(sizeof(struct in_pktinfo));

			struct in_pktinfo* const pktinfo = (struct in_pktinfo*)CMSG_DATA(cmsg);
			pktinfo->ipi_addr = ((struct sockaddr_in*)from->info->ai_addr)->sin_addr;
			pktinfo->ipi_ifindex = (int)ifidx;
		}
		else // AF_INET6
		{
			msg.msg_controllen = CMSG_SPACE(sizeof(struct in6_pktinfo));
			cmsg->cmsg_level = IPPROTO_IPV6;
			cmsg->cmsg_type = IPV6_PKTINFO;
			cmsg->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));

			struct in6_pktinfo* const pktinfo = (struct in6_pktinfo*)CMSG_DATA(cmsg);
			pktinfo->ipi6_addr = ((struct sockaddr_in6*)from->info->ai_addr)->sin6_addr;
			pktinfo->ipi6_ifindex = ifidx;
		}

		res = sendmsg(ctx->fd, &msg, MSG_NOSIGNAL);
	}
	else
	{
		res = sendto(ctx->fd, data, size, MSG_NOSIGNAL, to->info->ai_addr,
			to->info->ai_addrlen);
	}

	// Something went awry
	if (res == -1)
	{
		--ctx->writes_posted;

		// Ignore EAGAIN/EWOULDBLOCK since we're sending UDP; if there's not outgoing room
		// to immediately send, then just discard
		if (errno != EAGAIN && errno != EWOULDBLOCK)
		{
			lw_error error = lw_error_new();

			lw_error_add(error, errno);
			lw_error_addf(error, "Error sending to %s from local address %s, ifidx %u",
				lw_addr_tostring(to, lw_addr_tostring_flag_box_ipv6),
				lw_addr_tostring(from, lw_addr_tostring_flag_box_ipv6), ifidx);

			if (ctx->on_error)
				ctx->on_error(ctx, error);

			lw_error_delete(error);
		}

		// fall through to lwp_release
	}

	// Although sendmsg() allows partial write, it's not expected for UDP.
	// The docs on Linux are vague, saying partial writes are possible in general,
	// and explicitly speaking about UDP, but not noting UDP as an exception to that rule.
	// But other places lay out that the meaning of a datagram is all or nothing write, e.g.
	// https://github.com/libuv/libuv/blob/6b5aa669db4d57231e21b1ee97c63a06167e117e/src/unix/udp.c#L458
	// For now we'll assume if res is not -1, it is the full message.

	lwp_release(ctx, "udp write");
}

void lw_udp_send_unreachable(lw_udp ctx, lw_addr from, lw_ui32 ifidx, lw_addr to, const char* data, lw_ui32 size)
{
	lwp_socket icmpsock = lw_addr_ipv6(from) ? ctx->icmpv6fd : ctx->icmpfd;
	if (icmpsock == -1)
		return;
	lw_error err = lwp_send_icmp_unreachable(icmpsock, IPPROTO_UDP, from, ifidx, to, data, size);
	if (err)
	{
		lw_error_addf(err, "ICMP error");
		if (ctx->on_error)
			ctx->on_error(ctx, err);
		lw_error_delete(err);
	}
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

