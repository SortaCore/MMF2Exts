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
	WSAMSG winsock_msg;
	WSABUF winsock_buffer;

	// Size varies, but is based on cmsg size of ipv6_pktinfo + ip_pktinfo
	char cmsg[64];

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
	info->winsock_msg.lpBuffers = &info->winsock_buffer;
	info->winsock_msg.dwBufferCount = 1;
	info->winsock_msg.name = (LPSOCKADDR)&info->from;
	info->winsock_msg.namelen = info->from_length = sizeof (info->from);
	info->winsock_msg.Control.buf = info->cmsg;
	info->winsock_msg.Control.len = WSA_CMSG_SPACE(sizeof(struct in6_pktinfo))
		+ WSA_CMSG_SPACE(sizeof(struct in_pktinfo));
	assert(sizeof(info->cmsg) >= info->winsock_msg.Control.len);

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
	SOCKET icmpsocket;
	SOCKET icmpv6socket;

	lw_list(udp_overlapped, pending_receives);

	long receives_posted;

	long writes_posted;

	void * tag;

	// Function for sending with a specified outgoing address. Null on WinXP.
	fn_WSASendMsg WSASendMsgPtr;
	// Function for receiving msg and storing local address it was received to. Null on WinXP.
	fn_WSARecvMsg WSARecvMsgPtr;
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


		fn_WSARecvMsg recvMsg = ctx->WSARecvMsgPtr;
		int res;
		if (recvMsg)
		{
			res = recvMsg(ctx->socket,
				&receive_info->winsock_msg,
				NULL,
				&overlapped->overlapped,
				0);
		}
		else
		{
			res = WSARecvFrom(ctx->socket,
				&receive_info->winsock_buffer,
				1,
				0,
				&flags,
				(struct sockaddr*)&receive_info->from,
				&receive_info->from_length,
				&overlapped->overlapped,
				0);
		}

		if (res == SOCKET_ERROR)
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
			// Send errors should be reported at sendto/sendmsg
			// Abort is a user close of udp, Wine reports as handles closed
			if (error != 0 && error != ERROR_OPERATION_ABORTED && error != ERROR_HANDLES_CLOSED)
				always_log("Error in sending message: %d.\n", error);
			write_completed(ctx);
			break;
		}

		case overlapped_type_receive:
		{
			udp_receive_info info = (udp_receive_info) overlapped->tag;

			if (error == 0)
			{
				info->buffer[bytes_transferred] = 0;

				struct _lw_addr remoteAddr = { 0 };
				lwp_addr_set_sockaddr(&remoteAddr,
					ctx->WSARecvMsgPtr ? info->winsock_msg.name : (struct sockaddr*)&info->from);

				// If address doesn't match filter, it's a UDP message from unauthorised source.
				// There's no way to block UDP messages like that on Lacewing's side; firewall perhaps,
				// but user is unlikely to link up automatic firewall changes to Lacewing's error reports.
				// To avoid flooding server with reports, we don't report in release builds.
				if (!lw_filter_check_remote_addr(ctx->filter, &remoteAddr))
				{
					lw_addr filter_addr = lw_filter_remote(ctx->filter);
					lwp_trace("UDP from unexpected source \"%s\", outside of filter \"%s\".\n",
						lw_addr_tostring(&addr), lw_addr_tostring(filter_addr));
				}
				else if (ctx->on_data)
				{
					struct _lw_addr localAddr = { 0 };
					int ifIdx = -1;
					WSACMSGHDR* hdr = (WSACMSGHDR *)info->winsock_msg.Control.buf;
					if (hdr)
					{
						// In Windows, a dual-stack socket will provide local IPv4 if address is IPv6-mapped,
						// which is what you have to use to send back anyway. Linux provides local IPv6 mapped.
						//
						// IP4 pkt info given for IP6 incoming address, or vice versa
						// IPv6 dual-stack will pass both IPv6-mapped + IPv4 pktinfo, pick the second
						// note cmsg_type IP_PKTINFO + IPV6_PKTINFO are equal
						/*
						if ((info->winsock_msg.name->sa_family == AF_INET6) !=
							(hdr->cmsg_level == IPPROTO_IPV6))
						{
							hdr = WSA_CMSG_NXTHDR(&info->winsock_msg, hdr);
						}*/

						struct sockaddr_storage store = { 0 };
						if (hdr->cmsg_level == IPPROTO_IP)
						{
							store.ss_family = AF_INET;
							struct in_pktinfo* pktInfo = (struct in_pktinfo*)WSA_CMSG_DATA(info->winsock_msg.Control.buf);
							((struct sockaddr_in*)&store)->sin_addr = pktInfo->ipi_addr;
							((struct sockaddr_in*)&store)->sin_port = htons(ctx->port);
							lwp_addr_set_sockaddr(&localAddr, (struct sockaddr *)&store);
							ifIdx = pktInfo->ipi_ifindex;
						}
						else if (hdr->cmsg_level == IPPROTO_IPV6)
						{
							store.ss_family = AF_INET6;
							struct in6_pktinfo* pktInfo = (struct in6_pktinfo*)WSA_CMSG_DATA(info->winsock_msg.Control.buf);
							((struct sockaddr_in6*)&store)->sin6_addr = pktInfo->ipi6_addr;
							((struct sockaddr_in6*)&store)->sin6_port = htons(ctx->port);
							lwp_addr_set_sockaddr(&localAddr, (struct sockaddr*)&store);
							ifIdx = pktInfo->ipi6_ifindex;
						}
						else
							lwp_trace("Unrecognised control data level %d, type %d", hdr->cmsg_level, hdr->cmsg_type);
					}

					ctx->on_data(ctx, ifIdx == -1 ? 0 : &localAddr, ifIdx, &remoteAddr, info->buffer, bytes_transferred);
					lwp_addr_cleanup(&localAddr);
				}

				lwp_addr_cleanup(&remoteAddr);
			}
			// else, an error. Ignore op aborted, it indicates socket was closed abruptly
			// by something higher up, i.e. udp unhost.
			// Wine reports it as handles closed.
			else if (error != ERROR_OPERATION_ABORTED && error != ERROR_HANDLES_CLOSED)
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
	lw_bool madeipv6;

	if ((ctx->socket = lwp_create_server_socket
			(filter, SOCK_DGRAM, IPPROTO_UDP, &madeipv6, error)) == -1)
	{
		if (ctx->on_error)
			ctx->on_error (ctx, error);
		lw_error_delete (error);

		return;
	}
	lw_log_if_debug("Hosted UDP port %d for %s.\n", (int)ctx->socket, madeipv6 ? "IPv6 + mapped IPv4" : "IPv4");

	if (madeipv6 && (ctx->icmpv6socket = lwp_create_server_socket
		(filter, SOCK_RAW, IPPROTO_ICMPV6, NULL, error)) == -1)
	{
		lw_error_addf(error, "Creating ICMPv6 socket");
		if (ctx->on_error)
			ctx->on_error(ctx, error);
		// Non-fatal, we don't need ICMP
	}
	lw_filter ipv4filt = lw_filter_clone(filter);
	lw_filter_set_ipv6(ipv4filt, lw_false);
	if ((ctx->icmpsocket = lwp_create_server_socket
		(ipv4filt, SOCK_RAW, IPPROTO_ICMP, NULL, error)) == -1)
	{
		lw_error_addf(error, "Creating ICMPv4 socket");
		if (ctx->on_error)
			ctx->on_error(ctx, error);
		// Non-fatal, we don't need ICMP
	}
	lw_filter_delete(ipv4filt);

	lw_error_delete (error);

	ctx->filter = lw_filter_clone (filter);

	ctx->port = lwp_socket_port(ctx->socket);

	// Not available on WinXP, but all IPv6 stuff is half-baked there
	ctx->WSASendMsgPtr = compat_WSASendMsg(ctx->socket);
	ctx->WSARecvMsgPtr = compat_WSARecvMsg(ctx->socket);

	ctx->pump_watch = lw_pump_add (ctx->pump, (HANDLE) ctx->socket, "lw_udp_host", ctx, udp_socket_completion);

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

	if (ctx->icmpv6socket != -1)
	{
		s = ctx->icmpv6socket;
		ctx->icmpv6socket = INVALID_SOCKET;
		lwp_close_socket(s);
	}

	if (ctx->icmpsocket != -1)
	{
		s = ctx->icmpsocket;
		ctx->icmpsocket = INVALID_SOCKET;
		lwp_close_socket(s);
	}

	lw_filter_delete (ctx->filter);
	ctx->filter = 0;
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
		lw_pump_remove(ctx->pump, ctx->pump_watch, "lw_udp_dealloc");
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
	ctx->icmpsocket = INVALID_SOCKET;
	ctx->icmpv6socket = INVALID_SOCKET;

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

void lw_udp_send (lw_udp ctx, lw_addr from, lw_ui32 ifidx, lw_addr to, const char * buffer, size_t size)
{
	if (!to || (!lw_addr_ready (to)) || !to->info)
	{
		lw_error error = lw_error_new ();

		lw_error_addf (error, "The remote address passed to send() wasn't ready");
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

	udp_overlapped overlapped = (udp_overlapped) lw_calloc_or_exit (sizeof (*overlapped) + size, 1);

	lwp_retain(ctx, "udp write");
	++ctx->writes_posted;

	overlapped->type = overlapped_type_send;
	memcpy(((char*)overlapped) + sizeof(*overlapped), buffer, size);

	WSABUF winsock_buf = { (ULONG)size, ((CHAR*)overlapped) + sizeof(*overlapped) };

	int res;
	// if a non-local IPv6 destination, then specify the outgoing IP we send from explicitly
	fn_WSASendMsg wsaSendMsg = from ? ctx->WSASendMsgPtr : NULL;
	if (!wsaSendMsg)
	{
		res = WSASendTo(ctx->socket, &winsock_buf, 1, 0, /* MSG_XX flags */ 0, to->info->ai_addr,
			(int)to->info->ai_addrlen, (OVERLAPPED*)overlapped, 0);
	}
	else // specify source address
	{
		assert((int)ifidx > -1);
		char cmsgbuf[WSA_CMSG_SPACE(sizeof(struct in6_pktinfo)) + WSA_CMSG_SPACE(sizeof(struct in_pktinfo))];
		memset(cmsgbuf, 0, sizeof(cmsgbuf));
		WSACMSGHDR* cmsg = (WSACMSGHDR*)cmsgbuf;
		WSAMSG msg;

		// This will vomit if you use IPv4 address on an IPv6 dual-stack socket, claiming invalid pointer detection
		// It will also vomit if you pass a sizeof sockaddr_storage. So I guess it only wants mapped.
		// If you host a IPv6 dual-stack, only use IPv6 local and remote addresses.
		// If you host IPv4, only use IPv4 local and remote addresses.
		// Note that udp->host(port) will make IPv6 dual-stack by default.
		msg.name = to->info->ai_addr;
		msg.namelen = (int)to->info->ai_addrlen; // 16 = IPv4, 28 = IPv6, 128 = storage
		//assert(msg.namelen == sizeof(struct sockaddr_in6));

		msg.lpBuffers = &winsock_buf;
		msg.dwBufferCount = 1;
		msg.dwFlags = 0;
		msg.Control.buf = (CHAR *)cmsg;

		//if (ctx->is_ipv6)
		// If destination is IPv4 or IPv4-mapped IPv6, the cmsg must be in_pktinfo
		// If destination is IPv6 non-mapped, source must be IPv6 in in6_pktinfo
		if ((to->info->ai_family == AF_INET || IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6*)to->info->ai_addr)->sin6_addr)) &&
			from->info->ai_family != AF_INET)
		{
			lw_log_if_debug("Warning: expecting IPv4 outgoing for this scenario: from \"%s\", to \"%s\".",
				lw_addr_tostring(from, lw_addr_tostring_flag_box_ipv6 | lw_addr_tostring_flag_remove_port),
				lw_addr_tostring(to, lw_addr_tostring_flag_box_ipv6 | lw_addr_tostring_flag_remove_port));
		}
		if ((to->info->ai_family == AF_INET6 && !IN6_IS_ADDR_V4MAPPED(&((struct sockaddr_in6*)to->info->ai_addr)->sin6_addr)) &&
			from->info->ai_family != AF_INET6)
		{
			lw_log_if_debug("Warning: expecting IPv6 outgoing for this scenario: from \"%s\" to \"%s\".",
				lw_addr_tostring(from, lw_addr_tostring_flag_box_ipv6 | lw_addr_tostring_flag_remove_port),
				lw_addr_tostring(to, lw_addr_tostring_flag_box_ipv6 | lw_addr_tostring_flag_remove_port));
		}
		if (from->info->ai_family == AF_INET)
		{
			msg.Control.len = WSA_CMSG_SPACE(sizeof(struct in_pktinfo));
			cmsg->cmsg_level = IPPROTO_IP;
			cmsg->cmsg_type = IP_PKTINFO;
			cmsg->cmsg_len = WSA_CMSG_LEN(sizeof(struct in_pktinfo));

			struct in_pktinfo* const pktinfo = (struct in_pktinfo*)WSA_CMSG_DATA(cmsg);
			pktinfo->ipi_addr = ((struct sockaddr_in*)from->info->ai_addr)->sin_addr;
			pktinfo->ipi_ifindex = ifidx;
		}
		else // AF_INET6
		{
			assert(from->info->ai_family == AF_INET6);
			msg.Control.len = WSA_CMSG_SPACE(sizeof(struct in6_pktinfo));
			cmsg->cmsg_level = IPPROTO_IPV6;
			cmsg->cmsg_type = IPV6_PKTINFO;
			cmsg->cmsg_len = WSA_CMSG_LEN(sizeof(struct in6_pktinfo));
			struct in6_pktinfo* const pktinfo = (struct in6_pktinfo*)WSA_CMSG_DATA(cmsg);
			pktinfo->ipi6_addr = ((struct sockaddr_in6*)from->info->ai_addr)->sin6_addr;
			pktinfo->ipi6_ifindex = ifidx;
		}

		res = wsaSendMsg(ctx->socket, &msg, 0, NULL, &overlapped->overlapped, NULL);
	}

	const lw_addr_tostring_flags addrstringflags = lw_addr_tostring_flag_box_ipv6;
	do {
		if (res == SOCKET_ERROR)
		{
			const int code = WSAGetLastError();

			// no error, running as async
			if (code == WSA_IO_PENDING)
				break;

			--ctx->writes_posted;
			free(overlapped);

			// outgoing is overloaded; as it's UDP, just discard the message
			// Closest to the EAGAIN exception on Unix servers, but since we use overlapped, it's unlikely this will apply
			// This code as a response to WSASendTo may be Windows NT only
			if (code == WSAEWOULDBLOCK)
				break;

			// genuine error, whine about it
			lw_error error = lw_error_new();
			lw_error_add(error, code);
			lw_error_addf(error, "Error sending datagram via %s", wsaSendMsg ? "sendmsg" : "sendto");
#ifdef _DEBUG
			if (wsaSendMsg)
			{
				lw_error_addf(error, "Sending UDP from socket %i, local address \"%s\", addrlen %i, ifidx %u), to remote \"%s\" (addrlen %i)",
					(int)ctx->socket,
					lw_addr_tostring(from, addrstringflags),
					from->info->ai_addrlen,
					ifidx,
					lw_addr_tostring(to, addrstringflags),
					to->info->ai_addrlen);
			}
			else
			{
				lw_error_addf(error, "Sending from OS default local address/interface, to remote \"%s\" (addrlen %i)",
					lw_addr_tostring(to, addrstringflags), to->info->ai_addrlen);
			}
#endif

			if (ctx->on_error)
				ctx->on_error(ctx, error);

			lw_error_delete(error);
			break;
		}
		// else no error, completed as sync already (IOCP still has posted completion status)
#ifdef _DEBUG
		if (wsaSendMsg)
		{
			lw_log_if_debug("Sending UDP from socket %i, local address \"%s\" net interface ID %u), to remote \"%s\", success (instant).\n",
				(int)ctx->socket,
				lw_addr_tostring(from, addrstringflags),
				ifidx,
				lw_addr_tostring(to, addrstringflags));
		}
		else
		{
			lw_log_if_debug("Sending from OS default local address/interface, to remote \"%s\", success (instant).\n",
				lw_addr_tostring(to, addrstringflags));
		}
#endif
	} while (0);

	lwp_release(ctx, "udp write");
}

void lw_udp_send_unreachable(lw_udp ctx, lw_addr from, lw_ui32 ifidx, lw_addr to, const char * data, lw_ui32 size)
{
	lwp_socket icmpsock = lw_addr_ipv6(from) ? ctx->icmpv6socket : ctx->icmpsocket;
	lw_error err;
	if (icmpsock == -1)
	{
		err = lw_error_new();
		lw_error_addf(err, "ICMP%s send error - no socket", lw_addr_ipv6(from) ? "v6" : "v4");
		if (ctx->on_error)
			ctx->on_error(ctx, err);
		lw_error_delete(err);
	}

	err = lwp_send_icmp_unreachable(icmpsock, IPPROTO_UDP, from, ifidx, to, data, size);
	if (err)
	{
		lw_error_addf(err, "ICMP send error");
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
