/* vim: set noet ts=4 sw=4 sts=4 ft=c:
 *
 * Copyright (C) 2012 James McLaughlin et al.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "common.h"

void lwp_nvhash_set (lwp_nvhash * hash, const char * key, const char * value,
					lw_bool copy)
{
	lwp_nvhash_set_ex (hash, strlen (key), key, strlen (value), value, copy);
}

void lwp_nvhash_set_ex (lwp_nvhash * hash, size_t key_len, const char * key,
						size_t value_len, const char * value,
						lw_bool copy)
{
	lwp_nvhash item;

	HASH_FIND (hh, *hash, key, key_len, item);

	if (item)
	{
		if (copy)
		{
			item->value = (char *) lw_realloc_or_exit (item->value, value_len + 1);

			memcpy (item->value, value, value_len);
			item->value [value_len] = 0;
		}
		else
		{
			free (item->value);
			item->value = (char *) value;
		}

		return;
	}

	item = (lwp_nvhash) lw_calloc_or_exit (sizeof (*item), 1);

	if (copy)
	{
		item->key = (char *) lw_malloc_or_exit (key_len + 1);
		memcpy (item->key, key, key_len);
		item->key [key_len] = 0;

		item->value = (char *) lw_malloc_or_exit (value_len + 1);
		memcpy (item->value, value, value_len);
		item->value [value_len] = 0;
	}
	else
	{
		item->key = (char *) key;
		item->value = (char *) value;
	}

	HASH_ADD_KEYPTR (hh, *hash, item->key, key_len, item);
}

const char * lwp_nvhash_get (lwp_nvhash * hash, const char * key,
							const char * def)
{
	lwp_nvhash item;

	HASH_FIND (hh, *hash, key, strlen (key), item);

	if (item)
		return item->value;

	return def;
}

void lwp_nvhash_clear (lwp_nvhash * hash)
{
	lwp_nvhash tail, item, tmp;

	HASH_ITER (hh, *hash, item, tmp)
	{
		HASH_DEL (*hash, item);

		free (item->key);
		free (item->value);

		free (item);
	}

	while (*hash)
	{
		tail = (lwp_nvhash) (*hash)->hh.tbl->tail;

		HASH_DEL (*hash, tail);

		free (tail->key);
		free (tail->value);
	}
}
