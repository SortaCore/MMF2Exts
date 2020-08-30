
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
#include <time.h>
#include <ctime>

namespace lacewing
{
void serverpingtimertick  (lacewing::timer timer);

struct relayserverinternal
{
	friend relayserver;
	friend relayserver::channel;
	friend relayserver::client;

	relayserver &server;
	timer pingtimer;

	relayserver::handler_connect          handlerconnect;
	relayserver::handler_disconnect       handlerdisconnect;
	relayserver::handler_error            handlererror;
	relayserver::handler_message_server   handlermessage_server;
	relayserver::handler_message_channel  handlermessage_channel;
	relayserver::handler_message_peer     handlermessage_peer;
	relayserver::handler_channel_join     handlerchannel_join;
	relayserver::handler_channel_leave    handlerchannel_leave;
	relayserver::handler_nameset          handlernameset;

	relayserverinternal(relayserver &_server, pump pump)
		: server(_server), pingtimer(lacewing::timer_new(pump))
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

		numTotalClientsPerIP = 5;
		numPendingConnectsPerIP = 2;

		welcomemessage = "";

		pingtimer->tag(this);
		pingtimer->on_tick(serverpingtimertick);
		tcpPingMS = 5000;

		// Some firewalls/router set to mark UDP connections as over after 30 seconds of inactivity,
		// but a general consensus is around 60 seconds.
		// If this occurs, the server will stop accepting new UDP connections, basically keeping any
		// connecting clients frozen in the udp handshake section, as their udphello messages are
		// never received.
		// This code makes the ping thread repeat the request 3 times before 30 seconds are reached.
		// It's not perfect, but since the ping timer will keep repeating anyway past the 3 mark, and
		// activity on either side won't cause the no-new-connections bug to occur, it shouldn't
		// matter either way.
		udpKeepAliveMS = 30000 - (tcpPingMS * 3);

		// Some buggy client versions don't close their connection on end of app, forcing the app to stay
		// alive. We can't force them to close, but we can disconnect them.
		maxInactivityMS = 10 * 60 * 1000;

		channellistingenabled = true;
	}
	~relayserverinternal()
	{
		auto serverWriteLock = server.lock.createWriteLock();
		for (auto& c : clients)
		{
			auto cliWriteLock = c->lock.createWriteLock();
			c->channels.clear(); // no channel leave messages from dtor
			//delete c;
		}
		clients.clear();

		for (auto& c : channels)
		{
			auto chWriteLock = c->lock.createWriteLock();
			c->clients.clear(); // prevent channel dtor using already mem-free'd clients
		//	delete c;
		}
		channels.clear();

		lacewing::timer_delete(pingtimer);
		pingtimer = nullptr;
	}

	IDPool clientids;
	IDPool channelids;

	// Max number of Connect Request Approved and Pending allowed per IP.
	// Excess will be disconnected without On Connect being fired for them.
	size_t numTotalClientsPerIP;
	// Max number of Connect Request pending (on TCP level, or with Connect Request
	// events fired but not responded to)
	// Excess will be disconnected without On Connect being fired for them.
	size_t numPendingConnectsPerIP;

	std::string welcomemessage;

	std::vector<std::shared_ptr<relayserver::client>> clients;
	std::vector<std::shared_ptr<relayserver::channel>> channels;

	bool channellistingenabled;
	long tcpPingMS;
	long udpKeepAliveMS;
	long maxInactivityMS;

	/// <summary> Lacewing timer function for pinging and inactivity tests. </summary>
	///	<remarks> There are three things this function does:
	///			  1) If the client has not sent a TCP message within tcpPingMS milliseconds, send a ping request.
	///			  -> If client still hasn't responded after another tcpPingMS, notify server via error handler,
	///				 and disconnect client.
	///			  2) If the client has not sent a UDP message within udpKeepAliveMS milliseconds, send a ping request.
	///			  -> UDP ping response is not checked for by this function; one-way UDP activity is enough to keep the
	///				 UDP psuedo-connection alive in routers.
	///			     Note using default timing, three UDP messages will be sent before routers are likely to close connection.
	///			  3) If the client has only replied to pings, and not sent any channel, peer, or server messages besides,
	///				 within a period of maxInactivityMS, then the client will be messaged and disconnected, and the server notified
	///				 via error handler.
	///				 Worth noting channel messages when there is no other peers, and serve messages when there is no server message
	///				 handler, and channel join/leave requests as well as other messages, do not qualify as activity. </remarks>
	void pingtimertick()
	{
		std::vector<std::shared_ptr<relayserver::client>> pingUnresponsivesToDisconnect;
		std::vector<std::shared_ptr<relayserver::client>> inactivesToDisconnects;

		framebuilder msgBuilderTCP(false), msgBuilderUDP(true);
		msgBuilderTCP.addheader(11, 0);			/* ping header */
		msgBuilderUDP.addheader(11, 0, true);	/* ping header, true for UDP */

		std::chrono::steady_clock::time_point currentTime = std::chrono::steady_clock::now();
		auto serverReadLock = server.lock.createReadLock();
		for (const auto client : clients)
		{
			if (client->_readonly)
				continue;

			auto msElapsedTCP = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - client->lasttcpmessagetime).count();
			auto msElapsedNonPing = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - client->lastchannelorpeermessagetime).count();

			// Psuedo UDP is true unless a UDPHello packet is received, i.e. the client connect handshake UDP packet.
			decltype(msElapsedTCP) msElapsedUDP = 0;
			if (!client->pseudoUDP)
				msElapsedUDP = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - client->lastudpmessagetime).count();
			
			// detect a buggy version of std::chrono::steady_clock (was in a VS 2019 preview)
			if (msElapsedTCP < 0 || msElapsedUDP < 0 || msElapsedNonPing < 0)
				DebugBreak();

			// less than 5 seconds (or tcpPingMS) passed since last TCP message, skip the TCP ping
			if (msElapsedTCP < tcpPingMS)
			{
				client->pongedOnTCP = true;

				// No UDP keep-alive message needed either, skip both pings
				if (msElapsedUDP < udpKeepAliveMS)
					continue;
			}

			// More than 10 minutes passed, prep to kick for inactivity
			if (msElapsedNonPing > maxInactivityMS)
			{
				inactivesToDisconnects.push_back(client);
				continue;
			}

			// pongedOnTCP is true until client hasn't sent a message within PingMS period.
			// Then it's set to false and a ping message sent, which happens AFTER this if block.
			// The ping timer ticks again tcpPingMS ms later, and if pongedOnTCP is still false
			// (in this if condition), then client hasn't responded to ping, and so should be disconnected.
			if (!client->pongedOnTCP)
			{
				pingUnresponsivesToDisconnect.push_back(client);
				continue;
			}

			// Client is sent a ping request: on next pingtimertick(), pongedOnTCP is checked to still be false.
			auto cliWriteLock = client->lock.createWriteLock();
			if (client->_readonly)
				continue;

			if (msElapsedTCP >= tcpPingMS)
			{
				client->pongedOnTCP = false;
				msgBuilderTCP.send(client->socket, false);
			}

			// Keep UDP alive by sending a UDP message.
			// Worth noting Relay clients and Blue client b82 and below don't have UDP ping responses; they will
			// ignore the message entirely.
			// Fortunately, we don't actually *need* a ping responses from the client; one-way activity ought to be
			// enough to keep the UDP psuedo-connections open in routers... assuming, of course, that the UDP packet
			// goes all the way to the client and thus through all the routers.
			if (msElapsedUDP >= udpKeepAliveMS)
				msgBuilderUDP.send(server.udp, client->udpaddress, false);
		}

		if (pingUnresponsivesToDisconnect.empty() && inactivesToDisconnects.empty())
			return;

		serverReadLock.lw_unlock();

		// Loop all pending ping disconnects
		for (auto& client : pingUnresponsivesToDisconnect)
		{
			// One final disconnect check
			if (client->_readonly)
				continue;

			// To allow client disconnect handlers to run without clashes, we keep the lock open
			// as frequently as possible.
			if (!serverReadLock.isEnabled())
				serverReadLock.lw_relock();

			if (std::find(clients.begin(), clients.end(), client) != clients.end())
			{
				serverReadLock.lw_unlock();
				//auto clientWriteLock = clientsocket->lock.createWriteLock();
				if (client->_readonly)
					continue;
				client->_readonly = true;

				auto error = lacewing::error_new();
				error->add("Disconnecting client ID %i due to ping timeout", client->_id);
				handlererror(this->server, error);
				lacewing::error_delete(error);

				// If a client is ignoring messages, which happens in some ping failures,
				// we can't do a nice clean close(false) like clientsocket->disconnect() does.
				// We have to be mean, because we can't wait for pending write messages to finish,
				// as they never will.
				//
				// As a test scenario, a socket will fail to close if you run two local clients, join them
				// to the same channel, then use Close Windows in taskbar, so they both close simultaneously.
				// The reason for that is that the first to leave channel causes a write of "peer disconnect"
				// (or "channel leave" in case of autoclose + first client is master)
				// and the pending write of that message causes the closure of the second client to never finish,
				// due to the slower close(lw_false) waiting for writes to clear but they never can.
				//
				// Note that a socket timeout does not occur if there is pending write data, and there is no
				// decent "has other side closed connection". Since ping timeout may occur for malicious clients,
				// a decent way might not be good for us anyway.
				client->socket->close(lw_true);
			}
		}

		// Loop all pending inactivity disconnects
		for (auto& client : inactivesToDisconnects)
		{
			if (client->_readonly)
				continue;

			if (!serverReadLock.isEnabled())
				serverReadLock.lw_relock();
			if (std::find(clients.begin(), clients.end(), client) != clients.end())
			{
				serverReadLock.lw_unlock();
				//auto clientWriteLock = clientsocket->lock.createWriteLock();
				if (client->_readonly)
					continue;

				std::string impl = client->clientImplStr;
				if (!impl.empty() && impl.back() == '.')
					impl.erase(impl.cend());

				auto error = lacewing::error_new();
				error->add("Disconnecting client ID %i due to inactivity timeout; client impl \"%s\".", client->_id, impl.c_str());
				handlererror(this->server, error);
				lacewing::error_delete(error);

				client->send(0, "You're being kicked for inactivity.", 0);
				// Close nicely
				client->socket->close(lw_false);
			}
		}
	}

	// for debug
	void makestrstrerror(std::stringstream &err)
	{
		auto error = lacewing::error_new();
		error->add("%s", err.str().c_str(), 0, 0);
		if (handlererror) // null during server destruction
			handlererror(server, error);
		lacewing::error_delete(error);
	}

	// Called by program and by library
	void channel_addclient(std::shared_ptr<relayserver::channel> channel, std::shared_ptr<relayserver::client> client);
	void channel_removeclient(std::shared_ptr<relayserver::channel> channel, std::shared_ptr<relayserver::client> client);

	// Cleans up these
	void close_channel(std::shared_ptr<relayserver::channel> channel);
	void close_client(std::shared_ptr<relayserver::client> client);

	void generic_handlerconnect(lacewing::server server, lacewing::server_client clientsocket);
	void generic_handlerdisconnect(lacewing::server server, lacewing::server_client clientsocket);
	void generic_handlerreceive(lacewing::server server, lacewing::server_client clientsocket, std::string_view data);
	void generic_handlerudpreceive(lacewing::udp udp, lacewing::address address, std::string_view data);

	// Don't ask
	static bool tcpmessagehandler(void * tag, lw_ui8 type, const char * message, size_t size);
	// Used to be inside client, but we need the shared ptr
	bool client_messagehandler(std::shared_ptr<relayserver::client> client, unsigned char type, std::string_view message, bool blasted);
};
void handlerudperror(lacewing::udp udp, lacewing::error error);

void relayserverinternal::generic_handlerudpreceive(lacewing::udp udp, lacewing::address address, std::string_view data)
{
	// While we don't process the full message here, we do read the sending UDP client ID,
	// in order to call the right clientsocket's messagehandler().
	// Originally outside of relayserverinternal, but we need those delectable protected members.

	if (data.size() < (1 + sizeof(unsigned short)))
		return;

	lw_ui8 type = *(lw_ui8 *)data.data();
	lw_ui16 id = *(lw_ui16 *)(data.data() + 1);

	if (id == 0xFFFF)
		return; // this is a placeholder number, and normally indicates error with client

	data.remove_prefix(sizeof(type) + sizeof(id));

	auto serverReadLock = server.lock.createReadLock();
	for (const auto clientsocket : clients)
	{
		if (clientsocket->_id == id)
		{
			serverReadLock.lw_unlock();
			// Pay close attention to this * here. You can do
			// lacewing::address == lacewing::_address, but
			// not any other combo.
			if (*clientsocket->udpaddress != address)
			{
				// A client ID was used by the wrong IP... hack attempt?
				// Can occasionally occur during legitimate disconnects, but rarely (?)
#if false

				// faulty clients can use ID 0xFFFF and 0x0000

				auto rl = lock.createReadLock();

				std::shared_ptr<relayserver::client> realSender = nullptr;
				for (const auto& cs : clients)
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
				error->add("Dropping message");
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
				lacewing::handlerudperror(udp, error);
				error_delete(error);
				clientsocket->pseudoUDP = false;
			}

			clientsocket->udpaddress->port(address->port());
			client_messagehandler(clientsocket, type, data, true);

			return;
		}
	}

	serverReadLock.lw_unlock();
#if 0
	// http://web.archive.org/web/20020609030916/http://www.gamehigh.net/document/netdocs/docs/ping_src.htm

	// Craft ICMP Destination Unreachable
	if (unreachable)
	{
		auto sock = [=]() {
			SOCKET             sockRaw = INVALID_SOCKET;
			struct sockaddr_in dest,
				from;
			int				bread,
				fromlen = sizeof(from),
				timeout = 1000,
				ret;
			char            * icmp_data = NULL,
				* recvbuf = NULL;
			unsigned int	addr = 0;
			USHORT          seq_no = 0;
			struct hostent  * hp = NULL;
			IpOptionHeader  ipopt;

#define ICMP_DEST_UNREACH       3
#define ICMP_PORT_UNREACH       3

			//! ICMP packet structure.
			struct icmp
			{
				// ICMP message type.
				uint8_t icmp_type;
				// ICMP operation code.
				uint8_t icmp_code;
				// ICMP checksum.
				uint16_t icmp_chk;
			};

			auto sockRaw = WSASocket(address->ipv6() ? AF_INET6 : AF_INET,
				SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);
			if (sockRaw == INVALID_SOCKET)
			{
				printf("WSASocket() failed: %d\n", WSAGetLastError());
				return -1;
			}

			// Set the send/recv timeout values
			//
			bread = setsockopt(sockRaw, SOL_SOCKET, SO_RCVTIMEO,
				(char*)&timeout; , sizeof(timeout));
			if (bread == SOCKET_ERROR)
			{
				printf("setsockopt(SO_RCVTIMEO) failed: %d\n",
					WSAGetLastError());
				return -1;
			}
			timeout = 1000;
			bread = setsockopt(sockRaw, SOL_SOCKET, SO_SNDTIMEO,
				(char*)&timeout, sizeof(timeout));
			if (bread == SOCKET_ERROR)
			{
				printf("setsockopt(SO_SNDTIMEO) failed: %d\n",
					WSAGetLastError());
				return -1;
			}

			//
			// Create the ICMP packet
			//
			datasize += sizeof(IcmpHeader);

			icmp_data = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
				MAX_PACKET);
			recvbuf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY,
				MAX_PACKET);
			if (!icmp_data)
			{
				printf("HeapAlloc() failed: %d\n", GetLastError());
				return -1;
			}
			memset(icmp_data, 0, MAX_PACKET);
			FillICMPData(icmp_data, datasize);
			//
			// Start sending/receiving ICMP packets
			//
			while (1)
			{
				static int nCount = 0;
				int        bwrote;

				if (nCount++ == 4)
					break;

				((IcmpHeader*)icmp_data) ->i_cksum = 0;
				((IcmpHeader*)icmp_data) ->timestamp = GetTickCount();
				((IcmpHeader*)icmp_data) ->i_seq = seq_no++;
				((IcmpHeader*)icmp_data) ->i_cksum =
					checksum((USHORT*)icmp_data, datasize);

				bwrote = sendto(sockRaw, icmp_data, datasize, 0,
					(struct sockaddr*) & dest; , sizeof(dest));
				if (bwrote == SOCKET_ERROR)
				{
					if (WSAGetLastError() == WSAETIMEDOUT)
					{
						printf("timed out\n");
						continue;
					}
					printf("sendto() failed: %d\n", WSAGetLastError());
					return -1;
				}
				if (bwrote &lt; datasize)
				{
					printf("Wrote %d bytes\n", bwrote);
		}

		};

	}

#endif
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
	std::vector<relayserver::client *> todrop;
	for (const auto& clientsocket : internal.clients)
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

	for (const auto& c : todrop)
	{
		try {
			error->add("Dropping client ID %i due to shared IP", c->id);
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

bool relayserverinternal::tcpmessagehandler (void * tag, lw_ui8 type, const char * message, size_t size)
{
	auto clientPtr = ((relayserver::client *) tag);
	auto& server = clientPtr->server;
	auto clientIt = std::find_if(server.clients.cbegin(), server.clients.cend(),
		[=](auto const &p) { return p.get() == clientPtr; });
	if (clientIt == server.clients.cend())
	{
		lacewing::error error = lacewing::error_new();
		error->add("Dropped TCP message, shared client ptr not found");
		server.handlererror(server.server, error);
		lacewing::error_delete(error);
		return false;
	}

	return clientPtr->server.client_messagehandler(*clientIt, type, std::string_view(message, size), false);
}

void serverpingtimertick (lacewing::timer timer)
{   ((relayserverinternal *) timer->tag())->pingtimertick();
}

std::shared_ptr<relayserver::channel> relayserver::client::readchannel(messagereader &reader)
{
	int channelid = reader.get <lw_ui16> ();

	if (reader.failed)
		return nullptr;

	for (const auto& e : channels)
		if (e->_id == channelid)
			return e;

	reader.failed = true;
	return nullptr;
}

void relayserver::client::PeerToPeer(relayserver &server, std::shared_ptr<relayserver::channel> channel,
	std::shared_ptr<relayserver::client> receivingClient,
	bool blasted, lw_ui8 subchannel, lw_ui8 variant, std::string_view message)
{
	relayserverinternal & serverinternal = *(relayserverinternal *)server.internaltag;

	if (_id == receivingClient->_id)
	{
		lacewing::error error = error_new();
		error->add("Client ID %i attempted to send peer message to ID %i, e.g. themselves. Message dropped");
		serverinternal.handlererror(server, error);
		error_delete(error);
		return;
	}


	framebuilder builder(!blasted);
	builder.addheader(3, variant, blasted); /* binarypeermessage */

	builder.add <lw_ui8>(subchannel);
	builder.add <lw_ui16>(channel->_id);
	builder.add <lw_ui16>(_id);
	builder.add(message);


	auto channelReadLock = channel->lock.createReadLock();
	if (channel->_readonly)
		return;

	auto recvCliWriteLock = receivingClient->lock.createWriteLock();

	if (receivingClient->_readonly)
		return;

	if (blasted)
	{
		auto serverWriteLock = server.lock.createWriteLock();
		builder.send(server.udp, receivingClient->udpaddress);
	}
	else
		builder.send(receivingClient->socket);
}


std::string_view relayserver::client::getaddress() const
{
	// Addresses don't change, so don't require read lock
	return this->address;
}
in6_addr relayserver::client::getaddressasint() const
{
	// Addresses don't change, so don't require read lock
	return this->addressInt;
}

const char * relayserver::client::getimplementation() const
{
	if (!clientImplStr.empty())
		return clientImplStr.c_str();

	switch (clientImpl) {
		case clientimpl::Unknown:
			return "Unknown";
		case clientimpl::Windows:
			return "Windows (ANSI)";
		case clientimpl::Flash:
			return "Flash";
		case clientimpl::Windows_Unicode:
			return "Windows";
		case clientimpl::Android:
			return "Android";
		case clientimpl::iOS:
			return "iOS";
		case clientimpl::Macintosh:
			return "Macintosh";
		case clientimpl::HTML5:
			return "HTML5";
		default:
			return "Unknown [error]";
	}
}

std::shared_ptr<relayserver::client> relayserver::channel::readpeer(messagereader &reader)
{
	int peerid = reader.get <unsigned short> ();

	if (reader.failed)
		return nullptr;

	for (const auto e : clients)
		if (e->_id == peerid)
			return e;

	reader.failed = true;
	return nullptr;
}

void handlerconnect(lacewing::server server, lacewing::server_client clientsocket)
{
	relayserverinternal &serverinternal = *(relayserverinternal *)server->tag();
	serverinternal.generic_handlerconnect(server, clientsocket);
}
void handlerdisconnect(lacewing::server server, lacewing::server_client clientsocket)
{
	relayserverinternal &serverinternal = *(relayserverinternal *)server->tag();
	serverinternal.generic_handlerdisconnect(server, clientsocket);
}
void handlerreceive(lacewing::server server, lacewing::server_client clientsocket, const char * data, size_t size)
{
	relayserverinternal &internal = *(relayserverinternal *)server->tag();
	internal.generic_handlerreceive(server, clientsocket, std::string_view(data, size));
}

void relayserverinternal::generic_handlerconnect(lacewing::server server, lacewing::server_client clientsocket)
{
	// Check num of pending/active connections. Pending connections may not be in RelayServer's list.
	size_t numMatchIPTotal = 0U, numMatchIPFullyConnected = 0U;
	size_t numMatchIPPending = 0U;
	char * bootReason = nullptr;
	for (auto c = server->client_first(); c; c = c->next())
	{
		if (c == clientsocket)
			continue;

		if (c->address() == clientsocket->address())
		{
			++numMatchIPTotal;
			if (numTotalClientsPerIP < numMatchIPTotal)
			{
				bootReason = "";
				break;
			}

			// Client fully established connection
			if (c->tag() != nullptr &&
				((relayserver::client *)c->tag())->connectRequestApproved)
			{
				++numMatchIPFullyConnected;
			}
			else
			{
				++numMatchIPPending;
				if (numPendingConnectsPerIP < numMatchIPPending)
				{
					bootReason = "pending ";
					break;
				}
			}
		}
	}

	if (bootReason)
	{
		clientsocket->writef("Too many %sconnections from your IP.", bootReason);
		clientsocket->close();
		return;
	}

	// Add client to server's client list
	auto newClient = std::make_shared<relayserver::client>(*this, clientsocket);
	clientsocket->tag(newClient.get());
	{
		auto serverWriteLock = this->server.lock.createWriteLock();
		this->clients.push_back(newClient);
	}

	// Do not call handlerconnect on relayserverinternal.
	// That will be called when we get a Connect Request message, in Lacewing style.
	// Since this is a raw socket connect handler, we don't know it's Lacewing trying to connect yet.

	// if (serverinternal.handlerconnect)
	//	serverinternal.handlerconnect(serverinternal.server, c->public_);
}

void relayserverinternal::generic_handlerdisconnect(lacewing::server server, lacewing::server_client clientsocket)
{
	// Should store the relayclient * address...
	if (!clientsocket->tag())
	{
		std::stringstream err;
		err << "generic_handlerdisconnect: disconnect by client with null tag.";
		makestrstrerror(err);
		return;
	}

	// Find shared pointer.

	relayserver::client *client = (relayserver::client *)clientsocket->tag();
	lacewing::writelock cliWriteLock = client->lock.createWriteLock();
	client->_readonly = true;

	lacewing::writelock serverWriteLock = this->server.lock.createWriteLock();
	auto clientIt =
		std::find_if(clients.cbegin(), clients.cend(),
			[=](const auto &p) { return p.get() == client; });
	if (clientIt == clients.cend())
	{
		// The tag is only set as the result of a make_shared stored in server's client list
		OutputDebugStringA("relayserverinternal::generic_handlerdisconnect(): client not found in server's client list.\n");
		return;
	}
	std::shared_ptr<lacewing::relayserver::client> clientShd = *clientIt;

	clientsocket->tag(nullptr);

	lw_trace("socket closure for %p", this->server.socket);
	//client->socket->close(true);
	//client->disconnect();
	cliWriteLock.lw_unlock();

	if (client->connectRequestApproved && handlerdisconnect && server->hosting())
	{
		// We want count of clients to be accurate for the ondisconnect handler.
		// Note close_client() will also remove it, if it's the else block.
		clients.erase(clientIt);
		serverWriteLock.lw_unlock();

		handlerdisconnect(this->server, clientShd);
	}
	else
		serverWriteLock.lw_unlock();

	close_client(clientShd);

	// should run garbage collect for client...
	//delete &clientinternal;
}

void relayserverinternal::generic_handlerreceive(lacewing::server server, lacewing::server_client clientsocket, std::string_view data)
{
	// Null when closing down server
	auto clientPtr = (relayserver::client *)clientsocket->tag();
	if (!clientPtr)
		return;
	relayserver::client &client = *clientPtr;

	if (!client.gotfirstbyte)
	{
		client.gotfirstbyte = true;

		data.remove_prefix(1);

		// Only one byte long
		if (data.empty())
			return;
	}

	client.reader.process (data.data(), data.size());
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
	internal.generic_handlerudpreceive(udp, address, std::string_view(data, size));
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
 //   socket->nagle ();
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

void relayserver::host(lw_ui16 port)
{
	lacewing::filter filter = lacewing::filter_new();
	filter->local_port(port);

	host(filter);
	filter_delete(filter);
}

void relayserver::host(lacewing::filter &_filter)
{
	// temp copy to override port
	lacewing::filter filter = (lacewing::filter)lw_filter_clone((lw_filter)_filter);

	// Don't let a second server host on this port.
	filter->reuse(false);

	// If port is 0, make it 6121
	if (!filter->local_port())
		filter->local_port(6121);

	socket->host (filter);
	udp->host    (filter);

	lacewing::filter_delete(filter);

	relayserverinternal * serverInternal = (relayserverinternal *)internaltag;
	serverInternal->pingtimer->start(serverInternal->tcpPingMS);
}

void relayserver::unhost()
{
	socket->unhost();
	udp->unhost();

	relayserverinternal* serverInternal = (relayserverinternal*)internaltag;
	serverInternal->pingtimer->stop();

	// This will drop all clients, by doing so drop all channels
	// and both of those will free the IDs
	// We'll set them all as readonly so peer leave messages aren't sent as the clients leave their channels
	for (auto &c : serverInternal->clients)
		c->_readonly = true; // unhost() has already made clients inaccessible

	while (!serverInternal->clients.empty())
	{
		auto& c = serverInternal->clients.back();
		serverInternal->close_client(c);
	}

	// any with autoclose on
	while (!serverInternal->channels.empty())
	{
		auto& c = serverInternal->channels.back();
		c->_readonly = true;
		serverInternal->close_channel(c);
	}
}

bool relayserver::hosting()
{
	return socket->hosting();
}

lw_ui16 relayserver::port()
{
	return (lw_ui16) socket->port();
}

/// <summary> Gracefully closes the channel, including deleting memory, removing channel
/// 		  from server list, and messaging clients. </summary>
void relayserverinternal::close_channel(std::shared_ptr<relayserver::channel> channel)
{
	auto channelWriteLock = channel->lock.createWriteLock();
	if (channel->_readonly)
		return;
	channel->_readonly = true;
	channel->_channelmaster.reset(); // so garbage collection can happen

	// Remove the channel from server's list (if it exists)
	{
		auto serverWriteLock = server.lock.createWriteLock();
		for (auto e3 = channels.begin(); e3 != channels.end(); e3++)
		{
			if (*e3 == channel)
			{
				// LW_ESCALATION_NOTE
				// auto serverWriteLock = serverReadLock.lw_upgrade();
				channels.erase(e3);
				break;
			}
		}
	}

	// Message and remove channel from all clients
	if (!channel->clients.empty())
	{
		framebuilder builder(true);
		builder.addheader(0, 0);   /* response */
		builder.add <lw_ui8>(3);   /* leavechannel */
		builder.add <lw_ui8>(1);   /* success */
		builder.add <lw_ui16>(channel->_id); /* channel ID */

		while (!channel->clients.empty())
		{
			auto& cli = channel->clients[0];
			auto cliWriteLock = cli->lock.createWriteLock();
			if (!cli->_readonly)
				builder.send(cli->socket, false);

			// Go through client's channel list and remove this channel
			for (auto cliJoinedCh = cli->channels.begin(); cliJoinedCh != cli->channels.end(); cliJoinedCh++)
			{
				if (*cliJoinedCh == channel)
				{
					cli->channels.erase(cliJoinedCh);
					break;
				}
			}

			channel->clients.erase(channel->clients.cbegin());
		}
	}
}

void relayserverinternal::close_client (std::shared_ptr<lacewing::relayserver::client> client)
{
	auto clientWriteLock = client->lock.createWriteLock();
	client->_readonly = true;

	while (!client->channels.empty())
	{
		auto clientJoinedCh = client->channels[0];
		auto channelWriteLock = clientJoinedCh->lock.createWriteLock();
		// PHI NOTE 29TH DEC: loop server list of channels, upon match run this code
		// Ensure channel is still open; we rarely get a race condition where it's not
		channel_removeclient(clientJoinedCh, client);
		if (std::find(client->channels.cbegin(), client->channels.cend(), clientJoinedCh) != client->channels.cend())
			LacewingFatalErrorMsgBox();
	}

	// Should be empty; channel_removeclient drops channel from list.
	if (!client->channels.empty())
		LacewingFatalErrorMsgBox();
	clientWriteLock.lw_unlock();

	// LW_ESCALATION_NOTE
	//auto serverReadLock = server.lock.createReadLock();
	auto serverWriteLock = server.lock.createWriteLock();

	// Drop this client from server list (if it exists)
	for (auto cli = clients.begin(); cli != clients.end(); cli++)
	{
		if (*cli == client)
		{
			// LW_ESCALATION_NOTE
			// auto serverWriteLock = serverReadLock.lw_upgrade();
			clients.erase(cli);
			break;
		}
	}
}



void relayserver::channel_addclient(std::shared_ptr<relayserver::channel> channel, std::shared_ptr<relayserver::client> client)
{
	if (channel->_readonly || client->_readonly)
		return;

	return ((relayserverinternal *)internaltag)->channel_addclient(channel, client);
}
void relayserverinternal::channel_addclient(std::shared_ptr<relayserver::channel> channel, std::shared_ptr<relayserver::client> client)
{
	auto channelWriteLock = channel->lock.createWriteLock();
	if (channel->_readonly)
		return;

	for (auto e = channel->clients.begin(); e != channel->clients.end(); e++)
		if (*e == client)
			return; // Nothing to do, client is on channel already

	// LW_ESCALATION_NOTE
	// auto joiningClientReadLock = client->lock.createReadLock();
	auto joiningCliWriteLock = client->lock.createWriteLock();

	// Join channel is OK
	framebuilder builder(true);
	builder.addheader(0, 0);  /* response */
	builder.add <lw_ui8>(2);  /* joinchannel */
	builder.add <lw_ui8>(1);  /* success */
	builder.add <lw_ui8>(channel->_channelmaster == client);  /* whether they are the channel master */

	builder.add <lw_ui8>((lw_ui8)channel->_name.size());
	builder.add(channel->_name.c_str(), channel->_name.size());

	builder.add <lw_ui16>(channel->_id);

	for (const auto &cli : channel->clients)
	{
		auto cliOnChannelReadLock = cli->lock.createReadLock();
		builder.add <lw_ui16>(cli->_id);
		builder.add <lw_ui8>(cli == channel->_channelmaster ? 1 : 0);
		builder.add <lw_ui8>((lw_ui8)cli->_name.size());
		builder.add(cli->_name.c_str(), cli->_name.size());
	}

	{
		// LW_ESCALATION_NOTE
		// auto joiningCliWriteLock = joiningClientReadLock.lw_upgrade();
		builder.send(client->socket); // Send list of peers to joining client
		// LW_ESCALATION_NOTE
		// joiningCliWriteLock.lw_downgrade_to(joiningClientReadLock);
	}

	if (!channel->clients.empty())
	{
		builder.framereset();

		// Send peer join message to already existing peers
		builder.addheader(9, 0); /* peer */

		builder.add <lw_ui16>(channel->_id);
		builder.add <lw_ui16>(client->_id);
		builder.add <lw_ui8>(0); // if there are peers, we can't be creating, so channelmaster always false
		builder.add(client->_name.c_str(), client->_name.size());

		/* notify the other clients on the channel that this client has joined */

		for (const auto& cli : channel->clients)
		{
			if (cli->_readonly)
				continue;

			if (client == cli)
				LacewingFatalErrorMsgBox();

			auto peerWriteLock = cli->lock.createWriteLock();

			if (!cli->_readonly)
				builder.send(cli->socket, false);
		}
	}

	// Add passed client to this channel's list
	channel->clients.push_back(client);

	channelWriteLock.lw_unlock();

	// Add this channel to client's list of joined channels

	// LW_ESCALATION_NOTE
	// auto joiningClientWriteLock = joiningClientReadLock.lw_upgrade();
	client->channels.push_back(channel);
}

void relayserver::channel_removeclient(std::shared_ptr<relayserver::channel> channel, std::shared_ptr<relayserver::client> client)
{
	// readonly checks done in internal
	return ((relayserverinternal *)internaltag)->channel_removeclient(channel, client);
}
void relayserverinternal::channel_removeclient(std::shared_ptr<relayserver::channel> channel, std::shared_ptr<relayserver::client> client)
{
	auto channelWriteLock = channel->lock.createWriteLock();
	auto clientWriteLock = client->lock.createWriteLock();

	// Note: We still have to tell peers if only leaving client is readonly, so keep running.
	if (channel->_readonly /* || client->_readonly */)
		return;

	framebuilder builder(true);

	for (auto e = channel->clients.begin(); e != channel->clients.end(); e++)
	{
		if (*e == client)
		{
			channel->clients.erase (e);

			if (client->_readonly)
				break;

			// Tell client they were ok to leave

			builder.addheader(0, 0);    /* response */
			builder.add <lw_ui8>(3);    /* leavechannel */
			builder.add <lw_ui8>(1);    /* success */
			builder.add <lw_ui16>(channel->_id); /* channel ID */

			builder.send(client->socket);

			builder.framereset();

			break;
		}
	}

	// Drop channel from client's joined channel list
	for (auto e2 = client->channels.begin(); e2 != client->channels.end(); e2++)
	{
		if (*e2 == channel)
		{
			client->channels.erase(e2);
			break;
		}
	}

	// No clients left or master left and autoclose is on
	if (channel->clients.empty() || (channel->_channelmaster == client && channel->_autoclose))
	{
		close_channel(channel); // Sends Channel Leave Success to peers, and drops from server's channel list

		// Should garbage collect for channel, so we want channel write lock off
		channelWriteLock.lw_unlock();
		return;
	}

	// Note: this is where you can assign a different channel master.
	// If you do, don't forget to send a Peer message to change his flags,
	// and check the if statement above if you want to assign a new master
	// when old master leaves.
	if (channel->_channelmaster == client)
		channel->_channelmaster = nullptr;


	/* notify all the other peers that this client has left the channel */

	builder.addheader (9, 0); /* peer */

	builder.add <lw_ui16> (channel->_id);
	builder.add <lw_ui16> (client->_id);

	for (const auto &joinedCli : channel->clients)
	{
		auto joinedCliWriteLock = joinedCli->lock.createWriteLock();
		if (!joinedCli->_readonly)
			builder.send(joinedCli->socket, false);
	}

	builder.framereset();
}

bool relayserver::client::checkname(std::string_view name)
{
	// LW_ESCALATION_NOTE
	// auto cliReadLock = lock.createReadLock();
	auto cliWriteLock = lock.createWriteLock();
	if (name.size() > 255U)
	{
		framebuilder builder(true);

		builder.addheader(0, 0);  /* response */
		builder.add <lw_ui8>(1);  /* setname */
		builder.add <lw_ui8>(0);  /* failed */

		builder.add <lw_ui8>(255);
		builder.add(name.data(), 255);

		builder.add("name too long, 255 chars maximum", -1);

		// LW_ESCALATION_NOTE
		// auto cliWriteLock = cliReadLock.lw_upgrade();
		builder.send(socket);

		return false;
	}

	auto serverReadLock = server.server.lock.createReadLock();
	for (const auto e2 : server.clients)
	{
		if (e2->_readonly)
			continue;

		// Client is this one, don't check if it's already in use
		// OR other client has no name

		// LW_ESCALATION_NOTE
		// auto srvCliReadLock = e2->lock.createReadLock();
		auto srvCliWriteLock = e2->lock.createWriteLock();
		if (e2.get() == this || e2->_readonly || e2->_name.empty())
			continue;

		// Note: case insensitive.
		// Due to self being skipped above, a client is still allowed to rename
		// to a different capitalisation of its current name.
		if (lw_sv_icmp(e2->_name, name))
		{
			framebuilder builder(true);

			builder.addheader (0, 0);  /* response */
			builder.add <lw_ui8> (1);  /* setname */
			builder.add <lw_ui8> (0);  /* failed */

			builder.add <lw_ui8> ((lw_ui8)name.size());
			builder.add (name);

			builder.add ("name already taken", -1);

			// LW_ESCALATION_NOTE
			// auto srvCliWriteLock = srvCliReadLock.lw_upgrade();
			builder.send(socket);

			return false;
		}
	}

	return true;
}

bool relayserverinternal::client_messagehandler(std::shared_ptr<relayserver::client> client, lw_ui8 type, std::string_view messageP, bool blasted)
{
	auto cliReadLock = client->lock.createReadLock();

	unsigned char messagetypeid  = (type >> 4);
	unsigned char variant        = (type << 4);

	variant >>= 4;
	const char * message = messageP.data();
	size_t size = messageP.size();

	messagereader reader (message, size);
	framebuilder builder(true);

	if (messagetypeid != 0 && messagetypeid != 9 && !client->connectRequestApproved)
	{
		// Haven't got a Connect Request message (0) approved yet,
		// so don't listen to any other message types.
		// We also accept Ping messages (type 9).
		// Since this user is sending messages before approval we can safely say we don't want it.

		// Atomic, tell other threads we don't like this client anymore.
		client->_readonly = true;
		lacewing::error error = lacewing::error_new();

		// Note: at this point ID is used from server buffer, but Connect Request hasn't been triggered,
		// so using ID or name will make no sense as there'd be no preceeding Connect Request.

		char addrLocal[64];
		lw_addr_prettystring(client->address.c_str(), addrLocal, sizeof(addrLocal));
		error->add("Dropping connecting client from IP %s for sending messages before connection was approved", addrLocal, 0);

		handlererror(server, error);

		lacewing::error_delete(error);

		// LW_ESCALATION_NOTE
		// lacewing::writelock cliWriteLock = cliReadLock.lw_upgrade();
		cliReadLock.lw_unlock();
		auto cliWriteLock = client->lock.createWriteLock();

		//close();
		client->socket->close();
		// only return false if socket is emergency closing and
		// you cannot trust further message content is readable
		return false;
	}
	if (blasted)
		client->lastudpmessagetime = ::std::chrono::steady_clock::now();
	else
		client->lasttcpmessagetime = ::std::chrono::steady_clock::now();

	std::stringstream errStr;
	bool trustedClient = true;

	switch (messagetypeid)
	{
		case 0: /* request */
		{
			lw_ui8 requesttype = reader.get <lw_ui8> ();

			if (reader.failed)
			{
				errStr << "Incomplete request message.";
				trustedClient = false;
				break;
			}

			// Connect request not approved and user is sending a different type of request...
			if (requesttype != 0 && !client->connectRequestApproved)
			{
				errStr << "Request message of non-Connect type (" << requesttype << ") given when Connect not approved yet.";
				trustedClient = false;
				reader.failed = true;
				break;
			}

			switch (requesttype)
			{
				case 0: /* connect */
				{
					std::string_view version = reader.getremaining ();

					if (reader.failed)
					{
						errStr << "Malformed connect request message received";
						trustedClient = false;
						break;
					}

					if (client->connectRequestApproved)
					{
						errStr << "Error: received connect request but already approved connection. Ignoring.";
						return true;
					}

					if (!lw_sv_cmp(version, "revision 3"))
					{
						builder.addheader (0, 0);  /* response */
						builder.add <lw_ui8> (0);  /* connect */
						builder.add <lw_ui8> (0);  /* failed */
						builder.add ("version mismatch", -1);

						// LW_ESCALATION_NOTE
						// lacewing::writelock cliWriteLock = cliReadLock.lw_upgrade();
						cliReadLock.lw_unlock();
						auto cliWriteLock = client->lock.createWriteLock();

						builder.send(client->socket);

						reader.failed = true;
						errStr << "Version mismatch in connect request";
						break;
					}

					cliReadLock.lw_unlock();

					if (handlerconnect)
						handlerconnect(server, client);
					else
						server.connect_response(client, std::string_view());

					break;
				}

				case 1: /* setname */
				{
					std::string_view name = reader.getremaining (false);
					if (!reader.failed)
					{
						while (!name.empty() && name.back() == '\0')
							name.remove_suffix(1);
					}

					if (reader.failed || name.find_first_of('\0') != std::string_view::npos)
					{
						errStr << "Malformed Set Name request received, name could not be read.";
						reader.failed = true;
						trustedClient = false;
						break;
					}

					cliReadLock.lw_unlock();
					// checkname will grab itself a writelock
					if (!client->checkname (name))
						break; // checkname will make an error, if any

					if (handlernameset)
						handlernameset(server, client, name);
					else
						server.nameset_response(client, name, std::string_view());

					break;
				}

				case 2: /* joinchannel */
				{
					const lw_ui8 flags = reader.get <lw_ui8> ();
					size_t channelnamelength = reader.bytesleft();

					if (channelnamelength > 255)
					{
						builder.addheader(0, 0);  /* response */
						builder.add <lw_ui8>(2);  /* joinchannel */
						builder.add <lw_ui8>(0);  /* failed */

						builder.add <lw_ui8>(255);
						builder.add(reader.get(255));

						builder.add("Channel name too long.", -1);

						cliReadLock.lw_unlock();
						{
							auto cliWriteLock = client->lock.createWriteLock();
							if (!client->_readonly)
								builder.send(client->socket);
						}

						reader.failed = true;

						errStr << "Malformed Join Channel request, name too long.";
						trustedClient = false;

						break;
					}
					std::string_view channelname = reader.get(channelnamelength);

					if (reader.failed || channelname.find_first_of('\0') != std::string_view::npos)
					{
						errStr << "Malformed Join Channel request, name could not be read.";
						reader.failed = true;
						trustedClient = false;
						break;
					}

					std::shared_ptr<relayserver::channel> channel;

					//auto cliReadLock = client->lock.createReadLock();
					for (const auto& e : channels)
					{
						if (lw_sv_icmp (e->_name, channelname))
						{
							channel = e;
							break;
						}
					}
					cliReadLock.lw_unlock();
					std::stringstream dbg;

					/* creating a new channel */
					if (!channel)
					{
						channel = std::make_shared<relayserver::channel>(*this, channelname);

						channel->_channelmaster = client;
						channel->_hidden = (flags & 1) != 0;
						channel->_autoclose = (flags & 2) != 0;
						dbg << "Created new channel " << channelname << ", autoclose " << (channel->_autoclose ? "on" : "off") << "";
					}
					/* joining an existing channel */
					else
					{
						// If client tries to join channel they've already joined
						auto channelReadLock = channel->lock.createReadLock();
						if (std::find(channel->clients.cbegin(), channel->clients.cend(), client) != channel->clients.cend())
						{
							channelReadLock.lw_unlock();
							server.joinchannel_response(channel, client, "You are already on this channel.");
							break;
						}

						dbg << "Joining existing channel " << channelname << "";
					}

					// Until this channel is destroyed by loss of reference, the only resources it uses is a
					// potential reference to client via _channelmaster, and the id number, freed by channel dtor

					if (handlerchannel_join)
						handlerchannel_join(server, client, channel, channel->_hidden, channel->_autoclose);
					else // channel var is either deleted here, or added to server channel list.
						server.joinchannel_response(channel, client, std::string_view());

					break;
				}

				case 3: /* leavechannel */
				{
					std::shared_ptr<lacewing::relayserver::channel> channel;
					lw_ui16 channelid = *(lw_ui16 *)reader.cursor();
					// auto cliReadLock = client->lock.createReadLock();
					channel = client->readchannel(reader);

					if (reader.failed)
					{
						std::shared_ptr<lacewing::relayserver::channel> channelFromServerList;
						{
							auto serverReadLock = server.lock.createReadLock();
							auto channelFromServerListIt = std::find_if(channels.cbegin(), channels.cend(),
								[=](const auto &ch) { return ch->_id == channelid; });
							if (channelFromServerListIt != channels.cend())
								channelFromServerList = *channelFromServerListIt;
						}
						if (channelFromServerList)
						{
							errStr << "Malformed Leave Channel request for client ID " << client->_id << ", name \"" << client->_name << "\", channel ID " <<
								channelid << ", name \""  << channelFromServerList->name() << "\" was not found on client's channel list, but WAS on server channel list. Ignoring";
						}
						else
						{
							errStr << "Malformed Leave Channel request for client ID " << client->_id << ", name \"" << client->_name << "\", channel ID " <<
								channelid << " was not found on client's channel list, or server channel list. Ignoring";
						}

						cliReadLock.lw_unlock();

						// PHI DEBUG NOTE 29TH DEC: Shouldn't send this, if user requests to leave multiple times, it might cause confusion in client.

						framebuilder builder(true);

						builder.addheader(0, 0);  /* response */
						builder.add <lw_ui8>(3);  /* leavechannel */
						builder.add <lw_ui8>(0);  /* failed */
						builder.add <lw_ui16>(channelid); /* channel ID */

						// Blank reason replaced with "it was unspecified" message
						builder.add("Channel ID is not in your client's joined channel list.", -1);

						auto cliWriteLock = client->lock.createWriteLock();
						builder.send(client->socket);

						break;
					}

					cliReadLock.lw_unlock();

					if (handlerchannel_leave)
						handlerchannel_leave(server, client, channel);
					else // Auto-approve. Handles channel deletion.
						server.leavechannel_response(channel, client, std::string_view());

					break;
				}

				case 4: /* channellist */

					if (!channellistingenabled)
					{
						builder.addheader        (0, 0);  /* response */
						builder.add <lw_ui8> (4);  /* channellist */
						builder.add <lw_ui8> (0);  /* failed */

						builder.add ("channel listing is not enabled on this server", -1);

						cliReadLock.lw_unlock();
						{
							auto cliWriteLock = client->lock.createWriteLock();
							if (!client->_readonly)
								builder.send(client->socket);
						}

						break;
					}

					builder.addheader (0, 0);  /* response */
					builder.add <lw_ui8> (4);  /* channellist */
					builder.add <lw_ui8> (1);  /* success */

					for (const auto& e : channels)
					{
						auto chLoopReadLock = e->lock.createReadLock();
						if (e->_hidden)
							continue;

						builder.add <lw_ui16> ((lw_ui16)e->clients.size());
						builder.add <lw_ui8>  ((lw_ui8)e->_name.size());
						builder.add (e->_name.c_str(), e->_name.size());
					}

					cliReadLock.lw_unlock();
					{
						auto cliWriteLock = client->lock.createWriteLock();
						if (!client->_readonly)
							builder.send(client->socket);
					}

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
			cliReadLock.lw_unlock();

			const lw_ui8 subchannel = reader.get <lw_ui8> ();
			std::string_view message3 = reader.getremaining();

			if (reader.failed)
			{
				errStr << "Malformed server message received";
				trustedClient = false;
				break;
			}

			if (handlermessage_server)
			{
				handlermessage_server(server, client, blasted, subchannel, message3, variant);

				// Server messages, we'll assume it is activity.
				client->lastchannelorpeermessagetime = ::std::chrono::steady_clock::now();
			}

			break;
		}

		case 2: /* binarychannelmessage */
		{
			const lw_ui8 subchannel = reader.get <unsigned char> ();
			lw_ui16 channelid = *(lw_ui16 *)reader.cursor();
			auto channel = client->readchannel (reader);

			std::string_view message2 = reader.getremaining();

			if (reader.failed)
			{
				if (channel)
				{
					errStr << "Malformed channel message content, for client ID " << client->_id << ", name \"" << client->_name << "\", channel ID " <<
						channelid << ", name \"" << channel->name() << "\", discarding";
				}
				else
				{
					std::shared_ptr<lacewing::relayserver::channel> channelFromServerList;
					{
						auto serverReadLock = server.lock.createReadLock();
						auto channelFromServerListIt = std::find_if(channels.cbegin(), channels.cend(),
							[=](const auto &ch) { return ch->_id == channelid; });
						if (channelFromServerListIt != channels.cend())
							channelFromServerList = *channelFromServerListIt;
					}
					if (channelFromServerList)
					{
						errStr << "Malformed channel message content, for client ID " << client->_id << ", name \"" << client->_name << "\", channel ID " <<
							channelid << ", name \"" << channelFromServerList->name() << "\" was not found on client's channel list, but WAS on server channel list, discarding";
					}
					else
					{
						errStr << "Malformed channel message content, for client ID " << client->_id << ", name \"" << client->_name << "\", channel ID " <<
							channelid << " was not found on client's channel list, or server channel list, discarding";
					}
				}
				break;
			}
			cliReadLock.lw_unlock();

			// Channel messages not sent to anyone is not activity.
			if (channel->clientcount() > 1)
				client->lastchannelorpeermessagetime = ::std::chrono::steady_clock::now();

			if (handlermessage_channel)
				handlermessage_channel(server, client, channel,
					blasted, subchannel, message2, variant);
			else
				server.channelmessage_permit(client, channel,
					blasted, subchannel, message2, variant, true);

			break;
		}

		case 3: /* binarypeermessage */
		{
			const lw_ui8 subchannel		= reader.get <unsigned char> ();
			auto channel	= client->readchannel(reader);
			auto peer		= channel->readpeer(reader);

			// Message to yourself? Witchcraft!
			if (peer == client || peer == nullptr)
			{
				errStr << "Malformed peer message (invalid peer targeted), discarding";
				reader.failed = true;
				break;
			}

			cliReadLock.lw_unlock();

			const char * message2;
			size_t size2;
			reader.getremaining(message2, size2);
			std::string_view message3(message2, size2);

			if (reader.failed)
			{
				errStr << "Couldn't read content of peer message, discarding";
				break;
			}

			client->lastchannelorpeermessagetime = ::std::chrono::steady_clock::now();

			if (handlermessage_peer)
				handlermessage_peer(server, client, channel,
					peer, blasted, subchannel, message3, variant);
			else
				server.clientmessage_permit(client, channel, peer,
					blasted, subchannel, message3, variant, true);

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
		{
			// UDPHello on non-UDP port... what
			if (!blasted)
			{
				errStr << "UDPHello message sent on TCP, not allowed";
				trustedClient = false;
				reader.failed = true;
				break;
			}

			cliReadLock.lw_unlock();
			auto cliWriteLock = client->lock.createWriteLock();
			if (client->_readonly)
				break;

			client->pseudoUDP = false;

			builder.addheader (10, 0); /* udpwelcome */
			builder.send      (server.udp, client->udpaddress);

			break;
		}
		case 8: /* channelmaster */
			errStr << "Channel master message ID 8 not allowed";


			break;

		case 9: /* ping */
			if (!blasted)
				client->pongedOnTCP = true;
			break;

		case 10: /* implementation response */
		{
			std::string_view impl = reader.get(reader.bytesleft());
			if (reader.failed || impl.empty())
			{
				errStr << "Failed to read implementation response";
				trustedClient = false;
				break;
			}

			// LW_ESCALATION_NOTE
			cliReadLock.lw_unlock();
			auto cliWriteLock = client->lock.createWriteLock();
			if (client->_readonly)
				break;

			// Implementation responses were added in Bluewing Client build 70.

			if (impl.find("Windows") != std::string_view::npos)
			{
				if (impl.find("Unicode") != std::string_view::npos)
					client->clientImpl = relayserver::client::clientimpl::Windows_Unicode;
				else
					client->clientImpl = relayserver::client::clientimpl::Windows;
			}
			else if (impl.find("Android") != std::string_view::npos)
				client->clientImpl = relayserver::client::clientimpl::Android;
			else if (impl.find("Flash") != std::string_view::npos)
				client->clientImpl = relayserver::client::clientimpl::Flash;
			else if (impl.find("iOS") != std::string_view::npos)
				client->clientImpl = relayserver::client::clientimpl::iOS;
			else if (impl.find("Macintosh") != std::string_view::npos)
				client->clientImpl = relayserver::client::clientimpl::Macintosh;
			else if (impl.find("HTML5") != std::string_view::npos)
				client->clientImpl = relayserver::client::clientimpl::HTML5;
			else
			{
				errStr << "Failed to recognise platform of implementation \"" << impl << "\". Leaving it as Unknown.";
				reader.failed = true;
			}

			client->clientImplStr = impl;
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
		error->add("Reader failed!", client->_id);
		error->add("%s", errStr.str().c_str());
		if (!trustedClient)
			error->add("Booting client");

		lw_trace(error->tostring());
		handlererror(server, error);

		lacewing::error_delete(error);

		if (!trustedClient)
		{
			client->_readonly = true;

			// LW_ESCALATION_NOTE
			// auto cliWriteLock = cliReadLock.lw_upgrade();
			if (cliReadLock.isEnabled())
				cliReadLock.lw_unlock();
			auto cliWriteLock = client->lock.createWriteLock();

			client->socket->close(true); // immediate disconnect
		}

		// only return false if socket is emergency closing and
		// you cannot trust further message content is readable
		return trustedClient;
	}

	return true;
}

void relayserver::client::send(lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	framebuilder builder(true);

	builder.addheader (1, variant); /* binaryservermessage */
	builder.add (subchannel);
	builder.add (message);

	auto clientWriteLock = lock.createWriteLock();
	if (!_readonly)
		builder.send (socket);
}

void relayserver::client::blast(lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	framebuilder builder(false);

	builder.addheader(1, variant, true); /* binaryservermessage */
	builder.add (subchannel);
	builder.add (message);

	auto serverWriteLock = server.server.lock.createWriteLock();
	auto clientReadLock = lock.createReadLock();
	if (!_readonly)
		builder.send (server.server.udp, udpaddress);
}

void relayserver::channel::send(lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	framebuilder builder(true);

	builder.addheader (4, variant); /* binaryserverchannelmessage */
	builder.add <lw_ui8>(subchannel);
	builder.add <lw_ui16>(_id);
	builder.add (message);

	auto channelReadLock = lock.createReadLock();
	if (_readonly)
		return;

	for (const auto& e : clients)
	{
		auto clientReadLock = e->lock.createWriteLock();
		if (!e->_readonly)
			builder.send(e->socket, false);
	}
}

void relayserver::channel::blast(lw_ui8 subchannel, std::string_view message, lw_ui8 variant)
{
	framebuilder builder(false);

	builder.addheader (4, variant, true); /* binaryserverchannelmessage */
	builder.add (subchannel);
	builder.add (this->_id);
	builder.add (message);

	auto channelReadLock = lock.createReadLock();
	if (_readonly)
		return;

	auto serverWriteLock = server.server.lock.createWriteLock();
	for (const auto& e : clients)
	{
		auto clientReadLock = e->lock.createWriteLock();
		if (!e->_readonly)
			builder.send(server.server.udp, e->udpaddress, false);
	}
}

/// <summary> Throw all clients off this channel, sending Leave Request Success. </summary>
void relayserver::channel::close()
{
	auto serverReadLock = server.server.lock.createReadLock();
	auto ch = std::find_if(server.channels.begin(),
		server.channels.end(),
		[=](const auto &p) { return p.get() == this; });

	// Assume channel is already closed, as it's not on server channel list.
	if (ch == server.channels.end())
		return;

	serverReadLock.lw_unlock();
	server.close_channel(*ch);
}

relayserver::client::client(relayserverinternal &internal, lacewing::server_client _socket)
	: socket(_socket), server(internal),
	udpaddress(lacewing::address_new(socket->address()))
{
	//public_.internaltag = this;
	tag = 0;
	address = socket->address()->tostring();
	addressInt = socket->address()->toin6_addr();

	_id = internal.clientids.borrow();

	reader.tag = this;
	reader.messagehandler = &relayserverinternal::tcpmessagehandler;

	connectRequestApproved = false;
	pongedOnTCP = true;
	gotfirstbyte = false;
	pseudoUDP = false;

	clientImpl = clientimpl::Unknown;

	// connectTime not started until handshake is done
	// last message can be considered the connect time
	lastudpmessagetime = lastchannelorpeermessagetime = lasttcpmessagetime = ::std::chrono::steady_clock::now();
}

::lacewing::relayserver::channel::channel(relayserverinternal &_server, std::string_view _name) :
	server(_server), _name(_name)
{
	_id = server.channelids.borrow();
}

relayserver::channel::~channel() noexcept(false)
{
	auto channelWriteLock = lock.createWriteLock();

	for (const auto& i : clients)
	{
		auto readLock = i->lock.createReadLock();
		for (auto j = i->channels.begin(); j != i->channels.end(); j++)
		{
			if ((**j)._id == _id)
			{
				// LW_ESCALATION_NOTE
				readLock.lw_unlock();
				auto writeLock = i->lock.createWriteLock();
				i->channels.erase(j);
				break;
			}
		}
	}

	// Unlike Client, where clients are "peers" and have a new copy per channel,
	// server retains one copy of the client, for all the channels.
	// So we don't delete the clients in the channel dtor.
	clients.clear();

	if (_id == 0xFFFF)
		throw std::exception("ID already released");
	server.channelids.returnID(_id);
	_id = 0xFFFF;
}

lw_ui16 relayserver::client::id()
{
	// ID doesn't change, so we don't need readlock
	return _id;
}

lw_ui16 relayserver::channel::id()
{
	// ID doesn't change, so we don't need readlock
	return _id;
}

std::string relayserver::channel::name() const
{
	lacewing::readlock rl = lock.createReadLock();
	return _name;
}

// Renames channel.
// WARNING: Does not check if channel name is in use already.
void relayserver::channel::name(std::string_view name)
{
	if (_readonly)
		return;
	lacewing::writelock wl = lock.createWriteLock();
	_name = name;
}

bool relayserver::channel::hidden() const
{
	lacewing::readlock rl = lock.createReadLock();
	return _hidden;
}

bool relayserver::channel::readonly() const
{
	return _readonly;
}

bool relayserver::channel::autocloseenabled() const
{
	lacewing::readlock rl = lock.createReadLock();
	return _autoclose;
}

std::vector<std::shared_ptr<lacewing::relayserver::client>>& relayserver::channel::getclients()
{
	lock.checkHoldsRead();
	return clients;
}


void relayserver::setwelcomemessage(std::string_view message)
{
	relayserverinternal& serverinternal = *(relayserverinternal *)internaltag;
	serverinternal.welcomemessage = message;
}

std::string relayserver::getwelcomemessage()
{
	lacewing::readlock rl = lock.createReadLock();
	return ((relayserverinternal *)internaltag)->welcomemessage;
}

void relayserver::setchannellisting (bool enabled)
{
	lacewing::writelock wl = lock.createWriteLock();
	((relayserverinternal *) internaltag)->channellistingenabled = enabled;
}

std::shared_ptr<relayserver::client> relayserver::channel::channelmaster() const
{
	lacewing::readlock rl = lock.createReadLock();
	return _channelmaster;
}

void relayserver::client::disconnect()
{
	_readonly = true;

	lacewing::writelock wl = lock.createWriteLock();
	if (socket && socket->valid())
		socket->close();
}

std::string relayserver::client::name() const
{
	lacewing::readlock rl = lock.createReadLock();
	return _name;
}

void relayserver::client::name(std::string_view name)
{
	lacewing::writelock wl = lock.createWriteLock();
	_prevname = _name;
	_name = name;
}

bool relayserver::client::readonly() const
{
	return _readonly;
}

std::vector<std::shared_ptr<lacewing::relayserver::channel>> & relayserver::client::getchannels()
{
	lock.checkHoldsRead();
	return channels;
}

size_t relayserver::channelcount() const
{
	lacewing::readlock rl = lock.createReadLock();
	return ((relayserverinternal *) internaltag)->channels.size();
}

std::vector<std::shared_ptr<lacewing::relayserver::client>> & relayserver::getclients()
{
	lock.checkHoldsRead();
	return ((lacewing::relayserverinternal *)internaltag)->clients;
}
std::vector<std::shared_ptr<lacewing::relayserver::channel>>& relayserver::getchannels()
{
	lock.checkHoldsRead();
	return ((lacewing::relayserverinternal *)internaltag)->channels;
}


size_t relayserver::channel::clientcount() const
{
	lacewing::readlock rl = lock.createReadLock();
	return clients.size();
}

size_t relayserver::client::channelcount() const
{
	lacewing::readlock rl = lock.createReadLock();
	return channels.size();
}

using namespace ::std::chrono;

lw_i64 relayserver::client::getconnecttime() const
{
	// No need for read lock: connect time is set in ctor
	steady_clock::time_point end = steady_clock::now();
	auto time = end - connectTime;
	return duration_cast<seconds>(time).count();
}

size_t relayserver::clientcount() const
{
	lacewing::readlock rl = lock.createReadLock();
	return ((relayserverinternal *)internaltag)->clients.size();
}

relayserver::client::~client() noexcept(false)
{
	writelock wl = lock.createWriteLock();
	//lw_trace("~relayserver::client called for address %p, name %s, ID %hu\n", this, _name.c_str(), _id);

	channels.clear();
	clientImplStr.clear();

	server.clientids.returnID(_id);

	lacewing::address_delete(udpaddress);
	udpaddress = nullptr;

	// When refcount for the stream reaches 0, the stream will be freed.
	// Note lw_stream_delete does not free, as the IO Completion port might still have
	// pending reads/writes that will try to access the freed memory.
	//
	lw_trace("Should now delete stream %p.", socket);
	socket = nullptr;
}

std::shared_ptr<relayserver::channel> relayserver::createchannel(std::string_view channelName, std::shared_ptr<relayserver::client> master, bool hidden, bool autoclose)
{
	auto channel = std::make_shared<relayserver::channel>(*(lacewing::relayserverinternal *)internaltag, channelName);
	auto channelWriteLock = channel->lock.createWriteLock();

	channel->_channelmaster = master;
	channel->_hidden = hidden;
	channel->_autoclose = autoclose;

	joinchannel_response(channel, master, std::string_view());
	// calls serverinternal.channels.push_back(channel);
	channelWriteLock.lw_unlock();

	return channel;
}

/// <summary> Responds to a connect request. Pass null for deny reason if approving.
/// 		  You MUST run this event even if denying, or you will have a connection open and a memory leak. </summary>
/// <param name="client">	  [in] The client. Deleted if not approved. </param>
/// <param name="denyReason"> The deny reason. If null, request is approved. </param>
void relayserver::connect_response(
	std::shared_ptr<relayserver::client> client, std::string_view passedDenyReason)
{
	relayserverinternal &serverI = *(relayserverinternal *)this->internaltag;

	if (client->_readonly)
		return;

	auto cliWriteLock = client->lock.createWriteLock();

	if (client->_readonly)
		return;
	if (client->connectRequestApproved)
	{
		lacewing::error error = lacewing::error_new();
		error->add("connect_response closing early, already approved connection for client ID %i", client->_id, 1);
		serverI.handlererror(*this, error);
		lacewing::error_delete(error);
		return;
	}

	framebuilder builder(true);

	// Force a connect refusal if not hosting serevr
	std::string_view denyReason = passedDenyReason;
	if (denyReason.empty() && !hosting())
		denyReason = "Server has shut down.";

	// Connect request denied
	if (!denyReason.empty())
	{
		builder.addheader(0, 0);  /* response */
		builder.add <lw_ui8>(0);  /* connect */
		builder.add <lw_ui8>(0);  /* failed */
		builder.add(denyReason);

		builder.send(client->socket);
		client->disconnect();

		//delete client;
		return;
	}

	// Connect request accepted

	lw_trace("Connect request accepted in relayserver::connectresponse");
	client->connectRequestApproved = true;
	client->connectTime = std::chrono::steady_clock::now();
	client->clientImpl = relayserver::client::clientimpl::Unknown;

	builder.addheader(0, 0);  /* response */
	builder.add <lw_ui8>(0);  /* connect */
	builder.add <lw_ui8>(1);  /* success */

	builder.add <lw_ui16>(client->_id);
	builder.add(serverI.welcomemessage.c_str(), serverI.welcomemessage.size());

	builder.send(client->socket);

	// Now accepted earlier
	// serverI.clients.push_back(client);

	builder.framereset();

	// Send request implementation: this is the best time to send it

	builder.addheader(12, 0);  /* request implementation */
	// response on 10, only responded to by Bluewing b70+, Relay just ignores it

	builder.send(client->socket);
}

// Validates the StringView, or replaces it with the given other one.
static void validateorreplacestringview(std::string_view toValidate,
	std::string_view &writeTo,
	std::string_view replaceWith,
	std::string_view functionName, std::string_view paramName,
	relayserverinternal &serverI, std::shared_ptr<relayserver::client> client)
{
	if (toValidate.find_first_of('\0') == std::string_view::npos)
		return;

	// Later, we'll add UTF-8 validation here too.

	// Don't let the server get away with it!
	lacewing::error error = lacewing::error_new();
	error->add("Error in %s response: Embedded null chars not allowed in param %s", functionName.data(), paramName.data());
	serverI.handlererror(serverI.server, error);
	lacewing::error_delete(error);
	writeTo = replaceWith;
}
/// <summary> Approves or sends a deny response to channel join request. Pass null for deny reason if approving.
/// 		  Even if you're denying, you still MUST call this event, or you will have a memory leak.
/// 		  For new channels, this will add them to server's channel list if approved, or delete them. </summary>
/// <param name="channel">			    [in] The channel. Name is as originally requested. </param>
/// <param name="passedNewChannelName"> Name of the passed channel. If null, original request name is approved.
/// 									If non-null, must be 1-255 chars, or the channel join is denied entirely. </param>
/// <param name="client">			    [in] The client joining/creating the channel. </param>
/// <param name="denyReason">		    The deny reason. If null, the channel is approved (if new channel name is legal).
/// 									If non-null, channel join deny is sent, and channel is cleaned up as needed. </param>
void relayserver::joinchannel_response(std::shared_ptr<relayserver::channel> channel,
	std::shared_ptr<relayserver::client> client, std::string_view denyReason)
{

	// We can't take out the channel argument, as autoclose and hidden settings will be lost too.
	// At some point we could alter this, grant the server control over whether autoclose is turned on/off.
	// But as there's no way to inform the client that is was created with autoclose or not, and
	// the fact that it's not really a useful feature, this is shelved.

	// Hidden can be made pointless by disabling channel listing, and
	// autoclose can be run manually (e.g. on channel leave, and no clients left, close channel)

	relayserverinternal &serverinternal = *(relayserverinternal *)this->internaltag;

	lacewing::readlock channelReadLock = channel->lock.createReadLock();

	// Shouldn't happen... closed channels shouldn't be in responses...?
	if (channel->_readonly)
	{
		denyReason = "Channel has been closed. Try again in a few seconds.";

		lacewing::error error = lacewing::error_new();
		error->add("Join channel attempt on closed channel was refused");
		serverinternal.handlererror(*this, error);
		lacewing::error_delete(error);
	}

	// Attempting to join channel to
	if (std::find(channel->clients.cbegin(), channel->clients.cend(), client) != channel->clients.cend())
		denyReason = "You are already on this channel.";

	lacewing::writelock clientWriteLock = client->lock.createWriteLock();
	if (client->_readonly)
		return; // No response possible


	// If non-empty, request denied.
	if (!denyReason.empty())
	{
		validateorreplacestringview(denyReason, denyReason, "Server specified an invalid reason",
			"Join Channel", "deny reason", serverinternal, client);

		framebuilder builder(true);
		builder.addheader(0, 0);  /* response */
		builder.add <lw_ui8>(2);  /* joinchannel */
		builder.add <lw_ui8>(0);  /* failed */

		// actual length 1-255, checked by channel ctor
		builder.add <lw_ui8>((lw_ui8)channel->_name.size());
		builder.add(channel->_name.c_str(), channel->_name.size());
		builder.add(denyReason);
		builder.send(client->socket);

		// A shared pointer will be destroyed upon close?
		lw_trace("Channel %s should be auto-destroyed...\n", channel->_name.c_str());

		// Join request for new channel; request refused, so channel needs to be dropped.
		// Can't use channelinternal client count, autoclose may be off.
		//if (!channelIsInServerList)
		//	delete channelinternal;
		return;
	}

	lacewing::writelock serverWriteLock = lock.createWriteLock();
	if (std::find(serverinternal.channels.cbegin(), serverinternal.channels.cend(), channel) == serverinternal.channels.cend())
		serverinternal.channels.push_back(channel);

	// LW_ESCALATION_NOTE
	channelReadLock.lw_unlock();
	//lacewing::writelock channelWriteLock = channel->lock.createWriteLock();
	// writelock made by channel_addclient
	serverinternal.channel_addclient(channel, client);
}

/// <summary> Approves or sends a deny response to channel leave request. Pass null for deny reason if approving.
/// 		  Even if you're denying, you still MUST call this event, or you will have a memory leak. </summary>
/// <param name="channel">		[in] The channel.  </param>
/// <param name="client">		[in] The client leaving the channel. </param>
/// <param name="denyReason">	The deny reason. If null, the channel is approved (if new channel name is legal).
/// 							If non-null, channel join deny is sent, and channel is cleaned up as needed. </param>
void relayserver::leavechannel_response(std::shared_ptr<relayserver::channel> channel,
	std::shared_ptr<relayserver::client> client, std::string_view denyReason)
{
	relayserverinternal &serverinternal = *(relayserverinternal *)this->internaltag;

	// If non-empty, request denied
	if (!denyReason.empty())
	{
		validateorreplacestringview(denyReason, denyReason,
			"Channel leave refused by server for unspecified reason",
			"Leave Channel", "deny reason", serverinternal, client);

		framebuilder builder(true);

		builder.addheader(0, 0);         /* response */
		builder.add <lw_ui8>(3);  /* leavechannel */
		builder.add <lw_ui8>(0);  /* failed */
		builder.add <lw_ui16>(channel->_id); /* channel ID */

		// Blank reason replaced with "it was unspecified" message
		builder.add(denyReason);

		builder.send(client->socket);

		return;
	}

	lacewing::writelock wl = channel->lock.createWriteLock();
	serverinternal.channel_removeclient(channel, client);
}

// These two functions allow access to internal transmissions: PeerToChannel, PeerToPeer.
// The server shouldn't theoretically be able to fake a message from a client.
// The server's handlerclientmessage/channelmessage is responsible for cloning message content if
// the response is asynchronous to the request (different call stack).

void relayserver::channelmessage_permit(std::shared_ptr<relayserver::client> sendingclient, std::shared_ptr<relayserver::channel> channel,
	bool blasted, lw_ui8 subchannel, std::string_view data, lw_ui8 variant, bool accept)
{
	if (!accept || channel->_readonly || sendingclient->_readonly)
		return;
	channel->PeerToChannel(*this, sendingclient, blasted, subchannel, variant, data);
}

void relayserver::clientmessage_permit(std::shared_ptr<relayserver::client> sendingclient, std::shared_ptr<relayserver::channel> channel,
	std::shared_ptr<relayserver::client> receivingclient,
	bool blasted, lw_ui8 subchannel, std::string_view data, lw_ui8 variant, bool accept)
{
	if (!accept || channel->_readonly || receivingclient->_readonly)
		return;
	sendingclient->PeerToPeer(*this, channel, receivingclient, blasted, subchannel, variant, data);
}

void relayserver::nameset_response(std::shared_ptr<relayserver::client> client,
	std::string_view newClientName, std::string_view passedDenyReason)
{
	// LW_ESCALATION_NOTE
	// lacewing::readlock clientReadLock = client->lock.createReadLock();
	lacewing::writelock clientWriteLock = client->lock.createWriteLock();
	if (client->_readonly)
		return;

	// We use an altered denyReason if there's newClientName problems.
	char newDenyReason[250];
	std::string_view denyReason = passedDenyReason;
	auto &serverinternal = *(relayserverinternal *)internaltag;

	if (newClientName.empty())
	{
		static char const * const end = "pproved client name is null or empty. Name refused.";
		if (denyReason != nullptr)
			sprintf_s(newDenyReason, "%s\r\nPlus a%s", denyReason.data(), end);
		else
			sprintf_s(newDenyReason, "A%s", end);
		denyReason = newDenyReason;
	}
	else
	{

		if (newClientName.size() > 255U)
		{
			sprintf_s(newDenyReason, "New client name \"%.10s...\" (%u chars) is too long. Name must be 255 chars maximum.", newClientName.data(), (std::uint32_t)newClientName.size());
			denyReason = newDenyReason;
			newClientName = newClientName.substr(0, 255);
		}
	}

	std::string_view oldClientName = client->_name;

	// If not already denying, check and potentially deny if name is invalid.
	if (denyReason.empty())
	{
		validateorreplacestringview(newClientName, denyReason, "Name is invalid",
			"Client Name Set", "client name", serverinternal, client);

		// User attempted to name set to what they had already.
		// Note it's case sensitive, so same name with different caps causes no error.

		// Old Blue and usual Relay behaviour was to send a name set success response,
		// but this could potentially be abused.

		if (denyReason.empty() &&
			!oldClientName.empty() && !strcmp(newClientName.data(), oldClientName.data()))
		{
			denyReason = "Name set to what it was before";
		}
	}


	framebuilder builder(true);
	if (!denyReason.empty())
	{
		builder.addheader(0, 0);  /* response */
		builder.add <lw_ui8>(1);  /* setname */
		builder.add <lw_ui8>(0);  /* failed */

		builder.add <lw_ui8>((lw_ui8)newClientName.size());
		builder.add(newClientName);

		builder.add(denyReason);

		// LW_ESCALATION_NOTE
		// auto clientWriteLock = clientReadLock.lw_upgrade();
		if (!client->_readonly)
			builder.send(client->socket);
		return;
	}

	// check the new name provided by the handler
	// Checks that name is not blank, or used by another client
	if (client->checkname(newClientName))
		client->name(newClientName);
	else // Name check failed
	{
		builder.addheader(0, 0);  /* response */
		builder.add <lw_ui8>(1);  /* setname */
		builder.add <lw_ui8>(0);  /* failed */

		builder.add <lw_ui8>((lw_ui8)newClientName.size());
		builder.add(newClientName);

		builder.add("Name refused by server; the server customised your name "
			"and got an error doing so on its end.", -1);

		{
			// LW_ESCALATION_NOTE
			// auto clientWriteLock = clientReadLock.lw_upgrade();
			if (!client->_readonly)
				builder.send(client->socket);
		}

		auto error = lacewing::error_new();
		error->add("Cannot assign the name you altered the Set Name request to");
		serverinternal.handlererror(*this, error);
		return;
	}

	// Send Name Set Success

	builder.addheader(0, 0);  /* response */
	builder.add <lw_ui8>(1);  /* setname */
	builder.add <lw_ui8>(1);  /* success */

	builder.add <lw_ui8>((lw_ui8)newClientName.size());
	builder.add (newClientName);

	{
		// LW_ESCALATION_NOTE
		// auto clientWriteLock = clientReadLock.lw_upgrade();
		builder.send(client->socket);

		// Should keep read lock for peer messaging
		// LW_ESCALATION_NOTE
		// clientWriteLock.lw_downgrade_to(clientReadLock);
	}

	// if (nameWasAlreadyThat)
	//	return;

	// Send Peer (Name Change) message

	// Send peer name change messages
	for (const auto& e : client->channels)
	{
		builder.addheader(9, 0); /* peer */

		builder.add <lw_ui16>(e->_id);
		builder.add <lw_ui16>(client->_id);
		builder.add <lw_ui8>(client == e->_channelmaster ? 1 : 0);
		builder.add (newClientName);

		for (const auto& e2 : e->clients)
		{
			// Don't message yourself or readonly clients
			if (e2 == client || e2->_readonly)
				continue;

			auto peerWriteLock = e2->lock.createWriteLock();
			if (!e2->_readonly)
				builder.send(e2->socket, false);
		}

		builder.framereset();
	}
}

void relayserver::channel::PeerToChannel(relayserver &server, std::shared_ptr<relayserver::client> client,
	bool blasted, lw_ui8 subchannel, lw_ui8 variant, std::string_view message)
{
	//auto channelReadLock = lock.createReadLock();
	auto channelWriteLock = lock.createWriteLock();

	// Sending to no one or just self, no point
	if (clients.size() <= 1)
		return;

	if (_readonly)
		return;

	framebuilder builder(!blasted);
	builder.addheader(2, variant, blasted); /* binarychannelmessage */

	builder.add <lw_ui8>(subchannel);
	builder.add <lw_ui16>(this->_id);
	builder.add <lw_ui16>(client->_id);
	builder.add (message);

	// Loop through and send message to all clients that aren't this one

	// Only need server write lock for shared lw_udp socket
	auto serverWriteLock = server.lock.createWriteLock();
	if (!blasted)
		serverWriteLock.lw_unlock();

	for (const auto& e : clients)
	{
		if (e == client)
			continue;

		auto cliWriteLock = e->lock.createWriteLock();
		if (e->_readonly)
			continue;

		if (blasted)
			builder.send(server.udp, e->udpaddress, false);
		else
			builder.send(e->socket, false);
	}

	builder.framereset();
}


#define autohandlerfunctions(pub, intern, handlername)              \
	void pub::on##handlername(pub::handler_##handlername handler)   \
		{   ((intern *) internaltag)->handler##handlername = handler;      \
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
