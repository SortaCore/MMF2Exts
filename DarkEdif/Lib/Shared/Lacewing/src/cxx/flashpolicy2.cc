/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "../common.h"
#include "../flashpolicy.h"

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

void _flashpolicy::on_error(_flashpolicy::hook_error func)
{
	// WARNING: this function requires a definition of lw_flashpolicy which requires liblacewing source code to be edited.
	((lw_flashpolicy) this)->on_error = (lw_flashpolicy_hook_error)func;
}