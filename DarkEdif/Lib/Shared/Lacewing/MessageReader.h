/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include <algorithm>

#ifndef lacewingmessagereader
#define lacewingmessagereader

class messagereader
{
protected:

	const char * buffer;

	size_t size;

	std::vector<char *> tofree;

public:

	size_t offset;

	bool failed;

	inline messagereader(const char * buffer, size_t size)
	{
		failed = false;

		this->buffer = buffer;
		this->size = size;

		this->offset = 0;
	}

	inline ~messagereader()
	{
		std::for_each(tofree.begin(), tofree.end(), [&](char * &c) { free(c); });
		tofree.clear();
	}

	inline bool check(const size_t size)
	{
		if (failed)
			return false;

		if (offset + size > this->size)
		{
			failed = true;
			return false;
		}

		return true;
	}

	template<class t> inline t get()
	{
		if (!check(sizeof(t)))
			return 0;

		t value = *(t *) (buffer + offset);

		offset += sizeof(t);
		return value;
	}

	std::string_view get (size_t size)
	{
		if (!check(size))
			return std::string_view();

		std::string_view output (buffer + offset, size);
		offset += size;

		return output;
	}

	inline size_t bytesleft ()
	{
		return size - offset;
	}

	inline const char * cursor ()
	{
		return buffer + offset;
	}

	std::string_view getremaining(const lw_ui32 minimumlength = 0U, const bool nullTerminator = false, const bool stripNull = false, const lw_ui32 maximumlength = 0xffffffff)
	{
		if (failed)
			return std::string_view();

		assert(!nullTerminator || minimumlength > 0);
		assert(maximumlength >= minimumlength);

		std::string_view remaining(this->buffer + offset, bytesleft());
		offset += remaining.size();

		// Message must be in size limits
		if (remaining.size() > maximumlength || remaining.size() < minimumlength)
			failed = true;
		// Null terminator required, and not present
		else if (nullTerminator && (remaining.empty() || remaining.back() != '\0'))
			failed = true;
		if (!failed && stripNull)
		{
			if (!remaining.empty() && remaining.back() == '\0')
				remaining.remove_suffix(1);
			if (!remaining.empty() && remaining.back() == '\0')
				failed = true;
		}

		return remaining;
	}
};

#endif
