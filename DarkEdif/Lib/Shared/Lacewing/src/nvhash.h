/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#ifndef _lw_nvhash_h
#define _lw_nvhash_h

typedef struct _lwp_nvhash
{
	char * key;
	char * value;

	UT_hash_handle hh;

} * lwp_nvhash;

void lwp_nvhash_set (lwp_nvhash *, const char * key, const char * value,
					 lw_bool copy);

void lwp_nvhash_set_ex (lwp_nvhash *, size_t key_len, const char * key,
						size_t value_len, const char * value, lw_bool copy);

const char * lwp_nvhash_get (lwp_nvhash *, const char * key, const char * def);

void lwp_nvhash_clear (lwp_nvhash *);

#endif


