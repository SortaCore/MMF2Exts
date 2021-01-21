
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin.  All rights reserved.
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

#include "common.h"
#include "address.h"

struct _lw_filter
{
	lw_bool reuse, ipv6;

	lw_addr local, remote;
	long local_port, remote_port;

	void * tag;
};

lw_filter lw_filter_new ()
{
	lw_filter ctx = (lw_filter) malloc (sizeof (*ctx));

	ctx->local_port = 0;
	ctx->remote_port = 0;

	ctx->local = 0;
	ctx->remote = 0;

	ctx->reuse = lw_true;
	ctx->ipv6 = lw_true;

	return ctx;
}

lw_filter lw_filter_clone (lw_filter filter)
{
	lw_filter ctx = lw_filter_new ();

	lw_filter_set_ipv6 (ctx, lw_filter_ipv6 (filter));
	lw_filter_set_reuse (ctx, lw_filter_reuse (filter));

	lw_filter_set_local_port (ctx, lw_filter_local_port (filter));
	lw_filter_set_remote_port (ctx, lw_filter_remote_port (filter));

	lw_filter_set_local (ctx, lw_filter_local (filter));
	lw_filter_set_remote (ctx, lw_filter_remote (filter));

	return ctx;
}

void lw_filter_delete (lw_filter ctx)
{
	if (!ctx)
	  return;

	lw_addr_delete (ctx->local);
	lw_addr_delete (ctx->remote);

	free (ctx);
}

void lw_filter_set_remote (lw_filter ctx, lw_addr addr)
{
	if (addr)
	{
	  if (lw_addr_resolve (addr))
		 return;

	  lw_addr_delete (ctx->remote);
	  ctx->remote = lw_addr_clone (addr);

	  if (ctx->remote_port != 0)
		 lw_addr_set_port (ctx->remote, ctx->remote_port);
	}
	else
	{
	  if (ctx->remote)
		 ctx->remote_port = lw_addr_port (ctx->remote);

	  lw_addr_delete (ctx->remote);
	  ctx->remote = 0;
	}
}

lw_addr lw_filter_remote (lw_filter ctx)
{
	return ctx ? ctx->remote : nullptr;
}

void lw_filter_set_local (lw_filter ctx, lw_addr addr)
{
	if (addr)
	{
	  if (lw_addr_resolve (addr))
		 return;

	  lw_addr_delete (ctx->local);
	  ctx->local = lw_addr_clone (addr);

	  if (ctx->local_port != 0)
		 lw_addr_set_port (ctx->local, ctx->local_port);
	}
	else
	{
	  if (ctx->local)
		 ctx->local_port = lw_addr_port (ctx->local);

	  lw_addr_delete (ctx->local);
	  ctx->local = 0;
	}
}

lw_addr lw_filter_local (lw_filter ctx)
{
	return ctx->local;
}

void lw_filter_set_reuse (lw_filter ctx, lw_bool enabled)
{
	ctx->reuse = enabled;
}

lw_bool lw_filter_reuse (lw_filter ctx)
{
	return ctx->reuse;
}

void lw_filter_set_ipv6 (lw_filter ctx, lw_bool enabled)
{
	ctx->ipv6 = enabled;
}

lw_bool lw_filter_ipv6 (lw_filter ctx)
{
	return ctx->ipv6;
}

long lw_filter_local_port (lw_filter ctx)
{
	return ctx->local ? lw_addr_port (ctx->local) : ctx->local_port;
}

void lw_filter_set_local_port (lw_filter ctx, long port)
{
	if (ctx->local)
	  lw_addr_set_port (ctx->local, port);
	else
	  ctx->local_port = port;
}

long lw_filter_remote_port (lw_filter ctx)
{
	return ctx->remote ? lw_addr_port (ctx->remote) : ctx->remote_port;
}

void lw_filter_set_remote_port (lw_filter ctx, long port)
{
	if (ctx->remote)
	  lw_addr_set_port (ctx->remote, port);
	else
	  ctx->remote_port = port;
}

lwp_socket lwp_create_server_socket (lw_filter filter, int type,
									 int protocol, lw_error error)
{
	lwp_socket s;
	size_t addr_len;
	struct sockaddr_storage addr;
	lw_bool ipv6;
	int reuse;

	ipv6 = lw_filter_ipv6 (filter);

	if (!ipv6)
	{
	  if (  (filter->local && lw_addr_ipv6 (filter->local))
			|| (filter->remote && lw_addr_ipv6 (filter->remote)) )
	  {
		 lw_error_addf (error,
				"Filter has IPv6 disabled, but one of the attached addresses is an IPv6 address.  "
				"Try using HINT_IPv4 when creating the address.");

		 return -1;
	  }
	}

	#ifdef _WIN32

	  if (ipv6)
	  {
		 if ((s = WSASocket
			(AF_INET6, type, protocol, 0, 0, WSA_FLAG_OVERLAPPED)) == -1)
		 {
			if (WSAGetLastError () != WSAEAFNOSUPPORT)
			{
				lw_error_add (error, WSAGetLastError ());
				lw_error_addf (error, "Error creating socket");

				return -1;
			}

			ipv6 = lw_false;
		 }
		 else
			 goto ipv6success;
	  }

	  if (!ipv6)
	  {
		 if ((s = WSASocket
			(AF_INET, type, protocol, 0, 0, WSA_FLAG_OVERLAPPED)) == -1)
		 {
			lw_error_add (error, WSAGetLastError ());
			lw_error_addf (error, "Error creating socket");

			return -1;
		 }
	  }

	#else

	  if (ipv6)
	  {
		 if ((s = socket (AF_INET6, type, protocol)) == -1)
		 {
			if (errno != EAFNOSUPPORT)
			{
				lw_error_add (error, errno);
				lw_error_addf (error, "Error creating socket");

				return -1;
			}

			ipv6 = lw_false;
		 }
	  }

	  if (!ipv6)
	  {
		 if ((s = socket (AF_INET, type, protocol)) == -1)
		 {
			lw_error_add (error, errno);
			lw_error_addf (error, "Error creating socket");

			return -1;
		 }
	  }

	  fcntl (s, F_SETFL, fcntl (s, F_GETFL, 0) | O_NONBLOCK);

	#endif

	ipv6success:
	if (ipv6)
	  lwp_disable_ipv6_only (s);

	reuse = lw_filter_reuse (filter) ? 1 : 0;
	assert(setsockopt (s, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) == 0);

	memset (&addr, 0, sizeof (addr));

	addr_len = 0;

	if (lw_filter_local (filter))
	{
	  struct addrinfo * info = lw_filter_local (filter)->info;

	  if (info)
	  {
		 memcpy (&addr, info->ai_addr, info->ai_addrlen);
		 addr_len = info->ai_addrlen;
	  }
	}

	if (!addr_len)
	{
	  if (ipv6)
	  {
		 addr_len = sizeof (struct sockaddr_in6);

		 ((struct sockaddr_in6 *) &addr)->sin6_family = AF_INET6;
		 ((struct sockaddr_in6 *) &addr)->sin6_addr = in6addr_any;

		 ((struct sockaddr_in6 *) &addr)->sin6_port
			= lw_filter_local_port (filter) ?
				htons ((unsigned short) lw_filter_local_port (filter)) : 0;
	  }
	  else
	  {
		 addr_len = sizeof (struct sockaddr_in);

		 ((struct sockaddr_in *) &addr)->sin_family = AF_INET;
		 ((struct sockaddr_in *) &addr)->sin_addr.s_addr = INADDR_ANY;

		 ((struct sockaddr_in *) &addr)->sin_port
			= lw_filter_local_port (filter) ?
				htons ((unsigned short) lw_filter_local_port (filter)) : 0;
	  }
	}

	if (bind (s, (struct sockaddr *) &addr, (int) addr_len) == -1)
	{
	  lw_error_add (error, lwp_last_socket_error);

	  if (lwp_last_socket_error == 10013 || lwp_last_socket_error == 10048)
		  lw_error_addf(error, "Socket is in use already?");

	  lw_error_addf (error, "Error binding socket");

	  lwp_close_socket (s);

	  return -1;
	}

	return s;
}

void * lw_filter_tag (lw_filter ctx)
{
	return ctx->tag;
}

void lw_filter_set_tag (lw_filter ctx, void * tag)
{
	ctx->tag = tag;
}

