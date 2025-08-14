/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2011, 2012 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_heapbuffer_cxx_h
#define _lw_heapbuffer_cxx_h

extern "C"
{
	#include "heapbuffer.h"
}

namespace Lacewing
{
	class HeapBuffer
	{
		_lwp_heapbuffer buffer;

	public:

		inline HeapBuffer ()
		{
			lwp_heapbuffer_init (&buffer);
		}

		inline ~ HeapBuffer ()
		{
			lwp_heapbuffer_free (&buffer);
		}

		inline char * Buffer ()
		{
			return lwp_heapbuffer_buffer (&buffer);
		}

		inline size_t Length ()
		{
			return lwp_heapbuffer_length (&buffer);
		}

		inline operator char * ()
		{
			return Buffer ();
		}

		inline void Add (const char * buffer, size_t size)
		{
			lwp_heapbuffer_add (&this->buffer, buffer, size);
		}

		template <class T> inline void Add (T v)
		{
			lwp_heapbuffer_add (&this->buffer, (char *) &v, sizeof (v));
		}

		inline HeapBuffer &operator << (char * s)
		{
			Add (s, -1);
			return *this;
		}

		inline HeapBuffer &operator << (const char * s)
		{
			Add (s, -1);
			return *this;
		}

		inline HeapBuffer &operator << (lw_i64 v)
		{
			char buffer [128];
			sprintf (buffer, lwp_fmt_size, (size_t) v);
			Add (buffer, -1);
			return *this;
		}

		inline void Reset()
		{
			lwp_heapbuffer_reset (&buffer);
		}
	};
}

#endif

