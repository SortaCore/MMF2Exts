
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

#include "messagebuilder.h"

#ifndef lacewingframereader
#define lacewingframereader

class framereader
{
    
protected:

    messagebuilder buffer;
        
    int            state;
    int            sizebytesleft;
    size_t		   messagesize;
    unsigned char  messagetype; 

public:

    void  * tag;
    void (* messagehandler) (void * tag, unsigned char type, const char * message, size_t size);

    framereader()
    {
        messagehandler = 0;
        state          = 0;
		messagesize	   = 0;
		messagetype    = 0;
    }

    inline void process(char * data, unsigned int size)
    {
        while(state < 3 && size -- > 0)
        {
            unsigned char byte = *(data ++);

            switch(state)
            {
                case 0: /* haven't yet got type */
                    
                    messagetype = byte;
                    state = 1;
                    sizebytesleft = 0;

                    break;
            
                case 1: /* have type, but no size */
                {
                    if (sizebytesleft > 0)
                    {
                        buffer.add <char> (byte);
                        
                        if ((-- sizebytesleft) == 0)
                        {
                            switch(buffer.size)
                            {
                            case 2:

                                messagesize = *(lw_i16 *) buffer.buffer;
                                break;

                            case 4:

                                messagesize = *(lw_i32 *) buffer.buffer;
                                break;
                            }

                            buffer.reset();

                            state = 3;
                            break;
                        }

                        break;
                    }

                    /* byte is the first byte of the size */

                    if (byte == 254)
                    {
                        /* 16 bit message size to follow */

                        sizebytesleft = 2;
                        break;
                    }

                    if (byte == 255)
                    {
                        /* 32 bit message size to follow */

                        sizebytesleft = 4;
                        break;
                    }

                    /* 8 bit message size */

                    messagesize = byte;
                    state = 3;

                    break;
                }
            };
        }

        if (state < 3) /* header not complete yet */
            return;

        if (buffer.size == 0)
        {
            if (size == messagesize)
            {
                /* the message isn't fragmented, and it's the only message. */

                messagehandler(tag, messagetype, data, messagesize);
                state = 0;
				

                return;
            }

            if (size > messagesize)
            {
                /* there message isn't fragmented, but there are more messages than
                   this one.  lovely hack to give it a null terminator without copying
                   the message..!  */

				char nextbyte = data[messagesize];
				data[messagesize] = 0;

				messagehandler(tag, messagetype, data, messagesize);
				data[messagesize] = nextbyte;

				state = 0;
				process(data + messagesize, size - messagesize);

                return;
            }
        }

        unsigned int thismessagebytes = messagesize - buffer.size;

        if (size < thismessagebytes)
            thismessagebytes = size;

        buffer.add(data, thismessagebytes);
        
        size -= thismessagebytes;
        data += thismessagebytes;

        if (buffer.size == messagesize)
        {
            buffer.add <char> (0);

            messagehandler(tag, messagetype, buffer.buffer, messagesize);
            buffer.reset();

            state = 0;

            if (size > 0)
                process(data, size);
        }
    }


};

#endif

