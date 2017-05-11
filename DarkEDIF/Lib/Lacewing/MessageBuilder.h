
/* vim: set et ts=4 sw=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.  All rights reserved.
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
#include "Lacewing.h"
#include <vector>

#ifndef LacewingMessageBuilder
#define LacewingMessageBuilder

class messagebuilder
{
    
protected:

    unsigned int allocated;

public:
    
    char * buffer;
    unsigned int size;

    messagebuilder()
    {
        size   = allocated = 0;
        buffer = 0;
    }

    ~ messagebuilder()
    {
        free(buffer);
		buffer = nullptr;
    }

    void add(const char * const buffer, int size)
    {
        if (size == -1)
            size = strlen(buffer);

        if (this->size + size > allocated)
        {
            if (!allocated)
                allocated = 1024 * 4;
            else
                allocated *= 3;

            if (this->size + size > allocated)
                allocated += size;

            char * test = (char *) realloc(this->buffer, allocated);
			if (!test)
				throw std::exception("could not reallocate buffer for message.");
			this->buffer = test;
        }

        memcpy_s(this->buffer + this->size, size, buffer, size);
        this->size += size;
    }

    template<class t> inline void add (t value)
    {
		// If this second assertion triggers, you're passing a pointer value to be embedded in the message.
		// This will append the address the pointer points to, NOT the content of the pointer.
		// Since this is unlikely to be expected behaviour, you should check your code.
		// You probably want the add(data, sizeof(data))
		// If adding a string, pass add(data, -1)
		// If adding a single byte, pass add(&data, 1)
		static_assert(!std::is_pointer<decltype(value)>::value,
			"Check you meant to pass a pointer address. That doesn't make a lot of sense.");

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

#endif

