/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2013, 2014 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_refcount_h
#define _lw_refcount_h

#ifndef __cplusplus
#if !defined(_MSC_VER) || _STDC_VERSION__ >= 201112
# include <stdatomic.h>
#else
#define msvc_windows_atomic_workaround
# define _Atomic(X) volatile LONG
#endif
#else
// While these are implemented in a C way, and only accessible by the below C functions, we include it
// in C++ headers, so reference it properly
# include <atomic>
# define _Atomic(X) std::atomic< X >
#endif

struct lwp_refcount
{
	_Atomic(unsigned short) refcount;
	void (* on_dealloc) (void *);
};

static inline lw_bool _lwp_retain(struct lwp_refcount* refcount)
{
#ifdef msvc_windows_atomic_workaround
	InterlockedIncrement(&refcount->refcount);
#else
	if (refcount->refcount == 0)
		atomic_init(&refcount->refcount, (unsigned short)0);
	++ refcount->refcount;
#endif

	return lw_false;
}

static inline lw_bool _lwp_release (struct lwp_refcount * refcount)
{
#ifdef msvc_windows_atomic_workaround
	if (InterlockedDecrement(&refcount->refcount) == 0)
#else
	if ((--refcount->refcount) == 0)
#endif
	{
	  if (refcount->on_dealloc)
		 refcount->on_dealloc ((void *) refcount);
	  else
		 free (refcount);

	  return lw_true;
	}

	return lw_false;
}

#define lwp_refcounted														\
	struct lwp_refcount refcount

#define lwp_retain(x, name)													\
	_lwp_retain ((struct lwp_refcount *) (x))								 \

#define lwp_release(x, name)												  \
	_lwp_release ((struct lwp_refcount *) (x))								 \

#define lwp_set_dealloc_proc(x, proc) do {									\
  *(void **) &(((struct lwp_refcount *) (x))->on_dealloc) = (void *) (proc);  \
} while (0);																  \

#define lwp_set_retain_proc(x, proc)
#define lwp_set_release_proc(x, proc)
#define lwp_set_refcount_name(x, name)
#define lwp_enable_refcount_logging(x, name)
#endif

