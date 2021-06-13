
/* vim: set et ts=4 sw=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
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
