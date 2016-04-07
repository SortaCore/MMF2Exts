
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

flashpolicy lacewing::flashpolicy_new (lacewing::pump pump)
{
   return (flashpolicy) lw_flashpolicy_new ((lw_pump) pump);
}

void lacewing::flashpolicy_delete (lacewing::flashpolicy flashpolicy)
{
   lw_flashpolicy_delete ((lw_flashpolicy) flashpolicy);
}

void _flashpolicy::host (const char * filename)
{
   lw_flashpolicy_host ((lw_flashpolicy) this, filename);
}

void _flashpolicy::host (const char * filename, lacewing::filter filter)
{
   lw_flashpolicy_host_filter ((lw_flashpolicy) this,
                               filename,
                               (lw_filter) filter);
}

void _flashpolicy::unhost ()
{
   lw_flashpolicy_unhost ((lw_flashpolicy) this);
}

bool _flashpolicy::hosting ()
{
   return lw_flashpolicy_hosting ((lw_flashpolicy) this);
}

void * _flashpolicy::tag ()
{
   return lw_flashpolicy_tag ((lw_flashpolicy) this);
}

void _flashpolicy::tag (void * tag)
{
   lw_flashpolicy_set_tag ((lw_flashpolicy) this, tag);
}

