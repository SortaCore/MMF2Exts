
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

event lacewing::event_new ()
{
   return (event) lw_event_new ();
}

void lacewing::event_delete (lacewing::event event)
{
   lw_event_delete ((lw_event) event);
}

void _event::signal ()
{
   lw_event_signal ((lw_event) this);
}

void _event::unsignal ()
{
   lw_event_unsignal ((lw_event) this);
}

bool _event::signalled ()
{
   return lw_event_signalled ((lw_event) this);
}

bool _event::wait (long timeout)
{
   return lw_event_wait ((lw_event) this, timeout);
}

void * _event::tag ()
{
   return lw_event_tag ((lw_event) this);
}

void _event::tag (void * tag)
{
   lw_event_set_tag ((lw_event) this, tag);
}

