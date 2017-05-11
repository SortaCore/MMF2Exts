
#ifndef _lacewing_h
#include "Lacewing.h"
#endif

#include "IDPool.h"
#include "FrameReader.h"
#include "FrameBuilder.h"
#include "MessageReader.h"
#include "MessageBuilder.h"
#include <vector>
#include <chrono>

namespace lacewing
{
struct relayserverinternal;

void tcpmessagehandler    (void * tag, unsigned char type, const char * message, size_t size);
void serverpingtimertick  (lacewing::timer timer);

struct relayserverinternal
{
    lacewing::relayserver &server;
    lacewing::timer pingtimer;

    lacewing::relayserver::handler_connect          handlerconnect;
    lacewing::relayserver::handler_disconnect       handlerdisconnect;
    lacewing::relayserver::handler_error            handlererror;
    lacewing::relayserver::handler_message_server	handlermessage_server;
    lacewing::relayserver::handler_message_channel  handlermessage_channel;
    lacewing::relayserver::handler_message_peer     handlermessage_peer;
    lacewing::relayserver::handler_channel_join     handlerchannel_join;
    lacewing::relayserver::handler_channel_leave    handlerchannel_leave;
    lacewing::relayserver::handler_nameset          handlernameset;

    relayserverinternal(lacewing::relayserver &_server, pump pump)
		: server(_server), pingtimer((lacewing::timer)lw_timer_new((lw_pump)pump)),
		builder(false), builderAuto(false)
    {
        handlerconnect          = 0;
        handlerdisconnect       = 0;
        handlererror            = 0;
        handlermessage_server    = 0;
        handlermessage_channel   = 0;
        handlermessage_peer      = 0;
        handlerchannel_join      = 0;
        handlerchannel_leave     = 0;
        handlernameset          = 0;

        welcomemessage = _strdup("");
    
        pingtimer->tag(this);
        pingtimer->on_tick(serverpingtimertick);

        channellistingenabled = true;
    }

    IDPool clientids;
	IDPool channelids;
	framebuilder builder, builderAuto;

    struct channel;

    struct client
    {
        lacewing::relayserver::client public_;
        lacewing::server_client socket;
        relayserverinternal &server;
		// Can't use socket->address as when server_client is free'd this is no longer valid
		const char * address;
		::std::chrono::high_resolution_clock::time_point connectTime;
		
        client(lacewing::relayserverinternal &internal, lacewing::server_client _socket)
                :  socket(_socket), server(internal),
                    udpaddress(socket->address()), pseudoUDP(false)
        {
            public_.internaltag    = this;
            public_.tag            = 0;
			address = _strdup(socket->address()->tostring());

            id = server.clientids.borrow();

			reader.tag = this;
			reader.messagehandler = &tcpmessagehandler;

			lw_trace("New client internal, address %p. Handshake false.", this);
            handshook      = false;
            ponged         = true;
            gotfirstbyte   = false;
			
			name = nullptr;
			prevname = nullptr;
			// connectTime not started until handshake is done
        }

        ~client()
        {
			free((void *)address);
			address = nullptr;
			if (socket)
				socket->close(false);
			server.clientids.returnID(id);
			lw_trace("~client() dtor, for client internal address %p.", this);
        }

        framereader reader;
        
		void        messagehandler(unsigned char type, const char * message, size_t size, bool blasted);

        std::vector<channel *> channels;

		const char * name, * prevname;
        bool namealtered;

        bool checkname (const char * name);

        unsigned short id;
    
        channel * readchannel(messagereader &reader);
    
        bool handshook;
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

		const char * getimplementation();

		bool pseudoUDP; // Is UDP not supported (e.g. Flash) so "faked" by receiver

        lacewing::address udpaddress;
		void PeerToPeer(relayserver &server, relayserver::channel &viachannel, relayserver::client &receivingclient,
			bool blasted, int subchannel, int variant, const char * message, size_t size);
    };

    struct channel
    {
        lacewing::relayserver::channel public_;
        
        relayserverinternal &server;

        channel(relayserverinternal &_server, const char * const _name) : server(_server),
			channelmaster(nullptr), hidden(true), autoclose(false), isclosed(false)
        {
            public_.internaltag    = this;
            public_.tag            = 0;

			name = _strdup(_name);

            id = server.channelids.borrow();
        }

        ~channel()
        {
			free((char *)name);

			public_.internaltag = nullptr;
            server.channelids.returnID(id);
        }

        std::vector<relayserverinternal::client *> clients;

        const char * name;
        unsigned short id;
        bool hidden;
        bool autoclose;
		bool isclosed;
		client * channelmaster;

		bool readpeer(unsigned short id, client *& readto);
		relayserverinternal::client * readpeer(messagereader &r);
        void removeclient(client &);
        void close();
		client * firstclient;
		channel * next;

		void PeerToChannel(relayserver &server, relayserver::client &client,
			bool blasted, int subchannel, int variant, const char * message, size_t size);
    };
    
  //  backlog<lacewing::server_client, client>
  //      clientbacklog;

  //  backlog<relayserverinternal, channel>
  //      channelbacklog;

    const char * welcomemessage;

	std::vector<client *> clients;
    std::vector<channel *> channels;

    bool channellistingenabled;

    void pingtimertick()
    {
        lacewing::server &socket = server.socket;
		std::vector<relayserverinternal::client *> todisconnect;

        builderAuto.addheader(11, 0); /* ping */
        
        for each (auto clientsocket in clients)
        {
			relayserverinternal::client &client = *(relayserverinternal::client *) clientsocket->public_.internaltag;
            
            if (!client.ponged)
            {
                todisconnect.push_back(&client);
                continue;
            }

            client.ponged = false;
			builderAuto.send (client.socket, false);
        }

		builderAuto.framereset();

		for each (auto clientsocket in todisconnect)
			clientsocket->socket->close();
    }
};

void tcpmessagehandler (void * tag, unsigned char type, const char * message, size_t size)
{   ((relayserverinternal::client *) tag)->messagehandler(type, message, size, false);
}

void serverpingtimertick (lacewing::timer timer)
{   ((relayserverinternal *) timer->tag())->pingtimertick();
}

relayserverinternal::channel * relayserverinternal::client::readchannel(messagereader &reader)
{
    int channelid = reader.get <unsigned short> ();

    if (reader.failed)
        return 0;

	for each (auto e in channels)
        if (e->id == channelid)
            return e;
     
    reader.failed = true;
    return 0;
}

void relayserverinternal::client::PeerToPeer(relayserver &server, relayserver::channel &channel, 
	relayserver::client &receivingClient,
	bool blasted, int subchannel, int variant, const char * message, size_t size)
{
	relayserverinternal &serverinternal = *(relayserverinternal *)server.internaltag;
	framebuilder builder(true);
	builder.addheader(3, variant, blasted, blasted ? id : -1); /* binarypeermessage */

	builder.add <unsigned char>(subchannel);
	builder.add <unsigned short>(channel.id());
	builder.add <unsigned short>(id);
	builder.add(message, size);

	if (blasted)
	{
		relayserverinternal::client &clientinternal = *(relayserverinternal::client *)receivingClient.internaltag;
		builder.send(server.udp, clientinternal.udpaddress);
	}
	else
		builder.send(socket);
}

const char * relayserverinternal::client::getimplementation()
{
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
        return 0;

	for each (auto e in clients)
        if (e->id == peerid)
            return e;
     
    reader.failed = true;
    return 0;
}

void handlerconnect(lacewing::server server, lacewing::server_client clientsocket)
{
    relayserverinternal &internal = *(relayserverinternal *) server->tag();

	auto c = new lacewing::relayserverinternal::client(internal, clientsocket);
	internal.clients.push_back(c);
	clientsocket->tag(c);
}

void handlerdisconnect(lacewing::server server, lacewing::server_client clientsocket)
{
    relayserverinternal &serverinternal = *(relayserverinternal *) server->tag();
    relayserverinternal::client &clientinternal  = *(relayserverinternal::client *) clientsocket->tag();

	for each (auto e in clientinternal.channels)
        e->removeclient (clientinternal);
    
	if (clientinternal.handshook && serverinternal.handlerdisconnect)
	{
		serverinternal.handlerdisconnect(serverinternal.server, clientinternal.public_);
		// handlerdisconnect should delete
	}

	serverinternal.clients.erase(
		std::find(serverinternal.clients.cbegin(), serverinternal.clients.cend(), &clientinternal));
	clientinternal.socket = nullptr;

	if (!clientinternal.handshook || !serverinternal.handlerdisconnect)
		delete &clientinternal.public_;
}

void handlerreceive(lacewing::server server, lacewing::server_client clientsocket, const char * data, size_t size)
{
    relayserverinternal &internal = *(relayserverinternal *) server->tag();
    relayserverinternal::client &client = *(relayserverinternal::client *) clientsocket->tag();
    
    if (!client.gotfirstbyte)
    {
        client.gotfirstbyte = true;

        ++ data;
		lw_trace("Got first byte. Size is %i after decrementation.", size - 1);

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

	if (size < (sizeof(unsigned short) + 1))
		return;

	// While we don't process the message, we do read the sending UDP client ID,
	// in order to call the right clientsocket's messagehandler().

	unsigned char type = *(unsigned char  *)data;
	unsigned short id = *(unsigned short *)(data + 1);

	data += sizeof(unsigned short) + 1;
	size -= sizeof(unsigned short) + 1;

	lacewing::server &socket = internal.server.socket;

	for each (auto clientsocket in internal.clients)
	{
		if (clientsocket->id == id)
		{
			if (clientsocket->socket->address() != address)
				// TODO: A client ID was used by the wrong IP... hack attempt?
				return;
			if (clientsocket->pseudoUDP)
			{
				// TODO: A client ID is set to only have "fake UDP" but used real UDP.
				// Pseudo setting is wrong, not good.
				error error = error_new();
				error->add("Client ID %i is set to pseudo-UDP, but received a real UDP packet"
					" on matching address. Correcting pseudo-UDP; please check your config.", id);
				handlerudperror(udp, error);
				error_delete(error);
				clientsocket->pseudoUDP = false;
			}

			clientsocket->udpaddress->port(address->port());
			clientsocket->pseudoUDP = false;
			clientsocket->messagehandler(type, data, size, true);

			return;
		}
	}

	// TODO: A client ID was not recognised... hack attempt?
}

void handlerflasherror(lacewing::flashpolicy flash, lacewing::error error)
{
	relayserverinternal &internal = *(relayserverinternal *) flash->tag();

    error->add("Flash policy error");

    if (internal.handlererror)
        internal.handlererror(internal.server, error);
}
lacewing::relayserver::relayserver(lacewing::pump pump) : 
	socket((lacewing::server)lw_server_new((lw_pump)pump)),
	udp((lacewing::udp)lw_udp_new((lw_pump)pump)),
	flash((lacewing::flashpolicy)lw_flashpolicy_new((lw_pump)pump))
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
 //   socket->nagle ();
}

lacewing::relayserver::~relayserver()
{
    unhost();

    delete ((relayserverinternal *) internaltag);
}

void lacewing::relayserver::host(unsigned short port)
{
	lacewing::filter filter = lacewing::filter_new();
    filter->local_port(port);

    host(filter);
	filter_delete(filter);
}

void lacewing::relayserver::host(lacewing::filter &_filter)
{
    lacewing::filter filter(_filter);

    if (!filter->local_port())
        filter->local_port(6121);

    socket->host (filter);
    udp->host    (filter);

    ((relayserverinternal *) internaltag)->pingtimer->start(5000L);
}

void lacewing::relayserver::unhost()
{
    socket->unhost();
    udp->unhost();

    ((relayserverinternal *) internaltag)->pingtimer->stop();
}

bool lacewing::relayserver::hosting()
{
    return socket->hosting();
}

unsigned short lacewing::relayserver::port()
{
    return (unsigned short) socket->port();
}

void relayserverinternal::channel::close()
{
    framebuilder builder(true);

    /* tell all the clients that they've left, and remove this channel from their channel lists. */

    builder.addheader		   (0, 0); /* response */
    builder.add <unsigned char>   (3); /* leavechannel */
    builder.add <unsigned char>   (1); /* success */
    builder.add <unsigned short> (id);

	for each (auto e in clients)
    {
        builder.send(e->socket, false);

		for (auto e2 = e->channels.begin(); e2 != e->channels.end(); e2++)
        {
            if (*e2 == this)
            {
                e->channels.erase (e2);
                break;
            }
        }
    }

    builder.framereset();

    
    /* remove this channel from the channel list and return it to the backlog. */

	for (auto e = server.channels.begin(); e != server.channels.end(); e++)
	{
		if (*e == this)
		{
			server.channels.erase(e);
			break;
		}
	}
}

void relayserverinternal::channel::removeclient(relayserverinternal::client &client)
{
	for (auto e = server.clients.begin(); e != server.clients.end(); e++)
    {
        if (*e == &client)
        {
            clients.erase (e);
            break;
        }
    }

    if ((!clients.size()) || (channelmaster == &client && autoclose))
    {   
        close ();
        return;
    }

	// Note: this is where you can assign a different channel master.
	// If you do, don't forget to send a Peer message to change his flags.
    if (channelmaster == &client)
        channelmaster = 0;

	framebuilder builder(true);

    /* notify all the other peers that this client has left the channel */

    builder.addheader (9, 0); /* peer */
    
    builder.add <unsigned short> (id);
    builder.add <unsigned short> (client.id);

    for each(auto e in clients)
        builder.send(e->socket, false);

    builder.framereset();
}

bool relayserverinternal::client::checkname (const char * name)
{
	if (name == nullptr)
		throw std::exception("Null name passed to checkname().");

	for each (auto e in channels)
    {
		for each (auto e2 in e->clients)
        {
            if (e2 == this)
                continue;

            if (!_stricmp(e2->name, name))
            {
				framebuilder builder(true);

                builder.addheader        (0, 0);  /* response */
                builder.add <unsigned char> (1);  /* setname */
                builder.add <unsigned char> (0);  /* failed */

                builder.add <unsigned char> (strlen(name));
                builder.add (name, -1);

                builder.add ("name already taken", -1);

                builder.send(socket);

                return false;
            }
        }
    }

    return true;
}

void relayserverinternal::client::messagehandler(unsigned char type, const char * message, size_t size, bool blasted)
{
    unsigned char messagetypeid  = (type >> 4);
    unsigned char variant        = (type << 4);

    variant >>= 4;

	lw_trace(" relayserverinternal::client::messagehandler, messagetypeid %i, variant %i being processed...", messagetypeid, variant);

    messagereader reader (message, size);
	framebuilder builder(true);
	const char * denyReason;

    if (messagetypeid != 0 && messagetypeid != 9 && !handshook)
    {
		// Haven't got a Connect Request message (0) approved first.
		// We also accept Ping messages (9)
        socket->close();
		lw_trace("Message type id %i discarded. Closing socket forcibly?", messagetypeid);
        return;
    }

    switch(messagetypeid)
    {
        case 0: /* request */
        {
            unsigned char requesttype = reader.get <unsigned char> ();

            if (reader.failed)
                break;

            if (requesttype != 0 && !handshook)
            {
                reader.failed = true;
                break;
            }

            switch(requesttype)
            {
                case 0: /* connect */
                {
                    const char * version = reader.getremaining ();

                    if (reader.failed)
                        break;

                    if (handshook)
                    {
                        reader.failed = true;
                        break;
                    }

                    if (strcmp(version, "revision 3"))
                    {
                        builder.addheader        (0, 0);  /* response */
                        builder.add <unsigned char> (0);  /* connect */
                        builder.add <unsigned char> (0);  /* failed */
                        builder.add ("version mismatch", -1);

                        builder.send(socket);

                        reader.failed = true;
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
                        break;

                    if (!checkname (name))
                        break;

                    /* the .name() setter will also set namealtered to true.  this means that if the
                       handler sets the name explicitly, the default behaviour of setting the name to
                       the requested one will be skipped. */

                    namealtered = false;

					if (server.handlernameset)
						server.handlernameset(server.server, public_, name);
					else
						server.server.nameset_response(public_, name, nullptr);

                    break;
                }

                case 2: /* joinchannel */
                {        
					size_t namelength = strnlen(this->name, 256);
					if (!namelength || namelength == 256)
					{
						reader.failed = true;
						break;
					}

                    unsigned char flags = reader.get <unsigned char> ();
					size_t channelnamelength = reader.bytesleft();
					if (channelnamelength > 255)
					{
						reader.failed = true;
						break;
					}
                    const char *    channelname  = reader.getremaining(false);
                    
                    if (reader.failed)
                        break;

                    relayserverinternal::channel * channel = nullptr;

					for each (auto e in server.channels)
                    {
                        if (!_stricmp (e->name, name))
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

                        for each (auto e in channel->clients)
                        {
                            if (!_stricmp (e->name, this->name))
                            {
                                nametaken = true;
                                break;
                            }
                        }

                        if (nametaken)
                        {
                            builder.addheader        (0, 0);  /* response */
                            builder.add <unsigned char> (2);  /* joinchannel */
                            builder.add <unsigned char> (0);  /* failed */

                            builder.add <unsigned char> (channelnamelength);
                            builder.add (channelname, -1);

                            builder.add ("Channel already contains a client with your client name.", -1);

                            builder.send(socket);

                            break;
                        }

						if (server.handlerchannel_join)
							server.handlerchannel_join(server.server, public_, channel->public_);
						else
							server.server.joinchannel_response(channel->public_, nullptr, public_, nullptr);

                        break;
                    }

                    /* creating a new channel */

                    channel = new relayserverinternal::channel(server, channelname);

                    channel->channelmaster =  this;
                    channel->hidden        =  (flags & 1) != 0;
                    channel->autoclose     =  (flags & 2) != 0;

					if (server.handlerchannel_join)
						server.handlerchannel_join(server.server, public_, channel->public_);
					else
						server.server.joinchannel_response(channel->public_, nullptr, public_, nullptr);

                    break;
                }

                case 3: /* leavechannel */
                {
                    relayserverinternal::channel * channel = readchannel(reader);

                    if (reader.failed)
                        break;

					if (server.handlerchannel_leave)
						server.handlerchannel_leave(server.server, public_, channel->public_);
					#if 0
					// Why would we deny channel leaving if the server has no handler?
					// We should APPROVE by default, not deny.
					else
                    {
                        builder.addheader         (0, 0);  /* response */
                        builder.add <unsigned char>  (3);  /* leavechannel */
                        builder.add <unsigned char>  (0);  /* failed */
                        builder.add <unsigned short> (channel->id);

                        builder.add ("leave refused by server", -1);

                        builder.send(socket);

                        break;
                    }
					#endif

					for (auto e = channels.begin(); e != channels.end(); e++)
                    {
                        if (*e == channel)
                        {
                            channels.erase(e);
                            break;
                        }
                    } 

                    builder.addheader         (0, 0);  /* response */
                    builder.add <unsigned char>  (3);  /* leavechannel */
                    builder.add <unsigned char>  (1);  /* success */
                    builder.add <unsigned short> (channel->id);

                    builder.send(socket);

                    /* do this last, because it might delete the channel */

                    channel->removeclient(*this);

                    break;
                }

                case 4: /* channellist */

                    if (!server.channellistingenabled)
                    {
                        builder.addheader        (0, 0);  /* response */
                        builder.add <unsigned char> (4);  /* channellist */
                        builder.add <unsigned char> (0);  /* failed */
                        
                        builder.add ("channel listing is not enabled on this server", -1);

                        builder.send (socket);

                        break;
                    }

                    builder.addheader        (0, 0);  /* response */
                    builder.add <unsigned char> (4);  /* channellist */
                    builder.add <unsigned char> (1);  /* success */

                    for each (auto e in server.channels)
                    {
                        if (e->hidden)
                            continue;

                        builder.add <unsigned short> (e->clients.size());
                        builder.add <unsigned char>  (strlen(e->name));
                        builder.add (e->name, -1);
                    }

                    builder.send(socket);

                    break;

                default:
                    
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
                break;

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
                break;

			if (server.handlermessage_channel)
				server.handlermessage_channel(server.server, public_, channel->public_,
					blasted, subchannel, message2, size2, variant);
			else
				channel->PeerToChannel(server.server, public_, blasted, subchannel, variant, message2, size2);
			


            break;
        }

        case 3: /* binarypeermessage */
        {
            unsigned char subchannel			   = reader.get <unsigned char> ();
            relayserverinternal::channel * channel = readchannel      (reader);
            relayserverinternal::client  * peer    = channel->readpeer(reader);

            if (peer == this)
            {
                reader.failed = true;
                break;
            }

            const char * message2;
            unsigned int size2;

            reader.getremaining(message2, size2);
            
            if (reader.failed)
                break;

			if (server.handlermessage_peer)
				server.handlermessage_peer(server.server, public_, channel->public_,
					peer->public_, blasted, subchannel, message2, size2, variant);
			else
				PeerToPeer(server.server, channel->public_, public_, blasted, subchannel, variant, message2, size2);

            break;
        }
            
        case 4: /* objectservermessage */

            break;
            
        case 5: /* objectchannelmessage */

            break;
            
        case 6: /* objectpeermessage */

            break;
            
        case 7: /* udphello */

            if (!blasted)
            {
                reader.failed = true;
                break;
            }

            builder.addheader (10, 0); /* udpwelcome */
            builder.send      (server.server.udp, udpaddress);

            break;
            
        case 8: /* channelmaster */

            break;

        case 9: /* ping */

            ponged = true;
            break;

        default:

            reader.failed = true;
            break;
    };

    if (reader.failed)
    {
		lw_trace("Reader failed!");
        /* socket.disconnect(); */
    }
}

void lacewing::relayserver::client::send(int subchannel, const char * message, int size, int variant)
{
    relayserverinternal::client &internal = *(relayserverinternal::client *) internaltag;
	framebuilder builder(true);

    builder.addheader (1, variant); /* binaryservermessage */
    
    builder.add <unsigned char> (subchannel);
    builder.add (message, size);

    builder.send (internal.socket);
}

void lacewing::relayserver::client::blast(int subchannel, const char * message, int size, int variant)
{
    relayserverinternal::client &internal = *(relayserverinternal::client *) internaltag;
	framebuilder builder(true);

    builder.addheader (1, variant, true); /* binaryservermessage */
    
    builder.add <unsigned char> (subchannel);
    builder.add (message, size);

    builder.send (internal.server.server.udp, internal.udpaddress);
}

void lacewing::relayserver::channel::send(int subchannel, const char * message, size_t size, int variant)
{
    relayserverinternal::channel &internal = *(relayserverinternal::channel *) internaltag;
	framebuilder builder(true);

    builder.addheader (4, variant); /* binaryserverchannelmessage */
    
    builder.add <unsigned char> (subchannel);
    builder.add <unsigned short> (internal.id);
    builder.add (message, size);

	for each (auto e in internal.clients)
        builder.send (e->socket, false);

    builder.framereset ();
}

void lacewing::relayserver::channel::blast(int subchannel, const char * message, size_t size, int variant)
{
    relayserverinternal::channel &internal = *(relayserverinternal::channel *) internaltag;
	framebuilder builder(true);

    builder.addheader (4, variant, true); /* binaryserverchannelmessage */
    
    builder.add <unsigned char> (subchannel);
    builder.add <unsigned short> (internal.id);
    builder.add (message, size);

	for each (auto e in internal.clients)
        builder.send (internal.server.server.udp, e->udpaddress, false);

    builder.framereset ();
}

unsigned short lacewing::relayserver::client::id()
{
    return ((relayserverinternal::client *) internaltag)->id;
}

unsigned short lacewing::relayserver::channel::id()
{
	return ((relayserverinternal::channel *) internaltag)->id;
}

const char * lacewing::relayserver::channel::name()
{
    return ((relayserverinternal::channel *) internaltag)->name;
}

void lacewing::relayserver::channel::name(const char * name)
{
	free((char *)((relayserverinternal::channel *) internaltag)->name);
    ((relayserverinternal::channel *) internaltag)->name = name;
}

bool lacewing::relayserver::channel::hidden()
{
    return ((relayserverinternal::channel *) internaltag)->hidden;
}

bool lacewing::relayserver::channel::autocloseenabled()
{
    return ((relayserverinternal::channel *) internaltag)->autoclose;
}

void lacewing::relayserver::setwelcomemessage(const char * message)
{
	relayserverinternal& serverinternal = *(relayserverinternal *)internaltag;
	free((void *)serverinternal.welcomemessage);
	serverinternal.welcomemessage = _strdup(message);
}

const char * lacewing::relayserver::getwelcomemessage()
{
	return ((relayserverinternal *)internaltag)->welcomemessage;
}


void lacewing::relayserver::setchannellisting (bool enabled)
{
    ((relayserverinternal *) internaltag)->channellistingenabled = enabled;
}

lacewing::relayserver::client * lacewing::relayserver::channel::channelmaster()
{
    relayserverinternal::client * client = ((relayserverinternal::channel *) internaltag)->channelmaster;

    return client ? &client->public_ : 0;
}

void lacewing::relayserver::channel::close()
{
    ((relayserverinternal::channel *) internaltag)->close();
}

void lacewing::relayserver::client::disconnect()
{
	if (((relayserverinternal::client *) internaltag)->socket)
		((relayserverinternal::client *) internaltag)->socket->close();
}

const char * lacewing::relayserver::client::getaddress()
{
	return ((relayserverinternal::client *) internaltag)->address;
}

const char * lacewing::relayserver::client::getimplementation()
{
	return ((relayserverinternal::client *) internaltag)->getimplementation();
}


const char * lacewing::relayserver::client::name()
{
    return ((relayserverinternal::client *) internaltag)->name;
}

void lacewing::relayserver::client::name(const char * name)
{
    relayserverinternal::client &internal = *(relayserverinternal::client *) internaltag;

    internal.name = name;
    internal.namealtered = true;
}

size_t lacewing::relayserver::channelcount()
{
    return ((relayserverinternal *) internaltag)->channels.size();
}

size_t lacewing::relayserver::channel::clientcount()
{
    return ((relayserverinternal::channel *) internaltag)->clients.size();
}

size_t lacewing::relayserver::client::channelcount()
{
    return ((relayserverinternal::client *) internaltag)->channels.size();
}

using namespace ::std::chrono;

size_t lacewing::relayserver::client::getconnecttime()
{
	high_resolution_clock::time_point end = high_resolution_clock::now();
	nanoseconds time = end - ((relayserverinternal::client *) internaltag)->connectTime;
	return duration_cast<seconds>(time).count();
}


size_t lacewing::relayserver::clientcount()
{
    return socket->num_clients ();
}

lacewing::relayserver::client * lacewing::relayserver::firstclient ()
{
	return socket->client_first() ?
		&((lacewing::relayserverinternal::client *) socket->client_first()->tag())->public_ : 0;
}

lacewing::relayserver::client * lacewing::relayserver::client::next ()
{
    server_client nextsocket =
        ((relayserverinternal::client *) internaltag)->socket->next ();

    return nextsocket ? ((relayserver::client *) nextsocket->tag()) : 0;
}

lacewing::relayserver::channel * lacewing::relayserver::channel::next ()
{
    return ((relayserverinternal::channel *) internaltag)->next ?
        &((relayserverinternal::channel *) internaltag)->next->public_ : nullptr;
}

lacewing::relayserver::channel * lacewing::relayserver::firstchannel ()
{
    return ((relayserverinternal *) internaltag)->channels.empty() ? nullptr :
			&((relayserverinternal *)internaltag)->channels.front()->public_;
}

lacewing::relayserver::client * lacewing::relayserver::channel::firstclient()
{
	return ((lacewing::relayserverinternal::channel *)this->internaltag)->clients.empty() ? nullptr :
		&((lacewing::relayserverinternal::channel *)this->internaltag)->clients.front()->public_;
}

lacewing::relayserver::channel * lacewing::relayserver::client::firstchannel()
{
	return ((relayserverinternal::client *)internaltag)->channels.empty() ? nullptr :
		&((relayserverinternal::client *)internaltag)->channels.front()->public_;
}


void lacewing::relayserver::connect_response(
	lacewing::relayserver::client &client, const char * denyReason)
{
	auto &serverI = *(relayserverinternal *)this->internaltag;
	framebuilder builder(true);
	// builder (e.g. ping response) and Fusion (e.g. connect response) conflict.
	// BluewingClient gets around this via message vs messageMF.
	// Is it always apparent which to use?
	auto &clientI = *(relayserverinternal::client *)client.internaltag;

	// Connect request denied
	if (denyReason)
	{
		builder.addheader(0, 0);         /* response */
		builder.add <unsigned char>(0);  /* connect */
		builder.add <unsigned char>(0);  /* failed */
		builder.add(denyReason[0] ? denyReason : "Connection refused by server, no specified reason", -1);

		builder.send(clientI.socket);

		// TODO: Make sure this client disconnect is handled smoothly.
		clientI.socket->close();
		lw_trace("Connect request denied for client internal address %p.", &clientI);
		return;
	}

	// Connect request accepted

	lw_trace("Handshake true for client internal address %p.", &clientI);
	clientI.handshook = true;
	clientI.connectTime = std::chrono::high_resolution_clock::now();
	clientI.clientImpl = lacewing::relayserverinternal::client::clientimpl::Windows;

	builder.addheader(0, 0);  /* response */
	builder.add <unsigned char>(0);  /* connect */
	builder.add <unsigned char>(1);  /* success */

	builder.add <unsigned short>(clientI.id);
	builder.add(serverI.welcomemessage, -1);

	builder.send(clientI.socket);
}

void lacewing::relayserver::joinchannel_response(lacewing::relayserver::channel &channel, 
	const char * newChannelName, lacewing::relayserver::client &client, const char * denyReason)
{
	relayserverinternal * serverinternal = (relayserverinternal *)this->internaltag;
	relayserverinternal::client * clientinternal = (relayserverinternal::client *)client.internaltag;
	framebuilder builder(true);
	relayserverinternal::channel * channelinternal = (relayserverinternal::channel *)channel.internaltag;

	// If non-null, request denied. Deny reason can be blank for unspecified.
	if (denyReason)
	{
		builder.addheader(0, 0);         /* response */
		builder.add <unsigned char>(2);  /* joinchannel */
		builder.add <unsigned char>(0);  /* failed */

		builder.add <unsigned char>(strlen(channelinternal->name));
		builder.add(channelinternal->name, -1);

		// Blank reason replaced with "it was unspecified" message
		builder.add(denyReason[0] ? denyReason : "Join refused by server for unspecified reason", -1);

		builder.send(clientinternal->socket);

		// Join request for new channel; request refused, so channel needs to be dropped
		if (channelinternal->clients.empty())
			delete channelinternal;
		return;
	}

	// DENYED REQUEST ONLY ABOVE
	// CREATED CHANNEL ONLY BELOW
	// OUT OF THIS FUNC IS NONCREATED CHANNEL

	// Channel name was changed in join request
	if (newChannelName)
	{
		if (newChannelName[0] == 0 || strnlen(newChannelName, 256) == 256)
		{
			lacewing::error error = lacewing::error_new();
			error->add("Replacement channel name \"%s\"was blank or too long. Maximum channel name length is 255 bytes."
				" Denying join channel request.", newChannelName);
			denyReason = "Server-side error - server replaced the channel name to an invalid name. Join request denied.";
			handlererror(serverinternal->server.socket, error);
		}
		// This is more complicated than it first appears.
		// The channel/channelinternal vars looked up from this function's parameters
		// point to the channel with the OLD name.
		// Which is not the one we're joining this client to.
		else
		{
			lacewing::relayserverinternal::channel * newChannel = nullptr;
			for each (auto i in serverinternal->channels)
			{
				// Channel already exists. Delete parameter channel, and replace it.
				if (!_stricmp(i->name, newChannelName))
				{
					newChannel = i;
					break;
				}
			}

			// New channel exists; switch channelinternal/channel vars to new value
			if (newChannel)
			{
				// not in existing channel's client list yet.
				delete &channelinternal->public_;

				channelinternal = newChannel;
				channel = newChannel->public_;
			}
			// If the channel with that name does not exist, quietly replace channel name in
			// channelinternal variable. 
			else
			{
				free((void *)channelinternal->name);
				channelinternal->name = _strdup(newChannelName);
			}

		}
	}


	/* loop peers on channel currently and concat them with Join Channel Message */
	builder.addheader(0, 0);         /* response */
	builder.add <unsigned char>(2);  /* joinchannel */
	builder.add <unsigned char>(1);  /* success */
	builder.add <unsigned char>(channelinternal->channelmaster != clientinternal);  /* not the channel master */

	builder.add <unsigned char>(strlen(channelinternal->name));
	builder.add(channelinternal->name, -1);

	builder.add <unsigned short>(channelinternal->id);
	
	for each (auto i in channelinternal->clients)
	{
		builder.add <unsigned short>(i->id);
		builder.add <unsigned char>(channelinternal->channelmaster == i ? 1 : 0);
		builder.add <unsigned char>(strlen(i->name));
		builder.add(i->name, -1);
	}

	builder.send(clientinternal->socket);


	builder.addheader(9, 0); /* peer */

	builder.add <unsigned short>(channelinternal->id);
	builder.add <unsigned short>(clientinternal->id);
	builder.add <unsigned char>(0);
	builder.add(clientinternal->name, -1);

	/* notify the other clients on the channel that this client has joined */

	for each (auto i in channelinternal->clients)
		builder.send(i->socket, false);

	builder.framereset();

	/* add this client to the channel */

	serverinternal->channels.push_back(channelinternal);
	channelinternal->clients.push_back(clientinternal);
}

void lacewing::relayserver::channelmessage_permit(lacewing::relayserver::channel &channel, lacewing::relayserver::client &client,
	const char * data, size_t size, unsigned char subchannel, bool blasted, unsigned char variant, bool accept)
{
	lacewing::relayserverinternal::channel &channelinternal = *(lacewing::relayserverinternal::channel *)channel.internaltag;
	if (accept)
		channelinternal.PeerToChannel(*this, client, blasted, subchannel, variant, data, size);
	
	free((void *)data);
}
void lacewing::relayserver::clientmessage_permit(lacewing::relayserver::client &sendingclient, lacewing::relayserver::channel &channel,
	lacewing::relayserver::client &receivingclient,
	const char * data, size_t size, unsigned char subchannel, bool blasted, unsigned char variant, bool accept)
{
	lacewing::relayserverinternal::client &clientinternal = *(lacewing::relayserverinternal::client *)sendingclient.internaltag;
	if (accept)
		clientinternal.PeerToPeer(*this, channel, sendingclient, blasted, subchannel, variant, data, size);

	free((void *)data);
}


void lacewing::relayserver::nameset_response(lacewing::relayserver::client &client,
	const char * newClientName, const char * const denyReason_)
{
	// We use an altered denyReason if there's newClientName problems.
	const char * denyReason = denyReason_;
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

	if (strnlen(newClientName, 256) == 256)
	{
		char * newDenyReason2 = (char *)malloc(300);
		if (!newDenyReason2)
			throw std::exception("Out of memory.");

		sprintf_s(newDenyReason2, 300, "New client name \"%.256s...\" is too long. Name must be 255 chars maximum.", newClientName);
		denyReason = newDenyReason2;
	}

	auto &serverinternal = *(lacewing::relayserverinternal *)internaltag;
	framebuilder builder(true);
	auto clientinternal = ((lacewing::relayserverinternal::client *) client.internaltag);
	const char * const oldClientName = clientinternal->name;

	if (denyReason)
	{
		builder.addheader(0, 0);  /* response */
		builder.add <unsigned char>(1);  /* setname */
		builder.add <unsigned char>(0);  /* failed */

		builder.add <unsigned char>(strlen(newClientName));
		builder.add(newClientName, -1);

		builder.add(denyReason ? denyReason : "Name refused by server, no reason given.", -1);

		builder.send(clientinternal->socket);

		// denyReason was changed
		if (denyReason != denyReason_)
			free((void *)denyReason);
		return;
	}

	bool nameAltered = oldClientName == nullptr || _stricmp(newClientName, oldClientName) != 0;
	if (!nameAltered)
		client.name(_strdup(oldClientName));
	// check the new name provided by the handler
	else if (clientinternal->checkname(newClientName))
		client.name(_strdup(newClientName));
	else
	{
		builder.addheader(0, 0);  /* response */
		builder.add <unsigned char>(1);  /* setname */
		builder.add <unsigned char>(0);  /* failed */

		builder.add <unsigned char>(strlen(newClientName));
		builder.add(newClientName, -1);

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

	builder.add <unsigned char>(strlen(newClientName));
	builder.add (newClientName, -1);

	builder.send(clientinternal->socket);

	for each (auto e in clientinternal->channels)
	{
		builder.addheader(9, 0); /* peer */

		builder.add <unsigned short>(e->id);
		builder.add <unsigned short>(clientinternal->id);
		builder.add <unsigned char>(clientinternal == e->channelmaster ? 1 : 0);
		builder.add (newClientName, -1);

		for each (auto e2 in e->clients)
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
	auto server = (relayserverinternal *)server_.internaltag;
	auto client = (relayserverinternal::client *)client_.internaltag;
	auto &builder = server->builder;
	builder.addheader(2, variant, blasted); /* binarychannelmessage */

	builder.add <unsigned char>(subchannel);
	builder.add <unsigned short>(this->id);
	builder.add <unsigned short>(client->id);
	builder.add(message, size);

	// Loop through and send message to all clients that aren't this one
	for each (auto e in clients)
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


#define autohandlerfunctions(pub, intern, handlername)              \
    void pub::on##handlername(pub::handler_##handlername handler)   \
	    {   ((intern *) internaltag)->handler##handlername = handler;      \
	    }      
autohandlerfunctions(lacewing::relayserver, relayserverinternal, connect)
autohandlerfunctions(lacewing::relayserver, relayserverinternal, disconnect)
autohandlerfunctions(lacewing::relayserver, relayserverinternal, error)
autohandlerfunctions(lacewing::relayserver, relayserverinternal, message_server)
autohandlerfunctions(lacewing::relayserver, relayserverinternal, message_channel)
autohandlerfunctions(lacewing::relayserver, relayserverinternal, message_peer)
autohandlerfunctions(lacewing::relayserver, relayserverinternal, channel_join)
autohandlerfunctions(lacewing::relayserver, relayserverinternal, channel_leave)
autohandlerfunctions(lacewing::relayserver, relayserverinternal, nameset)

}