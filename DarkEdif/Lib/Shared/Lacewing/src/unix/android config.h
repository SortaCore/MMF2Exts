#define ENABLE_CXX
// #define ENABLE_SSL // define in project settings if using webserver https
#define ENABLE_THREADS

#define USE_EPOLL
//#define USE_KQUEUE

#define HAVE_MALLOC_H
#define HAVE_NETDB_H
#define HAVE_SYS_PRCTL_H
#define HAVE_SYS_SENDFILE_H
#if __ANDROID_API__ >= 19
#define HAVE_SYS_TIMERFD_H
#endif

#define HAVE_DECL_PR_SET_NAME
#define HAVE_DECL_TCP_CORK
//#define HAVE_DECL_TCP_NOPUSH
#define HAVE_DECL_MSG_NOSIGNAL

// Some guides say to use MSG_NOSIGNAL instead of SO_NOSIGPIPE on Android,
// but that seems to create error 92, protocol not available
//#define HAVE_DECL_SO_NOSIGPIPE

#define HAVE_TIMEGM
