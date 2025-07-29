#define ENABLE_CXX
// #define ENABLE_SSL // define in project settings if using webserver https
#define ENABLE_THREADS

//#define USE_EPOLL
#define USE_KQUEUE


#define HAVE_MALLOC_MALLOC_H
#define HAVE_NETDB_H
//#define HAVE_SYS_PRCTL_H
//#define HAVE_SYS_SENDFILE_H
//#define HAVE_SYS_TIMERFD_H

//#define HAVE_DECL_PR_SET_NAME
//#define HAVE_DECL_TCP_CORK
#define HAVE_DECL_TCP_NOPUSH
#if __DARWIN_C_LEVEL >= 200809L
#define HAVE_DECL_MSG_NOSIGNAL
#endif
#define HAVE_DECL_SO_NOSIGPIPE

#define HAVE_TIMEGM

// Define to include IPV6PKT_INFO in netinet.h; the choice is old RFC 2292, or newer RFC 3542
#define __APPLE_USE_RFC_3542
