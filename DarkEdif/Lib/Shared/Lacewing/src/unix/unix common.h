/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2011, 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sched.h>

#ifdef __ANDROID__
	#include "android config.h"

	#ifndef __LP64__
		#include <time64.h>
	#endif
	#include <android/log.h>
#endif
#ifdef __APPLE__
	#include "ios config.h"
#endif

#ifdef HAVE_SYS_TIMERFD_H
	#include <sys/timerfd.h>

	#ifndef USE_KQUEUE
		#define _lacewing_use_timerfd
	#endif
#endif

#ifdef HAVE_SYS_SENDFILE_H
	#include <sys/sendfile.h>
#endif

#ifdef HAVE_NETDB_H
	#include <netdb.h>
#endif

// TODO: Is this undef necessary? If so, document why.
// It seems Apple had a buggy implementation, but I think that's an ancient bug long fixed?
//#ifndef __APPLE__
	#ifdef TCP_CORK
		#define lw_cork TCP_CORK
	#else
		#ifdef TCP_NOPUSH
			#define lw_cork TCP_NOPUSH
		#endif
	#endif
//#endif

#if defined(USE_EPOLL)
	#include <sys/epoll.h>

	#ifndef EPOLLRDHUP
	  #define EPOLLRDHUP 0x2000
	#endif
#elif defined(USE_KQUEUE)
	#include <sys/event.h>
#endif

#include <string.h>
#include <limits.h>

#ifdef HAVE_SYS_PRCTL_H
	#include <sys/prctl.h>
#endif

#ifdef ENABLE_SSL
	#include <openssl/ssl.h>
	#include <openssl/md5.h>
	#include <openssl/sha.h>
	#include <openssl/err.h>
#endif

#ifdef OPENSSL_NPN_NEGOTIATED
	#define _lacewing_npn
#endif

#define lwp_last_error errno
#define lwp_last_socket_error errno

#define _atoi64 atoll

typedef int lwp_socket;

#define lwp_vsnprintf vsnprintf
#define lwp_snprintf snprintf
#define lwp_fmt_size "%zd"

