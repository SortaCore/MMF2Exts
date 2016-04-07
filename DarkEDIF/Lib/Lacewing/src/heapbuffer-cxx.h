
/* vim: set et ts=4 sw=4 ft=cpp:
 *
 * Copyright (C) 2011, 2012 James McLaughlin.  All rights reserved.
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

