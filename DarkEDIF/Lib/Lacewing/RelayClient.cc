
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
#include "FrameBuilder.h"
#include "FrameReader.h"
#include "MessageReader.h"
#include <vector>
#include <algorithm>

namespace lacewing
{
	struct channelinternal;

	struct relayclientinternal
	{
		lacewing::relayclient &		client;
		lacewing::client			socket;
		lacewing::udp 				udp;
		lacewing::timer				timer;

		relayclient::handler_connect               handler_connect;
		relayclient::handler_connectiondenied      handler_connectiondenied;
		relayclient::handler_disconnect            handler_disconnect;
		relayclient::handler_message_server        handler_message_server;
		relayclient::handler_message_channel       handler_message_channel;
		relayclient::handler_message_peer          handler_message_peer;
		relayclient::handler_message_serverchannel handler_message_serverchannel;
		relayclient::handler_error                 handler_error;
		relayclient::handler_channel_join          handler_channel_join;
		relayclient::handler_channel_joindenied    handler_channel_joindenied;
		relayclient::handler_channel_leave         handler_channel_leave;
		relayclient::handler_channel_leavedenied   handler_channel_leavedenied;
		relayclient::handler_name_set              handler_name_set;
		relayclient::handler_name_changed          handler_name_changed;
		relayclient::handler_name_denied           handler_name_denied;
		relayclient::handler_peer_connect          handler_peer_connect;
		relayclient::handler_peer_disconnect       handler_peer_disconnect;
		relayclient::handler_peer_changename       handler_peer_changename;
		relayclient::handler_channellistreceived   handler_channellistreceived;

		relayclientinternal(relayclient &_client, lacewing::client _socket,
			lacewing::udp _udp, pump _eventpump) : client(_client),
			socket(_socket), udp(_udp), message(true), messageMF(true),
			timer(lacewing::timer_new(_eventpump)),
			name(nullptr), welcomemessage(nullptr)
		{
			handler_connect = 0;
			handler_connectiondenied = 0;
			handler_disconnect = 0;
			handler_message_server = 0;
			handler_message_channel = 0;
			handler_message_peer = 0;
			handler_message_serverchannel = 0;
			handler_error = 0;
			handler_channel_join = 0;
			handler_channel_joindenied = 0;
			handler_channel_leave = 0;
			handler_channel_leavedenied = 0;
			handler_name_set = 0;
			handler_name_changed = 0;
			handler_name_denied = 0;
			handler_peer_connect = 0;
			handler_peer_disconnect = 0;
			handler_peer_changename = 0;
			handler_channellistreceived = 0;
			
			message.framereset();
			message.reset();

			messageMF.framereset();
			messageMF.reset();
			
			socket->tag(this);
			udp->tag(this);
			
			reader.tag = this;
			reader.messagehandler = messagehandler;

			timer->tag(this);
			timer->on_tick(clienttick);
			
			clear();
		}
		framereader reader;

		static void messagehandler(void * tag, unsigned char type, const char * message, size_t size);
		void        messagehandler(unsigned char type, const char * message, size_t size, bool blasted);

		static void clienttick(lacewing::timer timer);
		void        clienttick();

		channelinternal * findchannelbyid(unsigned short id);

		framebuilder message, messageMF;

		std::vector<relayclient::channellisting *> channellist;

		/// <summary> Empties the channel list. </summary>
		void clearchannellist()
		{
			std::for_each(channellist.begin(), channellist.end(), [&](relayclient::channellisting *&c) { delete c; });
			channellist.clear();
		}

		void clear();

		const char * name;
		unsigned short id;
		const char * welcomemessage;
		bool connected;

		std::vector<channelinternal *> channels;
		~relayclientinternal()
		{
			free((void *)name);
			free((void *)welcomemessage);
		}
	};

	struct peerinternal
	{
		relayclient::channel::peer public_;
		channelinternal &channel;

		unsigned short id;
		const char * name, *prevname;

		bool ischannelmaster;

		peerinternal(channelinternal &_channel, unsigned short id, unsigned char flags, const char * name)
			: channel(_channel), prevname(nullptr)
		{
			public_.internaltag = this;
			public_.tag = 0;

			// only flag available is 0x1: is channel master
			if (flags & ~0x1)
				throw std::exception("peerinternal ctor error: unrecognised peer flags.");
			
			if (!name || !name[0] || strnlen(name, 256) == 256)
				throw std::exception("peerinternal ctor error: null, blank, or too-long name used.");

			this->id = id;
			this->name = _strdup(name);
			this->ischannelmaster = ((flags & 0x1) == 0x1);
			this->public_.isclosed = false;
		}
		~peerinternal() noexcept(false)
		{
			if (!public_.isclosed)
				throw std::exception("Channel was not set to closed before it was deleted.");
			free((char *)name);
			free((char *)prevname);
			public_.internaltag = nullptr;
		}
	};

	struct channelinternal
	{
		relayclient::channel public_;
		relayclientinternal &client;

		unsigned short id;
		const char * name;
		bool ischannelmaster;

		std::vector<peerinternal *> peers;

		channelinternal(relayclientinternal &_client) : client(_client)
		{
			public_.internaltag = this;
			public_.tag = 0;
			public_.isclosed = false;
			
			id = 0xffff;
			name = nullptr;
			ischannelmaster = false;
		}

		~channelinternal() noexcept(false)
		{
			if (!public_.isclosed)
				throw std::exception("Channel was not set to closed before it was deleted.");
			
			id = 0xffff;
			free((char *)name);
			name = nullptr;
			ischannelmaster = false;
			free(public_.tag);
		}

		/// <summary> searches for the first peer by id number. </summary>
		/// <param name="id"> id to look up. </param>
		/// <returns> null if it fails, else the matching peer. </returns>
		peerinternal * findpeerbyid(unsigned short id)
		{
			// findchannelbyid() is false, thus channel->findpeerbyid() is false too
			if (this == nullptr)
				return nullptr;
			auto i = std::find_if(peers.cbegin(), peers.cend(), 
				[&](const peerinternal * const & p) { return p->id == id; });
			return (i == peers.cend() ? nullptr : *i);
		}

		/// <summary> adds a new peer. </summary>
		/// <param name="peerid"> id number for the peer. </param>
		/// <param name="flags"> the flags of the peer connect/channel join message.
		/// 					 0x1 = master. other flags are not accepted. </param>
		/// <param name="name"> the name. cannot be null or blank. </param>
		/// <returns> null if it fails, else a peerinternal*. </returns>
		peerinternal * addnewpeer(int peerid, unsigned char flags, const char * name)
		{
			peerinternal * p = new peerinternal(*this, peerid, flags, name);
			peers.push_back(p);
			return p;
		}
	};

	void relayclientinternal::clear()
	{
		std::for_each(channels.begin(), channels.end(), [&](channelinternal *&c) { c->public_.isclosed = true; delete c; });
		channels.clear();
		clearchannellist();

		free((char *)name);
		name = nullptr;

		id = 0xffff;
		connected = false;

		free((char *)welcomemessage);
		welcomemessage = nullptr;
	}

	/// <summary> searches for the first channel by id number. </summary>
	/// <param name="id"> id to look up. </param>
	/// <returns> null if it fails, else the matching channel. </returns>
	channelinternal * relayclientinternal::findchannelbyid(unsigned short id)
	{
		auto i = std::find_if(channels.cbegin(), channels.cend(), 
			[&](const channelinternal * const &c) { return c->id == id; });
		return (i == channels.cend() ? nullptr : *i);
	}

	void handlerconnect(client socket)
	{
		relayclientinternal &internal = *(relayclientinternal *)socket->tag();

		/* opening 0 byte */
		socket->write("", 1);
		
		// internal.udp->host(socket->server_address(), nullptr, 0U);

		framebuilder &message = internal.message;

		message.addheader(0, 0); /* request */

		message.add<unsigned char>(0); /* connect */
		message.add("revision 3", -1);

		message.send(internal.socket);
	}

	void handlerdisconnect(client socket)
	{
		relayclientinternal &internal = *(relayclientinternal *)socket->tag();

		internal.timer->stop();

		internal.connected = false;

		if (internal.handler_disconnect)
			internal.handler_disconnect(internal.client);

		internal.clear();
	}

	void handlerreceive(client socket, const char * data, size_t size)
	{
		relayclientinternal &internal = *(relayclientinternal *)socket->tag();

		char * dataCpy = (char *)malloc(size);
		if (!dataCpy)
			throw std::exception("Out of memory.");
		
		memcpy(dataCpy, data, size);

		internal.reader.process(dataCpy, size);

		free(dataCpy);
	}

	void handlererror(client socket, error error)
	{
		relayclientinternal &internal = *(relayclientinternal *)socket->tag();

		error->add("socket error");

		if (internal.handler_error)
			internal.handler_error(internal.client, error);
	}

	void handlerclientudpreceive(udp udp, address address, char * data, size_t size)
	{
		relayclientinternal &internal = *(relayclientinternal *)udp->tag();

		if (!size)
			return;

		internal.messagehandler(*data, data + 1, size - 1, true);
	}

	void handlerclientudperror(udp udp, error error)
	{
		relayclientinternal &internal = *(relayclientinternal *)udp->tag();

		error->add("socket error");

		if (internal.handler_error)
			internal.handler_error(internal.client, error);
	}

	relayclient::relayclient(pump eventpump) : 
		socket(client_new(eventpump)), udp(udp_new(eventpump))
	{
		// lacewinginitialise no longer needed?
		
		socket->on_connect(lacewing::handlerconnect);
		socket->on_disconnect(lacewing::handlerdisconnect);
		socket->on_data(lacewing::handlerreceive);
		socket->on_error(lacewing::handlererror);

		udp->on_data(lacewing::handlerclientudpreceive);
		udp->on_error(lacewing::handlerclientudperror);

		socket->nagle(false);

		lacewing::relayclientinternal * r = new lacewing::relayclientinternal(*this, socket, udp, eventpump);
		tag = 0;

		internaltag = r;
		socket->tag(r);
		udp->tag(r);
	}

	relayclient::~relayclient()
	{
		socket->on_connect(nullptr);
		socket->on_disconnect(nullptr);
		socket->on_data(nullptr);
		socket->on_error(nullptr);

		udp->on_data(nullptr);
		udp->on_error(nullptr);
		
		lw_eventpump_post_eventloop_exit((lw_eventpump)socket->pump());
		
		lw_udp_delete((lw_udp)udp);
		lw_stream_delete((lw_client)socket);

		delete ((relayclientinternal *)internaltag);
		internaltag = nullptr;

		WSACleanup();
	}

	void relayclient::connect(const char * host, int port)
	{
		socket->connect(host, port);
	}

	void relayclient::connect(address address)
	{
		if (!address->port())
			address->port(6121);

		socket->connect(address);
	}

	void relayclient::disconnect()
	{
		socket->close();
		udp->unhost();
	}

	bool relayclient::connected()
	{
		return ((relayclientinternal *)internaltag)->connected;
	}

	bool relayclient::connecting()
	{
		return (!connected()) && (socket->connected() || socket->connecting());
	}

	const char * relayclient::name() const
	{
		return ((relayclientinternal *)internaltag)->name;
	}

	void relayclient::listchannels()
	{
		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		framebuilder   &message = internal.messageMF;

		message.addheader(0, 0);  /* request */
		message.add <unsigned char>(4);  /* channellist */

		message.send(internal.socket);
	}

	void relayclient::name(const char * name)
	{
		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		framebuilder   &message = internal.messageMF;

		if (!*name)
		{
			error error = error_new();
			error->add("can't set a blank name");

			if (internal.handler_error)
				internal.handler_error(*this, error);

			return;
		}

		message.addheader(0, 0);  /* request */
		message.add <unsigned char>(1);  /* setname */
		message.add(name, -1);

		message.send(internal.socket);
	}

	void relayclient::join(const char * channel, bool hidden, bool autoclose)
	{
		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		framebuilder   &message = internal.messageMF;

		if (!*name())
		{
			error error = lacewing::error_new();
			error->add("set a name before joining a channel");

			if (internal.handler_error)
				internal.handler_error(*this, error);
			lacewing::error_delete(error);

			return;
		}

		if (!*channel)
		{
			error error = lacewing::error_new();
			error->add("can't join a channel with a blank name");

			if (internal.handler_error)
				internal.handler_error(*this, error);

			lacewing::error_delete(error);
			return;
		}

		message.addheader(0, 0);  /* request */
		message.add <unsigned char>(2);  /* joinchannel */
		message.add <unsigned char>((hidden ? 1 : 0) | (autoclose ? 2 : 0));
		message.add(channel, -1);

		message.send(internal.socket);
	}

	void relayclient::sendserver(int subchannel, const char * data, int size, int variant) const
	{
		if (size == -1)
			size = strlen(data);

		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		framebuilder &message = internal.messageMF;

		message.addheader(1, variant); /* binaryservermessage */
		message.add <unsigned char>(subchannel);
		message.add(data, size);

		message.send(internal.socket);
	}

	void relayclient::blastserver(int subchannel, const char * data, int size, int variant) const
	{
		if (size == -1)
			size = strlen(data);

		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		framebuilder &message = internal.messageMF;

		message.addheader(1, variant, true, internal.id); /* binaryservermessage */
		message.add <unsigned char>(subchannel);
		message.add(data, size);

		message.send(internal.udp, internal.socket->server_address());
	}

	void relayclient::channel::send(int subchannel, const char * data, int size, int variant) const
	{
		if (size == -1)
			size = strlen(data);

		channelinternal &channel = *((channelinternal *)internaltag);
		relayclientinternal &internal = channel.client;
		framebuilder &message = internal.messageMF;

		message.addheader(2, variant); /* binarychannelmessage */
		message.add <unsigned char>(subchannel);
		message.add <unsigned short>(channel.id);
		message.add(data, size);

		message.send(internal.socket);
	}

	void relayclient::channel::blast(int subchannel, const char * data, int size, int variant) const
	{
		if (size == -1)
			size = strlen(data);

		channelinternal &channel = *((channelinternal *)internaltag);
		relayclientinternal &internal = channel.client;
		framebuilder &message = internal.messageMF;

		message.addheader(2, variant, true, internal.id); /* binarychannelmessage */
		message.add <unsigned char>(subchannel);
		message.add <unsigned short>(channel.id);
		message.add(data, size);

		message.send(internal.udp, internal.socket->server_address());
	}

	void relayclient::channel::peer::send(int subchannel, const char * data, int size, int variant) const
	{
		if (size == -1)
			size = strlen(data);

		peerinternal &peer = *((peerinternal *)internaltag);
		channelinternal &channel = peer.channel;
		relayclientinternal &internal = channel.client;
		framebuilder &message = internal.messageMF;

		message.addheader(3, variant); /* binarypeermessage */
		message.add <unsigned char>(subchannel);
		message.add <unsigned short>(channel.id);
		message.add <unsigned short>(peer.id);
		message.add(data, size);

		message.send(internal.socket);
	}

	void relayclient::channel::peer::blast(int subchannel, const char * data, int size, int variant) const
	{
		if (size == -1)
			size = strlen(data);

		peerinternal &peer = *((peerinternal *)internaltag);
		channelinternal &channel = peer.channel;
		relayclientinternal &internal = channel.client;
		framebuilder &message = internal.messageMF;

		message.addheader(3, variant, true, internal.id); /* binarypeermessage */
		message.add <unsigned char>(subchannel);
		message.add <unsigned short>(channel.id);
		message.add <unsigned short>(peer.id);
		message.add(data, size);

		message.send(internal.udp, internal.socket->server_address());
	}

	void relayclient::channel::leave() const
	{
		channelinternal &channel = *((channelinternal *)internaltag);
		relayclientinternal  &internal = channel.client;
		framebuilder    &message = internal.messageMF;

		message.addheader(0, 0); /* request */

		message.add <unsigned char>(3);  /* leavechannel */
		message.add <unsigned short>(channel.id);

		message.send(internal.socket);
	}
	relayclient::channel::~channel() noexcept(false)
	{
		if (internaltag != nullptr)
		{
			delete internaltag;
			internaltag = nullptr;
		}

		if (tag)
			throw std::exception("Deleted a channel without a null tag: possible memory leak.");
	}

	relayclient::channel::peer::~peer() noexcept(false)
	{
		if (internaltag)
			throw std::exception("Internal tag was not null as expected.");

		if (tag)
			throw std::exception("Deleted a peer without a null tag: possible memory leak.");
	}

	const char * relayclient::channel::name() const
	{
		return ((channelinternal *)internaltag)->name;
	}

	int relayclient::channel::peercount() const
	{
		return ((channelinternal *)internaltag)->peers.size();
	}

	const char * relayclient::channel::peer::name() const
	{
		return ((peerinternal *)internaltag)->name;
	}

	int relayclient::channelcount() const
	{
		return ((relayclientinternal *)internaltag)->channels.size();
	}

	int relayclient::id() const
	{
		return ((relayclientinternal *)internaltag)->id;
	}

	address relayclient::serveraddress()
	{
		return socket->server_address();
	}

	const char * relayclient::welcomemessage() const
	{
		return ((relayclientinternal *)internaltag)->welcomemessage;
	}

	void relayclientinternal::messagehandler(unsigned char type, const char * message, size_t size, bool blasted)
	{
		unsigned char messagetypeid = (type >> 4);
		unsigned char variant = (type << 4);

		variant >>= 4;

		messagereader reader(message, size);

		switch (messagetypeid)
		{
		case 0: /* response */
		{
			unsigned char  responsetype = reader.get <unsigned char>();
			bool           succeeded = reader.get <unsigned char>() != 0;

			if (reader.failed)
				break;

			switch (responsetype)
			{
			case 0: /* connect */
			{
				if (succeeded)
				{
					id = reader.get <unsigned short>();
					const char * welcomemessageDup = reader.getremaining();

					if (reader.failed)
						break;
					welcomemessage = _strdup(welcomemessageDup);

					socket->server_address()->resolve();
					udp->host(socket->server_address());
					clienttick();

					timer->start(500); // see clienttick

					/* the connect handler will be called on udp acknowledged */
				}
				else
				{
					if (handler_connectiondenied)
						handler_connectiondenied(client, reader.getremaining());
				}

				break;
			}

			case 1: /* setname */
			{
				unsigned char namelength = reader.get <unsigned char>();
				char *        name = reader.get(namelength);

				if (reader.failed)
					break;

				if (succeeded)
				{
					if (!this->name)
					{
						this->name = _strdup(name);

						if (handler_name_set)
							handler_name_set(client);
					}
					else
					{
						const char * oldname = this->name;
						this->name = _strdup(name);

						if (handler_name_changed)
							handler_name_changed(client, oldname);
						free((char *)oldname);
					}
				}
				else
				{
					const char * denyreason = reader.getremaining();

					if (handler_name_denied)
						handler_name_denied(client, name, denyreason);
				}

				break;
			}

			case 2: /* joinchannel */
			{
				unsigned char flags = succeeded ? reader.get <unsigned char>() : 0;
				unsigned char namelength = reader.get <unsigned char>();
				char *        name = reader.get(namelength);

				if (reader.failed)
					break;

				if (succeeded)
				{
					int channelid = reader.get <unsigned short>();

					if (reader.failed)
						break;

					channelinternal * channel = new channelinternal(*this);

					channel->id = channelid;
					channel->name = _strdup(name);
					channel->ischannelmaster = (flags & 1) != 0;

					for (;;)
					{
						int peerid = reader.get <unsigned short>();
						int flags2 = reader.get <unsigned char>();
						int namelength2 = reader.get <unsigned char>();

						if (reader.failed)
							break;

						char * name2 = reader.get(namelength2);

						if (reader.failed)
							break;

						channel->addnewpeer(peerid, flags2, name2);
					}

					channels.push_back(channel);

					if (handler_channel_join)
						handler_channel_join(client, channel->public_);
				}
				else
				{
					const char * denyreason = reader.getremaining();

					if (handler_channel_joindenied)
						handler_channel_joindenied(client, name, denyreason);
				}

				break;
			}

			case 3: /* leavechannel */
			{
				unsigned short id = reader.get<unsigned short>();
				if (reader.failed)
					break;
				channelinternal * channel = findchannelbyid(id);

				if (reader.failed)
					break;

				if (succeeded)
				{
					auto i = std::find_if(channels.cbegin(), channels.cend(),
						[&](const channelinternal * const &c) { return c == channel; });
					if (i == channels.cend())
						break;
					channels.erase(i);

					if (handler_channel_leave)
						handler_channel_leave(client, channel->public_);
				}
				else
				{
					const char * denyreason = reader.getremaining();

					if (handler_channel_leavedenied)
						handler_channel_leavedenied(client, channel->public_, denyreason);
				}

				break;
			}

			case 4: /* channellist */
			{
				clearchannellist();

				for (;;)
				{
					int peercount = reader.get <unsigned short>();
					int namelength = reader.get <unsigned char>();

					if (reader.failed)
						break;

					char * name = reader.get(namelength);

					if (reader.failed)
						break;

					relayclient::channellisting * listing = new relayclient::channellisting;

					listing->name = _strdup(name);
					listing->peercount = peercount;
					channellist.push_back(listing);
				}

				if (handler_channellistreceived)
					handler_channellistreceived(client);

				break;
			}

			default:
			{
				lacewing::error error = error_new();
				error->add("Unrecognised response message received. Response type ID was %i, but expected response type IDs 0-4. Discarding message.");
				this->handler_error(client, error);
				error_delete(error);
				return;
			}
			}

			break;
		}

		case 1: /* binaryservermessage */
		{
			int subchannel = reader.get <unsigned char>();

			const char * message2;
			unsigned int size2;

			reader.getremaining(message2, size2);

			if (reader.failed)
				break;

			if (handler_message_server)
				handler_message_server(client, blasted, subchannel, message2, size2, variant);

			break;
		}

		case 2: /* binarychannelmessage */
		{
			int subchannel = reader.get <unsigned char>();
			unsigned short channel = reader.get<unsigned short>();
			unsigned short peer = reader.get<unsigned short>();
			if (reader.failed)
				break;
			channelinternal * channel2 = findchannelbyid(channel);
			peerinternal    * peer2 = channel2->findpeerbyid(peer);

			const char * message2;
			unsigned int size2;

			reader.getremaining(message2, size2);

			if (reader.failed)
				break;

			if (handler_message_channel)
				handler_message_channel(client, channel2->public_, peer2->public_, blasted,
					subchannel, message2, size2, variant);

			break;
		}

		case 3: /* binarypeermessage */
		{
			int subchannel = reader.get <unsigned char>();
			unsigned short channel = reader.get<unsigned short>();
			unsigned short peer = reader.get<unsigned short>();
			if (reader.failed)
				break;
			channelinternal * channel2 = findchannelbyid(channel);
			peerinternal    * peer2 = channel2->findpeerbyid(peer);

			const char * message2;
			unsigned int size2;

			reader.getremaining(message2, size2);

			if (reader.failed)
				break;

			if (handler_message_peer)
				handler_message_peer(client, channel2->public_, peer2->public_, blasted,
					subchannel, message2, size2, variant);

			break;
		}

		case 4: /* binaryserverchannelmessage */
		{
			int subchannel = reader.get <unsigned char>();
			unsigned short channel = reader.get<unsigned short>();
			if (reader.failed)
				break;
			channelinternal * channel2 = findchannelbyid(channel);

			const char * message2;
			unsigned int size2;

			reader.getremaining(message2, size2);

			if (reader.failed)
				break;

			if (handler_message_serverchannel)
				handler_message_serverchannel(client, channel2->public_, blasted,
					subchannel, message2, size2, variant);

			break;
		}

		case 5: /* objectservermessage */
		case 6: /* objectchannelmessage */
		case 7: /* objectpeermessage */
		case 8: /* objectserverchannelmessage */
		{
			// todo: replace every lacewingassert() with a real error.
			lacewing::error error = error_new();
			error->add("'Object' message type received, but Bluewing Client implementation does not support it.");
			this->handler_error(client, error);
			error_delete(error);
			return;
		}

		case 9: /* peer */
		{

			unsigned short channel = reader.get<unsigned short>();
			if (reader.failed)
				break;
			channelinternal * channel2 = findchannelbyid(channel);

			if (reader.failed)
				break;

			int peerid = reader.get <unsigned short>();


			auto i = std::find_if(channel2->peers.begin(), channel2->peers.end(),
				[&](peerinternal *&p) { return p->id == peerid; });
			peerinternal * peer = i == channel2->peers.cend() ? nullptr : *i;

			unsigned char flags = reader.get <unsigned char>();
			const char *  name = reader.getremaining();

			if (reader.failed)
			{
				/* no flags/name - the peer must have left the channel */

				if (!peer)
					return;

				channel2->peers.erase(i);
				peer->public_.isclosed = true;

				if (handler_peer_disconnect)
					handler_peer_disconnect(client, channel2->public_, peer->public_);

				return;
			}

			if (!peer)
			{
				/* new peer */

				peer = channel2->addnewpeer(peerid, flags, name);

				if (handler_peer_connect)
					handler_peer_connect(client, channel2->public_, peer->public_);

				break;
			}

			/* existing peer */

			if (strcmp(name, peer->name))
			{
				/* peer is changing their name */

				free((char *)peer->prevname);
				peer->prevname = peer->name;
				peer->name = _strdup(name);

				if (handler_peer_changename)
					handler_peer_changename(client, channel2->public_, peer->public_, peer->prevname);
			}

			peer->ischannelmaster = (flags & 1) != 0;
			if (flags & ~0x1)
			{
				lacewing::error error = error_new();
				error->add("Malformed message received (server error?). Unrecognised peer flags in peer message, expected 0 or 1, got %i. Ignoring unrecognised flags.", flags);
				this->handler_error(client, error);
				error_delete(error);
			}

			break;
		}

		case 10: /* udpwelcome */

			if (!blasted)
				break;

			timer->stop();
			connected = true;

			if (handler_connect)
				handler_connect(client);

			break;

		case 11: /* ping */

			this->message.addheader(9, 0); /* pong */
			this->message.send(socket);

			break;

		default:
		{
			lacewing::error error = error_new();
			error->add("Malformed message received (server error?). Unrecognised message type ID %i, expected type IDs 0-11. Discarding message.");
			this->handler_error(client, error);
			error_delete(error);
			return;
		}
		};

		if (reader.failed)
		{
			lacewing::error error = error_new();
			error->add("Malformed message received (server error?). Message type recognised but expected data was not found. Discarding message.");
			this->handler_error(client, error);
			error_delete(error);
		}
	}

	void relayclientinternal::messagehandler(void * tag, unsigned char type, const char * message, size_t size)
	{
		((relayclientinternal *)tag)->messagehandler(type, message, size, false);
	}

	void relayclientinternal::clienttick(lacewing::timer timer)
	{
		((relayclientinternal *)timer->tag())->clienttick();
	}

	void relayclientinternal::clienttick()
	{
		// clienttick just sends UDPHello every 0.5s, and is managed by the relayclientinternal::timer var.
		// It starts from the time the Connect Request Success message is sent.
		if (!udp->hosting())
			throw std::exception("clienttick() called, but not hosting UDP."); 
		
		message.addheader(7, 0, true, id); /* udphello */
		message.send(udp, socket->server_address());
	}

	bool relayclient::channel::peer::ischannelmaster() const
	{
		peerinternal &internal = *(peerinternal *)internaltag;

		return internal.ischannelmaster;
	}

	unsigned short relayclient::channel::peer::id() const
	{
		peerinternal &internal = *(peerinternal *)internaltag;

		return internal.id;
	}

	relayclient::channel::peer * relayclient::channel::peer::next() const
	{
		peerinternal &internal = *(peerinternal *)internaltag;
		auto end = internal.channel.peers.end();
		auto i = std::find_if(internal.channel.peers.begin(), internal.channel.peers.end(),
			[&](peerinternal * &p) { return p->id == internal.id; });
		return (i == end || ++i == end) ? nullptr : &(*i)->public_;
	}

	bool relayclient::channel::ischannelmaster() const
	{
		channelinternal &internal = *(channelinternal *)internaltag;

		return internal.ischannelmaster;
	}

	relayclient::channel * relayclient::channel::next() const
	{
		channelinternal &internal = *(channelinternal *)internaltag;
		auto end = internal.client.channels.end();
		
		auto i = std::find_if(internal.client.channels.begin(), end,
			[&](channelinternal * &c) { return c->id == internal.id; });
		return (i == end || ++i == end) ? nullptr : &(*i)->public_;
	}

	relayclient::channel * relayclient::firstchannel() const
	{
		std::vector<channelinternal *> &c = ((lacewing::relayclientinternal *)internaltag)->channels;
		return (c.begin() == c.end()) ? nullptr : &(*c.begin())->public_;
	}
	
	relayclient::channel::peer * relayclient::channel::firstpeer() const
	{
		std::vector<peerinternal *> &p = ((lacewing::channelinternal *)internaltag)->peers;
		return (p.begin() == p.end()) ? nullptr : &(*p.begin())->public_;
	}

	int relayclient::channellistingcount() const
	{
		return ((relayclientinternal *)internaltag)->channellist.size();
	}

	const relayclient::channellisting * relayclient::firstchannellisting() const
	{
		std::vector<channellisting *> &c = ((relayclientinternal *)internaltag)->channellist;
		return (c.cbegin() == c.cend()) ? nullptr : *c.cbegin();
	}
	const relayclient::channellisting * relayclient::channellisting::next() const
	{
		relayclientinternal &internal = *(relayclientinternal *)internaltag;
		auto end = internal.channellist.end();
		auto i = std::find_if(internal.channellist.begin(), end, 
			[&](channellisting * &c) { return c->name == name; });
		return (i == end || ++i == end) ? nullptr : *i;
	}

#define autohandlerfunctions(pub, intern, handlername)              \
    void pub::on##handlername(pub::handler_##handlername handler)   \
	    {   ((intern *) internaltag)->handler_##handlername = handler;      \
	    }                                                                   \


	autohandlerfunctions(relayclient, relayclientinternal, connect)
	autohandlerfunctions(relayclient, relayclientinternal, connectiondenied)
	autohandlerfunctions(relayclient, relayclientinternal, disconnect)
	autohandlerfunctions(relayclient, relayclientinternal, message_server)
	autohandlerfunctions(relayclient, relayclientinternal, message_channel)
	autohandlerfunctions(relayclient, relayclientinternal, message_peer)
	autohandlerfunctions(relayclient, relayclientinternal, message_serverchannel)
	autohandlerfunctions(relayclient, relayclientinternal, error)
	autohandlerfunctions(relayclient, relayclientinternal, channel_join)
	autohandlerfunctions(relayclient, relayclientinternal, channel_joindenied)
	autohandlerfunctions(relayclient, relayclientinternal, channel_leave)
	autohandlerfunctions(relayclient, relayclientinternal, channel_leavedenied)
	autohandlerfunctions(relayclient, relayclientinternal, name_set)
	autohandlerfunctions(relayclient, relayclientinternal, name_changed)
	autohandlerfunctions(relayclient, relayclientinternal, name_denied)
	autohandlerfunctions(relayclient, relayclientinternal, peer_connect)
	autohandlerfunctions(relayclient, relayclientinternal, peer_disconnect)
	autohandlerfunctions(relayclient, relayclientinternal, peer_changename)
	autohandlerfunctions(relayclient, relayclientinternal, channellistreceived)

}