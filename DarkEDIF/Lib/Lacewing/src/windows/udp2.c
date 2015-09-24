
/* vim: set et ts=3 sw=3 ft=c:
 *
 * Copyright (C) 2011, 2012 James McLaughlin et al.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
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

#include "../common.h"
#include "../address.h"

extern "C" {
const int ideal_pending_receive_count = 16;

#define overlapped_type_send     1
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

   info->winsock_buffer.buf = info->buffer;
   info->winsock_buffer.len = sizeof (info->buffer);

   info->from_length = sizeof (info->from);

   return info;
}

struct _lw_udp
{
   lw_pump pump;

   lw_udp_hook_data on_data;
   lw_udp_hook_error on_error;

   lw_filter filter;

   long port;

   SOCKET socket;

   long receives_posted;

   void * tag;

   lw_pump_watch watch; // Added by Phi, not used
};

void post_receives (lw_udp ctx)
{
	int error = 0;
   while (ctx->receives_posted < ideal_pending_receive_count)
   {
      udp_receive_info receive_info = udp_receive_info_new ();

      if (!receive_info)
         break;
	  
      udp_overlapped overlapped =
         (udp_overlapped) calloc (sizeof (*overlapped), 1);

      if (!overlapped)
         break;
	  
	  overlapped->type = overlapped_type_receive;
      overlapped->tag = receive_info;

      DWORD flags = 0;
	  DWORD bytesRecv = 0;

      if (WSARecvFrom (ctx->socket,
                       &receive_info->winsock_buffer,
                       1,
                       &bytesRecv,
                       &flags,
                       (struct sockaddr *) &receive_info->from,
                       &receive_info->from_length,
                       &overlapped->overlapped,
                       NULL) == -1)
      {
         error = WSAGetLastError();

		 if (error != WSA_IO_PENDING)
		 {
			 if (error != WSAECONNRESET)
				 break;
			 
			 // Ignore UDP Connection Resets; it's a quirk of Windoze, UDP is connectionless
			 // so there's no real way a 'UDP connection' can be reset.
			 lw_trace("Idle note: Received a WSAECONNRESET in post_receives, meaningless for a UDP connection.");
		 }
      }

      ++ ctx->receives_posted;
   }

   // Section added by Phi.
   if (error == WSAEINVAL)
   {
	   // Report nicely before committing seppuku
	   if (ctx->on_error)
	   {
		   lw_error e = lw_error_new();
		   lw_error_addf(e, "Received error WSAEINVAL in post_receives(). Call bind() or connect() before recvfrom(); the UDP socket has no local port yet.");

		   ctx->on_error(ctx, e);

		   lw_error_delete(e);
	   }

	   assert(error != WSAEINVAL);
   }
   assert(error == 0 || error == WSA_IO_PENDING || error == WSAECONNRESET);
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
         break;
      }

      case overlapped_type_receive:
      {
         udp_receive_info info = (udp_receive_info) overlapped->tag;

         info->buffer [bytes_transferred] = 0;

         struct _lw_addr addr = {};
         lwp_addr_set_sockaddr (&addr, (struct sockaddr *) &info->from);

         lw_addr filter_addr = lw_filter_remote (ctx->filter);

		 // Phi: Error here; lw_addr_equals is bugged for UDP, it seems.
		 // Note ai_flags is meant to differ; filter is 0, recv is 4.
		 if (filter_addr)
		 {
			 int custom = (addr.info->ai_addrlen != filter_addr->info->ai_addrlen ||
				 memcmp(addr.info->ai_addr, filter_addr->info->ai_addr, addr.info->ai_addrlen));
			 int old = lw_addr_equal(filter_addr, &addr);
			 
			 if (custom != old)
				 lw_trace("Differential in udp_socket_completion: custom %i, old %i.", custom, old);

			 // Can occur when multiple apps are connected on the same remote port
			 if (custom)
			 {
				 lw_trace("Received a message from a UDP address we didn't expect in udp_socket_completion; ignoring.");
				 break;
			 }
		 }

         if (ctx->on_data)
            ctx->on_data (ctx, &addr, info->buffer, bytes_transferred);

         free (info);

         -- ctx->receives_posted;
         post_receives (ctx);

         break;
      }
   };

   free (overlapped);
}

void lw_udp_host(lw_udp ctx, long port)
{
   lw_filter filter = lw_filter_new ();
   lw_filter_set_local_port (filter, port);

   lw_udp_host_filter (ctx, filter);

   lw_filter_delete (filter);
}

void lw_udp_host_addr(lw_udp ctx, lw_addr addr)
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
	
	if ((ctx->socket = lwp_create_server_socket
			(filter, SOCK_DGRAM, IPPROTO_UDP, error)) == -1)
	{
		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete(error);
		return;
	}
	   
	ctx->filter = lw_filter_clone (filter);
	
	ctx->watch = lw_pump_add (ctx->pump, (HANDLE) ctx->socket, ctx, udp_socket_completion);

	ctx->port = lwp_socket_port (ctx->socket);

	lw_addr a = lw_filter_remote(filter);
	
	// May be needed
	assert(lw_addr_resolve(a) == nullptr);
	
	// Not a valid address until the WSASendTo or bind is triggered
	post_receives(ctx);
}

lw_bool lw_udp_hosting (lw_udp ctx)
{
   return ctx->socket != INVALID_SOCKET;
}

void lw_udp_unhost (lw_udp ctx)
{
   lwp_close_socket (ctx->socket);
   ctx->socket = INVALID_SOCKET;

   lw_filter_delete (ctx->filter);
   ctx->filter = 0;
}

long lw_udp_port (lw_udp ctx)
{
   return ctx->port;
}

lw_udp lw_udp_new (lw_pump pump)
{
   lw_udp ctx = (lw_udp) calloc (sizeof (*ctx), 1);

   if (!ctx)
      return 0;

   lwp_init ();  

   ctx->pump = pump;
   ctx->socket = INVALID_SOCKET;
   ctx->watch = nullptr;

   return ctx;
}

void lw_udp_delete (lw_udp ctx)
{
   if (!ctx)
      return;

   lw_udp_unhost (ctx);

   free (ctx);
}

void lw_udp_send (lw_udp ctx, lw_addr addr, const char * buffer, size_t size)
{
   if ((!lw_addr_ready (addr)) || !addr->info)
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

   WSABUF winsock_buf = { size, (CHAR *) buffer };

   udp_overlapped overlapped = (udp_overlapped) calloc (sizeof (*overlapped), 1);

   if (!overlapped)
   {
      /* TODO: error */
	  assert(false);

      return;
   }
    
   overlapped->type = overlapped_type_send;
   overlapped->tag = 0;

   if (!addr->info)
      return;

   if (WSASendTo (ctx->socket, &winsock_buf, 1, 0, 0, addr->info->ai_addr,
                  addr->info->ai_addrlen, (OVERLAPPED *) overlapped, 0) == -1)
   {
      int code = WSAGetLastError();

      if (code != WSA_IO_PENDING)
      {
         lw_error error = lw_error_new ();

         lw_error_add (error, WSAGetLastError ());
         lw_error_addf (error, "Error sending datagram");

         if (ctx->on_error)
            ctx->on_error (ctx, error);

         lw_error_delete (error);

         return;
      }
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

}