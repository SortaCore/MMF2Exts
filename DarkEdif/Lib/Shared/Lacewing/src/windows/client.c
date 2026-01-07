/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin.
 * Copyright (C) 2012-2026 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"
#include "../address.h"
#include "ssl/clientssl.h"
#include "fdstream.h"

struct _lw_client
{
	struct _lw_fdstream fdstream;

	//lw_pump pump;

	lw_client_hook_connect		on_connect;
	lw_client_hook_disconnect	on_disconnect;
	lw_client_hook_data			on_data;
	lw_client_hook_error		on_error;

	lw_addr remote_address;
	lw_addr local_address;
	lw_ui32 ifidx;

	//HANDLE socket;
	lw_bool connecting;
	lw_ui16 local_port_next_connect;
};

lw_client lw_client_new (lw_pump pump)
{
	lwp_init ();

	lw_client ctx = (lw_client) calloc (sizeof (*ctx), 1);

	if (!ctx)
		return 0;

	lwp_init();
	lwp_fdstream_init (&ctx->fdstream, pump);
	ctx->fdstream.flags |= lwp_fdstream_flag_is_socket;

	assert(ctx->fdstream.stream.pump == pump);
	//ctx->pump = pump;

	return ctx;
}

void lw_client_delete (lw_client ctx)
{
	if (!ctx)
		return;

	lw_stream_close ((lw_stream) ctx, lw_true);

	lw_bool isValid = lw_fdstream_valid(&ctx->fdstream);
	lw_bool canClose = lwp_stream_may_close((lw_stream)ctx);

	lw_addr_delete (ctx->remote_address);
	ctx->remote_address = NULL;
	lw_addr_delete (ctx->local_address);
	ctx->local_address = NULL;
	ctx->ifidx = -1;

	lwp_deinit();
	free (ctx);
}

void lw_client_connect (lw_client ctx, const char * host, lw_ui16 port)
{
	lw_addr address = lw_addr_new_port (host, port);

	lw_client_connect_addr (ctx, address);
}

static void first_time_write_ready (void * tag, OVERLAPPED * overlapped,
									unsigned long bytes_transfered, int errorNum)
{
	lw_client ctx = (lw_client) tag;

	assert (ctx->connecting);

	if (errorNum)
	{
		ctx->connecting = lw_false;

		lw_error error = lw_error_new();
		if (errorNum == ERROR_NETWORK_UNREACHABLE && lw_addr_ipv6(ctx->remote_address))
			lw_error_addf(error, "Network unreachable - Non-IPv6 client connecting to IPv6 server?");
		else
			lw_error_add(error, errorNum);
		lw_error_addf(error, "Error connecting");

		if (ctx->on_error)
			ctx->on_error(ctx, error);

		lw_error_delete(error);
		return;
	}

	// Set to -1 when forced disconnect on local side happens during connecting phase,
	// by fdstream close handler
	assert(ctx->fdstream.fd != INVALID_HANDLE_VALUE);

	// Switches this first time callback to the standard one
	lw_fdstream_set_fd(&ctx->fdstream, ctx->fdstream.fd, lw_true, lw_true);
//	ctx->watch = ctx->fdstream.watch; // PHI: does an update with no return.

	// Store local address
	lw_addr_delete(ctx->local_address);
	struct sockaddr_storage ss = lwp_socket_addr((lwp_socket)ctx->fdstream.fd);
	ctx->local_address = lwp_addr_new_sockaddr((struct sockaddr*)&ss);
	ctx->ifidx = lwp_get_ifidx(&ss);

	ctx->connecting = lw_false;

	if (ctx->on_connect)
		ctx->on_connect (ctx);

	if (ctx->on_data)
		lw_stream_read ((lw_stream) ctx, -1);
}

void lw_client_connect_addr (lw_client ctx, lw_addr address)
{
	if (lw_client_connected (ctx) || lw_client_connecting (ctx))
	{
		lw_error error = lw_error_new ();
		if (lw_client_connecting(ctx))
			lw_error_addf (error, "Already connecting to a server");
		else
			lw_error_addf (error, "Already connected to a server");

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);

		return;
	}

	// This is an idiot check so we don't overwrite our FDs
	if (ctx->fdstream.fd != INVALID_HANDLE_VALUE)
	{
		lw_error error = lw_error_new();
		lw_error_addf(error, "Inconsistent socket state; socket should be invalid or marked connecting/connected");

		if (ctx->on_error)
			ctx->on_error(ctx, error);

		lw_error_delete(error);
		return;
	}

	ctx->connecting = lw_true;

	/* TODO : Resolve asynchronously? */

	{	lw_error error = lw_addr_resolve (address);

		if (error)
		{
			ctx->connecting = lw_false;
			if (ctx->on_error)
				ctx->on_error (ctx, error);

			lw_error_delete (error);

			return;
		}
	}

	if (!address->info)
	{
		ctx->connecting = lw_false;

		lw_error error = lw_error_new ();
		lw_error_addf (error, "The provided remote address is not ready for use");

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);

		return;
	}

	if ((ctx->fdstream.fd = (HANDLE) WSASocket
			(lw_addr_ipv6 (address) ? AF_INET6 : AF_INET, SOCK_STREAM, IPPROTO_TCP,
			 0, 0, WSA_FLAG_OVERLAPPED)) == INVALID_HANDLE_VALUE)
	{
		ctx->connecting = lw_false;
		lw_error error = lw_error_new ();

		lw_error_add (error, WSAGetLastError ());
		lw_error_addf (error, "Error creating socket");

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);

		return;
	}

	lwp_disable_ipv6_only ((lwp_socket) ctx->fdstream.fd);

	if (ctx->fdstream.stream.watch)
		lw_pump_update_callbacks(ctx->fdstream.stream.pump, ctx->fdstream.stream.watch, "lw_client_connect_addr setting first_time_write_ready", ctx, first_time_write_ready);
	else
		ctx->fdstream.stream.watch = lw_pump_add (ctx->fdstream.stream.pump, ctx->fdstream.fd, "lw_client_connect_addr starting with first_time_write_ready", ctx, first_time_write_ready);

	/* LPFN_CONNECTEX and WSAID_CONNECTEX aren't defined w/ MinGW */

	static BOOL (PASCAL FAR * lw_ConnectEx)
	  (
		SOCKET s,
		const struct sockaddr FAR *name,
		int namelen,
		PVOID lpSendBuffer,
		DWORD dwSendDataLength,
		LPDWORD lpdwBytesSent,
		LPOVERLAPPED lpOverlapped
	  );

	GUID ID = {0x25a207b9,0xddf3,0x4660,
	  {0x8e,0xe9,0x76,0xe5,0x8c,0x74,0x06,0x3e}};

	DWORD bytes = 0;

	WSAIoctl ((SOCKET)ctx->fdstream.fd, SIO_GET_EXTENSION_FUNCTION_POINTER,
		 &ID, sizeof (ID), &lw_ConnectEx, sizeof (lw_ConnectEx),
		 &bytes, 0, 0);

	assert (lw_ConnectEx);

	struct sockaddr_storage local_address = { 0 };

	// Lock to last outgoing local address if local port is being re-used,
	// which only happens for hole punch
	if (ctx->local_address && ctx->local_port_next_connect)
	{
		memcpy(&local_address, ctx->local_address->info->ai_addr, ctx->local_address->info->ai_addrlen);
		assert(lw_addr_ipv6(address) == lw_addr_ipv6(ctx->local_address));
	}
	// else bind to receiving from any address in remote's IPvX; possibly using a fixed local port,
	// but not using a fixed local address. Local port will be 0 by default, which is OS-pick local port.
	// inaddr_any/in6addr_any is all-zero, which we inited local_address to already.
	else if (lw_addr_ipv6 (address))
	{
		((struct sockaddr_in6 *) &local_address)->sin6_family = AF_INET6;
		((struct sockaddr_in6 *) &local_address)->sin6_port = htons(ctx->local_port_next_connect);
	}
	else
	{
		((struct sockaddr_in *) &local_address)->sin_family = AF_INET;
		((struct sockaddr_in *) &local_address)->sin_port = htons(ctx->local_port_next_connect);
	}

	// Reuse port or not, based on reserved port being non-zero
	const int was_locked_local = ctx->local_port_next_connect != 0 ? 1 : 0;
	ctx->local_port_next_connect = 0;
	lwp_setsockopt((SOCKET)ctx->fdstream.fd, SOL_SOCKET, SO_REUSEADDR, (const char*)&was_locked_local, sizeof(was_locked_local));

	if (bind ((SOCKET)ctx->fdstream.fd, (struct sockaddr *) &local_address,
			// sizeof sockaddr_storage doesn't work cross-platform
			lw_addr_ipv6(ctx->remote_address) ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in)) == -1)
	{
		ctx->connecting = lw_false;

		lw_error error = lw_error_new ();

		lw_error_add (error, WSAGetLastError ());
		lw_error_addf (error, "Error binding socket%s", was_locked_local != 0 ? " with fixed port" : "");

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);

		return;
	}

	lw_addr_delete (ctx->remote_address);
	ctx->remote_address = lw_addr_clone (address);

	OVERLAPPED * overlapped = (OVERLAPPED *) calloc (sizeof (*overlapped), 1);

	if (!lw_ConnectEx ((SOCKET) ctx->fdstream.fd, ctx->remote_address->info->ai_addr,
			(int) ctx->remote_address->info->ai_addrlen, 0, 0, 0, overlapped))
	{
		int code = WSAGetLastError ();

		if (code == WSA_IO_PENDING)
			return; // No problem

		ctx->connecting = lw_false;

		// Note: Connecting errors also occur in the first_time_write_ready callback.
		lw_error error = lw_error_new();

		lw_error_add(error, code);
		lw_error_addf(error, "Error connecting to address");

		if (ctx->on_error)
			ctx->on_error(ctx, error);

		lw_error_delete(error);
	}
}

void lw_client_set_local_port (lw_client ctx, lw_ui16 localport)
{
	ctx->local_port_next_connect = localport;
}

lw_bool lw_client_connected (lw_client ctx)
{
	return lw_fdstream_valid ((lw_fdstream) ctx);
}

lw_bool lw_client_connecting (lw_client ctx)
{
	return ctx->connecting;
}

lw_addr lw_client_server_addr (lw_client ctx)
{
	return ctx->remote_address;
}
lw_addr lw_client_local_addr(lw_client ctx)
{
	return ctx->local_address;
}
lw_ui32 lw_client_ifidx(lw_client ctx)
{
	return ctx->ifidx;
}

static void on_stream_data (lw_stream stream, void * tag,
							const char * buffer, size_t length)
{
	lw_client ctx = (lw_client) tag;

	ctx->on_data (ctx, buffer, (long)length);
}

void lw_client_on_data (lw_client ctx, lw_client_hook_data on_data)
{
	ctx->on_data = on_data;

	if (on_data)
	{
		lw_stream_add_hook_data ((lw_stream) ctx, on_stream_data, ctx);
		lw_stream_read ((lw_stream) ctx, -1);
	}
	else
	{
		lw_stream_remove_hook_data ((lw_stream) ctx, on_stream_data, ctx);
	}
}

static void on_close (lw_stream stream, void * tag)
{
	lw_client ctx = (lw_client) tag;

	ctx->on_disconnect (ctx);
}

void lw_client_on_disconnect (lw_client ctx,
							  lw_client_hook_disconnect on_disconnect)
{
	ctx->on_disconnect = on_disconnect;

	if (on_disconnect)
		lw_stream_add_hook_close ((lw_stream) ctx, on_close, ctx);
	else
		lw_stream_remove_hook_close ((lw_stream) ctx, on_close, ctx);
}

lwp_def_hook (client, connect)
lwp_def_hook (client, error)

