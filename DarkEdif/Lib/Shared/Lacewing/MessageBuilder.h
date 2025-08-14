/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "Lacewing.h"
#include <vector>
#include <assert.h>

#ifndef LacewingMessageBuilder
#define LacewingMessageBuilder

class messagebuilder
{

protected:

	lw_ui32 allocated = 0;

public:

	char * buffer = nullptr;
	lw_ui32 size = 0U;

	messagebuilder()
	{
	}

	~ messagebuilder()
	{
		free(buffer);
		buffer = nullptr;
	}

	void add(const char * const buffer, size_t sizeP)
	{
		if (sizeP == SIZE_MAX)
			sizeP = (lw_ui32)strlen(buffer);

		if constexpr (sizeof(sizeP) > 4)
			assert(sizeP < 0xFFFFFFFF);

		lw_ui32 size = (lw_ui32)sizeP;

		if (this->size + size > allocated)
		{
			if (!allocated)
				allocated = 1024 * 4;
			else
				allocated *= 3;

			if (this->size + size > allocated)
				allocated += size;

			this->buffer = (char *)lw_realloc_or_exit(this->buffer, allocated);
		}

		memcpy(this->buffer + this->size, buffer, size);
		this->size += size;
	}

	template<typename t>
	inline void add (t value)
	{
		// If this second assertion triggers, you're passing a pointer value to be embedded in the message.
		// This will append the address the pointer points to, NOT the content of the pointer.
		// Since this is unlikely to be expected behaviour, you should check your code.
		// You probably want the add(data, sizeof(data))
		// If adding a string, pass add(data, -1)
		// If adding a single byte, pass add(&data, 1)
		static_assert(!std::is_pointer<t>::value,
			"Check you meant to pass a pointer address. That doesn't make a lot of sense.");
		// std::string is nasty
		static_assert(!std::is_same<t, std::string>::value,
			"std::string data type being added.");
		static_assert(std::is_integral<t>::value,
			"Advanced data type being added.");

		add((const char *) &value, sizeof(t));
	}

	/*
	inline void AddNetwork16Bit (short Value)
	{
		Value = htons (Value);
		Add ((char *) &Value, sizeof(Value));
	}

	inline void AddNetwork24Bit (int Value)
	{
		Value = htonl (Value);
		Add (((char *) &Value) + 1, 3);
	}

	inline void AddNetwork32Bit (int Value)
	{
		Value = htonl (Value);
		Add ((char *) &Value, sizeof(Value));
	}

	inline void AddNetworkX31Bit (unsigned int Value)
	{
		Value = htonl (Value);

		*(char *) &Value &= 0x7F; // 0 first bit

		Add ((char *) &Value, sizeof(Value));
	}*/

	void reset()
	{
		size = 0;
	}

	void send(lacewing::client socket, int offset = 0)
	{
		socket->write(buffer + offset, size - offset);
	}

	void send(lacewing::server_client socket, int offset = 0)
	{
		socket->write(buffer + offset, size - offset);
	}

	void send(lacewing::udp udp, lacewing::address address, int offset = 0)
	{
		udp->send(address, buffer + offset, size - offset);
	}

};
template<> inline
void messagebuilder::add(std::string value)
{
	add(value.data(), (lw_i32)value.size());
}
template<> inline
void messagebuilder::add(std::string_view value)
{
	add(value.data(), (lw_i32)value.size());
}

#endif

