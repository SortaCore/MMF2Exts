
#ifndef _lacewing_h
#include "Lacewing.h"
#endif

#include "IDPool.h"
#include "FrameReader.h"
#include "FrameBuilder.h"
#include "MessageReader.h"
#include "MessageBuilder.h"
#include <vector>
#include <sstream>
#include <chrono>
#include <assert.h>

namespace lacewing
{
struct relayserverinternal;

bool tcpmessagehandler	(void * tag, unsigned char type, const char * message, size_t size);
void serverpingtimertick  (lacewing::timer timer);

struct relayserverinternal
{
	relayserver &server;
	timer pingtimer;

	relayserver::handler_connect		  handlerconnect;
	relayserver::handler_disconnect		handlerdisconnect;
	relayserver::handler_error			handlererror;
	relayserver::handler_message_server	handlermessage_server;
	relayserver::handler_message_channel  handlermessage_channel;
	relayserver::handler_message_peer	 handlermessage_peer;
	relayserver::handler_channel_join	 handlerchannel_join;
	relayserver::handler_channel_leave	handlerchannel_leave;
	relayserver::handler_nameset		  handlernameset;

	relayserverinternal(relayserver &_server, pump pump)
		: server(_server), pingtimer(lacewing::timer_new(pump)),
		builder(false), builderAuto(false)
	{
		handlerconnect		  = 0;
		handlerdisconnect		= 0;
		handlererror			= 0;
		handlermessage_server	= 0;
		handlermessage_channel	= 0;
		handlermessage_peer	  = 0;
		handlerchannel_join	  = 0;
		handlerchannel_leave	 = 0;
		handlernameset		  = 0;

		welcomemessage = _strdup("");
	
		pingtimer->tag(this);
		pingtimer->on_tick(serverpingtimertick);
		pingMS = 5000L;

		channellistingenabled = true;
	}
	~relayserverinternal()
	{
		free((char *)welcomemessage);
		welcomemessage = nullptr;

		for (auto& c : clients)
		{
			c->channels.clear(); // no channel leave messages from dtor
			delete c;
		}
		clients.clear();

		for (auto& c : channels)
		{
			c->clients.clear(); // prevent channel dtor using already mem-free'd clients
			delete c;
		}
		channels.clear();

		lacewing::timer_delete(pingtimer);
		pingtimer = nullptr;
	}

	IDPool clientids;
	IDPool channelids;
	framebuilder builder, builderAuto;

	struct channel;

	struct client
	{
		relayserver::client public_;
		lacewing::server_client socket;
		relayserverinternal &server;
		// Can't use socket->address, as when server_client is free'd it is no longer valid
		// Since there's a logical use for looking up address during closing, we'll keep a copy.
		const char * address;
		in6_addr addressInt;
		::std::chrono::high_resolution_clock::time_point connectTime;
		::std::chrono::steady_clock::time_point lastmessagetime;

		client(relayserverinternal &internal, lacewing::server_client _socket)
			: socket(_socket), server(internal),
				udpaddress(lacewing::address_new(socket->address()))
		{
			public_.internaltag	= this;
			public_.tag			= 0;
			address = _strdup(socket->address()->tostring());
			addressInt = socket->address()->toin6_addr();

			id = internal.clientids.borrow();

			reader.tag = this;
			reader.messagehandler = &tcpmessagehandler;

			connectRequestApproved = false;
			ponged			= true;
			gotfirstbyte	= false;
			pseudoUDP		= false;

			clientImpl		= Unknown;
			clientImplStr	= nullptr;
			
			name = nullptr;
			prevname = nullptr;
			// connectTime not started until handshake is done
			// last message can be considered the connect time
			lastmessagetime = ::std::chrono::steady_clock::now();
		}

		~client()
		{
			lw_trace("~relayserverinternal::client called");
		//	socket->tag(nullptr);
			public_.internaltag = nullptr;

			free((char *)address);
			address = nullptr;

			channels.clear();

			free((char *)name);
			name = nullptr;

			free((char *)prevname);
			prevname = nullptr;

			free((char *)clientImplStr);
			clientImplStr = nullptr;

		//	socket->close(true);
			server.clientids.returnID(id);

			lacewing::address_delete(udpaddress);
			udpaddress = nullptr;
			
			// TODO: This should be done by owner implicitly?
			//lacewing::stream_delete(socket);
			lw_trace("Should now delete stream %p.", socket);

			socket = nullptr;
		}

		framereader reader;
		
		bool messagehandler(unsigned char type, const char * message, size_t size, bool blasted);

		std::vector<channel *> channels;

		const char * name, * prevname;

		bool checkname(const char * name);

		/// <summary> Closes the client. </summary>
		void close();
		
		unsigned short id;
	
		channel * readchannel(messagereader &reader);
	
		bool connectRequestApproved;
		bool gotfirstbyte;
		bool ponged;

		enum clientimpl
		{
			Unknown,
			Windows,
			Flash,
			HTML5
			// Edit relayserverinternal::client::getimplementation if you add more lines
		} clientImpl;

		const char * clientImplStr;
		const char * getimplementation();

		bool pseudoUDP; // Is UDP not supported (e.g. Flash) so "faked" by receiver

		lacewing::address udpaddress;
		void PeerToPeer(relayserver &server, relayserver::channel &viachannel, relayserver::client &receivingclient,
			bool blasted, int subchannel, int variant, const char * message, size_t size);
	};

	struct channel
	{
		relayserver::channel public_;
		
		relayserverinternal &server;

		channel(relayserverinternal &_server, const char * const _name) : 
			server(_server), channelmaster(nullptr), hidden(true), autoclose(false)
		{
			public_.internaltag	= this;
			public_.tag			= nullptr;

			name = _strdup(_name);

			id = server.channelids.borrow();
		}

		~channel()
		{
			free((char *)name);
			name = nullptr;

			public_.internaltag = nullptr;
			
			for (auto i : clients)
			{
				for (auto j = i->channels.begin(); j != i->channels.end(); j++)
				{
					if ((**j).id == id)
					{
						i->channels.erase(j);
						break;
					}
				}
			}

			// Unlike Client, where clients are "peers" and have a new copy per channel,
			// server retains one copy of the client, for all the channels.
			// So we don't delete the clients in the channel dtor.
			clients.clear();
			
		}

		std::vector<relayserverinternal::client *> clients;

		const char * name;
		unsigned short id;
		bool hidden;
		bool autoclose;
		bool isclone;
		client * channelmaster;

		relayserverinternal::client * readpeer(messagereader &r);

		void addclient(client &);
		void removeclient(client &);

		/// <summary> Gracefully closes the channel, including deleting memory, removing channel
		/// 		  from server list, and messaging clients. </summary>
		void close();
		

		void PeerToChannel(relayserver &server, relayserver::client &client,
			bool blasted, int subchannel, int variant, const char * message, size_t size);
	};
	
  //  backlog<lacewing::server_client, client>
  //	  clientbacklog;

  //  backlog<relayserverinternal, channel>
  //	  channelbacklog;

	const char * welcomemessage;

	std::vector<client *> clients;
	std::vector<channel *> channels;

	bool channellistingenabled;
	long pingMS;

	void pingtimertick()
	{
		std::vector<relayserverinternal::client *> todisconnect;

		builderAuto.addheader(11, 0); /* ping */
		
		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		for (auto clientsocket : clients)
		{
			// Client was deleted but not from server's client list. Assume a delete process has not yet finished.
			if (!clientsocket->public_.internaltag)
				continue;

			relayserverinternal::client &client = *(relayserverinternal::client *) clientsocket->public_.internaltag;

			long long msElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - client.lastmessagetime).count();
			
			// less than 5 seconds passed, skip the ping
			if (msElapsed < pingMS)
			{
				client.ponged = true;
				continue;
			}
			
			if (!client.ponged)
			{
				todisconnect.push_back(&client);
				lw_trace("Disconnecting client ID %i due to ping timeout.", client.id);
				continue;
			}

			client.ponged = false;
			builderAuto.send (client.socket, false);
		}

		builderAuto.framereset();

		for (auto clientsocket : todisconnect)
		{
			if (std::find(clients.begin(), clients.end(), clientsocket) != clients.end())
			{
				lacewing::server_client s = clientsocket->socket;
				clientsocket->close();
				clientsocket->socket->close(true);
				
				// Num refs is 1; delete it
				if (*(unsigned short *)s == 1)
				{
					lw_trace("Running stream_delete for stream %p in ping disconnect hack", s);
					lw_stream_delete((lw_stream)s);
				}
			}
		}
	}
};

bool tcpmessagehandler (void * tag, unsigned char type, const char * message, size_t size)
{	return ((relayserverinternal::client *) tag)->messagehandler(type, message, size, false);
}

void serverpingtimertick (lacewing::timer timer)
{	((relayserverinternal *) timer->tag())->pingtimertick();
}

relayserverinternal::channel * relayserverinternal::client::readchannel(messagereader &reader)
{
	int channelid = reader.get <unsigned short> ();

	if (reader.failed)
		return nullptr;

	for (auto e : channels)
		if (e->id == channelid)
			return e;
	 
	reader.failed = true;
	return nullptr;
}

void relayserverinternal::client::PeerToPeer(relayserver &server, relayserver::channel &channel, 
	relayserver::client &receivingClient,
	bool blasted, int subchannel, int variant, const char * message, size_t size)
{
	relayserverinternal::client &receivingclientinternal = *(relayserverinternal::client *)receivingClient.internaltag;
	relayserverinternal & serverinternal = *(relayserverinternal *)server.internaltag;

	if (id == receivingclientinternal.id)
	{
		lacewing::error error = error_new();
		error->add("Client ID %i attempted to send peer message to ID %i, e.g. themselves. Message dropped.");
		serverinternal.handlererror(server, error);
		error_delete(error);
		return;
	}

	auto &builder = serverinternal.builder;
	builder.addheader(3, variant, blasted); /* binarypeermessage */

	builder.add <unsigned char>(subchannel);
	builder.add <unsigned short>(channel.id());
	builder.add <unsigned short>(id);
	builder.add(message, size);

	if (blasted)
		builder.send(server.udp, receivingclientinternal.udpaddress);
	else
		builder.send(receivingclientinternal.socket);
}

const char * relayserverinternal::client::getimplementation()
{
	if (clientImplStr)
		return clientImplStr;

	switch (clientImpl) {
		case clientimpl::Unknown:
			return "Unknown";
		case clientimpl::Windows:
			return "Windows";
		case clientimpl::Flash:
			return "Flash";
		case clientimpl::HTML5:
			return "HTML5";
		default:
			return "Unknown [error]";
	}
}

relayserverinternal::client * relayserverinternal::channel::readpeer(messagereader &reader)
{
	int peerid = reader.get <unsigned short> ();

	if (reader.failed)
		return nullptr;

	for (auto e : clients)
		if (e->id == peerid)
			return e;
	 
	reader.failed = true;
	return nullptr;
}

void handlerconnect(lacewing::server server, lacewing::server_client clientsocket)
{
	relayserverinternal &serverinternal = *(relayserverinternal *) server->tag();

	auto c = new relayserverinternal::client(serverinternal, clientsocket);
	clientsocket->tag(c);

	// Do not call handlerconnect on relayserverinternal.
	// That will be called when we get a Connect Request message, in Lacewing style.
	// Since this is a raw socket connect handler, we don't know it's Lacewing trying to connect yet.
	
	// if (serverinternal.handlerconnect)
	//	serverinternal.handlerconnect(serverinternal.server, c->public_);
}

void handlerdisconnect(lacewing::server server, lacewing::server_client clientsocket)
{
	if (!clientsocket->tag())
		return;

	relayserverinternal &serverinternal = *(relayserverinternal *) server->tag();
	relayserverinternal::client &clientinternal  = *(relayserverinternal::client *) clientsocket->tag();
	
	clientsocket->tag(nullptr);

	// clientinternal.socket->close(false);

	try {
		clientinternal.close();
	}
	catch (...)
	{
		// k
	}

	if (clientinternal.connectRequestApproved && serverinternal.handlerdisconnect)
	{
		serverinternal.handlerdisconnect(serverinternal.server, clientinternal.public_);
	}

	delete &clientinternal;
}

void handlerreceive(lacewing::server server, lacewing::server_client clientsocket, const char * data, size_t size)
{
	relayserverinternal &internal = *(relayserverinternal *) server->tag();
	relayserverinternal::client &client = *(relayserverinternal::client *) clientsocket->tag();
	
#ifdef _DEBUG
	if ((long)clientsocket->tag() == 0xFEEEFEEEL || (long)clientsocket->tag() == 0x0L || (long)clientsocket->tag() == 0xDDDDDDDDL)
		DebugBreak();
#endif

	if (!client.gotfirstbyte)
	{
		client.gotfirstbyte = true;

		++ data;

		if (!-- size)
			return;
	}

	char * swerve = (char *)malloc(size + 1);
	if (!swerve || memcpy_s(swerve, size + 1, data, size))
		throw std::exception("Couldn't copy message in memory");
	swerve[size] = '\0'; // null terminator because why not

	client.reader.process (swerve, size);

	free(swerve);
}

void handlererror(lacewing::server server, lacewing::error error)
{
	relayserverinternal &internal = *(relayserverinternal *) server->tag();

	error->add("TCP socket error");
	
	if (internal.handlererror)
		internal.handlererror(internal.server, error);
}

void handlerudperror(lacewing::udp udp, lacewing::error error)
{
	relayserverinternal &internal = *(relayserverinternal *) udp->tag();

	error->add("UDP socket error");

	if (internal.handlererror)
		internal.handlererror(internal.server, error);
}


void handlerudpreceive(lacewing::udp udp, lacewing::address address, char * data, size_t size)
{
	relayserverinternal &internal = *(relayserverinternal *)udp->tag();

	// While we don't process the full message here, we do read the sending UDP client ID,
	// in order to call the right clientsocket's messagehandler().

	if (size < (1 + sizeof(unsigned short)))
		return;

	unsigned char type = *(unsigned char  *)data;
	unsigned short id = *(unsigned short *)(data + 1);

	if (id == 0xFFFF)
		return; // this is a placeholder number, and normally indicates error with client

	data += sizeof(unsigned short) + 1;
	size -= sizeof(unsigned short) + 1;

	for (auto clientsocket : internal.clients)
	{
		if (clientsocket->id == id)
		{
			// Pay close attention to this * here. You can do
			// lacewing::address == lacewing::_address, but
			// not any other combo.
			if (*clientsocket->udpaddress != address)
			{
				// A client ID was used by the wrong IP... hack attempt?
				// Can occasionally occur during legitimate disconnects, but rarely (?)
			#if false

				// faulty clients can use ID 0xFFFF and 0x0000

				relayserverinternal::client * realSender = nullptr;
				for (auto cs : internal.clients)
				{
					if (*cs->udpaddress == address)
					{
						realSender = cs;
						break;
					}
				}

				error error = error_new();
				error->add("Received a UDP message (supposedly) from Client ID %i, but it doesn't have that client's IP. ", id);
				if (realSender)
				{
					error->add("Message ACTUALLY originated from client ID %i, on IP %s. Disconnecting client for impersonation attempt. ",
						realSender->id, realSender->address);
					realSender->socket->close();
				}
				error->add("Dropping message.");
				handlerudperror(udp, error);
				error_delete(error);
#endif
				return;
			}

			if (clientsocket->pseudoUDP)
			{
				// A client ID is set to only have "fake UDP" but used real UDP.
				// Pseudo setting is wrong, which means server didn't init client properly, not good.
				error error = error_new();
				error->add("Client ID %i is set to pseudo-UDP, but received a real UDP packet"
					" on matching address. Correcting pseudo-UDP; please check your config.", id);
				handlerudperror(udp, error);
				error_delete(error);
				clientsocket->pseudoUDP = false;
			}

			clientsocket->udpaddress->port(address->port());
			clientsocket->messagehandler(type, data, size, true);

			return;
		}
	}

#if false
	// This #if handles UDP clients that weren't found.
	// Part of it drops UDP clients with the same IP as missing sender.
	// Often in usage the server gets hundreds of UDP messages that are unrecognised.
	// This can go on for minutes, wasting download bandwidth and processing resources.
	// As UDP is connection-less, we can't force the connection shut on them like TCP.
	// As UDP is unreliable, their flooding interferes with other users' traffic.
	//
	// The dropping of matching IPs is to prevent a UDP flood.
	// Doesn't seem particularly effective - and what's worse, in normal operation a client disconnecting
	// will send a couple UDP messages after their disconnect, so we can't use this.
	// TODO: This needs a more legitimate "am I being flooded" check
	// ...and ideally, a ICMP destination-unreachable response.
	// I've heard that's a way to handle a UDP "connection" that won't shut up.
	// Unless it's a deliberate attack, killing all clients on that IP is unnecessary.
	// 
	// If it IS a deliberate attack, there won't even necesssarily BE any Lacewing clients with that IP.
	// Again, UDP is connection-less, so the format of messages (Lacewing or not) is irrelevant - the OS
	// will deliver it to this function here.
	std::vector<relayserverinternal::client *> todrop;
	for (auto clientsocket : internal.clients)
	{
		if (*clientsocket->udpaddress == address)
			todrop.push_back(clientsocket);
	}

	// No one to drop: don't make an error
	if (todrop.empty())
		return;
	// This occurs in regular usage.
	error error = error_new();
	error->add("Received UDP message from Client ID %i, IP %s, but couldn't find client with that ID. Dropping message",
		id, address->tostring());

	for (auto c : todrop)
	{
		try {
			error->add("Dropping client ID %i due to shared IP.", c->id);
			c->socket->close();
		}
		catch (...)
		{
			lw_trace("Dropping failed for ID %i.", c->id);
		}
	}

	handlerudperror(udp, error);
	error_delete(error);
#endif
}


void handlerflasherror(lacewing::flashpolicy flash, lacewing::error error)
{
	relayserverinternal &internal = *(relayserverinternal *) flash->tag();

	error->add("Flash policy error");

	if (internal.handlererror)
		internal.handlererror(internal.server, error);
}

relayserver::relayserver(lacewing::pump pump) : 
	socket(lacewing::server_new(pump)),
	udp(lacewing::udp_new(pump)),
	flash(lacewing::flashpolicy_new(pump))
{
	// lwp_init() not needed

	socket->on_connect		(lacewing::handlerconnect);
	socket->on_disconnect	(lacewing::handlerdisconnect);
	socket->on_data			(lacewing::handlerreceive);
	socket->on_error		(lacewing::handlererror);

	udp->on_data	(lacewing::handlerudpreceive);
	udp->on_error	(lacewing::handlerudperror);

	flash->on_error	(lacewing::handlerflasherror);

	auto s = new relayserverinternal(*this, pump);
	internaltag = s;
	socket->tag(s);
	udp->tag(s);
	flash->tag(s);
	
	// TODO: Disable Nagle here, but nothing in 0.5.4 to match?
 //	socket->nagle ();
}

relayserver::~relayserver()
{
	socket->on_connect(nullptr);
	socket->on_disconnect(nullptr);
	socket->on_data(nullptr);
	socket->on_error(nullptr);

	udp->on_data(nullptr);
	udp->on_error(nullptr);

	flash->on_error(nullptr);

	unhost();
	delete ((relayserverinternal *) internaltag);

	lacewing::server_delete(socket);
	socket = nullptr;
	lacewing::udp_delete(udp);
	udp = nullptr;
	lacewing::flashpolicy_delete(flash);
	flash = nullptr;
}

void relayserver::host(unsigned short port)
{
	lacewing::filter filter = lacewing::filter_new();
	filter->local_port(port);

	host(filter);
	filter_delete(filter);
}

void relayserver::host(lacewing::filter &_filter)
{
	// temp copy to override port
	lacewing::filter filter(_filter);
	
	// Don't let a second server host on this port.
	filter->reuse(false);

	if (!filter->local_port())
		filter->local_port(6121);

	socket->host (filter);
	udp->host	(filter);

	relayserverinternal * serverInternal = (relayserverinternal *)internaltag;
	serverInternal->pingtimer->start(serverInternal->pingMS);
}

void relayserver::unhost()
{
	socket->unhost();
	udp->unhost();

	((relayserverinternal *) internaltag)->pingtimer->stop();
}

bool relayserver::hosting()
{
	return socket->hosting();
}

unsigned short relayserver::port()
{
	return (unsigned short) socket->port();
}

/// <summary> Gracefully closes the channel, including deleting memory, removing channel
/// 		  from server list, and messaging clients. </summary>
void relayserverinternal::channel::close()
{
	// Remove the channel from server's list (if it exists)
	for (auto e3 = server.channels.begin(); e3 != server.channels.end(); e3++)
	{
		if (*e3 == this)
		{
			server.channels.erase(e3);
			break;
		}
	}

	// Message and remove channel from all clients
	if (!clients.empty())
	{
		framebuilder builder(true);
		builder.addheader(0, 0);		  /* response */
		builder.add <unsigned char>(3);	/* leavechannel */
		builder.add <unsigned char>(1);	/* success */
		builder.add <unsigned short>(id); /* channel ID */

		for (auto e : clients)
		{
			builder.send(e->socket, false);

			// Go through client's channel list and remove this channel
			for (auto e2 = e->channels.begin(); e2 != e->channels.end(); e2++)
			{
				if (*e2 == this)
				{
					e->channels.erase(e2);
					break;
				}
			}
		}
	}
	
	// Remove memory for this channel
	delete this;
}

void relayserverinternal::client::close ()
{
	for (auto e : channels)
	{
		// Ensure channel is still open; we rarely get a race condition where it's not
		for (auto e2 = server.channels.begin(); e2 != server.channels.end(); e2++)
		{
			if (*e2 == e)
			{
				e->removeclient(*this);
				break;
			}
		}
	}
	channels.clear();

	// Drop this client from server list (if it exists)
	for (auto cli = server.clients.begin(); cli != server.clients.end(); cli++)
	{
		if (*cli == this)
		{
			server.clients.erase(cli);
			break;
		}
	}
}



void relayserverinternal::channel::addclient(relayserverinternal::client &clientinternal)
{
	for (auto e = clients.begin(); e != clients.end(); e++)
		if (*e == &clientinternal)
			return; // Nothing to do, client is on channel already

	// Join channel is OK
	framebuilder builder(true);
	builder.addheader(0, 0);		 /* response */
	builder.add <unsigned char>(2);  /* joinchannel */
	builder.add <unsigned char>(1);  /* success */
	builder.add <unsigned char>(channelmaster == &clientinternal);  /* whether they are the channel master */

	builder.add <unsigned char>((unsigned char)strlen(name));
	builder.add(name, -1);

	builder.add <unsigned short>(id);

	for (auto i : clients)
	{
		builder.add <unsigned short>(i->id);
		builder.add <unsigned char>(i == channelmaster ? 1 : 0);
		builder.add <unsigned char>((unsigned char)strlen(i->name));
		builder.add(i->name, -1);
	}

	builder.send(clientinternal.socket); // Send list of peers to joining client

	if (!clients.empty())
	{
		builder.framereset();

		// Send peer join message to already existing peers
		builder.addheader(9, 0); /* peer */

		builder.add <unsigned short>(id);
		builder.add <unsigned short>(clientinternal.id);
		builder.add <unsigned char>(0); // if there are peers, we can't be creating, so channelmaster always false
		builder.add(clientinternal.name, -1);

		/* notify the other clients on the channel that this client has joined */

		for (auto i : clients)
			builder.send(i->socket, false);
	}

	// Add this client to channel's list
	clients.push_back(&clientinternal);

	// Add this channel to client's list of joined channels
	if (std::find(clientinternal.channels.begin(), clientinternal.channels.end(), this) == clientinternal.channels.end())
		clientinternal.channels.push_back(this);
}

void relayserverinternal::channel::removeclient(relayserverinternal::client &client)
{
	framebuilder builder(true);

	for (auto e = clients.begin(); e != clients.end(); e++)
	{
		if (*e == &client)
		{
			clients.erase (e);

			// Tell client they were ok to leave

			builder.addheader(0, 0);		 /* response */
			builder.add <unsigned char>(3);  /* leavechannel */
			builder.add <unsigned char>(1);  /* success */
			builder.add <unsigned short>(id); /* channel ID */

			builder.send(client.socket);

			builder.framereset();

			break;
		}
	}


	// No clients left or master left and autoclose is on
	if ((!clients.size()) || (channelmaster == &client && autoclose))
	{	
		close();
		return;
	}

	// Note: this is where you can assign a different channel master.
	// If you do, don't forget to send a Peer message to change his flags,
	// and check the if statement above if you want to assign a new master
	// when old master leaves.
	if (channelmaster == &client)
		channelmaster = nullptr;


	/* notify all the other peers that this client has left the channel */

	builder.addheader (9, 0); /* peer */
	
	builder.add <unsigned short> (id);
	builder.add <unsigned short> (client.id);

	for (auto e : clients)
		builder.send(e->socket, false);

	builder.framereset();
}

bool relayserverinternal::client::checkname (const char * name)
{
	if (name == nullptr)
		throw std::exception("Null name passed to checkname().");

	size_t nameLenFull = strnlen(name, 256U);
	unsigned char nameLen = (unsigned char)nameLenFull;
	if (nameLenFull == 256U)
	{
		framebuilder builder(true);

		builder.addheader(0, 0);  /* response */
		builder.add <unsigned char>(1);  /* setname */
		builder.add <unsigned char>(0);  /* failed */

		builder.add <unsigned char>(255);
		builder.add(name, 255);

		builder.add("name too long, 255 chars maximum", -1);

		builder.send(socket);

		return false;
	}

	for (auto e2 : server.clients)
	{
		if (e2 == this || !e2->name)
			continue;

		if (!_strnicmp(e2->name, name, nameLen))
		{
			framebuilder builder(true);

			builder.addheader		(0, 0);  /* response */
			builder.add <unsigned char> (1);  /* setname */
			builder.add <unsigned char> (0);  /* failed */

			builder.add <unsigned char> (nameLen);
			builder.add (name, nameLen);

			builder.add ("name already taken", -1);

			builder.send(socket);

			return false;
		}
	}

	return true;
}

bool relayserverinternal::client::messagehandler(unsigned char type, const char * message, size_t size, bool blasted)
{
	unsigned char messagetypeid  = (type >> 4);
	unsigned char variant		= (type << 4);

	variant >>= 4;

	messagereader reader (message, size);
	framebuilder builder(true);

	if (messagetypeid != 0 && messagetypeid != 9 && !connectRequestApproved)
	{
		// Haven't got a Connect Request message (0) approved yet,
		// so don't listen to any other message types.
		// We also accept Ping messages (type 9).
		// Since this user is sending messages before approval we can safely say we don't want it.
		
		lacewing::error error = lacewing::error_new();

		// Note: at this point ID is used from server buffer, but Connect Request hasn't been triggered,
		// so using ID or name will make no sense as there'd be no preceeding Connect Request.
		
		char * addr = (char *)malloc(64U);
		if (addr)
			lw_addr_prettystring(address, addr, 64U);
		error->add("Dropping connecting client from IP %s for sending messages before connection was approved", addr ? addr : address);
		free(addr);
		
		server.handlererror(server.server, error);
		
		lacewing::error_delete(error);

		//close();
		socket->close();
		// only return false if socket is emergency closing and
		// you cannot trust further message content is readable
		return false; 
	}

	std::stringstream errStr;
	bool trustedClient = true;

	switch (messagetypeid)
	{
		case 0: /* request */
		{
			unsigned char requesttype = reader.get <unsigned char> ();

			if (reader.failed)
			{
				errStr << "Incomplete request message.";
				trustedClient = false;
				break;
			}

			// Connect request not approved and user is sending a different type of request...
			if (requesttype != 0 && !connectRequestApproved)
			{
				errStr << "Request message of non-Connect type (" << requesttype << ") given when Connect not approved yet.";
				trustedClient = false;
				reader.failed = true;
				break;
			}

			switch(requesttype)
			{
				case 0: /* connect */
				{
					const char * version = reader.getremaining ();

					if (reader.failed)
					{
						errStr << "Malformed connect request message received";
						trustedClient = false;
						break;
					}

					if (connectRequestApproved)
					{
						errStr << "Error: received connect request but already approved connection. Ignoring.";
						return true;
					}

					if (strcmp(version, "revision 3"))
					{
						builder.addheader		(0, 0);  /* response */
						builder.add <unsigned char> (0);  /* connect */
						builder.add <unsigned char> (0);  /* failed */
						builder.add ("version mismatch", -1);

						builder.send(socket);

						reader.failed = true;
						errStr << "Version mismatch in connect request";
						break;
					}

					if (server.handlerconnect)
						server.handlerconnect(server.server, public_);
					else
						server.server.connect_response(public_, nullptr);

					break;
				}

				case 1: /* setname */
				{
					const char * name = reader.getremaining (false);

					if (reader.failed)
					{
						errStr << "Malformed Set Name request received";
						trustedClient = false;
						break;
					}

					if (!checkname (name))
						break; // checkname will make an error, if any

					if (server.handlernameset)
						server.handlernameset(server.server, public_, name);
					else
						server.server.nameset_response(public_, name, nullptr);

					break;
				}

				case 2: /* joinchannel */
				{		
					unsigned char flags = reader.get <unsigned char> ();
					size_t channelnamelength = reader.bytesleft();
					if (channelnamelength > 255)
					{
						builder.addheader(0, 0);  /* response */
						builder.add <unsigned char>(2);  /* joinchannel */
						builder.add <unsigned char>(0);  /* failed */

						builder.add <unsigned char>(255);
						builder.add(reader.get(255), 255);

						builder.add("Channel name too long.", -1);

						builder.send(socket);

						reader.failed = true;

						errStr << "Malformed Join Channel request, name too long.";
						trustedClient = false;

						break;
					}
					const char *	channelname  = reader.getremaining(false);
					
					if (reader.failed)
					{
						errStr << "Malformed Join Channel request, name could not be read.";
						trustedClient = false;
						break;
					}

					relayserverinternal::channel * channel = nullptr;

					for (auto e : server.channels)
					{
						if (!_stricmp (e->name, channelname))
						{
							channel = e;
							break;
						}
					}
					
					if (channel)
					{
						/* joining an existing channel */

						bool nametaken = false;
						// Clients with same name cannot be on same channel

						for (auto e : channel->clients)
						{
							if (!_stricmp (e->name, this->name))
							{
								nametaken = true;
								break;
							}
						}

						if (nametaken)
						{
							builder.addheader		(0, 0);  /* response */
							builder.add <unsigned char> (2);  /* joinchannel */
							builder.add <unsigned char> (0);  /* failed */

							builder.add <unsigned char> ((unsigned char)channelnamelength);
							builder.add (channelname, -1);

							builder.add ("Channel already contains a client with your client name.", -1);

							builder.send(socket);

							break;
						}

						if (server.handlerchannel_join)
							server.handlerchannel_join(server.server, public_, channel->public_, (flags & 1) != 0, (flags & 2) != 0);
						else
							server.server.joinchannel_response(channel->public_, public_, nullptr);

						break;
					}

					/* creating a new channel */

					channel = new relayserverinternal::channel(server, channelname);

					channel->channelmaster =  this;
					channel->hidden		=  (flags & 1) != 0;
					channel->autoclose	 =  (flags & 2) != 0;

					if (server.handlerchannel_join)
						server.handlerchannel_join(server.server, public_, channel->public_, channel->hidden, channel->autoclose);
					else // channel var is either deleted here, or added to server channel list.
						server.server.joinchannel_response(channel->public_, public_, nullptr);

					break;
				}

				case 3: /* leavechannel */
				{
					relayserverinternal::channel * channel = readchannel(reader);

					if (reader.failed)
					{
						errStr << "Malformed Leave Channel request, ID could not be found. Ignoring";
						break;
					}

					if (server.handlerchannel_leave)
						server.handlerchannel_leave(server.server, public_, channel->public_);
					else // Auto-approve. Handles channel deletion.
						server.server.leavechannel_response(channel->public_, public_, nullptr);

					break;
				}

				case 4: /* channellist */

					if (!server.channellistingenabled)
					{
						builder.addheader		(0, 0);  /* response */
						builder.add <unsigned char> (4);  /* channellist */
						builder.add <unsigned char> (0);  /* failed */
						
						builder.add ("channel listing is not enabled on this server", -1);

						builder.send (socket);

						break;
					}

					builder.addheader		(0, 0);  /* response */
					builder.add <unsigned char> (4);  /* channellist */
					builder.add <unsigned char> (1);  /* success */

					for (auto e : server.channels)
					{
						if (e->hidden)
							continue;

						unsigned char nameLen = (unsigned char)strlen(e->name);
						builder.add <unsigned short> ((unsigned short)e->clients.size());
						builder.add <unsigned char>  (nameLen);
						builder.add (e->name, nameLen);
					}

					builder.send(socket);

					break;

				default:

					errStr << "Malformed Request message type, ID " << requesttype << " not recognised.";
					trustedClient = false;
					reader.failed = true;
					break;
			}

			break;
		}

		case 1: /* binaryservermessage */
		{
			unsigned char subchannel = reader.get <unsigned char> ();
			
			const char * message2;
			unsigned int size2;

			reader.getremaining(message2, size2);
			
			if (reader.failed)
			{
				errStr << "Malformed server message received";
				trustedClient = false;
				break;
			}

			if (server.handlermessage_server)
				server.handlermessage_server(server.server, public_, blasted, subchannel, message2, size2, variant);

			break;
		}

		case 2: /* binarychannelmessage */
		{
			unsigned char subchannel = reader.get <unsigned char> ();
			relayserverinternal::channel * channel = readchannel (reader);
			
			const char * message2;
			unsigned int size2;

			reader.getremaining(message2, size2);
			
			if (reader.failed)
			{
				errStr << "Malformed channel message content, discarding";
				break;
			}

			if (server.handlermessage_channel)
				server.handlermessage_channel(server.server, public_, channel->public_,
					blasted, subchannel, message2, size2, variant);
			else
				server.server.channelmessage_permit(public_, channel->public_,
					blasted, subchannel, message2, size2, variant, true);
			
			break;
		}

		case 3: /* binarypeermessage */
		{
			unsigned char subchannel				= reader.get <unsigned char> ();
			relayserverinternal::channel * channel = readchannel	  (reader);
			relayserverinternal::client  * peer	= channel->readpeer(reader);

			// Message to yourself? Witchcraft!
			if (peer == this || peer == nullptr)
			{
				errStr << "Malformed peer message (invalid peer targeted), discarding";
				reader.failed = true;
				break;
			}

			const char * message2;
			unsigned int size2;

			reader.getremaining(message2, size2);
			
			if (reader.failed)
			{
				errStr << "Couldn't read content of peer message, discarding";
				break;
			}

			if (server.handlermessage_peer)
				server.handlermessage_peer(server.server, public_, channel->public_,
					peer->public_, blasted, subchannel, message2, size2, variant);
			else
				server.server.clientmessage_permit(public_, channel->public_, peer->public_,
					blasted, subchannel, message2, size2, variant, true);

			break;
		}
			
		case 4: /* objectservermessage */

			errStr << "ObjectServerMessage not allowed";
			trustedClient = false;
			break;
			
		case 5: /* objectchannelmessage */

			errStr << "ObjectChannelMessage not allowed";
			trustedClient = false;
			break;
			
		case 6: /* objectpeermessage */

			errStr << "ObjectPeerMessage not allowed";
			trustedClient = false;
			break;
			
		case 7: /* udphello */

			// UDPHello on non-UDP port... what
			if (!blasted)
			{
				errStr << "UDPHello message sent on TCP, not allowed";
				trustedClient = false;
				reader.failed = true;
				break;
			}

			pseudoUDP = false;

			builder.addheader (10, 0); /* udpwelcome */
			builder.send	  (server.server.udp, udpaddress);

			break;
			
		case 8: /* channelmaster */
			errStr << "Channel master message ID 8 not allowed";


			break;

		case 9: /* ping */

			ponged = true;
			break;

		case 10: /* implementation response */
		{
			char * impl = reader.get(reader.bytesleft());
			std::string implStr(impl);
			if (reader.failed)
			{
				errStr << "Failed to read implementation response";
				trustedClient = false;
				break;
			}
			
			if (implStr.find("Windows"))
				this->clientImpl = Windows;

			free((char *)this->clientImplStr);
			this->clientImplStr = _strdup(impl);
			break;
		}

		default:

			errStr << "Unrecognised message type ID " << messagetypeid;
			trustedClient = false;
			reader.failed = true;
			break;
	};

	if (reader.failed)
	{
		lacewing::error error = lacewing::error_new();
		error->add("Reader failed!", id);
		error->add("%s", errStr.str().c_str());
		if (!trustedClient)
			error->add("Booting client");

		lw_trace(error->tostring());
		server.handlererror(server.server, error);

		lacewing::error_delete(error);

		if (!trustedClient)
			socket->close();

		// only return false if socket is emergency closing and
		// you cannot trust further message content is readable
		return trustedClient;
		/* socket.disconnect(); */
	}

	return true;
}

void relayserver::client::send(int subchannel, const char * message, int size, int variant)
{
	relayserverinternal::client &internal = *(relayserverinternal::client *) internaltag;
	framebuilder builder(true);

	builder.addheader (1, variant); /* binaryservermessage */
	
	builder.add <unsigned char> (subchannel);
	builder.add (message, size);

	builder.send (internal.socket);
}

void relayserver::client::blast(int subchannel, const char * message, int size, int variant)
{
	relayserverinternal::client &internal = *(relayserverinternal::client *) internaltag;
	framebuilder builder(true);

	builder.addheader (1, variant, true); /* binaryservermessage */
	
	builder.add <unsigned char> (subchannel);
	builder.add (message, size);

	builder.send (internal.server.server.udp, internal.udpaddress);
}

void relayserver::channel::send(int subchannel, const char * message, size_t size, int variant)
{
	relayserverinternal::channel &internal = *(relayserverinternal::channel *) internaltag;
	framebuilder builder(true);

	builder.addheader (4, variant); /* binaryserverchannelmessage */
	
	builder.add <unsigned char> (subchannel);
	builder.add <unsigned short> (internal.id);
	builder.add (message, size);

	for (auto e : internal.clients)
		builder.send (e->socket, false);

	builder.framereset ();
}

void relayserver::channel::blast(int subchannel, const char * message, size_t size, int variant)
{
	relayserverinternal::channel &internal = *(relayserverinternal::channel *) internaltag;
	framebuilder builder(true);

	builder.addheader (4, variant, true); /* binaryserverchannelmessage */
	
	builder.add <unsigned char> (subchannel);
	builder.add <unsigned short> (internal.id);
	builder.add (message, size);

	for (auto e : internal.clients)
		builder.send (internal.server.server.udp, e->udpaddress, false);

	builder.framereset ();
}

unsigned short relayserver::client::id()
{
	return ((relayserverinternal::client *) internaltag)->id;
}

unsigned short relayserver::channel::id()
{
	return ((relayserverinternal::channel *) internaltag)->id;
}

const char * relayserver::channel::name()
{
	return ((relayserverinternal::channel *) internaltag)->name;
}

void relayserver::channel::name(const char * name)
{
	free((char *)((relayserverinternal::channel *) internaltag)->name);
	((relayserverinternal::channel *) internaltag)->name = name;
}

bool relayserver::channel::hidden()
{
	return ((relayserverinternal::channel *) internaltag)->hidden;
}

bool relayserver::channel::autocloseenabled()
{
	return ((relayserverinternal::channel *) internaltag)->autoclose;
}

void relayserver::setwelcomemessage(const char * message)
{
	relayserverinternal& serverinternal = *(relayserverinternal *)internaltag;
	free((void *)serverinternal.welcomemessage);
	serverinternal.welcomemessage = _strdup(message);
}

const char * relayserver::getwelcomemessage()
{
	return ((relayserverinternal *)internaltag)->welcomemessage;
}

void relayserver::setchannellisting (bool enabled)
{
	((relayserverinternal *) internaltag)->channellistingenabled = enabled;
}

relayserver::client * relayserver::channel::channelmaster()
{
	relayserverinternal::client * client = ((relayserverinternal::channel *) internaltag)->channelmaster;

	return client ? &client->public_ : 0;
}

void relayserver::channel::close()
{
	((relayserverinternal::channel *) internaltag)->close();
}

void relayserver::client::close()
{
	((relayserverinternal::client *) internaltag)->close();
}

void relayserver::client::disconnect()
{
	if (((relayserverinternal::client *) internaltag)->socket)
		((relayserverinternal::client *) internaltag)->socket->close();
}
void relayserver::channel::addclient(lacewing::relayserver::client &newClient)
{
	((relayserverinternal::channel *) internaltag)->addclient(*(relayserverinternal::client *) newClient.internaltag);
}
void relayserver::channel::removeclient(lacewing::relayserver::client &clientToDrop)
{
	((relayserverinternal::channel *) internaltag)->removeclient(*(relayserverinternal::client *) clientToDrop.internaltag);
}

const char * relayserver::client::getaddress()
{
	return ((relayserverinternal::client *) internaltag)->address;
}

in6_addr relayserver::client::getaddressasint()
{
	return ((relayserverinternal::client *) internaltag)->addressInt;
}

const char * relayserver::client::getimplementation()
{
	return ((relayserverinternal::client *) internaltag)->getimplementation();
}

const char * relayserver::client::name()
{
	return ((relayserverinternal::client *) internaltag)->name;
}

void relayserver::client::name(const char * name)
{
	relayserverinternal::client &internal = *(relayserverinternal::client *) internaltag;

	free((char *)internal.prevname);
	internal.prevname = internal.name;
	internal.name = _strdup(name);
}

size_t relayserver::channelcount()
{
	return ((relayserverinternal *) internaltag)->channels.size();
}

size_t relayserver::channel::clientcount()
{
	return ((relayserverinternal::channel *) internaltag)->clients.size();
}

size_t relayserver::client::channelcount()
{
	return ((relayserverinternal::client *) internaltag)->channels.size();
}

using namespace ::std::chrono;

__int64 relayserver::client::getconnecttime()
{
	high_resolution_clock::time_point end = high_resolution_clock::now();
	nanoseconds time = end - ((relayserverinternal::client *) internaltag)->connectTime;
	return duration_cast<seconds>(time).count();
}

size_t relayserver::clientcount()
{
	return ((relayserverinternal *)internaltag)->clients.size();
}

relayserver::client::~client() noexcept(false)
{
	/*
	if (internaltag != nullptr)
	{
		// While you can delete a void * pointer and free memory, destructors aren't called.
		// Since internal encloses the public client, we'll defer to it for "real" deletion.
		delete ((relayserverinternal::client *)internaltag);
		internaltag = nullptr;
	}

	// 0xDD used by VS (in debug builds) to check for erroneous writing to freed memory, so we should expect it.
	if (tag && (long)tag != 0xDDDDDDDDL)
		throw std::exception("Deleted a client without a null tag: possible memory leak.");*/
}

relayserver::client * relayserver::firstclient ()
{
	return socket->client_first() ?
		&((relayserverinternal::client *) socket->client_first()->tag())->public_ : 0;
}


relayserver::channel * relayserver::createchannel(const char * channelName, lacewing::relayserver::client &master, bool hidden, bool autoclose)
{
	auto channel = new relayserverinternal::channel(*(lacewing::relayserverinternal *)internaltag, channelName);

	channel->channelmaster = (relayserverinternal::client *)master.internaltag;
	channel->hidden = hidden;
	channel->autoclose = autoclose;

	joinchannel_response(channel->public_, master, nullptr);
	// calls serverinternal.channels.push_back(channel);
	
	return &channel->public_;
}
relayserver::channel * relayserver::createchannel(const char * channelName, bool hidden, bool autoclose)
{
	auto& serverinternal = *(lacewing::relayserverinternal *)internaltag;
	auto channel = new relayserverinternal::channel(serverinternal, channelName);

	channel->hidden = hidden;
	channel->autoclose = autoclose;

	// There's no point calling joinchannel_response, as there's no client to message.
	// The only relevant joinchannel_response action is this:
	serverinternal.channels.push_back(channel);

	return &channel->public_;
}

relayserver::client * relayserver::client::next ()
{
	server_client nextsocket =
		((relayserverinternal::client *) internaltag)->socket->next ();

	return nextsocket ? ((relayserver::client *) nextsocket->tag()) : 0;
}

relayserver::channel * relayserver::client::nextchannel (relayserver::channel * last)
{
	relayserverinternal::client &clientinternal = *(relayserverinternal::client *) internaltag;
	relayserverinternal::channel &channelinternal = *(relayserverinternal::channel *)last->internaltag;
	
	for (auto i = clientinternal.channels.cbegin(); i != clientinternal.channels.cend(); i++)
	{
		if (*i == &channelinternal)
			return &(*i)->public_;
	}

	return nullptr;
}

relayserver::channel::~channel() noexcept(false)
{
	if (internaltag != nullptr)
	{
		// While you can delete a void * pointer and free memory, destructors aren't called.
		// Since internal encloses the public channel, we'll defer to it for "real" deletion.
		delete ((relayserverinternal::channel *)internaltag);
		internaltag = nullptr;
	}

	// 0xDD used by VS (in debug builds) to check for erroneous writing to freed memory, so we should expect it.
	if (tag && (long)tag != 0xDDDDDDDDL)
		throw std::exception("Deleted a channel without a null tag: possible memory leak.");
}

relayserver::channel * relayserver::channel::next ()
{
	relayserverinternal::channel &channelinternal = *(relayserverinternal::channel *)internaltag;
	relayserverinternal & serverinternal = channelinternal.server;

	for (auto i = serverinternal.channels.cbegin(); i != serverinternal.channels.cend(); i++)
	{
		if (*i == &channelinternal)
		{
			++i;
			return i == serverinternal.channels.cend() ? nullptr : &(*i)->public_;
		}
	}

	return nullptr;
}

relayserver::channel * relayserver::firstchannel ()
{
	return ((relayserverinternal *) internaltag)->channels.empty() ? nullptr :
			&((relayserverinternal *)internaltag)->channels.front()->public_;
}

relayserver::client * relayserver::channel::firstclient()
{
	return ((relayserverinternal::channel *)this->internaltag)->clients.empty() ? nullptr :
		&((relayserverinternal::channel *)this->internaltag)->clients.front()->public_;
}

relayserver::channel * relayserver::client::firstchannel()
{
	return ((relayserverinternal::client *)internaltag)->channels.empty() ? nullptr :
		&((relayserverinternal::client *)internaltag)->channels.front()->public_;
}

/// <summary> Responds to a connect request. Pass null for deny reason if approving.
/// 		  You MUST run this event even if denying, or you will have a connection open and a memory leak. </summary>
/// <param name="client">	  [in] The client. Deleted if not approved. </param>
/// <param name="denyReason"> The deny reason. If null, request is approved. </param>
void relayserver::connect_response(
	relayserver::client &client, const char * const denyReason_)
{
	relayserverinternal &serverI = *(relayserverinternal *)this->internaltag;
	relayserverinternal::client * clientI = (relayserverinternal::client *)client.internaltag;
	
	if (clientI->connectRequestApproved)
	{
		lw_trace("connect_response closing early, already approved this connection.");
		return;
	}

	framebuilder builder(true);

	// We don't modify denyReason - if it's going to be freed and we modify the address, the sender can't free it reliably.
	const char * denyReason = denyReason_;
	if (!denyReason_ && !hosting())
		denyReason = "Server has shut down.";

	// Connect request denied
	if (denyReason)
	{
		builder.addheader(0, 0);		 /* response */
		builder.add <unsigned char>(0);  /* connect */
		builder.add <unsigned char>(0);  /* failed */
		builder.add(denyReason[0] ? denyReason : "Connection refused by server, no specified reason", -1);

		builder.send(clientI->socket);
		clientI->close();
		delete clientI;
		return;
	}

	// Connect request accepted

	lw_trace("Connect request accepted in relayserver::connectresponse");
	clientI->connectRequestApproved = true;
	clientI->connectTime = std::chrono::high_resolution_clock::now();
	clientI->clientImpl = relayserverinternal::client::clientimpl::Unknown;
	
	builder.addheader(0, 0);  /* response */
	builder.add <unsigned char>(0);  /* connect */
	builder.add <unsigned char>(1);  /* success */

	builder.add <unsigned short>(clientI->id);
	builder.add(serverI.welcomemessage, -1);

	builder.send(clientI->socket);

	serverI.clients.push_back(clientI);

	builder.framereset();
	builder.addheader(12, 0);  /* request implementation */
	// response on 10, only responded to by Bluewing

	builder.send(clientI->socket);
}

/// <summary> Approves or sends a deny response to channel join request. Pass null for deny reason if approving.
/// 		  Even if you're denying, you still MUST call this event, or you will have a memory leak.
/// 		  For new channels, this will add them to server's channel list if approved, or delete them. </summary>
/// <param name="channel">				[in] The channel. Name is as originally requested. </param>
/// <param name="passedNewChannelName"> Name of the passed channel. If null, original request name is approved.
/// 									If non-null, must be 1-255 chars, or the channel join is denied entirely. </param>
/// <param name="client">				[in] The client joining/creating the channel. </param>
/// <param name="denyReason">			The deny reason. If null, the channel is approved (if new channel name is legal).
/// 									If non-null, channel join deny is sent, and channel is cleaned up as needed. </param>
void relayserver::joinchannel_response(relayserver::channel &channel,
	relayserver::client &client, const char * denyReason)
{
	
	// We can't take out the channel argument, as autoclose and hidden settings will be lost too.
	// At some point we could alter this, grant the server control over whether autoclose is turned on/off.
	// But as there's no way to inform the client that is was created with autoclose or not, and
	// the fact that it's not really a useful feature, this is shelved.
	
	// Hidden can be made pointless by disabling channel listing, and 
	// autoclose can be run manually (e.g. on channel leave, and no clients left, close channel)
	
	relayserverinternal &serverinternal = *(relayserverinternal *)this->internaltag;
	relayserverinternal::client &clientinternal = *(relayserverinternal::client *)client.internaltag;
	relayserverinternal::channel *channelinternal = (relayserverinternal::channel *)channel.internaltag;
	auto channelInServerList = std::find(serverinternal.channels.cbegin(), serverinternal.channels.cend(), channelinternal);
	bool channelIsInServerList = channelInServerList != serverinternal.channels.cend();


	// If non-null, request denied. Deny reason can be blank for unspecified.
	if (denyReason)
	{
		framebuilder builder(true);
		builder.addheader(0, 0);		 /* response */
		builder.add <unsigned char>(2);  /* joinchannel */
		builder.add <unsigned char>(0);  /* failed */

		// actual length 1-255, checked by channel ctor
		builder.add <unsigned char>((unsigned char)strlen(channelinternal->name));
		builder.add(channelinternal->name, -1);

		// Blank reason replaced with "it was unspecified" message
		builder.add(denyReason[0] ? denyReason : "Join refused by server for unspecified reason", -1);

		builder.send(clientinternal.socket);

		// Join request for new channel; request refused, so channel needs to be dropped.
		// Can't use channelinternal client count, autoclose may be off.
		if (!channelIsInServerList)
			delete channelinternal;
		return;
	}

	channelinternal->addclient(clientinternal);

	if (!channelIsInServerList)
		serverinternal.channels.push_back(channelinternal);
}

/// <summary> Approves or sends a deny response to channel leave request. Pass null for deny reason if approving.
/// 		  Even if you're denying, you still MUST call this event, or you will have a memory leak. </summary>
/// <param name="channel">		[in] The channel.  </param>
/// <param name="client">		[in] The client leaving the channel. </param>
/// <param name="denyReason">	The deny reason. If null, the channel is approved (if new channel name is legal).
/// 							If non-null, channel join deny is sent, and channel is cleaned up as needed. </param>
void relayserver::leavechannel_response(lacewing::relayserver::channel &channel,
	lacewing::relayserver::client & client, const char * const denyReason)
{
	relayserverinternal &serverinternal = *(relayserverinternal *)this->internaltag;
	relayserverinternal::client &clientinternal = *(relayserverinternal::client *)client.internaltag;
	relayserverinternal::channel *channelinternal = (relayserverinternal::channel *)channel.internaltag;
	
	// If non-null, request denied. Deny reason can be blank for unspecified.
	if (denyReason)
	{
		framebuilder builder(true);

		builder.addheader(0, 0);		 /* response */
		builder.add <unsigned char>(3);  /* leavechannel */
		builder.add <unsigned char>(0);  /* failed */
		builder.add <unsigned short>(channelinternal->id); /* channel ID */

		// Blank reason replaced with "it was unspecified" message
		builder.add(denyReason[0] ? denyReason : "Leave refused by server for unspecified reason", -1);

		builder.send(clientinternal.socket);

		return;
	}

	channelinternal->removeclient(clientinternal);
}

// These two functions allow access to internal transmissions: PeerToChannel, PeerToPeer.
// The server shouldn't theoretically be able to fake a message from a client.
// The server's handlerclientmessage/channelmessage is responsible for cloning message content if
// the response is asynchronous to the request (different call stack).

void relayserver::channelmessage_permit(relayserver::client &sendingclient, relayserver::channel &channel,
	bool blasted, unsigned char subchannel, const char * data, size_t size, unsigned char variant, bool accept)
{
	if (!accept)
		return;
	relayserverinternal::channel &channelinternal = *(relayserverinternal::channel *)channel.internaltag;
	channelinternal.PeerToChannel(*this, sendingclient, blasted, subchannel, variant, data, size);
}

void relayserver::clientmessage_permit(relayserver::client &sendingclient, relayserver::channel &channel,
	relayserver::client &receivingclient,
	bool blasted, unsigned char subchannel, const char * data, size_t size, unsigned char variant, bool accept)
{
	if (!accept)
		return;
	relayserverinternal::client &clientinternal = *(relayserverinternal::client *)sendingclient.internaltag;
	clientinternal.PeerToPeer(*this, channel, receivingclient, blasted, subchannel, variant, data, size);
}

void relayserver::nameset_response(relayserver::client &client,
	const char * newClientName, const char * const denyReason_)
{
	// We use an altered denyReason if there's newClientName problems.
	const char * denyReason = denyReason_;
	unsigned char newClientNameSize = 0;
	if (!newClientName || !newClientName[0])
	{
		char * newDenyReason = (char *)malloc(150);
		if (!newDenyReason)
			throw std::exception("Out of memory.");

		static char const * const end = "pproved client name is null or empty. Name refused.";
		if (denyReason != nullptr)
			sprintf_s(newDenyReason, 150, "%s\r\nPlus a%s", denyReason, end);
		else
			sprintf_s(newDenyReason, 150, "A%s", end);
		denyReason = newDenyReason;
	}
	else
	{
		size_t newClientNameSizeFull = strnlen(newClientName, 256);
		if (newClientNameSizeFull == 256U)
		{
			char * newDenyReason2 = (char *)malloc(300);
			if (!newDenyReason2)
				throw std::exception("Out of memory.");

			sprintf_s(newDenyReason2, 300, "New client name \"%.256s...\" is too long. Name must be 255 chars maximum.", newClientName);
			denyReason = newDenyReason2;
		}

		newClientNameSize = (unsigned char)newClientNameSizeFull;
	}

	auto &serverinternal = *(relayserverinternal *)internaltag;
	framebuilder builder(true);
	auto clientinternal = ((relayserverinternal::client *) client.internaltag);
	const char * const oldClientName = clientinternal->name;

	if (denyReason)
	{
		builder.addheader(0, 0);  /* response */
		builder.add <unsigned char>(1);  /* setname */
		builder.add <unsigned char>(0);  /* failed */

		builder.add <unsigned char>(newClientNameSize);
		builder.add(newClientName, newClientNameSize);

		builder.add(denyReason ? denyReason : "Name refused by server, no reason given.", -1);

		builder.send(clientinternal->socket);

		// denyReason was changed
		if (denyReason != denyReason_)
			free((void *)denyReason);
		return;
	}

	bool nameAltered = oldClientName == nullptr || strcmp(newClientName, oldClientName);
	if (!nameAltered)
		client.name(oldClientName);
	// check the new name provided by the handler
	else if (clientinternal->checkname(newClientName))
		client.name(newClientName);
	else
	{
		builder.addheader(0, 0);  /* response */
		builder.add <unsigned char>(1);  /* setname */
		builder.add <unsigned char>(0);  /* failed */

		builder.add <unsigned char>(newClientNameSize);
		builder.add(newClientName, newClientNameSize);

		builder.add("Name refused by server; the server customised your name "
			"and got an error doing so on its end.", -1);

		builder.send(clientinternal->socket);
		auto e = lacewing::error_new();
		e->add("Cannot assign the name you altered the Set Name request to.");
		serverinternal.handlererror(serverinternal.server, e);
		return;
	}


	builder.addheader(0, 0);  /* response */
	builder.add <unsigned char>(1);  /* setname */
	builder.add <unsigned char>(1);  /* success */

	builder.add <unsigned char>(newClientNameSize);
	builder.add (newClientName, newClientNameSize);

	builder.send(clientinternal->socket);

	for (auto e : clientinternal->channels)
	{
		builder.addheader(9, 0); /* peer */

		builder.add <unsigned short>(e->id);
		builder.add <unsigned short>(clientinternal->id);
		builder.add <unsigned char>(clientinternal == e->channelmaster ? 1 : 0);
		builder.add (newClientName, newClientNameSize);

		for (auto e2 : e->clients)
		{
			if (e2 == clientinternal)
				continue;

			builder.send(e2->socket, false);
		}

		builder.framereset();
	}
}

void relayserverinternal::channel::PeerToChannel(relayserver &server_, relayserver::client &client_,
	bool blasted, int subchannel, int variant, const char * message, size_t size)
{
	if (clients.size() <= 1)
		return;

	auto server = (relayserverinternal *)server_.internaltag;
	auto client = (relayserverinternal::client *)client_.internaltag;
	auto &builder = server->builder;
	builder.addheader(2, variant, blasted); /* binarychannelmessage */

	builder.add <unsigned char>(subchannel);
	builder.add <unsigned short>(this->id);
	builder.add <unsigned short>(client->id);
	builder.add(message, size);

	// Loop through and send message to all clients that aren't this one
	for (auto e : clients)
	{
		if (e == client)
			continue;

		if (blasted)
			builder.send(server->server.udp, e->udpaddress, false);
		else
			builder.send(e->socket, false);
	}

	builder.framereset();
}


#define autohandlerfunctions(pub, intern, handlername)			  \
	void pub::on##handlername(pub::handler_##handlername handler)	\
		{	((intern *) internaltag)->handler##handlername = handler;	  \
		}	  
autohandlerfunctions(relayserver, relayserverinternal, connect)
autohandlerfunctions(relayserver, relayserverinternal, disconnect)
autohandlerfunctions(relayserver, relayserverinternal, error)
autohandlerfunctions(relayserver, relayserverinternal, message_server)
autohandlerfunctions(relayserver, relayserverinternal, message_channel)
autohandlerfunctions(relayserver, relayserverinternal, message_peer)
autohandlerfunctions(relayserver, relayserverinternal, channel_join)
autohandlerfunctions(relayserver, relayserverinternal, channel_leave)
autohandlerfunctions(relayserver, relayserverinternal, nameset)

}
