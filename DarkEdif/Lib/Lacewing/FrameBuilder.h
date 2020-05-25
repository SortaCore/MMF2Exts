
/* vim: set et ts=4 sw=4 ft=cpp:
 *
 * copyright (c) 2011 james mclaughlin.  all rights reserved.
 *
 * redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. redistributions of source code must retain the above copyright
 *	notice, this list of conditions and the following disclaimer.
 *
 * 2. redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
 *
 * this software is provided by the author and contributors ``as is'' and
 * any express or implied warranties, including, but not limited to, the
 * implied warranties of merchantability and fitness for a particular purpose
 * are disclaimed.  in no event shall the author or contributors be liable
 * for any direct, indirect, incidental, special, exemplary, or consequential
 * damages (including, but not limited to, procurement of substitute goods
 * or services; loss of use, data, or profits; or business interruption)
 * however caused and on any theory of liability, whether in contract, strict
 * liability, or tort (including negligence or otherwise) arising in any way
 * out of the use of this software, even if advised of the possibility of
 * such damage.
 */
#include "messagebuilder.h"

#ifndef lacewingframebuilder
#define lacewingframebuilder

class framebuilder : public messagebuilder
{
protected:

	void preparefortransmission()
	{
		if (tosend)
			return;

		int type = *(unsigned int *) buffer;
		int messagesize = size - 8;

		int headersize;

		if (messagesize < 254)
		{
			buffer[6] = type;
			buffer[7] = messagesize;

			headersize = 2;
		}
		else if (messagesize < 0xffff)
		{
			buffer[4] = type;

			(*(unsigned char  *) (buffer + 5)) = 254;
			(*(unsigned short *) (buffer + 6)) = messagesize;

			headersize = 4;
		}
		else if (messagesize < 0xffffffff)
		{
			buffer[2] = type;

			(*(unsigned char  *) (buffer + 3)) = 255;
			(*(unsigned int	*) (buffer + 4)) = messagesize;

			headersize = 6;
		}
		else
			return;

		tosend	 = (buffer + 8) - headersize;
		tosendsize =  messagesize + headersize;
	}

	bool isudpclient;

	char * tosend;
	int tosendsize;

public:

	framebuilder(bool isudpclient)
	{
		this->isudpclient = isudpclient;
		tosend = nullptr;
		tosendsize = 0;
	}

	inline void addheader(unsigned char type, unsigned char variant, bool forudp = false, int udpclientid = -1)
	{
		if (size != 0)
			throw std::exception("lacewing framebuilder.addheader() error: adding header to message that already has one.");

		if (!forudp)
		{
			add <unsigned int> ((type << 4) | variant);
			add <unsigned int> (0);

			return;
		}

		add <unsigned char>  ((type << 4) | variant);

		if (isudpclient)
			add <unsigned short> (udpclientid);
	}

	inline void send(lacewing::server_client client, bool clear = true)
	{
		preparefortransmission();
		client->write(tosend, tosendsize);

		if (clear)
			framereset();
	}

	inline void send(lacewing::client client, bool clear = true)
	{
		preparefortransmission();
		client->write(tosend, tosendsize);

		if (clear)
			framereset();
	}

	inline void send(lacewing::udp udp, lacewing::address address, bool clear = true)
	{
		udp->send(address, buffer, size);
 
		if (clear)
			framereset();
	}

	inline void framereset()
	{
		reset();
		tosend = 0;
		tosendsize = 0;
	}

};

#endif

