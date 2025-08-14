/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"
#include "../address.h"
#include "ssl/serverssl.h"
#include "fdstream.h"
#include <tchar.h>

typedef struct _accept_overlapped
{
	OVERLAPPED overlapped;

	SOCKET socket;

	struct _lw_addr addr;
	char addr_buffer [(sizeof (struct sockaddr_storage) + 16) * 2];

} * accept_overlapped;

static void on_client_close (lw_stream, void * tag);
static void on_client_data (lw_stream, void * tag, const char * buffer, size_t size);

struct _lw_server
{
	lwp_refcounted;

	SOCKET socket;

	lw_pump pump;
	lw_pump_watch pump_watch;

	lw_server_hook_connect on_connect;
	lw_server_hook_disconnect on_disconnect;
	lw_server_hook_data on_data;
	lw_server_hook_error on_error;

	lw_bool cert_loaded;
	time_t cert_expiry_time; // Expiry time in UTC
	CredHandle ssl_creds;

	lw_list (struct _accept_overlapped, pending_accepts);

	lw_list (lw_server_client, clients);

	void * tag;
};

struct _lw_server_client
{
	struct _lw_fdstream fdstream;

	lw_server server;

	lw_bool on_connect_called;
	lw_bool is_websocket;

	void* relay_tag;

	/* IPv4 and IPv6 both accepted, but IPv4 is mapped to IPv6.
	  When looking up string representation make sure to check. */
	lw_addr addr;

	lw_server_client * elem;

	/* TODO: don't include this for non SSL clients
	*/
	struct _lwp_serverssl ssl;
};

// Called by refcounter when it reaches zero
static void lw_server_dealloc(lw_server ctx)
{
	// No refs, so there should be no pending accepts
	assert(list_length(ctx->pending_accepts) == 0);
	list_clear(ctx->pending_accepts);

	free(ctx);
}

lw_server lw_server_new (lw_pump pump)
{
	lw_server ctx = (lw_server) calloc (sizeof (*ctx), 1);

	if (!ctx)
	  return 0;

	lwp_enable_refcount_logging(ctx, "server");
	lwp_set_dealloc_proc(ctx, lw_server_dealloc);
	lwp_init ();

	ctx->socket = -1;
	ctx->pump = pump;
	lwp_retain(ctx, "server_new");

	return ctx;
}

void lw_server_delete (lw_server ctx)
{
	lw_server_unhost (ctx);
	lwp_release(ctx, "server_new");

	lwp_deinit ();

	//free (ctx);
}

lw_ui16 lw_server_hole_punch (lw_server ctx, const char* remote_ip_and_port, lw_ui16 local_port)
{
	lw_addr addr = lw_addr_new(remote_ip_and_port, "");
	lw_error err = lw_addr_resolve(addr);
	if (err)
	{
		lw_error_addf(err, "Error resolving hole punch address");
		if (ctx->on_error)
			ctx->on_error(ctx, err);
		lw_addr_delete(addr);
		return 0;
	}
	err = lw_error_new();

	if (ctx->socket != INVALID_SOCKET)
	{
		lw_error_addf(err, "Server already hosting, cannot hole punch");
		if (ctx->on_error)
			ctx->on_error(ctx, err);
		lw_error_delete(err);
		lw_addr_delete(addr);
		return 0;
	}

	// Init to an invalid ID ping message
	WSABUF b;
	b.buf = (char[]){ (char)(11 << 4), (char)0xFF, (char)0xFF };
	b.len = 3;
	int type = lw_addr_type_tcp;
	lw_bool broke = lw_false;
	lw_bool isIPV6 = lw_addr_ipv6(addr);
	do {
		SOCKET sock = socket(isIPV6 ? AF_INET6 : AF_INET,
			type == lw_addr_type_tcp ? SOCK_STREAM : SOCK_DGRAM,
			type == lw_addr_type_tcp ? IPPROTO_TCP : IPPROTO_UDP);
		char yes = 1;
		if (sock == -1)
		{
			lw_error_add(err, WSAGetLastError());
			lw_error_addf(err, "create for %s hole punch failed", type == lw_addr_type_tcp ? "tcp" : "udp");
			broke = lw_true;
			break;
		}
		// reuse addr on
		lwp_setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
		struct sockaddr_storage s = { 0 };
		s.ss_family = isIPV6 ? AF_INET6 : AF_INET;
		// Port is at same offset in both sockaddr_in and sockaddr_in6
		((struct sockaddr_in6*)&s)->sin6_port = htons((lw_ui16)local_port);

		/*// Tiny timeout?
		int timeout = 3000;  // user timeout in milliseconds [ms]
		int SOL_TCP = 6, TCP_USER_TIMEOUT = 18;
		lwp_setsockopt(sock, SOL_TCP, TCP_USER_TIMEOUT, (char*)&timeout, sizeof(timeout));
		*/

		if (bind(sock, (struct sockaddr*)&s, isIPV6 ? sizeof(struct sockaddr_in6) : sizeof(struct sockaddr_in)) == -1)
		{
			lw_error_add(err, WSAGetLastError());
			lw_error_addf(err, "bind on hole punch failed, continuing");
			broke = lw_true;
		}
		u_long mode = 1;  // 1 to enable non-blocking socket
		ioctlsocket(sock, FIONBIO, &mode);
			
		int conRet = -1;
		if (broke == lw_false)
		{
			if (type == lw_addr_type_tcp)
				conRet = connect(sock, addr->info->ai_addr, (int)addr->info->ai_addrlen);
			// UDP v4: send plainly
			else if (!isIPV6)
				conRet = sendto(sock, b.buf, b.len, 0, addr->info->ai_addr, (int)addr->info->ai_addrlen);
			// UDP v6: set fixed output public IP, and send
			else
			{
				fn_WSASendMsg wsaSendMsg = compat_WSASendMsg();
				if (!wsaSendMsg)
				{
					WSASetLastError(WSAEPROTONOSUPPORT);
					conRet = -1;
				}
				else
				{
					WSACMSGHDR* cmsg = (WSACMSGHDR*)lw_calloc_or_exit(WSA_CMSG_SPACE(sizeof(struct in6_pktinfo)), 1);
					if (!lwp_set_ipv6pktinfo_cmsg(cmsg))
					{
						lw_error_addf(err, "no suitable public IPv6 found");
						conRet = -1;
					}
					else
					{
						WSAMSG msg;
						msg.name = addr->info->ai_addr;
						msg.namelen = (int)addr->info->ai_addrlen;
						msg.lpBuffers = &b;
						msg.dwBufferCount = 1;
						msg.Control.buf = (CHAR*)cmsg;
						msg.Control.len = WSA_CMSG_SPACE(sizeof(struct in6_pktinfo));
						msg.dwFlags = 0;

						DWORD sentCount;
						conRet = wsaSendMsg(ctx->socket, (LPWSAMSG)&msg, 0, &sentCount, NULL, NULL);
					}
					free(cmsg);
				}
			}
		}

		if (conRet == -1 && WSAGetLastError() == WSAEWOULDBLOCK)
		{
			// 0.5 second timeout
			struct timeval t;
			t.tv_sec = 0;
			t.tv_usec = (long)(0.5 * 1000000);
			fd_set fdset;
			fdset.fd_count = 1;
			fdset.fd_array[0] = sock;
			conRet = select(-1, NULL, &fdset, NULL, &t);
			// if conRet == 0, connect timeout - which we ignore

			// these errors are OK! Refused connection mean we got there, hopefully.
			if (conRet == -1 &&
				WSAGetLastError() != WSAETIMEDOUT &&
				WSAGetLastError() != ERROR_CONNECTION_REFUSED && WSAGetLastError() != ERROR_SEM_TIMEOUT)
			{
				lw_error_add(err, WSAGetLastError());
				lw_error_addf(err, "connect/send on hole punch likely failed");
				broke = lw_true;
			}
		}
		else if (conRet == -1)
		{
			lw_error_add(err, WSAGetLastError());
			lw_error_addf(err, "connect/send on hole punch failed");
			broke = lw_true;
		}
		// else successful start of connect - since non-blocking, not actual connect done yet

		if (local_port == 0)
		{
			// TODO: lwp_socket_port(sock) equiv?
			int socklen = (int)sizeof(s);
			if (getsockname(sock, (struct sockaddr*)&s, &socklen) == -1)
			{
				lw_error_add(err, WSAGetLastError());
				lw_error_addf(err, "Couldn't get auto-port");
			}
			else
			{
				local_port = ntohs(((struct sockaddr_in6*)&s)->sin6_port);
				lw_error_addf(err, "auto-assigned port %hu", local_port);
			}
		}
		else
			lw_error_addf(err, "sendTo on hole punch, local port %hu should be OK; you can now host", local_port);

		// If TCP connection is active, end it fast without trying to push through (disable lingering)
		if (broke && type == lw_addr_type_tcp)
		{
			struct linger sl = { 0 };
			lwp_setsockopt(sock, SOL_SOCKET, SO_LINGER, (char *)&sl, sizeof(sl));
		}

		closesocket(sock);
	} while (++type != lw_addr_type_udp + 1);

	// In debug, always report
#ifdef _DEBUG
	broke = lw_true;
#endif
	if (broke && ctx->on_error)
		ctx->on_error(ctx, err);
	lw_error_delete(err);
	lw_addr_delete(addr);
	return local_port;
}

void lw_server_set_tag (lw_server ctx, void * tag)
{
	ctx->tag = tag;
}

void * lw_server_tag (lw_server ctx)
{
	return ctx->tag;
}
void on_ssl_error (lw_server_client client, lw_error error)
{
	lw_error_addf(error, "SSL error");

	if (client->server->on_error)
		client->server->on_error(client->server, error);

	// SSL errors are generally unrecoverable
	lw_stream_close((lw_stream)client, lw_true);
}

lw_server_client lwp_server_client_new (lw_server ctx, SOCKET socket)
{
	lw_server_client client = (lw_server_client) calloc (sizeof (*client), 1);

	if (!client)
	  return 0;

	client->server = ctx;
	client->is_websocket = lw_false;

	lwp_fdstream_init ((lw_fdstream) client, ctx->pump);

	/* The first added close handler is always the last called.
	* This is important, because ours will destroy the client.
	*/

	lw_stream_add_hook_close ((lw_stream) client, on_client_close, client);

	if (ctx->cert_loaded)
	{
	  lwp_serverssl_init (&client->ssl, ctx->ssl_creds, client);
	  client->ssl.ssl.handle_error = on_ssl_error;
	}

	lw_fdstream_set_fd ((lw_fdstream) client, (HANDLE) socket, 0, lw_true, lw_true);

	return client;
}

const int ideal_pending_accept_count = 32;

lw_bool accept_completed(lw_server ctx, accept_overlapped overlapped, lw_bool release, lw_bool deleteSocket)
{
	if (deleteSocket)
		closesocket(overlapped->socket);
	list_elem_remove(overlapped); // frees overlapped
	return release && lwp_release(ctx, "pending accept");
}

static lw_bool issue_accept (lw_server ctx)
{
	struct _accept_overlapped _overlapped = { 0 };
	_overlapped.socket = INVALID_SOCKET;

	list_push (struct _accept_overlapped, ctx->pending_accepts, _overlapped);
	accept_overlapped overlapped = list_elem_back (struct _accept_overlapped, ctx->pending_accepts);

	if ((overlapped->socket = WSASocket (lwp_socket_addr (ctx->socket).ss_family,
										SOCK_STREAM,
										IPPROTO_TCP,
										0,
										0,
										WSA_FLAG_OVERLAPPED)) == INVALID_SOCKET)
	{
	  list_elem_remove (overlapped);
	  return lw_false;
	}

	lwp_disable_ipv6_only ((lwp_socket) overlapped->socket);

	DWORD bytes_received;

	/* TODO : Use AcceptEx to receive the first data? */

	lwp_retain(ctx, "pending accept");
	if (!AcceptEx (ctx->socket,
				  overlapped->socket,
				  overlapped->addr_buffer,
				  0,
				  sizeof (struct sockaddr_storage) + 16,
				  sizeof (struct sockaddr_storage) + 16,
				  &bytes_received,
				  (OVERLAPPED *) overlapped))
	{
	  int error = WSAGetLastError ();

	  if (error != ERROR_IO_PENDING)
	  {
		 accept_completed(ctx, overlapped, lw_true, lw_true);
		 return lw_false;
	  }
	}
	// else completed in sync (still queued in IOCP)

	return lw_true;
}

static void listen_socket_completion (void * tag, OVERLAPPED * _overlapped,
									  unsigned long bytes_transferred, int error)
{
	lw_server ctx = (lw_server) tag;
	accept_overlapped overlapped = (accept_overlapped) _overlapped;

	if (error)
	{
		accept_completed(ctx, overlapped, lw_true, lw_true);
		return;
	}

	while (list_length (ctx->pending_accepts) < (size_t)ideal_pending_accept_count)
	  if (!issue_accept (ctx))
		 break;

	setsockopt ((SOCKET) overlapped->socket, SOL_SOCKET,
				SO_UPDATE_ACCEPT_CONTEXT,
				(char *) &ctx->socket, sizeof (ctx->socket));

	struct sockaddr_storage * local_addr, * remote_addr;
	int local_addr_len, remote_addr_len;

	GetAcceptExSockaddrs
	(
	  overlapped->addr_buffer,
	  0,

	  sizeof (struct sockaddr_storage) + 16,
	  sizeof (struct sockaddr_storage) + 16,

	  (struct sockaddr **) &local_addr,
	  &local_addr_len,

	  (struct sockaddr **) &remote_addr,
	  &remote_addr_len
	);

	lw_server_client client = lwp_server_client_new (ctx, overlapped->socket);
	lwp_release(ctx, "pending accept");

	if (!client)
	{
		accept_completed(ctx, overlapped, lw_false, lw_true);
		return;
	}

	client->addr = lwp_addr_new_sockaddr ((struct sockaddr *) remote_addr);

	accept_completed(ctx, overlapped, lw_false, lw_false);

	lwp_retain (client, "on_connect");

	client->on_connect_called = lw_true;

	if (ctx->on_connect)
	  ctx->on_connect (ctx, client);

	if (lwp_release(client, "on_connect"))
	{
		if (ctx->on_disconnect)
			ctx->on_disconnect(ctx, client);
		return;  /* client was deleted by connect hook; client->addr will be too */
	}

	list_push (lw_server_client, ctx->clients, client);
	client->elem = list_elem_back (lw_server_client, ctx->clients);

	if (ctx->on_data)
	{
	  lwp_trace ("*** READING on behalf of the handler, client %p", client);

	  lw_stream_add_hook_data ((lw_stream) client, on_client_data, client);
	  lw_stream_read ((lw_stream) client, -1);
	}
}

void lw_server_host (lw_server ctx, long port)
{
	lw_filter filter = lw_filter_new ();
	lw_filter_set_local_port (filter, port);

	lw_server_host_filter (ctx, filter);

	lw_filter_delete (filter);
}

void lw_server_host_filter (lw_server ctx, lw_filter filter)
{
	lw_server_unhost (ctx);

	lw_error error = lw_error_new ();

	if ((ctx->socket = lwp_create_server_socket
			(filter, SOCK_STREAM, IPPROTO_TCP, error)) == -1)
	{
	  if (ctx->on_error)
		 ctx->on_error (ctx, error);

	  lw_error_delete (error);
	  return;
	}

	if (listen (ctx->socket, SOMAXCONN) == -1)
	{
	  lw_error error = lw_error_new ();

	  lw_error_add (error, WSAGetLastError ());
	  lw_error_addf (error, "Error listening");

	  if (ctx->on_error)
		 ctx->on_error (ctx, error);

	  lw_error_delete (error);
	  return;
	}

	ctx->pump_watch = lw_pump_add
	  (ctx->pump, (HANDLE) ctx->socket, ctx, listen_socket_completion);

	while (list_length (ctx->pending_accepts) < (size_t)ideal_pending_accept_count)
	  if (!issue_accept (ctx))
		 break;

	lw_error_delete (error);
}

void lw_server_unhost (lw_server ctx)
{
	if (!lw_server_hosting (ctx))
		return;

	closesocket (ctx->socket);
	ctx->socket = -1;

	// We keep on_disconnect while unhosting so user data can be freed; Relay servers will check
	// if they're hosting before running their user handler.
	//ctx->on_disconnect = nullptr;
	list_each (lw_server_client, ctx->clients, client)
	{
		// If you call just lw_stream_delete, on_client_close close hook is never called, so memory is leaked.
		// on_client_close will call lw_stream_delete.
		lw_stream_close((lw_stream)client, lw_true);
	}
	list_clear(ctx->clients);

	list_each(struct _accept_overlapped, ctx->pending_accepts, overlapped)
	{
		if (overlapped.socket != INVALID_SOCKET)
			closesocket(overlapped.socket);
		overlapped.socket = INVALID_SOCKET;
	}

//	list_clear (ctx->pending_accepts);

	lw_pump_post_remove (ctx->pump, ctx->pump_watch);
	ctx->pump_watch = NULL;
}

lw_bool lw_server_hosting (lw_server ctx)
{
	return ctx->socket != -1;
}

size_t lw_server_num_clients (lw_server ctx)
{
	return list_length (ctx->clients);
}

int lw_server_port (lw_server ctx)
{
	return lwp_socket_port (ctx->socket);
}

lw_bool lw_server_load_sys_cert (lw_server ctx,
								 const char * common_name,
								 const char * location,
								 const char * store_name)
{
	// Allow already-hosting server - see lw_server_load_cert_file for why
#if 0
	if (lw_server_hosting (ctx) || lw_server_cert_loaded (ctx))
	{
	  lw_error error = lw_error_new ();

	  lw_error_addf (error,
			"Either the server is already hosting, or a certificate has already been loaded");

	  if (ctx->on_error)
		 ctx->on_error (ctx, error);

	  lw_error_delete (error);

	  return lw_false;
	}
#endif

	if(!location || !*location)
	  location = "CurrentUser";

	if(!store_name || !*store_name)
	  store_name = "MY";

	int location_id = -1;

	do
	{
	  if(!strcasecmp (location, "CurrentService"))
	  {
		 location_id = 0x40000; /* CERT_SYSTEM_STORE_CURRENT_SERVICE */
		 break;
	  }

	  if(!strcasecmp (location, "CurrentUser"))
	  {
		 location_id = 0x10000; /* CERT_SYSTEM_STORE_CURRENT_USER */
		 break;
	  }

	  if(!strcasecmp (location, "CurrentUserGroupPolicy"))
	  {
		 location_id = 0x70000; /* CERT_SYSTEM_STORE_CURRENT_USER_GROUP_POLICY */
		 break;
	  }

	  if(!strcasecmp (location, "LocalMachine"))
	  {
		 location_id = CERT_SYSTEM_STORE_LOCAL_MACHINE; /* CERT_SYSTEM_STORE_LOCAL_MACHINE */
		 break;
	  }

	  if(!strcasecmp (location, "LocalMachineEnterprise"))
	  {
		 location_id = 0x90000; /* CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE */
		 break;
	  }

	  if(!strcasecmp (location, "LocalMachineGroupPolicy"))
	  {
		 location_id = 0x80000; /* CERT_SYSTEM_STORE_LOCAL_MACHINE_GROUP_POLICY */
		 break;
	  }

	  if(!strcasecmp (location, "Services"))
	  {
		 location_id = 0x50000; /* CERT_SYSTEM_STORE_SERVICES */
		 break;
	  }

	  if(!strcasecmp (location, "Users"))
	  {
		 location_id = 0x60000; /* CERT_SYSTEM_STORE_USERS */
		 break;
	  }

	} while(0);

	if (location_id == -1)
	{
	  lw_error error = lw_error_new ();

	  lw_error_addf (error, "Unknown certificate location: %s", location);
	  lw_error_addf (error, "Error loading certificate");

	  if (ctx->on_error)
		 ctx->on_error (ctx, error);

	  return lw_false;
	}

	HCERTSTORE cert_store = CertOpenStore
	(
		(LPCSTR) 9, /* CERT_STORE_PROV_SYSTEM_A */
		0,
		0,
		location_id | CERT_STORE_READONLY_FLAG,
		store_name
	);

	if (!cert_store)
	{
	  lw_error error = lw_error_new ();

	  lw_error_add (error, GetLastError ());
	  lw_error_addf (error, "Error loading certificate");

	  if (ctx->on_error)
		 ctx->on_error (ctx, error);

	  lw_error_delete (error);

	  return lw_false;
	}

	PCCERT_CONTEXT context = CertFindCertificateInStore
	(
	  cert_store,
	  X509_ASN_ENCODING,
	  0,
	  CERT_FIND_SUBJECT_STR_A,
	  common_name,
	  0
	);

	if (!context)
	{
	  int code = GetLastError();

	  context = CertFindCertificateInStore
	  (
		 cert_store,
		 PKCS_7_ASN_ENCODING,
		 0,
		 CERT_FIND_SUBJECT_STR_A,
		 common_name,
		 0
	  );

	  if (!context)
	  {
		 lw_error error = lw_error_new ();

		 lw_error_add (error, code);
		 lw_error_addf (error, "Error finding certificate in store");

		 if (ctx->on_error)
			ctx->on_error (ctx, error);

		 lw_error_delete (error);

		 return lw_false;
	  }
	}

	SCHANNEL_CRED creds = {0};

	creds.dwVersion			  = SCHANNEL_CRED_VERSION;
	creds.cCreds				 = 1;
	creds.paCred				 = &context;
	creds.grbitEnabledProtocols = SP_PROT_TLS1_X_SERVER;

	{
		TimeStamp expiry_time;

		int result = AcquireCredentialsHandleA
		(
			0,
			(SEC_CHAR *) UNISP_NAME_A,
			SECPKG_CRED_INBOUND,
			0,
			&creds,
			0,
			0,
			&ctx->ssl_creds,
			&expiry_time
		);

		if (result != SEC_E_OK)
		{
			lw_error error = lw_error_new ();

			// if error 0x8009030e, make sure you have certificate imported into MMC from pfx file (pfx is a combo of both private and public key)
			// cer file will only have public key, and so it will import, but not be usable for server
			// Also see https://stackoverflow.com/questions/32114246/ssl-certificate-sometimes-stops-working
			// If you get the same error after a reboot, go back in MMC, find the cert under Trusted Root, drag-drop into Personal, right-click cert
			// in Personal, press All Tasks > Manage Private Keys, add Users role with Read permissions, then drag back to Trusted Root.
			lw_error_add (error, result);
			lw_error_addf (error, "Error acquiring credentials handle");

			if (ctx->on_error)
				ctx->on_error (ctx, error);

			lw_error_delete (error);

			return lw_false;
		}

		// Get expiry time, convert from SECURITY_INTEGER i.e. FILETIME to tm (FILETIME has epoch in 1600 AD)
		// Expiry time is also available at context->pCertInfo.NotAfter
		{
			time_t tmt = expiry_time.QuadPart / 10000000ULL - 11644473600ULL;
			struct tm* tm = localtime(&tmt);
			char buff[50];
			if (!tm || strftime(buff, sizeof(buff), "%I:%M:%S%p on %A %d %B %Y AD", tm) < 0)
				always_log("time conversion failed, error %d", errno);
			else
				always_log("SSL certificate will expire at %s (local time).", buff);

			if (difftime(tmt, time(NULL)) < 0)
			{
				lw_error error = lw_error_new();
				lw_error_addf(error, "SSL certificate expired already, at %s (local time)", buff);

				if (ctx->on_error)
					ctx->on_error(ctx, error);

				lw_error_delete(error);

				return lw_false;
			}

			ctx->cert_expiry_time = tmt;
		}
	}

	ctx->cert_loaded = lw_true;

	return lw_true;
}
struct buffer { BYTE* data; DWORD size; };
static lw_error read_cert_file(CRYPT_DATA_BLOB * res, const char* filenameUTF8, const WCHAR* filenameWCHAR, lw_bool textual) {
	lw_error ret = NULL;
	LARGE_INTEGER fileSize;
	DWORD amountRead;
	HANDLE fil;
	do {
		res->pbData = NULL;
		fil = CreateFileW(filenameWCHAR, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if (fil == INVALID_HANDLE_VALUE)
		{
			ret = lw_error_new();
			lw_error_add(ret, GetLastError());
			lw_error_addf(ret, "Couldn't open file \"%s\"", filenameUTF8);
			break;
		}

		if (GetFileSizeEx(fil, &fileSize) == FALSE)
		{
			ret = lw_error_new();
			lw_error_add(ret, GetLastError());
			lw_error_addf(ret, "Couldn't read size of file \"%s\"", filenameUTF8);
			break;
		}
		if (fileSize.QuadPart > 10 * 1024 * 1024)
		{
			ret = lw_error_new();
			lw_error_addf(ret, "Size of file \"%s\" too big (read " lw_PRId64 " bytes)", filenameUTF8, fileSize.QuadPart);
			break;
		}

		res->pbData = (BYTE*)malloc(fileSize.LowPart);
		res->cbData = fileSize.LowPart;
		if (res->pbData == NULL)
		{
			ret = lw_error_new();
			lw_error_addf(ret, "Couldn't allocate %u bytes", res->cbData);
			break;
		}
		if (!ReadFile(fil, res->pbData, res->cbData, &amountRead, NULL) || amountRead != res->cbData)
		{
			ret = lw_error_new();
			lw_error_add(ret, GetLastError());
			lw_error_addf(ret, "Couldn't read %u bytes", res->cbData);
			break;
		}

		// PEM file to DER
		if (textual == lw_false)
			break;

		DWORD expDERSize = 0;
		if (!CryptStringToBinaryA(res->pbData, res->cbData, CRYPT_STRING_BASE64HEADER,
			NULL, &expDERSize, NULL, NULL))
		{
			ret = lw_error_new();
			lw_error_add(ret, GetLastError());
			lw_error_addf(ret, "CryptStringToBinary failed");
			break;
		}
		BYTE * DERdata = (BYTE *) malloc(expDERSize);
		if (!DERdata)
		{
			ret = lw_error_new();
			lw_error_addf(ret, "Couldn't allocate %u bytes", expDERSize);
			break;
		}

		if (!CryptStringToBinaryA(res->pbData, res->cbData, CRYPT_STRING_BASE64HEADER,
			DERdata, &expDERSize, NULL, NULL))
		{
			ret = lw_error_new();
			lw_error_add(ret, GetLastError());
			lw_error_addf(ret, "CryptStringToBinary failed");
		}

		free(res->pbData);
		res->pbData = DERdata;
		res->cbData = expDERSize;
	} while (lw_false);

	if (fil != INVALID_HANDLE_VALUE)
		CloseHandle(fil);
	if (ret && res->pbData)
	{
		free(res->pbData);
		res->pbData = NULL;
	}
	return ret;
}

// Allows priv key to be null or match cert_chain for pfx files
lw_bool lw_server_load_cert_file (lw_server ctx,
								  const char * filename_cert_chain,
								  const char * filename_privkey,
								  const char * passphrase)
{
	if (filename_cert_chain == filename_privkey ||
		filename_privkey && (filename_privkey[0] == '\0' || !strcmp(filename_cert_chain, filename_privkey)))
		filename_privkey = NULL;
	if (passphrase && passphrase[0] == '\0')
		passphrase = NULL;

	// Certificates for SSL are managed in three+ layers. Each layer confirms the next one is valid.
	// The root certificate is stored in OS and/or browser, updated regularly and held by the largest security companies.
	// The intermediate certificate(s) is the resellers and middle-men. There can be more than one of these.
	// The top-level or end certificate is your personal one.
	// We need both public top-level certificate, and private key to host; these can be combined in some file formats.

	PCCERT_CONTEXT cert_context = NULL;
	LPCSTR provider = CERT_STORE_PROV_FILENAME;

	// CertSetCertificateContextProperty() is Unicode only, so no point with TCHAR here
	wchar_t* filename_cert_chain_wchar = lw_char_to_wchar(filename_cert_chain, -1);
	wchar_t* filename_privkey_wchar = filename_privkey ? lw_char_to_wchar(filename_privkey, -1) : NULL;

	if (filename_cert_chain_wchar == NULL || !lw_file_exists (filename_cert_chain) ||
		(filename_privkey && (filename_privkey_wchar == NULL || !lw_file_exists (filename_privkey))))
	{
	  lw_error error = lw_error_new ();

	  lw_error_addf (error, "File not found: %s", !lw_file_exists(filename_cert_chain) ? filename_cert_chain : filename_privkey);
	  lw_error_addf (error, "Error loading certificate");

	  if (ctx->on_error)
		 ctx->on_error (ctx, error);

	  lw_error_delete (error);

	  free(filename_cert_chain_wchar);
	  free(filename_privkey_wchar);
	  return lw_false;
	}

	// TODO: Don't disable hosting midway, switch certificate
	// Certificate changing should not affect already set up connections; see https://security.stackexchange.com/a/158328
	//if (lw_server_hosting (ctx))
	//  lw_server_unhost (ctx);

	// lw_sync_lock(ctx->cert_lock);

	if (lw_server_cert_loaded (ctx))
	{
		ctx->cert_loaded = lw_false;
		FreeCredentialsHandle (&ctx->ssl_creds);
	}

	enum encodeType
	{
		// Plain X509 certificate - public key, CERT/CER/CRT file extension, generally
		x509 = 0,
		// Plain PKCS#7 format
		// ???
		pkcs7,
		// pfx - proprietary Windows format, bag of anything; can have X509 cert chain, private keys, can be password-protected
		pkcs12,
		// pem format - base64 with header, can be password-protected, can have X509 cert chain, private and public keys
		// In the two-file pem format, based on Let's Encrypt for Unix-based systems, four PEM files are made.
		// cert, chain, fullchain, privkey
		// cert.pem is the top-level certificate
		// chain.pem is top-level and down to the root certificate, listed in one file
		// fullchain.pem is cert.pem + chain.pem
		// privkey.pem is the private key that isn't shared to the client - can be encrypted
		// For importing, we use cert and privkey; but since cert is first in fullchain.pem's list, and only one cert
		// is read by this code, then fullchain.pem is also usable.
		pem_two_files
	} encodeType = x509;
	HCERTSTORE cert_store = NULL;

	// Private key was supplied and differs from cert chain, so private and public keys aren't in same file.
	// Having both together is possible for pfx files, and is usual practice for Windows server configs.
	// We require both private and public for SSL hosting.
	if (filename_privkey != NULL && 0 != _stricmp(filename_cert_chain, filename_privkey))
	{
		encodeType = pem_two_files;
		lw_error error = NULL;
		CRYPT_DATA_BLOB cert_chain = { 0 }, priv_key = { 0 };
		do {
			error = read_cert_file(&cert_chain, filename_cert_chain, filename_cert_chain_wchar, lw_true);
			if (error)
			{
				lw_error_addf(error, "Error loading certificate chain file as PEM");
				break;
			}
			error = read_cert_file(&priv_key, filename_privkey, filename_privkey_wchar, lw_true);
			if (error)
			{
				lw_error_addf(error, "Error loading private key file");
				break;
			}

			DWORD privKeyBlobLen = 8192;
			char* privKeyBlob;
			HCRYPTPROV cryptProvHandle = 0;
			HCRYPTKEY keyHandle;
			WCHAR privateKeyName[128] = L"SSL/TLS liblacewing ";
			WCHAR* slash = wcsrchr(filename_privkey_wchar, L'\\'), * slash2 = wcsrchr(filename_privkey_wchar, L'/');
			slash = slash > slash2 ? slash : slash2;
			wcscat(privateKeyName, slash + 1);

			// Get size then decode
			if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY,
				priv_key.pbData, priv_key.cbData, 0, NULL, NULL, &privKeyBlobLen))
			{
				error = lw_error_new();
				lw_error_add(error, GetLastError());
				lw_error_addf(error, "CryptDecodeObjectEx priv key failed");
				break;
			}
			privKeyBlob = (char *)malloc(privKeyBlobLen);
			if (!CryptDecodeObjectEx(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, PKCS_RSA_PRIVATE_KEY,
				priv_key.pbData, priv_key.cbData, 0, NULL, privKeyBlob, &privKeyBlobLen))
			{
				error = lw_error_new();
				lw_error_add(error, GetLastError());
				lw_error_addf(error, "CryptDecodeObjectEx priv key failed");
				break;
			}

			// Open the key container
			if (!CryptAcquireContextW(&cryptProvHandle, privateKeyName, MS_DEF_RSA_SCHANNEL_PROV_W, PROV_RSA_SCHANNEL, 0))
			{
				if (GetLastError() == NTE_BAD_KEYSET)
				{
					// No key container accessible, let's make one
					if (!CryptAcquireContextW(&cryptProvHandle, privateKeyName, MS_DEF_RSA_SCHANNEL_PROV_W, PROV_RSA_SCHANNEL, CRYPT_NEWKEYSET))
					{
						error = lw_error_new();
						lw_error_add(error, GetLastError());
						lw_error_addf(error, "CryptAcquireContext open and create failed for priv key \"%s\".", privateKeyName);
						break;
					}
					else
						lwp_trace("SSL SChannel key container created OK");
				}
				else
				{
					error = lw_error_new();
					lw_error_add(error, GetLastError());
					lw_error_addf(error, "CryptAcquireContext failed for priv key");
					break;
				}
			}
			if (!CryptImportKey(cryptProvHandle, privKeyBlob, privKeyBlobLen, (HCRYPTKEY)NULL, 0, &keyHandle))
			{
				error = lw_error_new();
				lw_error_add(error, GetLastError());
				lw_error_addf(error, "CryptImportKey failed");
				break;
			}

			CryptDestroyKey(keyHandle);

			if (!CryptReleaseContext(cryptProvHandle, 0))
			{
				error = lw_error_new();
				lw_error_add(error, GetLastError());
				lw_error_addf(error, "CryptReleaseContext failed");
				break;
			}

			/*
			 * Acquire context
			 */
			cert_context = CertCreateCertificateContext(X509_ASN_ENCODING, cert_chain.pbData, cert_chain.cbData);
			if (!cert_context)
			{
				error = lw_error_new();
				lw_error_add(error, GetLastError());
				lw_error_addf(error, "CertCreateCertificateContext failed on cert chain");
				break;
			}

			// Tell the Crypto API how to find the private key
			CRYPT_KEY_PROV_INFO keyProvInfo = { 0 };
			keyProvInfo.pwszContainerName = privateKeyName;
			keyProvInfo.pwszProvName = MS_DEF_RSA_SCHANNEL_PROV_W;
			keyProvInfo.dwProvType = PROV_RSA_SCHANNEL;
			keyProvInfo.dwKeySpec = AT_KEYEXCHANGE;
			if (!CertSetCertificateContextProperty(cert_context, CERT_KEY_PROV_INFO_PROP_ID, 0, &keyProvInfo))
			{
				error = lw_error_new();
				lw_error_add(error, GetLastError());
				lw_error_addf(error, "CertSetCertificateContextProperty failed, couldn't store private key");
			}
		} while (lw_false);

		if (error)
		{
			if (ctx->on_error)
				ctx->on_error(ctx, error);

			lw_error_delete(error);

			free(filename_cert_chain_wchar);
			free(filename_privkey_wchar);
			free(cert_chain.pbData);
			free(priv_key.pbData);
			return lw_false;
		}
	}
	else
	{

		// Convert fullchain/privkey to pfx:
		// openssl pkcs12 -export -in "..fullchain.pem" -inkey "..privkey.pem" -out "..sslcert.pfx"
		//
		// TODO: Password protection handling.
		// TODO: Load cert and private key separately (OpenSSL style), or both as PFX.
		// Might be PEM and CER files needed.
		if (!cert_store)
			cert_store = CertOpenStore
			(
				provider,
				X509_ASN_ENCODING,
				0,
				CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG,
				filename_cert_chain_wchar
			);

		encodeType = x509;
		if (!cert_store)
		{
			cert_store = CertOpenStore
			(
				provider,
				PKCS_7_ASN_ENCODING,
				0,
				CERT_STORE_OPEN_EXISTING_FLAG | CERT_STORE_READONLY_FLAG,
				filename_cert_chain_wchar
			);
			encodeType = pkcs7;

			if (!cert_store)
			{
				CRYPT_DATA_BLOB pfx;
				lw_error error = read_cert_file(&pfx, filename_cert_chain, filename_cert_chain_wchar, lw_false);
				if (error)
				{
					lw_error_addf(error, "Error loading cert chain file \"%s\", tried x509, pkcs#7, pkcs#12", filename_cert_chain);

					if (ctx->on_error)
						ctx->on_error(ctx, error);

					lw_error_delete(error);

					return lw_false;
				}

				// PFXImportCertStore is Unicode only
				wchar_t * passphrase_wchar = passphrase ? lw_char_to_wchar(passphrase, -1) : NULL;
				// despite SAL annotation, PFXImportCertStore docs allow null passphrase
				#pragma warning (suppress: 6387) 
				cert_store = PFXImportCertStore(&pfx, passphrase_wchar, 0);
				if (passphrase_wchar)
				{
					SecureZeroMemory(passphrase_wchar, _msize(passphrase_wchar));
					free(passphrase_wchar);
				}
				encodeType = pkcs12;

				if (!cert_store)
				{
					lw_error error = lw_error_new();

					lw_error_add(error, GetLastError());
					lw_error_addf(error, "Error loading cert chain file \"%s\", tried x509, pkcs#7, pkcs#12", filename_cert_chain);

					if (ctx->on_error)
						ctx->on_error(ctx, error);

					lw_error_delete(error);

					free(filename_cert_chain_wchar);
					free(filename_privkey_wchar);
					return lw_false;
				}

			}
		}

		cert_context = CertFindCertificateInStore(cert_store,
			encodeType == pkcs7 ? PKCS_7_ASN_ENCODING : X509_ASN_ENCODING,
			0, CERT_FIND_ANY, NULL, 0
		);

		if (!cert_context)
		{
			int code = GetLastError();

			cert_context = CertFindCertificateInStore(cert_store,
				encodeType == pkcs7 ? X509_ASN_ENCODING : PKCS_7_ASN_ENCODING,
				0, CERT_FIND_ANY, NULL, 0
			);

			if (!cert_context)
			{
				lw_error error = lw_error_new();

				lw_error_add(error, code);
				lw_error_addf(error, "Error finding certificate in store");

				if (ctx->on_error)
					ctx->on_error(ctx, error);

				lw_error_delete(error);

				free(filename_cert_chain_wchar);
				free(filename_privkey_wchar);
				return lw_false;
			}
		}
	}


	SCHANNEL_CRED creds = { 0 };

	creds.dwVersion = SCHANNEL_CRED_VERSION;
	creds.cCreds = 1;
	creds.paCred = &cert_context;
	creds.grbitEnabledProtocols = SP_PROT_TLS1_X_SERVER;

	TimeStamp expiry_time;

	int result = AcquireCredentialsHandle(0, UNISP_NAME, SECPKG_CRED_INBOUND,
		0, &creds, 0, 0, &ctx->ssl_creds, &expiry_time
	);

	if (result != SEC_E_OK)
	{
		// 0x80090305 SEC_E_SECPKG_NOT_FOUND
		lw_error error = lw_error_new ();

		lw_error_add (error, result);
		lw_error_addf (error, "Error acquiring credentials handle");

		if (ctx->on_error)
			ctx->on_error (ctx, error);

		lw_error_delete (error);

		free(filename_cert_chain_wchar);
		free(filename_privkey_wchar);
		return lw_false;
	}

	// Get expiry time, convert from SECURITY_INTEGER i.e. FILETIME to tm (FILETIME has epoch in 1600 AD)
	// FILETIME is stored in UTC. https://learn.microsoft.com/en-us/openspecs/office_file_formats/ms-offcrypto/04433e0f-b53b-4d5d-990c-6d91a9232eb3
	// Expiry time is also available at context->pCertInfo.NotAfter
	{
		time_t tmt = expiry_time.QuadPart / 10000000ULL - 11644473600ULL;

		struct tm* tm = localtime(&tmt);
		char buff[50];
		if (!tm || strftime(buff, sizeof(buff), "%I:%M:%S%p on %A %d %B %Y AD", tm) < 0)
			always_log("time conversion failed, error %d", errno);
		else
			always_log("SSL certificate will expire at %s (local time).", buff);

		if (difftime(tmt, time(NULL)) < 0)
		{
			lw_error error = lw_error_new();
			lw_error_addf(error, "SSL certificate expired already, at %s (local time)", buff);

			if (ctx->on_error)
				ctx->on_error(ctx, error);

			lw_error_delete(error);

			free(filename_cert_chain_wchar);
			free(filename_privkey_wchar);
			return lw_false;
		}

		ctx->cert_expiry_time = tmt;
	}
	ctx->cert_loaded = lw_true;

	free(filename_cert_chain_wchar);
	free(filename_privkey_wchar);
	return lw_true;
}

lw_bool lw_server_cert_loaded (lw_server ctx)
{
	return ctx->cert_loaded;
}

time_t lw_server_cert_expiry_time (lw_server ctx)
{
	return ctx->cert_expiry_time;
}

lw_bool lw_server_can_npn (lw_server ctx)
{
	/* NPN is currently not available w/ schannel */

	return lw_false;
}

void lw_server_add_npn (lw_server ctx, const char * protocol)
{
}

const char * lw_server_client_npn (lw_server_client client)
{
	return "";
}

lw_bool lw_server_client_is_websocket (lw_server_client client)
{
	return client->is_websocket;
}

lw_addr lw_server_client_addr (lw_server_client client)
{
	return client->addr;
}

void * lw_server_client_get_relay_tag (lw_server_client client)
{
	return client->relay_tag;
}

void lw_server_client_set_relay_tag (lw_server_client client, void * ptr)
{
	client->relay_tag = ptr;
}

void lw_server_client_set_websocket(lw_server_client client, lw_bool isWebSocket)
{
	client->is_websocket = isWebSocket;
}

lw_server_client lw_server_client_next (lw_server_client client)
{
	lw_server_client * next_client = list_elem_next (lw_server_client, client->elem);

	if (!next_client)
	  return NULL;

	return *next_client;
}

lw_server_client lw_server_client_first (lw_server ctx)
{
	if (list_length (ctx->clients) == 0)
	  return NULL;

	return list_front (lw_server_client, ctx->clients);
}

void on_client_data (lw_stream stream, void * tag, const char * buffer, size_t size)
{
	lw_server_client client = (lw_server_client) tag;
	lw_server server = client->server;

	assert (server->on_data);

	server->on_data (server, client, buffer, size);
}

void on_client_close (lw_stream stream, void * tag)
{
	lw_server_client client = (lw_server_client) tag;

	lw_server ctx = client->server;

	lwp_trace ("Server: on_client_close for client %p", client);

	lwp_retain (client, "on_client_close");

	if (client->on_connect_called)
	{
	  if (ctx->on_disconnect)
		 ctx->on_disconnect (ctx, client);
	}

	if (client->elem)
	{
	  list_elem_remove (client->elem);
	  client->elem = 0;
	}

	if (ctx->cert_loaded)
	{
	  lwp_serverssl_cleanup (&client->ssl);
	}

	lw_addr_delete (client->addr);
	client->addr = NULL;

	lw_stream_delete ((lw_stream) client);

	lwp_release (client, "on_client_close");

	// lw_pump_remove happens in close_fd(), which happens in lw_stream_close()

	// free(client); will happen when the stream delete handler is called
}

void lw_server_on_data (lw_server ctx, lw_server_hook_data on_data)
{
	ctx->on_data = on_data;

	if (on_data)
	{
	  /* Setting on_data to a handler */

	  if (!ctx->on_data)
	  {
		 list_each (lw_server_client, ctx->clients, client)
		 {
			lw_stream_add_hook_data ((lw_stream) client, on_client_data, client);
			lw_stream_read ((lw_stream) client, -1);
		 }
	  }

	  return;
	}

	/* Setting on_data to 0 */

	list_each (lw_server_client, ctx->clients, client)
	{
	  lw_stream_remove_hook_data ((lw_stream) client, on_client_data, client);
	}
}

lwp_def_hook (server, connect)
lwp_def_hook (server, disconnect)
lwp_def_hook (server, error)

