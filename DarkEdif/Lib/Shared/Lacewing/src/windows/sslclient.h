/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_winsslclient_h
#define _lw_winsslclient_h

typedef struct _lwp_winsslclient * lwp_winsslclient;

lwp_winsslclient lwp_winsslclient_new (CredHandle server_creds, lw_stream socket);

void lwp_winsslclient_delete (lwp_winsslclient);

#endif


