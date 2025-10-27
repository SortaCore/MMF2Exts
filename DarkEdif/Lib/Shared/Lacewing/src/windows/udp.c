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
#include <Iphlpapi.h>

const int ideal_pending_receive_count = 16;

#define overlapped_type_send	 1
#define overlapped_type_receive  2

typedef struct _udp_overlapped
{
	OVERLAPPED overlapped;

	char type;

	void * tag;

} * udp_overlapped;

typedef struct _udp_receive_info
{
	char buffer [lwp_default_buffer_size];
	WSABUF winsock_buffer;

	struct sockaddr_storage from;
	int from_length;

} * udp_receive_info;

udp_receive_info udp_receive_info_new ()
{
	udp_receive_info info = (udp_receive_info) calloc (sizeof (*info), 1);

	if (!info)
		return NULL;

	info->winsock_buffer.buf = info->buffer;
	info->winsock_buffer.len = sizeof (info->buffer);

	info->from_length = sizeof (info->from);

	return info;
}

struct _lw_udp
{
	lwp_refcounted;

	lw_pump pump;
	lw_pump_watch pump_watch;

	lw_udp_hook_data on_data;
	lw_udp_hook_error on_error;

	lw_filter filter;

	lw_ui16 port;

	SOCKET socket;

	lw_list(udp_overlapped, pending_receives);

	long receives_posted;

	long writes_posted;

	void * tag;

	// True if socket is bound for sending to multiple IPv6 clients
	lw_bool is_ipv6_server;

	// For IPv6 hosters, multiple IPv6 outgoing addresses may be valid.
	// This will confuse clients who will ignore the messages from a different source.
	// We lock to one IPv6 address, ideally one with infinite lease, although a limited
	// lease should be kept active by activity.
	char public_fixed_ip6_addr_cmsg[WSA_CMSG_SPACE(sizeof(struct in6_pktinfo))];

	// Function for sending with a specified outgoing address - null if not necessary
	fn_WSASendMsg WSASendMsgPtr;
};

// Returns true if lw_udp was freed.
static lw_bool read_completed(lw_udp ctx)
{
	--ctx->receives_posted;
	return lwp_release(ctx, "udp read");
}

// Returns true if lw_udp was freed.
static lw_bool write_completed(lw_udp ctx)
{
	--ctx->writes_posted;
	return lwp_release(ctx, "udp write");
}


static void post_receives (lw_udp ctx)
{
	while (ctx->receives_posted < ideal_pending_receive_count)
	{
		if (ctx->socket == -1)
			break;

		udp_receive_info receive_info = udp_receive_info_new ();

		if (!receive_info)
			break;

		udp_overlapped overlapped =
			(udp_overlapped) calloc (sizeof (*overlapped), 1);

		if (!overlapped)
		{
			free(receive_info);
			break;
		}

		overlapped->type = overlapped_type_receive;
		overlapped->tag = receive_info;

		DWORD flags = 0;
		lwp_retain(ctx, "udp read");

		if (WSARecvFrom (ctx->socket,
						&receive_info->winsock_buffer,
						1,
						0,
						&flags,
						(struct sockaddr *) &receive_info->from,
						&receive_info->from_length,
						&overlapped->overlapped,
						0) == SOCKET_ERROR)
		{
			int error = WSAGetLastError();

			if (error != WSA_IO_PENDING)
			{
				free(receive_info);
				free(overlapped);
				lwp_release(ctx, "udp read");

				lw_error err = lw_error_new();
				lw_error_addf(err, "Error posting UDP receive");
				lw_error_add(err, error);
				if (ctx->on_error)
					ctx->on_error(ctx, err);
				lw_error_delete(err);
				break;
			}
			// else no error, running as async

			// fall through
		}
		// else no error, running as sync

		list_push(udp_overlapped, ctx->pending_receives, overlapped);
		++ ctx->receives_posted;
	}
}

static void udp_socket_completion (void * tag, OVERLAPPED * _overlapped,
									unsigned long bytes_transferred, int error)
{
	lw_udp ctx = (lw_udp) tag;

	udp_overlapped overlapped = (udp_overlapped) _overlapped;

	switch (overlapped->type)
	{
		case overlapped_type_send:
		{
			write_completed(ctx);
			break;
		}

		case overlapped_type_receive:
		{
			udp_receive_info info = (udp_receive_info) overlapped->tag;

			if (error == 0)
			{
				info->buffer[bytes_transferred] = 0;

				struct _lw_addr addr = { 0 };
				lwp_addr_set_sockaddr(&addr, (struct sockaddr*)&info->from);

				lw_addr filter_addr = lw_filter_remote(ctx->filter);

				// If address doesn't match filter, it's a UDP message from unauthorised source.
				// There's no way to block UDP messages like that on Lacewing's side; firewall perhaps,
				// but user is unlikely to link up automatic firewall changes to Lacewing's error reports.
				// To avoid flooding server with reports, we don't report in release builds.
				if (filter_addr && !lw_addr_equal(&addr, filter_addr))
				{
					lwp_trace("UDP from unexpected source \"%s\", outside of filter \"%s\".\n",
						lw_addr_tostring(&addr), lw_addr_tostring(filter_addr));
				}
				else if (ctx->on_data)
					ctx->on_data(ctx, &addr, info->buffer, bytes_transferred);

				lwp_addr_cleanup(&addr);
			}
			// ignore aborted, it indicates socket was closed abruptly by something higher up,
			// i.e. udp unhost
			else if (error != ERROR_OPERATION_ABORTED)
			{
				lw_error err = lw_error_new();
				lw_error_addf(err, "Error receiving datagram (completion)");
				lw_error_add(err, error);

				if (ctx->on_error)
					ctx->on_error(ctx, err);
				lw_error_delete(err);
			}
			free (info);

			list_remove(udp_overlapped, ctx->pending_receives, overlapped);

			// read_completed may free ctx (and thus return true); if not, post more receives
			if (!read_completed(ctx) && error == 0)
				post_receives(ctx);
			break;
		}
		default:
		{
			lw_error err = lw_error_new();
			lw_error_addf(err, "Error with udp completion, unrecognised type %hhi; %p, %lu, %d",
				overlapped->type, overlapped, bytes_transferred, error);
			if (ctx->on_error)
				ctx->on_error(ctx, err);
			lw_error_delete(err);
			return; // leak in case pointer is invalid
		}
	};

	free (overlapped);
}

void lw_udp_host (lw_udp ctx, lw_ui16 port)
{
	lw_filter filter = lw_filter_new ();
	lw_filter_set_local_port (filter, port);

	lw_udp_host_filter (ctx, filter);

	lw_filter_delete (filter);
}

void lw_udp_host_addr (lw_udp ctx, lw_addr addr, lw_ui16 local_port /* = 0 */)
{
	lw_filter filter = lw_filter_new ();

	if (local_port != 0)
	{
		lw_filter_set_local_port(filter, local_port);
		lw_filter_set_reuse(filter, lw_true);
	}

	lw_filter_set_remote (filter, addr);

	lw_filter_set_ipv6 (filter, lw_addr_ipv6 (addr));

	lw_udp_host_filter (ctx, filter);

	lw_filter_delete (filter);
}

void lw_udp_host_filter (lw_udp ctx, lw_filter filter)
{
	if (ctx->socket != INVALID_SOCKET)
		lw_udp_unhost(ctx);

	lw_error error = lw_error_new ();

	if ((ctx->socket = lwp_create_server_socket
			(filter, SOCK_DGRAM, IPPROTO_UDP, error)) == -1)
	{
		if (ctx->on_error)
			ctx->on_error (ctx, error);
		lw_error_delete (error);

		return;
	}

	lw_error_delete (error);

	ctx->filter = lw_filter_clone (filter);

	ctx->port = lwp_socket_port(ctx->socket);

	/* Each IPv6 adapter uses temporary addresses for outgoing connections for privacy reasons,
	   as covered in RFC 4941.
	   If we serve public IPv6 addresses (no filter, or unspec/empty addr filter),
	   then we'll need a consistent outgoing IPv6 address,
	   or clients won't recognise the incoming messages as from this server. */
	const lw_addr remoteFilterAddr = lw_filter_remote(ctx->filter);
	ctx->is_ipv6_server = !remoteFilterAddr || !remoteFilterAddr->info ||
		(remoteFilterAddr->info->ai_addr->sa_family == AF_INET6 &&
			IN6_IS_ADDR_UNSPECIFIED(&((struct sockaddr_in6*)&remoteFilterAddr->info->ai_addr)->sin6_addr));

	if (ctx->is_ipv6_server)
	{
		if (lwp_set_ipv6pktinfo_cmsg(ctx->public_fixed_ip6_addr_cmsg))
			ctx->WSASendMsgPtr = compat_WSASendMsg(ctx->socket);
		else // Couldn't find a matching IPv6; we'll have to let the OS pick a default
		{
			lw_error error = lw_error_new();
			lw_error_addf(error, "Hosting will continue, but some remote IPv6 clients may be unable to connect");
			lw_error_addf(error, "Error hosting UDP - couldn't find a stable global IPv6 address");
			if (ctx->on_error)
				ctx->on_error(ctx, error);
			lw_error_delete(error);
		}
	}
	ctx->pump_watch = lw_pump_add (ctx->pump, (HANDLE) ctx->socket, ctx, udp_socket_completion);

	post_receives (ctx);
}

lw_bool lw_udp_hosting (lw_udp ctx)
{
	return ctx->socket != INVALID_SOCKET;
}

void lw_udp_unhost (lw_udp ctx)
{
	if (!lw_udp_hosting(ctx))
		return;

	SOCKET s = ctx->socket;
	ctx->socket = INVALID_SOCKET;
	lwp_close_socket (s);

	lw_filter_delete (ctx->filter);
	ctx->filter = 0;

	ctx->is_ipv6_server = lw_false;
}

lw_ui16 lw_udp_port (lw_udp ctx)
{
	return ctx->port;
}

// Called by refcounter when it reaches zero
static void lw_udp_dealloc(lw_udp ctx)
{
	// No refs, so there should be no pending read/writes
	assert(ctx->receives_posted == 0 && ctx->writes_posted == 0);
	list_clear(ctx->pending_receives);

	if (ctx->pump_watch)
	{
		lw_pump_remove(ctx->pump, ctx->pump_watch);
		ctx->pump_watch = NULL;
	}

	free(ctx);
}

lw_udp lw_udp_new (lw_pump pump)
{
	lw_udp ctx = (lw_udp) calloc (sizeof (*ctx), 1);

	if (!ctx)
	  return 0;

	lwp_enable_refcount_logging(ctx, "udp");
	lwp_set_dealloc_proc(ctx, lw_udp_dealloc);
	lwp_retain(ctx, "udp new");

	lwp_init ();

	ctx->pump = pump;
	ctx->socket = INVALID_SOCKET;
	ctx->is_ipv6_server = lw_false;

	return ctx;
}

void lw_udp_delete (lw_udp ctx)
{
	if (!ctx)
	  return;

	// delete succeeded, ctx is now freed
	if (lwp_release(ctx, "udp new"))
		return;

	if (ctx->socket != INVALID_SOCKET)
		lw_udp_unhost (ctx);

	// memset(ctx, 0, sizeof(_lw_udp));

	lwp_deinit();

	// free (ctx) called by refcount reaching zero
}

void lw_udp_send (lw_udp ctx, lw_addr addr, const char * buffer, size_t size)
{
	if (!addr || (!lw_addr_ready (addr)) || !addr->info)
	{
	  lw_error error = lw_error_new ();

	  lw_error_addf (error, "The address object passed to write() wasn't ready");
	  lw_error_addf (error, "Error sending datagram");

	  if (ctx->on_error)
		 ctx->on_error (ctx, error);

	  lw_error_delete (error);

	  return;
	}

	if (size == -1)
	  size = strlen (buffer);

	if (sizeof(size) > 4)
		assert(size < 0xFFFFFFFF);

	udp_overlapped overlapped = (udp_overlapped) calloc (sizeof (*overlapped) + size, 1);

	if (!overlapped)
	{
		// no point trying to allocate lw_error
		exit(ENOMEM);
		return;
	}

	lwp_retain(ctx, "udp write");
	++ctx->writes_posted;

	overlapped->type = overlapped_type_send;
	memcpy(((char*)overlapped) + sizeof(*overlapped), buffer, size);

	WSABUF winsock_buf = { (ULONG)size, ((CHAR*)overlapped) + sizeof(*overlapped) };

	int res;
	// if a non-local IPv6 destination, then specify the outgoing IP we send from explicitly
	fn_WSASendMsg wsaSendMsg = ctx->is_ipv6_server && lw_addr_ipv6(addr) &&
		IN6_IS_ADDR_GLOBAL(&((struct sockaddr_in6 *)addr->info->ai_addr)->sin6_addr) ?
		ctx->WSASendMsgPtr : NULL;
	if (!wsaSendMsg)
		res = WSASendTo(ctx->socket, &winsock_buf, 1, 0, /* MSG_XX flags */ 0, addr->info->ai_addr,
			(int)addr->info->ai_addrlen, (OVERLAPPED*)overlapped, 0);
	else // specify source address
	{
		WSAMSG msg;
		msg.name = addr->info->ai_addr;
		msg.namelen = (int)addr->info->ai_addrlen;
		msg.lpBuffers = &winsock_buf;
		msg.dwBufferCount = 1;
		msg.Control.buf = ctx->public_fixed_ip6_addr_cmsg;
		msg.Control.len = WSA_CMSG_SPACE(sizeof(struct in6_pktinfo));
		msg.dwFlags = 0;

		res = wsaSendMsg(ctx->socket, &msg, 0, NULL, &overlapped->overlapped, NULL);
	}

	if (res == SOCKET_ERROR)
	{
		const int code = WSAGetLastError();

		// no error, running as async
		if (code == WSA_IO_PENDING)
			return;

		// outgoing is overloaded; as it's UDP, just discard the message
		// Closest to the EAGAIN exception on Unix servers, but since we use overlapped, it's unlikely this will apply
		// This code as a response to WSASendTo may be Windows NT only
		// TODO: Do we need to undo writes_posted and lwp_retain?
		if (code == WSAEWOULDBLOCK)
			return;

		free(overlapped);

		// genuine error, whine about it
		lw_error error = lw_error_new ();

		lw_error_add (error, code);
		lw_error_addf (error, "Error sending datagram");

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);

		// fall through
	}
	// else no error, completed as sync already (IOCP still has posted completion status)
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
