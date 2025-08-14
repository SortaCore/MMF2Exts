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

// TODO: This isn't an ideal workaround.
extern "C" size_t lwp_stream_write(lw_stream ctx, const char* buffer, size_t size, int flags);

#ifndef lacewingframebuilder
#define lacewingframebuilder
static const char zerothree[3] = { 0, 0, 0 };

class framebuilder : public messagebuilder
{
protected:

	void preparefortransmission(bool iswebsocketclient)
	{
		if (tosend)
			return;

		lw_ui32 type = *(lw_ui32 *) buffer;
		lw_i32 messagesize = size - 8;

		lw_ui32 headersize;

		// We're sending to a websocket client, we need to mash this into WebSocket format
		if (iswebsocketclient)
		{
			// If we're sending to a websocket client, we must be a server.
			// If we're a server, the UDP header has one byte: the type.
			if (origUDP != UINT32_MAX)
				type = buffer[7] | 0x8;

			// Since we send text messages to channels and so on, we can't use text opcode for text messages
			const lw_ui8 flagopcode = 0b10000010; // fin flag enabled + binary message
			if (messagesize + 1 <= 125)
			{
				(*(lw_ui8*)(buffer + 5)) = flagopcode;
				(*(lw_ui8*)(buffer + 6)) = (lw_ui8)(messagesize + 1);
				(*(lw_ui8*)(buffer + 7)) = (lw_ui8)type;
				headersize = 3;
				tosend = (buffer + 8) - headersize;
				tosendsize = messagesize + headersize;
			}
			else if (messagesize <= 0xFFFF)
			{
				(*(lw_ui8*)(buffer + 3)) = flagopcode;
				(*(lw_ui8*)(buffer + 4)) = (lw_ui8)126; // indicate uint16 following size
				(*(lw_ui16*)(buffer + 5)) = htons((lw_ui16)(messagesize + 1));
				(*(lw_ui8*)(buffer + 7)) = (lw_ui8)type;
				headersize = 5;
				tosend = buffer + 8 - headersize;
				tosendsize = messagesize + headersize;
			}
			else
			{
				// The TCP header uses only 8 bytes, and we need 11 for uint64 size, so hack an extra three bytes in
				// It's not efficient to memmove like this, but anyone passing this much data shouldn't expect speed
				// TODO: For speed, add extra header space, so there's room for the full thing without memmove()
				add(zerothree, sizeof(zerothree));

				memmove(buffer + 11, buffer + 8, messagesize);

				(*(lw_ui8*)(buffer)) = flagopcode;
				(*(lw_ui8*)(buffer + 1)) = (lw_ui8)127; // indicate uint64 following size

				const lw_i16 endianTest = 42;
				if (*(lw_i8*)&endianTest == endianTest)
				{
					(*(lw_ui32*)(buffer + 2)) = 0; // assumes messagesize is not > 32bit, which is expected due to sizeof(this->size) and sizeof(messagesize)
					(*(lw_ui32*)(buffer + 6)) = htonl(messagesize + 1);
				}
				else // big-endian
				{
					(*(lw_ui32*)(buffer + 2)) = htonl(messagesize + 1);
					(*(lw_ui32*)(buffer + 6)) = 0; // see above comment
				}

				(*(lw_ui8*)(buffer + 10)) = (lw_ui8)type;

				tosend = buffer;
				tosendsize = size;
			}

			return;
		}

		// Message size < 254; store as type byte + size byte
		if (messagesize < 0xfe)
		{
			(*(lw_ui8*)(buffer + 6)) = (lw_ui8)type;
			(*(lw_ui8*)(buffer + 7)) = (lw_ui8)messagesize;

			headersize = 2;
		}
		// Message size >= 0xFF and <= 0xFFFF; store as type byte, plus size indicator byte of 254, plus size uint16
		else if (messagesize < 0xffff)
		{
			(*(lw_ui8*)(buffer + 4)) = (lw_ui8)type;

			(*(lw_ui8 *) (buffer + 5))	= 254;
			(*(lw_ui16 *) (buffer + 6)) = (lw_ui16)messagesize;

			headersize = 4;
		}
		// Message size > 0xFFFF and <= 0xFFFFFFFF; store as type byte, plus size indicator byte of 255, plus size uint32
		else if ((lw_ui32)messagesize < 0xffffffff)
		{
			(*(lw_ui8*)(buffer + 2)) = (lw_ui8)type;

			(*(lw_ui8 *) (buffer + 3))	= 255;
			(*(lw_ui32 *) (buffer + 4)) = messagesize;

			headersize = 6;
		}
		else
			return;

		tosend	 = (buffer + 8) - headersize;
		tosendsize =  messagesize + headersize;
	}

	bool isudpclient;

	char* tosend;
	int tosendsize;
	lw_ui32 origUDP;
	lw_i8 wasWebLast;

public:

	framebuilder(bool isudpclient)
	{
		this->isudpclient = isudpclient;
		tosend = nullptr;
		tosendsize = 0;
		origUDP = UINT32_MAX;
		wasWebLast = -1;
	}

	inline void addheader(lw_ui8 type, lw_ui8 variant, bool forudp = false, int udpclientid = -1)
	{
		assert(size == 0 && "lacewing framebuilder.addheader() error: adding header to message that already has one.");

		if (!forudp)
		{
			add <lw_ui32> ((type << 4) | variant);
			add <lw_ui32> (0); // this is used for reserving space for adding message size later, in preparefortransmission()

			return;
		}
		// Pad to 8 bytes in buffer
		add(std::string(8 - 1 - (isudpclient ? 2 : 0), '\xCD'));

		add <lw_ui8> ((lw_ui8)((type << 4) | variant));

		if (isudpclient)
			add <lw_ui16> ((lw_ui16)udpclientid);
		else
			origUDP = ((lw_ui32*)buffer)[1];
	}

	inline void send(lacewing::server_client client, bool clear = true)
	{
		if (wasWebLast == -1 || client->is_websocket() != wasWebLast)
		{
			wasWebLast = client->is_websocket();
			tosend = nullptr; // or preparefortransmission does nothing
			preparefortransmission(wasWebLast);
		}

		if (wasWebLast)
			lwp_stream_write((lw_stream)client, tosend, tosendsize, 2 /* lwp_stream_write_ignore_busy */);
		else
			client->write(tosend, tosendsize);

		if (clear)
			framereset();
	}

	inline void send(lacewing::client client, bool clear = true)
	{
		preparefortransmission(false);
		client->write(tosend, tosendsize);

		if (clear)
			framereset();
	}

	inline void revert() {
		((lw_ui32*)buffer)[1] = origUDP;
		tosend = nullptr;
		tosendsize = 0;
	}

	inline void send(lacewing::udp udp, lacewing::address address, bool clear = true)
	{
		udp->send(address, &buffer[isudpclient ? 5 : 7], size - (isudpclient ? 5 : 7));

		if (clear)
			framereset();
	}

	inline void framereset()
	{
		reset();
		tosend = NULL;
		tosendsize = 0;
		wasWebLast = -1;
	}

};

#endif

