
/* vim: set et ts=3 sw=3 ft=cpp:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.  All rights reserved.
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

#include "../common.h"

error lacewing::error_new ()
{
   return (error) lw_error_new ();
}

void lacewing::error_delete (lacewing::error error)
{
   lw_error_delete ((lw_error) error);
}
    
void _error::add (const char * format, ...)
{
   va_list args;
   va_start (args, format);

   lw_error_addv ((lw_error) this, format, args);

   va_end (args);
}

void _error::add (int code)
{
   lw_error_add ((lw_error) this, code);
}

void _error::add (const char * format, va_list args)
{
   lw_error_addv ((lw_error) this, format, args);
}

size_t _error::size ()
{
   return lw_error_size ((lw_error) this);
}

const char * _error::tostring ()
{
   return lw_error_tostring ((lw_error) this);
}

_error::operator const char * ()
{
   return tostring ();
}

error _error::clone ()
{
   return (error) lw_error_clone ((lw_error) this);
}

void * _error::tag ()
{
   return lw_error_tag ((lw_error) this);
}

void _error::tag (void * tag)
{
   lw_error_set_tag ((lw_error) this, tag);
}

