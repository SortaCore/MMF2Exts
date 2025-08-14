/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2012, 2013 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 * 
 * liblacewing and Lacewing Relay/Blue are provided under MIT license.
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

