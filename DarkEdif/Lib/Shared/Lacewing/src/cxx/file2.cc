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

file lacewing::file_new (lacewing::pump pump)
{
	return (file) lw_file_new ((lw_pump) pump);
}

file lacewing::file_new (lacewing::pump pump, const char * filename, const char * mode)
{
	return (file) lw_file_new_open ((lw_pump) pump, filename, mode);
}

bool _file::open (const char * filename, const char * mode)
{
	return lw_file_open ((lw_file) this, filename, mode);
}

bool _file::open_temp ()
{
	return lw_file_open_temp ((lw_file) this);
}

const char * _file::name ()
{
	return lw_file_name ((lw_file) this);
}

