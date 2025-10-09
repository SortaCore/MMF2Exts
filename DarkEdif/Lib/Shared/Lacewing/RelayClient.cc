/* vim: set noet ts=4 sw=4 sts=4 ft=cpp:
 *
 * liblacewing and Lacewing Relay/Blue source code are available under MIT license.
 * Copyright (C) 2012-2025 Darkwire Software.
 * All rights reserved.
 *
 * https://opensource.org/license/mit
*/

#include "Lacewing.h"
#include "FrameBuilder.h"
#include "FrameReader.h"
#include "MessageReader.h"
#include "src/common.h" // lwp_trace
#include <vector>
#include <algorithm>

namespace lacewing
{
	struct relayclientinternal
	{
		lacewing::relayclient &		client;
		lacewing::client			socket;
		lacewing::udp 				udp;
		lacewing::timer				udphellotimer;

		relayclient::handler_connect				handler_connect;
		relayclient::handler_connectiondenied		handler_connectiondenied;
		relayclient::handler_disconnect				handler_disconnect;
		relayclient::handler_message_server			handler_message_server;
		relayclient::handler_message_channel		handler_message_channel;
		relayclient::handler_message_peer			handler_message_peer;
		relayclient::handler_message_serverchannel	handler_message_serverchannel;
		relayclient::handler_error					handler_error;
		relayclient::handler_channel_join			handler_channel_join;
		relayclient::handler_channel_joindenied		handler_channel_joindenied;
		relayclient::handler_channel_leave			handler_channel_leave;
		relayclient::handler_channel_leavedenied	handler_channel_leavedenied;
		relayclient::handler_name_set				handler_name_set;
		relayclient::handler_name_changed			handler_name_changed;
		relayclient::handler_name_denied			handler_name_denied;
		relayclient::handler_peer_connect			handler_peer_connect;
		relayclient::handler_peer_disconnect		handler_peer_disconnect;
		relayclient::handler_peer_changename		handler_peer_changename;
		relayclient::handler_channellistreceived	handler_channellistreceived;

		relayclientinternal(relayclient &_client, pump _eventpump);
		framereader reader;

		static bool messagehandler(void * tag, lw_ui8 type, const char * message, size_t size);
		bool		messagehandler(lw_ui8 type, const char * message, size_t size, bool blasted);

		static void udphellotick(lacewing::timer timer);
		void		udphellotick();

		// Searches for the first channel by id number, null if no match
		std::shared_ptr<relayclient::channel> findchannelbyid(lw_ui16 id);

		// message: used by lacewing internal (e.g. automatic ping response)
		// messageMF: used by program
		framebuilder message, messageMF;

		std::vector<std::shared_ptr<relayclient::channellisting>> channellist;

		// Empties the channel list.
		void clearchannellist()
		{
			lacewing::writelock wl = client.lock.createWriteLock();
			channellist.clear();
		}

		void clear();

		std::string name;
		lw_ui16 id = 0xFFFF, local_port = 0;
		std::string welcomemessage;
		// Indicates connected on a Lacewing level; full Lacewing TCP/UDP handshake finished
		bool connected = false;

		std::vector<std::shared_ptr<relayclient::channel>> channels;

		void initsocket(lacewing::pump pump);

		void disconnect_mark_all_as_readonly();

		~relayclientinternal() noexcept(false)
		{
			lacewing::writelock wl = client.lock.createWriteLock();
			clear();

			udphellotimer->on_tick(nullptr);
			lacewing::timer_delete(udphellotimer);
			udphellotimer = nullptr;

			// Lacewing will self-delete on disconnect... we replace with a new, blank client
			if (socket)
			{
				socket->on_connect(nullptr);
				socket->on_disconnect(nullptr);
				socket->on_data(nullptr);
				socket->on_error(nullptr);

				lacewing::stream_delete(socket);
				socket = nullptr;
			}

			// UDP has no "close" as it's a connectionless protocol, so Lacewing doesn't clean it up automatically
			udp->on_data(nullptr);
			udp->on_error(nullptr);
			lacewing::udp_delete(udp);
			udp = nullptr;
		}
	};

	void relayclientinternal::clear()
	{
		lacewing::writelock cliWriteLock = client.lock.createWriteLock();
		channels.clear();
		clearchannellist();

		id = 0xffff;
		connected = false;
		name.clear();
	}
	void relayclientinternal::disconnect_mark_all_as_readonly()
	{
		// Private members
		auto cliWriteLock = client.lock.createWriteLock();
		for (auto& ch : channels)
		{
			ch->_readonly = true;
			for (auto& p : ch->peers)
				p->_readonly = true;
		}
	}

	std::shared_ptr<relayclient::channel> relayclientinternal::findchannelbyid(lw_ui16 id)
	{
		lacewing::readlock rl = this->client.lock.createReadLock();
		auto i = std::find_if(channels.cbegin(), channels.cend(),
			[id](const std::shared_ptr<const relayclient::channel> &c) { return c->id() == id; });
		return i == channels.cend() ? nullptr : *i;
	}

	void handlerconnect(client socket)
	{
		relayclientinternal &internal = *(relayclientinternal *)socket->tag();

		/* opening 0 byte */
		socket->write("", 1);

		// internal.udp->host(socket->server_address(), nullptr, 0U);

		framebuilder &message = internal.message;

		message.addheader(0, 0);	 /* request */
		message.add<lw_ui8>(0);		 /* connect */
		message.add("revision 3"sv); /* version, not null terminated */

		message.send(internal.socket);
	}

	void handlerdisconnect(client socket)
	{
		relayclientinternal &internal = *(relayclientinternal *)socket->tag();

		auto cliWriteLock = internal.client.lock.createWriteLock();
		internal.udphellotimer->stop();

		internal.connected = false;

		internal.disconnect_mark_all_as_readonly();

		cliWriteLock.lw_unlock();
		if (internal.handler_disconnect)// && handlerNeedsRunning)
			internal.handler_disconnect(internal.client);

		cliWriteLock.lw_relock();
		internal.clear();

		// Lacewing self-deletes streams on socket close - while client variable is valid here,
		// it won't be after calling function continues. We quietly replace it with something usable.
		internal.socket = nullptr;
		internal.initsocket(socket->pump());
	}

	void handlerreceive(client socket, const char * data, size_t size)
	{
		relayclientinternal &internal = *(relayclientinternal *)socket->tag();

		// To prevent stack overflow from a big TCP packet with multiple Lacewing messages, I've reworked
		// this function to prevent it recursively calling process() after shaving a message off.
		const char * dataPtr = data;
		size_t sizePtr = size;

		constexpr size_t maxMessagesInOneProcess = 300;
		for (size_t i = 0; i < maxMessagesInOneProcess; ++i)
		{
			// Ran out of messages, or error occurred and rest should be ignored; exit quietly
			if (!internal.reader.process(&dataPtr, &sizePtr))
				return;
		}

		if (internal.handler_error)
		{
			lacewing::error error = lacewing::error_new();
			error->add("Overload of message stack; got more than %zu messages in one packet (sized %zu) from server.",
				maxMessagesInOneProcess, size);

			internal.handler_error(internal.client, error);
			lacewing::error_delete(error);
		}
	}

	void handlererror(client socket, error error)
	{
		relayclientinternal &internal = *(relayclientinternal *)socket->tag();

		error->add("socket error");
		lw_trace("Error event: \"%s\".", error->tostring());

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

	relayclient::relayclient(pump eventpump)
	{
		lacewing::writelock wl = this->lock.createWriteLock();
		// relayclient is just an idiot-proof layer hiding relayclientinternal
		lacewing::relayclientinternal * r = new lacewing::relayclientinternal(*this, eventpump);
		tag = nullptr;
		internaltag = r;
	}

	relayclient::~relayclient()
	{
		lacewing::writelock wl = this->lock.createWriteLock();
		delete ((relayclientinternal *)internaltag);
		internaltag = nullptr;
		wl.lw_unlock();
	}

	void relayclient::connect(const char * host, lw_ui16 remote_port)
	{
		lacewing::writelock wl = this->lock.createWriteLock();
		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		internal.socket->connect(host, remote_port);
		if (internal.local_port)
		{
			address addr = internal.socket->server_address();
			const lw_bool makeAddr = addr == nullptr;
			if (makeAddr)
				addr = lacewing::address_new(host, remote_port);

			// Host early for UDP hole punch message - which must be sent closely with TCP connect
			internal.udp->host(addr, internal.local_port);

			// UDPHello with an ignored ID 0xFFFF, which will be ignored by server,
			// but its reception at all will cause hole punch success
			internal.udp->send(addr, "\xa0\xFF\xFF", 3);
			internal.local_port = 0;
			if (makeAddr)
				lacewing::address_delete(addr);
		}
	}

	void relayclient::connect(address address)
	{
		lacewing::writelock wl = this->lock.createWriteLock();
		if (!address->port())
			address->port(6121);

		// Socket will fuss if we're connecting/connected already, so don't bother checking.
		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		internal.socket->connect(address);
		// Host early for possible UDP hole punch
		internal.udp->host(internal.socket->server_address(), internal.local_port);
		// UDPHello, although we probably won't receive response in time for it to be processed
		internal.udp->send(internal.socket->server_address(), "\xa0\xFF\xFF", 3);
		internal.local_port = 0;
	}

	void relayclient::setlocalport(lw_ui16 port)
	{
		relayclientinternal& internal = *((relayclientinternal*)internaltag);
		internal.local_port = port;
		internal.socket->setlocalport(port);
	}

	void relayclient::disconnect()
	{ 
		relayclientinternal &internal = *((relayclientinternal *)internaltag);

		// If you run relayclient::disconnect() while a connection/connect attempt isn't pending,
		// the disconnect is effectively "stored" inside the socket, causing the next
		// successful connection to be met with an instant disconnect.
		// So, make it a no-op when not trying to use a connection yet.
		if (!internal.socket->connecting() && !internal.socket->connected())
			return;

		internal.connected = false;

		if (internal.udphellotimer)
			internal.udphellotimer->stop();

		lacewing::writelock wl = this->lock.createWriteLock();

		// In future versions we could use a timer to immediate close after a while,
		// in case server is lagging with the polite close response, but we'd have
		// to watch it on app close.
		internal.socket->close(lw_false);
		// lacewing::stream_delete(internal.socket);
		// internal.socket = nullptr;
		internal.udp->unhost();
		// lacewing::udp_delete(internal.udp);
		// internal.udp = nullptr;
	}

	bool relayclient::connected()
	{
		lacewing::readlock rl = this->lock.createReadLock();
		return ((relayclientinternal *)internaltag)->connected;
	}

	bool relayclient::connecting()
	{
		lacewing::readlock rl = this->lock.createReadLock();
		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		// RelayClient takes a bit longer than raw socket to make a connection, so we check both.
		return (!connected()) && (internal.socket->connected() || internal.socket->connecting());
	}

	std::string relayclient::name() const
	{
		lacewing::readlock rl = this->lock.createReadLock();
		return ((relayclientinternal *)internaltag)->name;
	}

	void relayclient::listchannels()
	{
		lacewing::writelock wl = this->lock.createWriteLock();
		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		framebuilder &message = internal.messageMF;

		message.addheader(0, 0);  /* request */
		message.add <lw_ui8>(4);  /* channellist */

		message.send(internal.socket);
	}

	void relayclient::name(std::string_view name)
	{
		lacewing::writelock wl = lock.createWriteLock();
		relayclientinternal & internal = *((relayclientinternal *)internaltag);
		framebuilder & message = internal.messageMF;

		if (name.empty())
		{
			error error = error_new();
			error->add("can't set a blank name");

			if (internal.handler_error)
				internal.handler_error(*this, error);

			return;
		}

		message.addheader(0, 0);  /* request */
		message.add <lw_ui8>(1);  /* setname */
		message.add (name);

		message.send(internal.socket);
	}

	void relayclient::join(std::string_view channelName, bool hidden, bool autoclose)
	{
		relayclientinternal &internal = *((relayclientinternal *)internaltag);

		if (name().empty())
		{
			error error = lacewing::error_new();
			error->add("set a name before joining a channel");

			if (internal.handler_error)
				internal.handler_error(*this, error);
			lacewing::error_delete(error);

			return;
		}

		if (channelName.empty())
		{
			error error = lacewing::error_new();
			error->add("can't join a channel with a blank name");

			if (internal.handler_error)
				internal.handler_error(*this, error);

			lacewing::error_delete(error);
			return;
		}

		lacewing::writelock wl = lock.createWriteLock();

		framebuilder & message = internal.messageMF;
		message.addheader (0, 0);  /* request */
		message.add <lw_ui8>(2);  /* joinchannel */
		message.add <lw_ui8>((hidden ? 1 : 0) | (autoclose ? 2 : 0));
		message.add (channelName);

		message.send(internal.socket);
	}

	void relayclient::sendserver(lw_ui8 subchannel, std::string_view data, lw_ui8 variant) const
	{
		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		framebuilder &message = internal.messageMF;

		lacewing::writelock wl = lock.createWriteLock();

		message.addheader (1, variant); /* binaryservermessage */
		message.add (subchannel);
		message.add (data);

		message.send(internal.socket);
	}

	void relayclient::blastserver(lw_ui8 subchannel, std::string_view data, lw_ui8 variant) const
	{
		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		framebuilder &message = internal.messageMF;

		if (data.size() > relay_max_udp_payload)
		{
			lwp_trace("UDP message too large, discarded");
			return;
		}

		lacewing::writelock wl = lock.createWriteLock();

		message.addheader (1, variant, true, internal.id); /* binaryservermessage */
		message.add (subchannel);
		message.add (data);

		message.send(internal.udp, internal.socket->server_address());
	}

	const std::vector<std::shared_ptr<relayclient::channel>> & relayclient::getchannels() const
	{
		lock.checkHoldsRead();
		return ((relayclientinternal *)internaltag)->channels;
	}

	void relayclient::channel::send(lw_ui8 subchannel, std::string_view data, lw_ui8 variant) const
	{
		if (peers.empty() || _readonly)
			return;

		relayclientinternal &clientinternal = client;
		framebuilder &message = clientinternal.messageMF;

		lacewing::writelock wl = lock.createWriteLock();

		message.addheader (2, variant); /* binarychannelmessage */
		message.add <lw_ui8>(subchannel);
		message.add <lw_ui16>(this->_id);
		message.add (data);

		message.send(clientinternal.socket);
	}

	void relayclient::channel::blast(lw_ui8 subchannel, std::string_view data, lw_ui8 variant) const
	{
		if (peers.empty() || _readonly)
			return;

		if (data.size() > relay_max_udp_payload)
		{
			lwp_trace("UDP message too large, discarded");
			return;
		}

		relayclientinternal &clientinternal = client;
		framebuilder &message = clientinternal.messageMF;
		lacewing::writelock wl = lock.createWriteLock();

		message.addheader(2, variant, true, clientinternal.id); /* binarychannelmessage */
		message.add <lw_ui8>(subchannel);
		message.add <lw_ui16>(this->_id);
		message.add (data);

		message.send(clientinternal.udp, clientinternal.socket->server_address());
	}

	void relayclient::channel::peer::send(lw_ui8 subchannel, std::string_view data, lw_ui8 variant) const
	{
		if (_readonly)
			return;

		relayclientinternal &clientinternal = channel.client;
		framebuilder &message = clientinternal.messageMF;

		lacewing::writelock wl = lock.createWriteLock();
		message.addheader (3, variant); /* binarypeermessage */
		message.add <lw_ui8>(subchannel);
		message.add <lw_ui16>(channel._id);
		message.add <lw_ui16>(_id);
		message.add (data);

		message.send(clientinternal.socket);
	}

	void relayclient::channel::peer::blast(lw_ui8 subchannel, std::string_view data, lw_ui8 variant) const
	{
		if (_readonly)
			return;

		if (data.size() > relay_max_udp_payload)
		{
			lwp_trace("UDP message too large, discarded");
			return;
		}

		relayclientinternal &internal = channel.client;
		framebuilder &message = internal.messageMF;

		lacewing::writelock wl = lock.createWriteLock();

		message.addheader(3, variant, true, internal.id); /* binarypeermessage */
		message.add <lw_ui8>(subchannel);
		message.add <lw_ui16>(channel._id);
		message.add <lw_ui16>(_id);
		message.add (data);

		message.send(internal.udp, internal.socket->server_address());
	}

	void relayclient::channel::leave() const
	{
		// Leaving channel aborted: already in readonly mode, which means we've already left
		if (_readonly)
			return;

		relayclientinternal &clientinternal = client;
		framebuilder &message = clientinternal.messageMF;

		lacewing::writelock wl = lock.createWriteLock();

		message.addheader(0, 0); /* request */
		message.add <lw_ui8>(3);  /* leavechannel */
		message.add <lw_ui16>(_id);

		message.send(clientinternal.socket);
	}

	std::string relayclient::channel::name() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return _name;
	}
	std::string relayclient::channel::namesimplified() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return _namesimplified;
	}

	int relayclient::channel::peercount() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return (int)peers.size();
	}

	lw_ui16 relayclient::channel::id() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return _id;
	}

	std::string relayclient::channel::peer::name() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return _name;
	}

	std::string relayclient::channel::peer::namesimplified() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return _namesimplified;
	}

	std::string relayclient::channel::peer::prevname() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return _prevname;
	}

	bool relayclient::channel::peer::readonly() const
	{
		// Atomic, no point changing it
		return _readonly;
	}

	size_t relayclient::channelcount() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return ((relayclientinternal *)internaltag)->channels.size();
	}

	lw_ui16 relayclient::id() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return ((relayclientinternal *)internaltag)->id;
	}

	address relayclient::serveraddress()
	{
		lacewing::readlock rl = lock.createReadLock();
		relayclientinternal &internal = *((relayclientinternal *)internaltag);
		return internal.socket->server_address();
	}

	std::string relayclient::welcomemessage() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return ((relayclientinternal *)internaltag)->welcomemessage;
	}

	bool relayclientinternal::messagehandler(lw_ui8 type, const char * message, size_t size, bool blasted)
	{
		lw_ui8 messagetypeid = (type >> 4);
		lw_ui8 variant = (type << 4);

		variant >>= 4;

		messagereader reader(message, size);

		switch (messagetypeid)
		{
		case 0: /* response */
		{
			// Client received the initial null byte from a client side, probably from
			// a bad hole punch doing client -> client instead of server -> client
			if (size == 0 && !connected)
			{
				lacewing::error errorholepunch = error_new();
				errorholepunch->add("Got the start of a connection from a client side (bad hole punch?)");
				this->handler_error(client, errorholepunch);
				error_delete(errorholepunch);
				return false;
			}

			lw_ui8  responsetype = reader.get <lw_ui8>();
			bool	succeeded	= reader.get <lw_ui8>() != 0;

			if (reader.failed)
				break;

			switch (responsetype)
			{
			case 0: /* connect */
			{
				if (succeeded)
				{
					id = reader.get <lw_ui16>();

					// Don't expect welcome message to be null terminated
					const std::string_view welcomemessage = reader.getremaining();

					if (reader.failed)
						break;

					this->welcomemessage = welcomemessage;
					lacewing::error error = nullptr;
					// If midway during connection when Disconnect is called, returned address can be null.
					lacewing::address srvAddress = socket->server_address();
					if (!srvAddress)
					{
						error = error_new();
						error->add("Server address not valid during connect response");
						this->handler_error(client, error);
						error_delete(error);
						break;
					}
					error = srvAddress->resolve();
					if (error)
					{
						error->add("Server address not resolved during connect response");
						this->handler_error(client, error);
						error_delete(error);
						break;
					}

					if (!udp->hosting())
						udp->host(srvAddress);
					udphellotick();

					udphellotimer->start(500); // see udphellotick

					/* the connect handler will be called on udp acknowledged */
				}
				else
				{
					if (handler_connectiondenied)
					{
						// Not null terminated, can't use getremaining()
						const std::string_view denyReason = reader.get(reader.bytesleft());

						if (reader.failed)
							break;

						handler_connectiondenied(client, denyReason);
					}
				}

				break;
			}

			case 1: /* setname */
			{
				const lw_ui8 namelength = reader.get <lw_ui8>();
				const std::string_view name = reader.get(namelength);

				if (reader.failed)
					break;

				if (succeeded)
				{
					if (this->name.empty())
					{
						this->name = name;

						if (handler_name_set)
							handler_name_set(client);
					}
					else
					{
						const std::string oldname = this->name;
						this->name = name;

						if (handler_name_changed)
							handler_name_changed(client, oldname);
					}
				}
				else
				{
					if (handler_name_denied)
					{
						// Not null terminated, can't use getremaining()
						const std::string_view denyReason = reader.get(reader.bytesleft());

						if (reader.failed)
							break;

						handler_name_denied(client, name, denyReason);
					}
				}

				break;
			}

			case 2: /* joinchannel */
			{
				const lw_ui8 flags = succeeded ? reader.get <lw_ui8>() : 0;
				const lw_ui8 namelength = reader.get <lw_ui8>();
				const std::string_view name = reader.get(namelength);

				if (reader.failed)
				{
					lw_trace("Reader failed getting channel name.");
					break;
				}

				if (succeeded)
				{
					const lw_ui16 channelid = reader.get <lw_ui16>();

					if (reader.failed)
					{
						lw_trace("Reader failed getting channel ID.");
						break;
					}

					auto channel = std::make_shared<relayclient::channel>(*this);
					auto channelWriteLock = channel->lock.createWriteLock();

					channel->_id = channelid;
					channel->_name = name;
					channel->_namesimplified = lw_u8str_simplify(name);
					channel->_ischannelmaster = (flags & 1) != 0;

					for (; reader.bytesleft() > 0;)
					{
						lw_ui16 peerid	 = reader.get <lw_ui16>();
						lw_ui8 flags2	  = reader.get <lw_ui8>();
						lw_ui8 namelength2 = reader.get <lw_ui8>();
						std::string_view name2 = reader.get(namelength2);

						if (reader.failed)
							break;

						channel->addnewpeer(peerid, flags2, name2);
					}

					channelWriteLock.lw_unlock();

					{
						lacewing::writelock serverWriteLock = this->client.lock.createWriteLock();
						channels.push_back(channel);
					}

					if (handler_channel_join)
						handler_channel_join(client, channel);
				}
				else
				{

					if (handler_channel_joindenied)
					{
						const std::string_view denyReason = reader.getremaining();

						if (reader.failed)
							break;

						handler_channel_joindenied(client, name, denyReason);
					}
				}

				break;
			}

			case 3: /* leavechannel */
			{
				const lw_ui16 id = reader.get<lw_ui16>();
				if (reader.failed)
					break;
				const std::shared_ptr<relayclient::channel> channel = findchannelbyid(id);

				if (!channel)
				{
					reader.failed = true;
					break;
				}

				if (succeeded)
				{
					auto channelWriteLock = channel->lock.createWriteLock();
					channel->_readonly = true;
					for (auto& p : channel->peers)
						p->_readonly = true;
					channelWriteLock.lw_unlock();

					if (handler_channel_leave)
						handler_channel_leave(client, channel);

					// Handler BEFORE finding it in channel list, in case leave handler calls disconnect.
					if (!connected)
						break;

					// LW_ESCALATION_NOTE
					// auto relayCliReadLock = this->client.lock.createReadLock();
					auto relayCliWriteLock = client.lock.createWriteLock();
					const auto i = std::find(channels.cbegin(), channels.cend(), channel);
					if (i == channels.cend())
						break; // Not found...
					// LW_ESCALATION_NOTE
					// auto relayCliWriteLock = rl.lw_upgrade();
					channels.erase(i);
				}
				else
				{
					if (handler_channel_leavedenied)
					{
						// Not null terminated, can't use getremaining()
						const std::string_view denyReason = reader.get(reader.bytesleft());

						if (reader.failed)
							break;

						handler_channel_leavedenied(client, channel, denyReason);
					}
				}

				break;
			}

			case 4: /* channellist */
			{
				auto relayCliWriteLock = this->client.lock.createWriteLock();
				clearchannellist();

				if (!succeeded)
				{
					lacewing::error error = error_new();
					error->add("Channel listing request failed, got error %s from server.", std::string(reader.getremaining()).c_str());
					this->handler_error(client, error);
					error_delete(error);
					return false;
				}

				for (; reader.bytesleft() > 0;)
				{
					const lw_ui16 peercount = reader.get <lw_ui16>();
					const lw_ui8 namelength = reader.get <lw_ui8>();
					const std::string_view name = reader.get(namelength);

					if (reader.failed)
						break;

					auto listing = std::make_shared<relayclient::channellisting>();
					listing->_name = name;
					listing->_namesimplified = lw_u8str_simplify(name);
					listing->_peercount = peercount;
					listing->internaltag = this;

					channellist.push_back(listing);
				}
				relayCliWriteLock.lw_unlock();

				if (handler_channellistreceived)
					handler_channellistreceived(client);

				break;
			}

			default:
			{
				lacewing::error error = error_new();
				error->add("Unrecognised response message received. Response type ID was %hhu, but "
					"expected response type IDs 0-4. Discarding message.", responsetype);
				this->handler_error(client, error);
				error_delete(error);
				return true;
			}
			}

			break;
		}


		// Used in getremaining() for data messages.
		// Text messages are not null-terminated, binary messages can be 0-sized too,
		// but the number messages (variant 1) are 4 bytes in size.
		#define Require4BytesForNumberMessages (variant == 1 ? sizeof(int) : 0), false, false, (variant == 1 ? sizeof(int) : 0xFFFFFFFFU)

		case 1: /* binaryservermessage */
		{
			const lw_ui8 subchannel = reader.get <lw_ui8>();
			const std::string_view data = reader.getremaining(Require4BytesForNumberMessages);

			if (reader.failed)
				break;

			if (handler_message_server)
				handler_message_server(client, blasted, subchannel, data, variant);

			break;
		}

		case 2: /* binarychannelmessage */
		{
			const  lw_ui8 subchannel = reader.get <lw_ui8>();
			const lw_ui16 channel	 = reader.get<lw_ui16>();
			const lw_ui16 peer		 = reader.get<lw_ui16>();

			if (reader.failed)
				break;

			const auto channel2 = findchannelbyid(channel);
			if (reader.failed || !channel2)
			{
				// Disconnected by user request before this message was processed; drop the message
				// without processing future messages (return false), and make no error.
				if (!connected)
					return false;
				break;
			}
			std::shared_ptr<relayclient::channel::peer> peer2;
			{
				auto channelReadLock = channel2->lock.createReadLock();
				peer2 = channel2->findpeerbyid(peer);
			}

			// A UDP message might be faster than the TCP Channel Join/Peer Connect message,
			// making a message for a channel that's OK'd to join on server side be delivered
			// before the "this" client receives its channel join message.
			// Thus, invalid channel.
			if (!peer2 || channel2->_readonly || peer2->_readonly)
				break;

			const std::string_view data = reader.getremaining(Require4BytesForNumberMessages);

			if (reader.failed)
				break;

			if (handler_message_channel)
				handler_message_channel(client, channel2, peer2, blasted,
					subchannel, data, variant);

			break;
		}

		case 3: /* binarypeermessage */
		{
			const lw_ui8 subchannel = reader.get <lw_ui8>();
			const lw_ui16 channel	= reader.get<lw_ui16>();
			const lw_ui16 peer		= reader.get<lw_ui16>();
			if (reader.failed)
				break;

			const auto channel2 = findchannelbyid(channel);
			if (reader.failed || !channel2)
			{
				// Disconnected by user request before this message was processed; drop the message
				// without processing future messages (return false), and make no error.
				if (!connected)
					return false;
				break;
			}
			std::shared_ptr<relayclient::channel::peer> peer2;
			{
				auto channelReadLock = channel2->lock.createReadLock();
				peer2 = channel2->findpeerbyid(peer);
			}

			// A UDP message might be faster than the TCP Channel Join/Peer Connect message,
			// making a message for a channel that's OK'd to join on server side be delivered
			// before the "this" client receives its channel join message.
			// Thus, invalid channel.
			if (!peer2 || channel2->_readonly || peer2->_readonly)
			{
				if (!blasted)
				{
					lacewing::error error = error_new();
					error->add("Peer message received from invalid %s. Discarding.", channel2 ? "peer" : "channel", 1);
					this->handler_error(client, error);
					error_delete(error);
					return true; // Trust client, though
				}
				break;
			}

			const std::string_view data = reader.getremaining(Require4BytesForNumberMessages);

			if (handler_message_peer)
				handler_message_peer(client, channel2, peer2, blasted,
					subchannel, data, variant);

			break;
		}

		case 4: /* binaryserverchannelmessage */
		{
			const lw_ui8 subchannel = reader.get <lw_ui8>();
			const lw_ui16 channel   = reader.get<lw_ui16>();
			if (reader.failed)
				break;

			const auto channel2 = findchannelbyid(channel);

			// A UDP message might outspeed against the TCP Channel Join message
			if (!channel2 || channel2->_readonly)
			{
				// Disconnected by user request before this message was processed; drop the message
				// without processing future messages (return false), and make no error.
				if (!connected)
					return false;
				break;
			}

			const std::string_view data = reader.getremaining(Require4BytesForNumberMessages);

			if (handler_message_serverchannel)
				handler_message_serverchannel(client, channel2, blasted,
					subchannel, data, variant);

			break;
		}

		case 5: /* objectservermessage */
		case 6: /* objectchannelmessage */
		case 7: /* objectpeermessage */
		case 8: /* objectserverchannelmessage */
		{
			lacewing::error error = lacewing::error_new();
			error->add("'Object' message type received, but Bluewing Client implementation does not support it.");
			this->handler_error(client, error);
			error_delete(error);
			return true;
		}

		case 9: /* peer */
		{
			const lw_ui16 channel = reader.get<lw_ui16>();
			if (reader.failed)
				break;
			auto channel2 = findchannelbyid(channel);

			// Channel not found... only UDP peer-to-channel/peer-to-peer messages should
			// be able to outrace a TCP Channel Leave message, not a TCP Peer message,
			// so we'll error it.
			if (!channel2)
			{
				// Disconnected by user request before this message was processed; drop the message
				// without processing future messages (return false), and make no error.
				if (!connected)
					return false;

				lacewing::error error = error_new();
				error->add("Peer message received, but channel ID %hu was not found in this client's joined channel list. Discarding.", channel);
				this->handler_error(client, error);
				error_delete(error);
				return true;
			}

			const lw_ui16 peerid = reader.get <lw_ui16>();

			// LW_ESCALATION_NOTE
			// auto channelReadLock = channel2->lock.createReadLock();
			auto channelWriteLock = channel2->lock.createWriteLock();
			auto peer = channel2->findpeerbyid(peerid);

			const lw_ui8 flags = reader.get <lw_ui8>();
			const std::string_view name = reader.getremaining(0U, false, false, 255U); // name's not null terminated

			if (reader.failed)
			{
				/* no flags/name - the peer must have left the channel */

				// Peer not found - we don't know about the peer, so no point reacting to them
				// leaving
				if (!peer)
					return true;

				peer->_readonly = true;

				channelWriteLock.lw_unlock(); // Don't leave it locked while handler is run

				if (handler_peer_disconnect)
					handler_peer_disconnect(client, channel2, peer);

				channel2 = findchannelbyid(channel);

				// Handler called disconnect, so channel is no longer accessible
				if (!channel2 || !connected)
					return true;

				// LW_ESCALATION_NOTE
				// auto channelWriteLock = channelReadLock.lw_upgrade();
				channelWriteLock.lw_relock();
				const auto i = std::find_if(channel2->peers.begin(), channel2->peers.end(),
					[=](std::shared_ptr<relayclient::channel::peer> &p) { return p->_id == peerid; });
				if (i != channel2->peers.end())
					channel2->peers.erase(i);

				return true;
			}

			if (!peer)
			{
				/* new peer */

				{
					// LW_ESCALATION_NOTE
					// auto channelWriteLock2 = channelReadLock.lw_upgrade();

					// this does channel2->peers.push_back()
					peer = channel2->addnewpeer(peerid, flags, name);

					channelWriteLock.lw_unlock(); // Don't leave it locked while handler is run (when escalation happens, it will unlock by {}s
				}

				if (handler_peer_connect)
					handler_peer_connect(client, channel2, peer);

				break;
			}

			/* existing peer */

			{
				auto peerWriteLock = peer->lock.createWriteLock();
				// LW_ESCALATION_NOTE
				// channelReadLock.lw_unlock();
				channelWriteLock.lw_unlock();

				peer->_ischannelmaster = (flags & 1) != 0;
				// TODO: Check channel for current master and rewrite?

				if (!lw_sv_cmp(name, peer->_name))
				{
					/* peer is changing their name */

					peer->_prevname = peer->_name;
					peer->_name = name;
					peer->_namesimplified = lw_u8str_simplify(name);

					const std::string prevNameLocal = peer->_prevname;

					peerWriteLock.lw_unlock();

					if (handler_peer_changename)
						handler_peer_changename(client, channel2, peer, prevNameLocal);
				}
			}

			if (flags & ~0x1)
			{
				lacewing::error error = error_new();
				error->add("Malformed message received (server error?). Unrecognised peer flags in peer message,"
					" expected 0 or 1, got %i. Ignoring unrecognised flags.", flags);
				this->handler_error(client, error);
				error_delete(error);
			}

			break;
		}

		case 10: /* udpwelcome */

			if (!blasted)
				break;

			if (connected)
			{
				lw_trace("Swallowing extra UDPWelcome at message address %p, already connected.", message);
				break;
			}

			// UDP connection completed before TCP, possibly from bad use of hole punch.
			if (!socket->connected())
			{
				lw_trace("Swallowing UDPWelcome at message address %p, TCP is not ready.", message);
				break;
			}

			lw_trace("UDPWelcome received for message address %p, now connected.", message);

			udphellotimer->stop();
			connected = true;

			if (handler_connect)
				handler_connect(client);

			break;

		case 11: /* ping */
		{
			auto relayCliWriteLock = client.lock.createWriteLock();
			if (blasted)
			{
				// Hole punch connections send ping messages, as they can be received before or after connection
				// Continuing here is bad as server_address() is not guaranteed to be set
				if (!connected)
				{
					always_log("Swallowing early UDP ping (assuming it was a hole punch).\n");
					break;
				}

				this->message.addheader(9, 0, true, id); /* pong */
				this->message.send(this->udp, this->socket->server_address());
			}
			else
			{
				this->message.addheader(9, 0); /* pong */
				this->message.send(socket);
			}

			break;
		}
		case 12: /* implementation */
		{
			static char build[128] = { 0 };

			if (!build[0])
			{
				const char * platform;
				#ifdef _WIN32
					#ifdef _UNICODE
						platform = "Windows Unicode";
					#else
						platform = "Windows ANSI";
					#endif
				#elif defined (__ANDROID__)
					platform = "Android";
				#elif defined(__APPLE__)
					// Not ifdef!
					#if MacBuild
						platform = "Macintosh";
					#else
						platform = "iOS";
					#endif
				#else
					platform = "Unix-based";
					struct utsname name;
					if (uname(&name) == 0)
						platform = name.sysname;
				#endif

				sprintf(build, "Bluewing %s b%i", platform, relayclient::buildnum);
			}

			auto relayCliWriteLock = client.lock.createWriteLock();
			this->message.addheader(10, 0);
			this->message.add(build, -1);
			this->message.send(socket);
			break;
		}

		default:
		{
			lacewing::error error = error_new();
			error->add("Malformed message received (server error?). Unrecognised message type ID %hhu, expected type IDs 0-11. Discarding message.", type, 0);
			this->handler_error(client, error);
			error_delete(error);
			return true;
		}
		};

		if (reader.failed)
		{
			lacewing::error error = error_new();
			error->add("Malformed message received (server error?). Message type %hhu recognised but expected data was not found. Discarding message.", messagetypeid);
			this->handler_error(client, error);
			error_delete(error);
		}
		return true;
	}

	relayclientinternal::relayclientinternal(relayclient &_client, pump _eventpump) :
		client(_client), socket(nullptr), udp(udp_new(_eventpump)),
		udphellotimer(timer_new(_eventpump, "udphello")),
		message(true), messageMF(true)
	{
		initsocket(_eventpump);

		udp->on_data(lacewing::handlerclientudpreceive);
		udp->on_error(lacewing::handlerclientudperror);

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

		udp->tag(this);

		reader.tag = this;
		reader.messagehandler = messagehandler;

		udphellotimer->tag(this);
		udphellotimer->on_tick(udphellotick);

		clear();
	}

	bool relayclientinternal::messagehandler(void * tag, lw_ui8 type, const char * message, size_t size)
	{
		return ((relayclientinternal *)tag)->messagehandler(type, message, size, false);
	}

	void relayclientinternal::udphellotick(lacewing::timer timer)
	{
		((relayclientinternal *)timer->tag())->udphellotick();
	}

	void relayclientinternal::udphellotick()
	{
		// udphellotick just sends UDPHello every 0.5s, and is managed by the relayclientinternal::udphellotimer var.
		// It starts from the time the Connect Request Success message is sent.
		assert(udp->hosting() && "udphellotick() called, but not hosting UDP.");

		message.addheader(7, 0, true, id); /* udphello */
		message.send(udp, socket->server_address());
	}

	void relayclientinternal::initsocket(lacewing::pump pump)
	{
		socket = lacewing::client_new(pump);

		socket->tag(this);
		socket->on_connect(lacewing::handlerconnect);
		socket->on_disconnect(lacewing::handlerdisconnect);
		socket->on_data(lacewing::handlerreceive);
		socket->on_error(lacewing::handlererror);

		socket->nagle(false);
	}


	relayclient::channel::channel(relayclientinternal &_client) noexcept : client(_client)
	{
		_id = 0xffff;
		_name.clear();
		_ischannelmaster = false;
	}

	relayclient::channel::~channel() noexcept
	{
		lacewing::writelock wl = lock.createWriteLock();
		_id = 0xffff;
		_ischannelmaster = false;
	}

	std::shared_ptr<relayclient::channel::peer> relayclient::channel::findpeerbyid(lw_ui16 id)
	{
		if (!lock.checkHoldsRead(false) && !lock.checkHoldsWrite(false))
			assert(false && "Readlock/writelock not held in findpeerbyid().");

		auto i = std::find_if(peers.cbegin(), peers.cend(),
			[&](const std::shared_ptr<relayclient::channel::peer> & p) { return p->_id == id; });
		return (i == peers.cend() ? nullptr : *i);
	}

	std::shared_ptr<relayclient::channel::peer> relayclient::channel::addnewpeer(lw_ui16 peerid, lw_ui8 flags, std::string_view name)
	{
		auto p = std::make_shared<relayclient::channel::peer>(*this, peerid, flags, name);
		peers.push_back(p);
		return p;
	}

	relayclient::channel::peer::peer(relayclient::channel &_channel, lw_ui16 id, lw_ui8 flags, std::string_view name) noexcept
		: channel(_channel)
	{
		// noexcept enforced, so we can't check params
		this->_id = id;
		this->_name = name;
		this->_namesimplified = lw_u8str_simplify(name);
		this->_ischannelmaster = flags;
	}

	relayclient::channel::peer::~peer() noexcept
	{
		// Not strictly needed, but causes deadlock if someone else is using this peer
		lacewing::writelock wl = lock.createWriteLock();
		_id = 0xFFFF;
	}
	bool relayclient::channel::peer::ischannelmaster() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return _ischannelmaster;
	}

	lw_ui16 relayclient::channel::peer::id() const
	{
		// We won't check for read lock, as ID cannot change, so there's no use to threadsafe-ing its access.
		return _id;
	}

	/*
	relayclient::channel::peer * relayclient::channel::peer::next() const
	{
		relayclientinternal::peer &internal = *(relayclientinternal::peer *)internaltag;
		auto end = internal.channel.peers.end();
		auto i = std::find_if(internal.channel.peers.begin(), internal.channel.peers.end(),
			[&](relayclientinternal::peer * &p) { return p->id == internal.id; });
		return (i == end || ++i == end) ? nullptr : &(*i)->public_;
	}*/

	bool relayclient::channel::ischannelmaster() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return _ischannelmaster;
	}

	bool relayclient::channel::readonly() const
	{
		// Atomic, and only changed once, so no point checking
		return _readonly;
	}

	const std::vector<std::shared_ptr<lacewing::relayclient::channel::peer>> & lacewing::relayclient::channel::getpeers() const
	{
		lock.checkHoldsRead();
		return this->peers;
	}

	/*relayclient::channel * relayclient::channel::next() const
	{
		relayclientinternal::channel &internal = *(relayclientinternal::channel *)internaltag;
		auto end = internal.client.channels.end();

		auto i = std::find_if(internal.client.channels.begin(), end,
			[&](relayclientinternal::channel * &c) { return c->id == internal.id; });
		return (i == end || ++i == end) ? nullptr : &(*i)->public_;
	}

	relayclient::channel * relayclient::firstchannel() const
	{
		std::vector<relayclientinternal::channel *> &c = ((lacewing::relayclientinternal *)internaltag)->channels;
		return (c.begin() == c.end()) ? nullptr : &(*c.begin())->public_;
	}

	relayclient::channel::peer * relayclient::channel::firstpeer() const
	{
		std::vector<relayclientinternal::peer *> &p = ((lacewing::relayclientinternal::channel *)internaltag)->peers;
		return (p.begin() == p.end()) ? nullptr : &(*p.begin())->public_;
	}*/
	lw_ui16 relayclient::channellisting::peercount() const
	{
		lacewing::readlock rl = ((relayclientinternal *)this->internaltag)->client.lock.createReadLock();
		return this->_peercount;
	}
	std::string relayclient::channellisting::name() const
	{
		(void)this->tag; // Mark as used
		lacewing::readlock rl = ((relayclientinternal *)this->internaltag)->client.lock.createReadLock();
		return this->_name;
	}

	std::string relayclient::channellisting::namesimplified() const
	{
		lacewing::readlock rl = ((relayclientinternal *)this->internaltag)->client.lock.createReadLock();
		return this->_namesimplified;
	}

	size_t relayclient::channellistingcount() const
	{
		lacewing::readlock rl = lock.createReadLock();
		return ((relayclientinternal *)internaltag)->channellist.size();
	}


	const std::vector<std::shared_ptr<relayclient::channellisting>>& relayclient::getchannellisting() const
	{
		lock.checkHoldsRead();
		return ((relayclientinternal *)internaltag)->channellist;
	}

#define autohandlerfunctions(pub, intern, handlername)			  \
	void pub::on##handlername(pub::handler_##handlername handler)	\
		{	((intern *) internaltag)->handler_##handlername = handler;	  \
		}																	\


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
