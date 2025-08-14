/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2014 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_refcount_h
#define _lw_refcount_h

// in C++ mode, define atomic as std::atomic
// In C, if MSVC and an ancient version of C standard, then use workaround InterlockedXX functions,
// which use a volatile LONG, so the type of _Atomic must be ignored and it must be aligned on 32 bit address
#ifndef __cplusplus
#if !defined(_MSC_VER) || _STDC_VERSION__ >= 201112
# include <stdatomic.h>
#else
#define msvc_windows_atomic_workaround
# define _Atomic(X) volatile LONG
#endif
#else
# include <atomic>
# define _Atomic(X) std::atomic< X >
#endif

#define MAX_REFS  256

struct lwp_refcount
{
	unsigned short refcount;
	void (* on_dealloc) (void *);

	char name [64];
	const char * refs[MAX_REFS];

	lw_bool enable_logging;
#ifdef msvc_windows_atomic_workaround
	lw_i8 pad; // aligns reflock to 32-bit boundary
#endif
	_Atomic(lw_bool) reflock;
};

lw_bool _lwp_retain (struct lwp_refcount * refcount, const char * name);
lw_bool _lwp_release (struct lwp_refcount * refcount, const char * name);

#define lwp_refcounted \
struct lwp_refcount refcount

#define lwp_retain(x, name) \
	_lwp_retain ((struct lwp_refcount *) (x), name)

#define lwp_release(x, name) \
	_lwp_release ((struct lwp_refcount *) (x), name)

#define lwp_set_dealloc_proc(x, proc) \
	*(void **)&(((struct lwp_refcount *) (x))->on_dealloc) = (void *)(proc);

#define lwp_set_refcount_name(x, n) \
	strcpy (((struct lwp_refcount *) (x))->name, (n));

#define lwp_enable_refcount_logging(x, name) \
	lwp_set_refcount_name (x, name);		 \
	((struct lwp_refcount *) (x))->enable_logging = lw_true;

#endif
