/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_sslclient_h
#define _lw_sslclient_h

typedef struct _lwp_sslclient * lwp_sslclient;

typedef void (* lwp_sslclient_on_handshook) (lwp_sslclient, void * tag);

lwp_sslclient lwp_sslclient_new (SSL_CTX * server_context, lw_server_client socket,
								 lwp_sslclient_on_handshook, void * tag);

void lwp_sslclient_delete (lwp_sslclient);

lw_bool lwp_sslclient_handshook (lwp_sslclient);

const char * lwp_sslclient_npn (lwp_sslclient);

#endif


