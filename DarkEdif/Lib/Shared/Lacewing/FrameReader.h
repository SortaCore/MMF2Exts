/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * Copyright (C) 2011 James McLaughlin.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * https://opensource.org/license/mit
*/

#include "MessageBuilder.h"

#ifndef lacewingframereader
#define lacewingframereader

class framereader
{

protected:

	messagebuilder buffer;

	lw_i32	state = 0;
	lw_i32	sizebytesleft = 0;
	lw_ui32	messagesize = 0;
	lw_ui8  messagetype = 0;

public:

	void  * tag = nullptr;
	bool (* messagehandler) (void * tag, unsigned char type, const char * message, size_t size) = nullptr;

	framereader() {
	}

	// Processes a message, returns true if more messages will follow in same data packet.
	// Sets up dataPtr and sizePtr to point to the next one, for the next process() call.
	// If no more messages or error, returns false.
	inline bool process(const char ** dataPtr, size_t * sizePtr)
	{
		const char *& data = *dataPtr;
		size_t &size = *sizePtr;

		while (state < 3 && size -- > 0)
		{
			lw_ui8 byte = *(data ++);

			switch (state)
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
						buffer.add <lw_ui8 > (byte);

						if ((-- sizebytesleft) == 0)
						{
							switch (buffer.size)
							{
							case 2:
								messagesize = *(lw_ui16 *) buffer.buffer;
								break;
							case 4:
								messagesize = *(lw_ui32 *) buffer.buffer;
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
			}
		}

		if (state < 3) /* header not complete yet */
			return false; // No message to do, exit out

		if (buffer.size == 0)
		{
			if (size == messagesize)
			{
				// The message isn't fragmented, and it's the only message.
				if (!messagehandler(tag, messagetype, data, messagesize))
					return false; // Error, exit out

				state = 0;
				return false; // No more messages, exit out
			}

			if (size > messagesize)
			{
				/* their message isn't fragmented, but there are more messages than
					this one.  lovely hack to give it a null terminator without copying
					the message..!  */

				char nextbyte = data[messagesize];
				*(char *)&data[messagesize] = 0;

				const bool mhRet = messagehandler(tag, messagetype, data, messagesize);
				if (!mhRet)
					return false; // Error, exit out

				// Not safe to do this if messagehandler() is false, if we're using const memory
				*(char *)&data[messagesize] = nextbyte;
				state = 0;

				// Was process(data + messagesize, size - messagesize);
				*dataPtr = data + messagesize;
				*sizePtr = size - messagesize;
				return true; // Message follows
			}
		}

		size_t thismessagebytes = messagesize - buffer.size;

		if (size < thismessagebytes)
			thismessagebytes = size;

		buffer.add(data, thismessagebytes);

		size -= thismessagebytes;
		data += thismessagebytes;

		if (buffer.size == messagesize)
		{
			buffer.add <lw_ui8> (0);

			if (!messagehandler(tag, messagetype, buffer.buffer, messagesize))
				return false; // Error, exit out
			buffer.reset();

			state = 0;

			if (size > 0)
			{
				// Was process(data, size);
				*dataPtr = data;
				*sizePtr = size;
				return true; // Message follows
			}
		}

		return false;
	}
};

#endif

