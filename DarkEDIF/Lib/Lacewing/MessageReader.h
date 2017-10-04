
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

    unsigned int offset;

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

    inline bool check(unsigned int size)
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

    char * get (unsigned int size)
    {
        if (!check(size))
            return 0;

        char * output = (char *) calloc (size + 1, 1);

        if (!output)
        {
            failed = true;
            return 0;
        }

        tofree.push_back (output);

        memcpy(output, buffer + offset, size);
        output[size] = 0;

        offset += size;

        return output;
    }

    inline int bytesleft ()
    {
        return size - offset;
    }

    inline const char * cursor ()
    {
        return buffer + offset;
    }

    inline const char * getremaining(bool allowempty = true)
    {
        if (failed)
            return this->buffer;

        const char * remaining = this->buffer + offset;
        offset += size;

        if (!allowempty && !*remaining)
            failed = true;

        return remaining;
    }

    inline void getremaining(const char * &buffer, unsigned int &size, unsigned int minimumlength = 0U, unsigned int maximumlength = 0xffffffff)
    {
        buffer = this->buffer + offset;
        size   = this->size - offset;

        if (size > maximumlength || size < minimumlength)
            failed = true;

        offset += size;
    }

   /* inline short Network16Bit ()
    {
        return ntohs (Get <short> ());
    }
    
    inline int Network24Bit ()
    {
        if (!Check (3))
            return 0;
        
        return Read24Bit (Buffer + Offset);
    }

    inline int Network32Bit ()
    {
        return ntohl (Get <int> ());
    }
    
    inline int NetworkX31Bit ()
    {
        int value = Get <int> ();
   
        *(char *) &value &= 0x7F;
        
        return ntohl (value);
    }*/
};

#endif

