
#ifndef _lacewing_h
#include "Lacewing.h"
#endif

#include "IDPool.h"
#include "FrameReader.h"
#include "FrameBuilder.h"
#include "MessageReader.h"
#include "MessageBuilder.h"
#include <vector>

namespace lacewing
{
struct relayserverinternal;

void servermessagehandler(void * tag, unsigned char type, char * message, size_t size, bool blasted);
void servertimertick      (lacewing::timer timer);

struct relayserverinternal
{
    lacewing::relayserver &server;
    lacewing::timer timer;

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
		: server(_server), timer((lacewing::timer)lw_timer_new((lw_pump)pump)), builder(false)
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

        welcomemessage = "";
    
        timer->tag(this);
        timer->on_tick(servertimertick);

        channellistingenabled = true;
    }

    IDPool clientids;
	IDPool channelids;
	framebuilder builder;

    struct channel;

    struct client
    {
        lacewing::relayserver::client public_;
        lacewing::server_client socket;
        relayserverinternal &server;
        
        client(lacewing::server_client _socket)
                : server(*(relayserverinternal *) socket->tag()), socket(_socket),
                    udpaddress(socket->address())
        {
            public_.internaltag    = this;
            public_.tag            = 0;

            id = server.clientids.borrow();

            handshook      = false;
            ponged         = true;
            gotfirstbyte   = false;
        }

        ~client()
        {
            server.clientids.returnID(id);  
        }

        framereader reader;
        
		// UDP only
        void messagehandler (unsigned char type, const char * message, size_t size, bool blasted);

        std::vector<channel *> channels;

		const char * name, * prevname;
        bool namealtered;

        bool checkname (const char * name);

        unsigned short id;
    
        channel * readchannel(messagereader &reader);
    
        bool handshook;
        bool gotfirstbyte;
        bool ponged;

		enum ClientImpl
		{
			Unknown,
			Windows,
			Flash,
			HTML5
		} clientType;

        lacewing::address udpaddress;
		void PeerToPeer(relayserver &server, relayserver::client &client,
			bool blasted, int subchannel, int variant, const char * message, size_t size);
    };

    struct channel
    {
        lacewing::relayserver::channel public_;
        
        relayserverinternal &server;

        channel(relayserverinternal &_server) : server(_server)
        {
            public_.internaltag    = this;
            public_.tag            = 0;

            id = server.channelids.borrow();
        }

        ~channel()
        {
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

    void timertick()
    {
        lacewing::server &socket = server.socket;
		std::vector<relayserverinternal::client *> todisconnect;

        builder.addheader(11, 0); /* ping */
        
        for each (auto clientsocket in clients)
        {
			relayserverinternal::client &client = *(relayserverinternal::client *) clientsocket->public_.internaltag;
            
            if (!client.ponged)
            {
                todisconnect.push_back(&client);
                continue;
            }

            client.ponged = false;
            builder.send (client.socket, false);
        }

        builder.framereset();

		for each (auto clientsocket in todisconnect)
			clientsocket->socket->close();
    }
};

void servermessagehandler (void * tag, unsigned char type, char * message, size_t size, bool blasted)
{   ((relayserverinternal::client *) tag)->messagehandler(type, message, size, false);
}

void servertimertick (lacewing::timer timer)
{   ((relayserverinternal *) timer->tag())->timertick();
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
     
	auto c = new lacewing::relayserverinternal::client(clientsocket);
	internal.clients.push_back(c);
    clientsocket->tag(c);
}

void handlerdisconnect(lacewing::server server, lacewing::server_client clientsocket)
{
    relayserverinternal &internal = *(relayserverinternal *) server->tag();
    relayserverinternal::client * client  = (relayserverinternal::client *) clientsocket->tag();

	for each (auto e in client->channels)
        e->removeclient (*client);
    
    if (client->handshook && internal.handlerdisconnect)
        internal.handlerdisconnect(internal.server, client->public_);

	internal.clients.erase(
		std::find(internal.clients.cbegin(), internal.clients.cend(), client));
	delete client;
}

void handlerreceive(lacewing::server server, lacewing::server_client clientsocket, const char * data, size_t size)
{
    relayserverinternal &internal = *(relayserverinternal *) server->tag();
    relayserverinternal::client &client = *(relayserverinternal::client *) clientsocket->tag();
    
    if (!client.gotfirstbyte)
    {
        client.gotfirstbyte = true;

        ++ data;

        if (!-- size)
            return;
    }
	char * swerve = (char *)malloc(size + 1);
	if (!swerve || memcpy_s(swerve, size + 1, data, size))
		throw std::exception("Sempai why");
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

void handlerudpreceive(lacewing::udp udp, lacewing::address address, char * data, size_t size)
{
    relayserverinternal &internal = *(relayserverinternal *) udp->tag();

    if (size < (sizeof(unsigned short) + 1))
        return;

    unsigned char type = *(unsigned char  *) data;
    unsigned short id  = *(unsigned short *) (data + 1);

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

			clientsocket->udpaddress->port(address->port());
			clientsocket->messagehandler(type, data, size, true);

            break;
        }
    }
}

void handlerudperror(lacewing::udp udp, lacewing::error error)
{
    relayserverinternal &internal = *(relayserverinternal *) udp->tag();

    error->add("UDP socket error");

    if (internal.handlererror)
        internal.handlererror(internal.server, error);
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
    // lacewinginitialise();

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

    ((relayserverinternal *) internaltag)->timer->start(5000L);
}

void lacewing::relayserver::unhost()
{
    socket->unhost();
    udp->unhost();

    ((relayserverinternal *) internaltag)->timer->stop();
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
    framebuilder &builder = server.builder;

    /* tell all the clients that they've left, and remove this channel from their channel lists. */

    builder.addheader   (0, 0, false); /* response */
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

    if (channelmaster == &client)
        channelmaster = 0;


    framebuilder &builder = server.builder;

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
	for each (auto e in channels)
    {
		for each (auto e2 in e->clients)
        {
            if (e2 == this)
                continue;

            if (!_stricmp(e2->name, name))
            {
                framebuilder &builder = server.builder;

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

    messagereader reader (message, size);
    framebuilder &builder = server.builder;
	const char * denyReason;

    if (messagetypeid != 0 && !handshook)
    {
		// Haven't got a Connect Request message approved first.
        socket->close();
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

					std::string namecopy = this->name;

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
					size_t namelength = strnlen(this->name, 257);
					if (!namelength || namelength == 257)
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
                    const char *        name  = reader.getremaining(false);
                    
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

                            builder.add <unsigned char> (namelength);
                            builder.add (name, -1);

                            builder.add ("Channel name used by client.", -1);

                            builder.send(socket);

                            break;
                        }

						if (server.handlerchannel_join)
							server.handlerchannel_join(server.server, public_, channel->public_);
						else
							server.server.joinchannel_response(name, public_, nullptr);
                    
						

                        break;
                    }

                    /* creating a new channel */

                    channel = new relayserverinternal::channel(server);

                    channel->name          =  name;
                    channel->channelmaster =  this;
                    channel->hidden        =  (flags & 1) != 0;
                    channel->autoclose     =  (flags & 2) != 0;

					if (server.handlerchannel_join)
						server.handlerchannel_join(server.server, public_, channel->public_);
					else
						server.server.joinchannel_response(name, public_, nullptr);

                    break;
                }

                case 3: /* leavechannel */
                {
                    relayserverinternal::channel * channel = readchannel(reader);

                    if (reader.failed)
                        break;

					if (server.handlerchannel_leave)
						server.handlerchannel_leave(server.server, public_, channel->public_);
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
                        
                        builder.add ("channel listing is not enabled on this server");

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
                        builder.add (e->name);
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
				peer->public_.send(subchannel, message2, size2, variant);

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
        /* socket.disconnect(); */
    }
}

void lacewing::relayserver::client::send(int subchannel, const char * message, int size, int variant)
{
    relayserverinternal::client &internal = *(relayserverinternal::client *) internaltag;
    framebuilder &builder = internal.server.builder;

    builder.addheader (1, variant); /* binaryservermessage */
    
    builder.add <unsigned char> (subchannel);
    builder.add (message, size);

    builder.send (internal.socket);
}

void lacewing::relayserver::client::blast(int subchannel, const char * message, int size, int variant)
{
    relayserverinternal::client &internal = *(relayserverinternal::client *) internaltag;
    framebuilder &builder = internal.server.builder;

    builder.addheader (1, variant, true); /* binaryservermessage */
    
    builder.add <unsigned char> (subchannel);
    builder.add (message, size);

    builder.send (internal.server.server.udp, internal.udpaddress);
}

void lacewing::relayserver::channel::send(int subchannel, const char * message, size_t size, int variant)
{
    relayserverinternal::channel &internal = *(relayserverinternal::channel *) internaltag;
    framebuilder &builder = internal.server.builder;

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
    framebuilder &builder = internal.server.builder;

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

const char * lacewing::relayserver::channel::name()
{
    return ((relayserverinternal::channel *) internaltag)->name;
}

void lacewing::relayserver::channel::name(const char * name)
{
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
    ((relayserverinternal *) internaltag)->welcomemessage = message;
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
    ((relayserverinternal::client *) internaltag)->socket->close();
}

lacewing::address lacewing::relayserver::client::getaddress()
{
    return ((relayserverinternal::client *) internaltag)->socket->address();
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
	auto &builder = serverI.builder;
	auto &clientI = *(relayserverinternal::client *)client.internaltag;

	builder.addheader(0, 0);  /* response */
	builder.add <unsigned char>(0);  /* connect */
	builder.add <unsigned char>(1);  /* success */

	builder.add <unsigned short>(clientI.id);
	builder.add(serverI.welcomemessage);

	builder.send(clientI.socket);
	clientI.handshook = true;

	// Switch it up and disconnect
	builder.addheader(0, 0);  /* response */
	builder.add <unsigned char>(0);  /* connect */
	builder.add <unsigned char>(0);  /* failed */
	builder.add(denyReason ? denyReason : "Connection refused by server, no specified reason", -1);

	builder.send(clientI.socket);

	clientI.socket->close();
}

void lacewing::relayserver::joinchannel_response(const char * const channelName, 
	lacewing::relayserver::client &client, const char * denyReason)
{
	if (!channelName || !channelName[0])
		throw std::exception("Don't screw around with the channel name.");

	relayserverinternal * serverinternal = (relayserverinternal *)this->internaltag;
	relayserverinternal::client * clientinternal = (relayserverinternal::client *)client.internaltag;
	framebuilder &builder = serverinternal->builder;
	
	
	if (denyReason)
	{
		builder.addheader(0, 0);  /* response */
		builder.add <unsigned char>(2);  /* joinchannel */
		builder.add <unsigned char>(!denyReason);  /* failed */

		builder.add <unsigned char>(strlen(channelName));
		builder.add(channelName);

		builder.add(denyReason[0] ? denyReason : "join refused by server", -1);

		builder.send(clientinternal->socket);
		return;
	}

	relayserverinternal::channel * channel = nullptr;
	for each (auto e in serverinternal->channels)
	{
		if (!_stricmp(e->name, channelName))
		{
			channel = e; break;
		}
	}

	// DENYED REQUEST ONLY ABOVE
	// CREATED CHANNEL ONLY BELOW
	// OUT OF THIS FUNC IS NONCREATED CHANNEL

	/* loop peers on channel currently and concat them with Join Channel Message */
	builder.addheader(0, 0);  /* response */
	builder.add <unsigned char>(2);  /* joinchannel */
	builder.add <unsigned char>(1);  /* success */
	builder.add <unsigned char>(!channel);  /* not the channel master */

	builder.add <unsigned char>(strlen(channel->name));
	builder.add(channel->name);

	builder.add <unsigned short>(channel->id);

	for each (auto e in channel->clients)
	{
		builder.add <unsigned short>(e->id);
		builder.add <unsigned char>(channel->channelmaster == e ? 1 : 0);
		builder.add <unsigned char>(strlen(e->name));
		builder.add(e->name);
	}

	builder.send(clientinternal->socket);


	builder.addheader(9, 0); /* peer */

	builder.add <unsigned short>(channel->id);
	builder.add <unsigned short>(clientinternal->id);
	builder.add <unsigned char>(0);
	builder.add(clientinternal->name);

	/* notify the other clients on the channel that this client has joined */

	for each (auto e in channel->clients)
		builder.send(e->socket, false);

	builder.framereset();


	/* add this client to the channel */

	serverinternal->channels.push_back(channel);
	channel->clients.push_back(clientinternal);
}
void lacewing::relayserver::nameset_response(lacewing::relayserver::client &client,
	const char * newClientName, const char * denyReason)
{
	if (!newClientName || !newClientName[0])
		throw std::exception("Altered client name is null or empty. Name refused.");

	auto &serverinternal = *(lacewing::relayserverinternal *)internaltag;
	framebuilder &builder = ((lacewing::relayserverinternal *)internaltag)->builder;
	auto clientinternal = ((lacewing::relayserverinternal::client *) client.internaltag);
	const char * const oldClientName = clientinternal->name;
	
	if (denyReason)
	{
		builder.addheader(0, 0);  /* response */
		builder.add <unsigned char>(1);  /* setname */
		builder.add <unsigned char>(0);  /* failed */

		builder.add <unsigned char>(strlen(oldClientName));
		builder.add(oldClientName, -1);

		builder.add(denyReason ? denyReason : "Name refused by server, no reason given.", -1);

		builder.send(clientinternal ->socket);
		return;
	}

	bool nameAltered = _stricmp(newClientName, oldClientName) != 0;
	if (!nameAltered)
		client.name(_strdup(oldClientName));
	// check the new name provided by the handler
	else if (!clientinternal->checkname(newClientName))
	{
		builder.addheader(0, 0);  /* response */
		builder.add <unsigned char>(1);  /* setname */
		builder.add <unsigned char>(0);  /* failed */

		builder.add <unsigned char>(strlen(oldClientName));
		builder.add(oldClientName, -1);

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
	builder.add (newClientName);

	builder.send(clientinternal->socket);

	for each (auto e in clientinternal->channels)
	{
		builder.addheader(9, 0); /* peer */

		builder.add <unsigned short>(e->id);
		builder.add <unsigned short>(clientinternal->id);
		builder.add <unsigned char>(clientinternal == e->channelmaster ? 1 : 0);
		builder.add (newClientName);

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