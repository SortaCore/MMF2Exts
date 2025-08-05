#include "Common.hpp"

static char errtext[1024];
static void ErrNoToErrText()
{
	int error = errno; // strerror_s may change errno
#ifdef _WIN32
	if (strerror_s(errtext, error))
	{
		strcpy_s(errtext, std::size(errtext), "errno failed to convert");
		errno = error;
	}
#else
	char * strError = strerror(error);
	if (strError == NULL)
	{
		strcpy(errtext, "errno failed to convert");
		errno = error;
	}
	else
		strcpy(errtext, strError);
#endif
}

#define EventsToRun globals->_eventsToRun
#define Remake(name) DarkEdif::MsgBox::Error(_T("Remake action"), _T("Your "#name" actions need to be recreated.\r\n") \
						_T("This is probably due to parameter changes."))

void Extension::Replaced_Connect(const TCHAR * hostname, int port)
{
	Remake("Connect");
}
void Extension::Disconnect()
{
	Cli.disconnect(); // calls LacewingHandler.cpp's OnDisconnect, so clear-all event is scheduled there

	// While it's neat to have all the old messages processed prior to disconnect,
	// as shared_ptr will keep client/channel readable, it's safe to say that
	// if the user has asked to disconnect, they don't care about pending messages.
	//
	// Note that we don't clear pending messages if the server disconnects us, though.
	// Otherwise, if the server sends a text message explanation then disconnects the client,
	// the explanation message would be discarded.
	globals->lock.edif_lock();

	// Keep specific event IDs 0-3, which are error, connect, connect denied, disconnect; erase the rest.
	globals->_eventsToRun.erase(
		std::remove_if(globals->_eventsToRun.begin(), globals->_eventsToRun.end(),
			[](const auto &e) { return e->condTrig[0] > 3; }),
		globals->_eventsToRun.end()
	);
	globals->lock.edif_unlock();
}
void Extension::SetName(const TCHAR * name)
{
	if (name[0] == _T('\0'))
		return CreateError("Set Name was called with name \"\".");

	if (!Cli.connected())
		return CreateError("Connect to a server before setting name.");

	std::string nameU8(DarkEdif::TStringToUTF8(name));
	if (!lw_u8str_normalize(nameU8))
		return CreateError("Set Name was called with malformed name \"%s\".", nameU8.c_str());

	Cli.name(nameU8);
}
void Extension::Replaced_JoinChannel(const TCHAR * channelName, int hideChannel)
{
	Remake("Join channel");
}
void Extension::LeaveChannel()
{
	if (selChannel && !selChannel->readonly())
		selChannel->leave();
}
void Extension::SendTextToServer(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Send Text to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else
		Cli.sendserver(subchannel, DarkEdif::TStringToUTF8(textToSend), 0);
}
void Extension::SendTextToChannel(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Send Text to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Send Text to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Send Text to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else
		selChannel->send(subchannel, DarkEdif::TStringToUTF8(textToSend), 0);
}
void Extension::SendTextToPeer(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Send Text to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Send Text to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Send Text to Peer was called with read-only peer \"%s\".", selPeer->name().c_str());
	else
		selPeer->send(subchannel, DarkEdif::TStringToUTF8(textToSend), 0);
}
void Extension::SendNumberToServer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Send Number to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else
		Cli.sendserver(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::SendNumberToChannel(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Send Number to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Send Number to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Send Number to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else
		selChannel->send(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::SendNumberToPeer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Send Number to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Send Number to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Send Number to Peer was called with a read-only peer \"%s\".", selPeer->name().c_str());
	else
		selPeer->send(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::BlastTextToServer(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Text to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else
	{
		const std::string utf8Msg = DarkEdif::TStringToUTF8(textToSend);
		if (utf8Msg.size() > globals->maxUDPSize)
			return CreateError("Blast Text to Server was called with text too large (%zu bytes).", utf8Msg.size());
		Cli.blastserver(subchannel, utf8Msg, 0);
	}
}
void Extension::BlastTextToChannel(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Text to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Blast Text to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Blast Text to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else
	{
		const std::string utf8Msg = DarkEdif::TStringToUTF8(textToSend);
		if (utf8Msg.size() > globals->maxUDPSize)
			return CreateError("Blast Text to Channel was called with text too large (%zu bytes).", utf8Msg.size());
		selChannel->blast(subchannel, utf8Msg, 0);
	}
}
void Extension::BlastTextToPeer(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Text to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Blast Text to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Blast Text to Peer was called with read-only peer \"%s\".", selPeer->name().c_str());
	else
	{
		const std::string utf8Msg = DarkEdif::TStringToUTF8(textToSend);
		if (utf8Msg.size() > globals->maxUDPSize)
			return CreateError("Blast Text to Peer was called with text too large (%zu bytes).", utf8Msg.size());
		selPeer->blast(subchannel, utf8Msg, 0);
	}
}
void Extension::BlastNumberToServer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Number to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else
		Cli.blastserver(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::BlastNumberToChannel(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Number to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Blast Number to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Blast Number to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else
		selChannel->blast(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::BlastNumberToPeer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Number to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Blast Number to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Blast Number to Peer was called with read-only peer \"%s\".", selPeer->name().c_str());
	else
		selPeer->blast(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::SelectChannelWithName(const TCHAR * channelName)
{
	const std::string channelNameU8 = DarkEdif::TStringToUTF8(channelName);

	// For reselecting in new channel
	// auto origPeerId = selPeer ? selPeer->id() : -1;

	selChannel = nullptr;
	selPeer = nullptr;
	{
		auto cliReadLock = Cli.lock.createReadLock();
		const auto &channels = Cli.getchannels();
		const std::string channelNameU8Simplified = lw_u8str_simplify(channelNameU8);
		auto foundChIt = std::find_if(channels.cbegin(), channels.cend(),
			[&](const auto & ch) {
				return lw_sv_cmp(ch->namesimplified(), channelNameU8Simplified);
		});

		if (foundChIt != channels.cend())
		{
			selChannel = *foundChIt;

			/* Attempt to reselect the selected peer?
			if (origPeerId != -1)
			{
				cliReadLock.lw_unlock();
				auto chReadLock = selChannel->lock.createReadLock();
				const auto & peers = selChannel->getpeers();
				auto foundPeerIt = std::find_if(peers.cbegin(), peers.cend(),
					[origPeerId](const auto & p) {
						return p->id() == origPeerId;
					});
				if (foundPeerIt != peers.cend())
					selPeer = *foundPeerIt;
			}*/
			return;
		}
	}

	// Only modify selected channel if we found it
	CreateError("Could not select channel, channel name \"%s\" not found.", channelNameU8.c_str());
}
void Extension::ReplacedNoParams()
{
	Remake("Unknown action");
}
void Extension::LoopClientChannels()
{
	const auto origSelChannel = selChannel;
	const auto origSelPeer = selPeer;
	const auto origLoopName = loopName;

	std::vector<decltype(selChannel)> channelListDup;
	{
		auto cliReadLock = Cli.lock.createReadLock();
		channelListDup = Cli.getchannels(); // duplicate list
	}

	for (const auto &ch : channelListDup)
	{
		selChannel = ch;
		selPeer = nullptr;
		loopName = std::tstring_view();
		Runtime.GenerateEvent(14);
	}

	selChannel = origSelChannel;
	selPeer = origSelPeer;
	loopName = std::tstring_view();
	Runtime.GenerateEvent(18);

	loopName = origLoopName;
}
void Extension::SelectPeerOnChannelByName(const TCHAR * peerName)
{
	if (peerName[0] == _T('\0'))
		return CreateError("Select Peer On Channel By Name was called with a blank name.");
	if (!selChannel)
		return CreateError("Select Peer On Channel By Name was called without a channel being selected.");

	selPeer = nullptr;
	{
		const std::string peerNameU8Simplified = TStringToUTF8Simplified(peerName);
		auto chReadLock = selChannel->lock.createReadLock();
		const auto & peers = selChannel->getpeers();

		auto foundPeerIt = std::find_if(peers.cbegin(), peers.cend(),
			[&](const auto & p) {
				return lw_sv_cmp(p->namesimplified(), peerNameU8Simplified);
			});
		if (foundPeerIt != peers.cend())
		{
			selPeer = *foundPeerIt;
			return;
		}
	}

	CreateError("Peer with name %s not found on channel %s.", DarkEdif::TStringToUTF8(peerName).c_str(), selChannel->name().c_str());
}
void Extension::SelectPeerOnChannelByID(int peerID)
{
	if (peerID < 0 || peerID > 0xFFFE)
		return CreateError("Select Peer On Channel By ID was called with ID %i, which is not in valid range of 0 - 65535.", peerID);
	if (!selChannel)
		return CreateError("Select Peer On Channel By ID was called without a channel being selected.");

	selPeer = nullptr;
	{
		auto channelReadLock = selChannel->lock.createReadLock();
		const auto & peers = selChannel->getpeers();
		auto foundPeerIt = std::find_if(peers.cbegin(), peers.cend(),
			[=](const auto & p) { return p->id() == peerID;
		});

		// Only modify selPeer if we found it
		if (foundPeerIt != peers.cend())
		{
			selPeer = *foundPeerIt;
			return;
		}
	}

	CreateError("Peer with ID %i not found on selected channel \"%s\".", peerID, selChannel->name().c_str());
}
void Extension::LoopPeersOnChannel()
{
	// Store selected channel
	if (!selChannel)
		return CreateError("Loop Peers On Channel was called without a channel being selected.");

	const auto origSelChannel = selChannel;
	const auto origSelPeer = selPeer;
	const auto origLoopName = loopName;

	std::vector<decltype(selPeer)> peerListDup;
	{
		auto channelReadLock = selChannel->lock.createReadLock();
		peerListDup = selChannel->getpeers();
	}

	for (const auto &peer : peerListDup)
	{
		selChannel = origSelChannel;
		selPeer = peer;
		loopName = std::tstring_view();
		Runtime.GenerateEvent(13);
	}

	selChannel = origSelChannel;
	selPeer = origSelPeer;
	loopName = std::tstring_view();
	Runtime.GenerateEvent(17);

	loopName = origLoopName;
}
void Extension::RequestChannelList()
{
	Cli.listchannels();
}
void Extension::LoopListedChannels()
{
	const auto origLoopName = loopName;
	const auto origChannelList = threadData->channelListing;

	std::vector<decltype(threadData->channelListing)> channelListingDup;
	{
		auto clientReadLock = Cli.lock.createReadLock();
		channelListingDup = Cli.getchannellisting();
	}

	for (const auto &chLst : channelListingDup)
	{
		threadData->channelListing = chLst;
		loopName = std::tstring_view();
		Runtime.GenerateEvent(27);
	}

	threadData->channelListing = origChannelList;
	loopName = std::tstring_view();
	Runtime.GenerateEvent(28);

	loopName = origLoopName;
}
void Extension::SendBinaryToServer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Send Binary to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else
		Cli.sendserver(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		SendMsg_Clear();
}
void Extension::SendBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Send Binary to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Send Binary to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Send Binary to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else
		selChannel->send(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		SendMsg_Clear();
}
void Extension::SendBinaryToPeer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Send Binary to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Send Binary to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Send Binary to Peer was called with a read-only peer.");
	else
		selPeer->send(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		SendMsg_Clear();
}
void Extension::BlastBinaryToServer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Binary to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (SendMsgSize > globals->maxUDPSize)
		CreateError("Blast Binary to Server was called with binary too large (%zu bytes).", SendMsgSize);
	else
		Cli.blastserver(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		SendMsg_Clear();
}
void Extension::BlastBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Binary to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Blast Binary to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Blast Binary to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else if (SendMsgSize > globals->maxUDPSize)
		CreateError("Blast Binary to Channel was called with binary too large (%zu bytes).", SendMsgSize);
	else
		selChannel->blast(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		SendMsg_Clear();
}
void Extension::BlastBinaryToPeer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Blast Binary to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Blast Binary to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Blast Binary to Peer was called with a read-only peer.");
	else if (SendMsgSize > globals->maxUDPSize)
		CreateError("Blast Binary to Peer was called with binary too large (%zu bytes).", SendMsgSize);
	else
		selPeer->blast(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		SendMsg_Clear();
}
void Extension::SendMsg_AddASCIIByte(const TCHAR * byte)
{
	const std::string u8Str(DarkEdif::TStringToUTF8(byte));
	if (u8Str.size() != 1)
		return CreateError("Adding ASCII character to binary failed: byte \"%s\" supplied was part of a string, not a single byte.", u8Str.c_str());

	// ANSI byte, not ASCII; or not displayable, so probably a corrupt string.
	if (reinterpret_cast<const std::uint8_t &>(u8Str[0]) > 127 || !std::isprint(u8Str[0]))
		return CreateError("Adding ASCII character to binary failed: byte \"%u\" was not a valid ASCII character.", (unsigned int) reinterpret_cast<const std::uint8_t &>(u8Str[0]));

	SendMsg_Sub_AddData(u8Str.c_str(), sizeof(char));
}
void Extension::SendMsg_AddByteInt(int byte)
{
	if (byte > UINT8_MAX || byte < INT8_MIN)
	{
		return CreateError("Adding byte to binary (as int) failed: the supplied number %i will not fit in range "
			"%i to %i (signed byte) or range 0 to %i (unsigned byte).", byte, INT8_MIN, INT8_MAX, UINT8_MAX);
	}

	SendMsg_Sub_AddData(&byte, sizeof(char));
}
void Extension::SendMsg_AddShort(int _short)
{
	if (_short > UINT16_MAX || _short < INT16_MIN)
	{
		return CreateError("Adding short to binary failed: the supplied number %i will not fit in range "
			"%i to %i (signed short) or range 0 to %i (unsigned short).", _short, INT16_MIN, INT16_MAX, UINT16_MAX);
	}

	SendMsg_Sub_AddData(&_short, sizeof(short));
}
void Extension::SendMsg_AddInt(int _int)
{
	SendMsg_Sub_AddData(&_int, sizeof(int));
}
void Extension::SendMsg_AddFloat(float _float)
{
	SendMsg_Sub_AddData(&_float, sizeof(float));
}
void Extension::SendMsg_AddStringWithoutNull(const TCHAR * string)
{
	if (!IsValidPtr(string))
		return CreateError("Adding string without null terminator failed: string address %p supplied was invalid.", string);

	const std::string stringU8(DarkEdif::TStringToUTF8(string));
	SendMsg_Sub_AddData(stringU8.c_str(), stringU8.size());
}
void Extension::SendMsg_AddString(const TCHAR *string)
{
	if (!IsValidPtr(string))
		return CreateError("Adding string failed: string address %p supplied was invalid.", string);

	const std::string stringU8(DarkEdif::TStringToUTF8(string));
	SendMsg_Sub_AddData(stringU8.c_str(), stringU8.size() + 1);
}
void Extension::SendMsg_AddBinaryFromAddress(unsigned int address, int size)
{
	// Address is checked in SendMsg_Sub_AddData()
	if (size < 0)
		return CreateError("Add binary failed: Size %i is less than 0.", size);

	SendMsg_Sub_AddData((void *)(long)address, size);
}
void Extension::SendMsg_Clear()
{
	if (SendMsg)
	{
		free(SendMsg);
		SendMsg = NULL;
	}
	SendMsgSize = 0;
}
void Extension::RecvMsg_SaveToFile(int position, int size, const TCHAR * filename)
{
	if (position < 0)
		return CreateError("Cannot save received binary to file; supplied position %i is less than 0.", position);
	if (size <= 0)
		return CreateError("Cannot save received binary to file; supplied size %i is equal or less than 0.", size);
	if (filename[0] == _T('\0'))
		return CreateError("Cannot save received binary to file; supplied filename \"\" is invalid.");

	if (((unsigned int)position) + size > threadData->receivedMsg.content.size())
		return CreateError("Cannot save received binary to file; message doesn't the supplied position range %i to %i.", position, position + size);

#ifdef _WIN32
	FILE * File = _tfsopen(filename, _T("wb"), SH_DENYWR);
#else
	FILE * File = fopen(filename, "wb");
#endif

	if (!File)
	{
		ErrNoToErrText();
		return CreateError("Cannot save received binary to file \"%s\", error number %i \"%s\" occurred with opening the file.", DarkEdif::TStringToUTF8(filename).c_str(), errno, errtext);
	}

	size_t amountWritten;
	if ((amountWritten = fwrite(threadData->receivedMsg.content.data() + position, 1, size, File)) != size)
	{
		ErrNoToErrText();
		CreateError("Cannot save received binary to file, error %i \"%s\" occurred with writing the file. Wrote %zu"
			" bytes total. The message has not been modified.", errno, errtext, amountWritten);
	}

	if (fclose(File))
	{
		ErrNoToErrText();
		CreateError("Cannot save received binary to file, error %i \"%s\" occurred with writing the last part of the file."
			" The message has not been modified.", errno, errtext);
	}
}
void Extension::RecvMsg_AppendToFile(int position, int size, const TCHAR * filename)
{
	if (position < 0)
		return CreateError("Cannot append received binary to file; supplied position %i is less than 0.", position);
	if (size <= 0)
		return CreateError("Cannot append received binary to file; supplied size %i is equal or less than 0.", size);
	if (filename[0] == _T('\0'))
		return CreateError("Cannot append received binary to file; supplied filename \"\" is invalid.");

	if (((unsigned int)position) + size > threadData->receivedMsg.content.size())
		return CreateError("Cannot append received binary to file; message doesn't have the supplied index range %i to %i.", position, position + size);

	// Open while denying write of other programs
#ifdef _WIN32
	FILE * File = _tfsopen(filename, _T("ab"), SH_DENYWR);
#else
	FILE * File = fopen(filename, "ab");
#endif
	if (!File)
	{
		ErrNoToErrText();
		return CreateError("Cannot append received binary to file, error %i \"%s\" occurred with opening file \"%s\".", errno, errtext, DarkEdif::TStringToUTF8(filename).c_str());
	}

	size_t amountWritten;
	if ((amountWritten = fwrite(threadData->receivedMsg.content.data() + position, 1, size, File)) != size)
	{
		fseek(File, 0, SEEK_END);
#ifdef _WIN32
		std::int64_t filesize = _ftelli64(File);
#else
		std::int64_t filesize = ftell(File);
#endif
		ErrNoToErrText();
		CreateError("Cannot append received binary to file \"%s\", error %i \"%s\" occurred with writing the file. "
			"Wrote %zu bytes, leaving file at size %" PRId64 " bytes.", DarkEdif::TStringToUTF8(filename).c_str(), errno, errtext, amountWritten, filesize);
	}

	if (fclose(File))
		CreateError("Cannot append received binary to file \"%s\", error number %i occurred with writing last part of the file.", DarkEdif::TStringToUTF8(filename).c_str(), errno);
}
void Extension::SendMsg_AddFileToBinary(const TCHAR * filenameParam)
{
	if (filenameParam[0] == _T('\0'))
		return CreateError("Cannot add file to send binary; supplied filename \"\" is invalid.");

	// Unembed file if necessary
	const std::tstring filename = DarkEdif::MakePathUnembeddedIfNeeded(this, filenameParam);
	if (filename[0] == _T('>'))
	{
		return CreateError("Cannot add file \"%s\" to send binary, error %s occurred with opening the file."
			" The send binary has not been modified.",
			DarkEdif::TStringToUTF8(filenameParam).c_str(), DarkEdif::TStringToUTF8(filename.substr(1)).c_str());
	}

	// Open and deny other programs write privileges
#ifdef _WIN32
	FILE * File = _tfsopen(filename.c_str(), _T("rb"), SH_DENYWR);
#else
	FILE * File = fopen(filename.c_str(), "rb");
#endif
	if (!File)
	{
		ErrNoToErrText();
		return CreateError("Cannot add file \"%s\" (original \"%s\") to send binary, error number %i \"%s\" occurred with opening the file.",
			DarkEdif::TStringToUTF8(filename).c_str(), DarkEdif::TStringToUTF8(filenameParam).c_str(), errno, errtext);
	}

	// Jump to end
	fseek(File, 0, SEEK_END);

	// Read current position as file size
	long filesize = ftell(File);

	// Go back to start
	fseek(File, 0, SEEK_SET);

	std::byte * buffer = (std::byte *)malloc(filesize);
	if (!buffer)
	{
		CreateError("Couldn't read file \"%s\" into binary to send; couldn't reserve %li bytes of memory to add file into message.",
			DarkEdif::TStringToUTF8(filenameParam).c_str(), filesize);
	}
	else
	{
		size_t amountRead;
		if ((amountRead = fread_s(buffer, filesize, 1, filesize, File)) != filesize)
		{
			CreateError("Couldn't read file \"%s\" into binary to send; reading file caused error %i \"%s\".",
				DarkEdif::TStringToUTF8(filenameParam).c_str(), errno, errtext);
		}
		else
			SendMsg_Sub_AddData(buffer, amountRead);

		free(buffer);
	}
	fclose(File);
}
void Extension::SelectChannelMaster()
{
	if (!selChannel)
		return CreateError("Could not select channel master: no channel selected.");

	selPeer = nullptr;

	// Can't select self as a peer
	if (selChannel->ischannelmaster())
		return CreateError("Could not select channel master as peer: you're the channel master.");

	auto channelReadLock = selChannel->lock.createReadLock();
	const auto &peers = selChannel->getpeers();
	auto masterIt = std::find_if(peers.cbegin(), peers.cend(),
		[](const auto &p) { return p->ischannelmaster(); });

	// master could have left if autoclose is off
	if (masterIt != peers.cend())
		selPeer = *masterIt;
}
void Extension::JoinChannel(const TCHAR * channelName, int hidden, int closeAutomatically)
{
	const std::string channelNameU8(DarkEdif::TStringToUTF8(channelName));
	if (channelName[0] == _T('\0'))
		return CreateError("Cannot join channel: invalid channel name %s supplied.", channelNameU8.c_str());
	Cli.join(channelNameU8, hidden != 0, closeAutomatically != 0);
}
void Extension::SendMsg_CompressBinary()
{
	if (SendMsgSize <= 0)
		return CreateError("Cannot compress send binary; message is too small.");
	int ret;
	z_stream strm = {};
	ret = deflateInit(&strm, 9); // 9 is maximum compression level
	if (ret)
		return CreateError("Zlib error %i: %s occurred with initiating compression.", ret, strm.msg ? "No details" : strm.msg);

	// 4: precursor lw_ui32 with uncompressed size, required by Relay
	// 256: if compression results in larger message, it shouldn't be *that* much larger.

	unsigned char * output_buffer = (unsigned char *)malloc(4 + SendMsgSize +  256);
	if (!output_buffer)
	{
		deflateEnd(&strm);
		return CreateError("Error with compressing send binary, could not allocate %zu bytes of memory.", 4U + SendMsgSize + 256U);
	}

	// Store size as precursor - required by Relay
	*(lw_ui32 *)output_buffer = (lw_ui32)SendMsgSize;

	strm.next_in = (unsigned char *)SendMsg;
	strm.avail_in = (std::uint32_t)SendMsgSize;

	// Allocate memory for compression
	strm.avail_out = (std::uint32_t)SendMsgSize + 256;
	strm.next_out = output_buffer + 4;

	ret = deflate(&strm, Z_FINISH);
	if (ret != Z_STREAM_END)
	{
		const char *strmMsg = strm.msg ? strm.msg : "(no description)";
		free(output_buffer);
		deflateEnd(&strm);
		return CreateError("Error with compressing send binary, deflate() returned %i. Zlib error: %s.", ret, strmMsg);
	}

	deflateEnd(&strm);

	char * output_bufferResize = (char *)realloc(output_buffer, 4 + strm.total_out);
	if (!output_bufferResize)
	{
		free(output_buffer); // realloc will not free on error
		CreateError("Error with compressing send binary, reallocating memory to remove excess space after compression failed with error %i.", errno);
		return;
	}

	free(SendMsg);

	SendMsg = output_bufferResize;
	SendMsgSize = 4 + strm.total_out;
}
void Extension::RecvMsg_DecompressBinary()
{
	if (threadData->receivedMsg.content.size() <= 4)
		return CreateError("Cannot decompress received binary; message is too small.");

	z_stream strm = { };
	int ret = inflateInit(&strm);
	if (ret)
	{
		const char * strmMsg = strm.msg ? strm.msg : "(no description)";
		return CreateError("Decompression failed; error %d: %s with initiating decompression.", ret, strmMsg);
	}

	// Lacewing provides a precursor to the compressed data, with uncompressed size.
	lw_ui32 expectedUncompressedSize = *(lw_ui32 *)threadData->receivedMsg.content.data();
	if (expectedUncompressedSize > 0x0F000000U)
		return CreateError("Decompression failed; message anticipated to be too large. Expected %u byte output.", expectedUncompressedSize);

	const std::string_view inputData(threadData->receivedMsg.content.data() + sizeof(lw_ui32), threadData->receivedMsg.content.size() - sizeof(lw_ui32));

	// Has exception support
#if !defined(__clang__) || defined(__EXCEPTIONS)
	std::unique_ptr<unsigned char[]> output_buffer;
	try {
		output_buffer = std::make_unique<unsigned char[]>(expectedUncompressedSize);
	}
	catch (std::bad_alloc)
	{
		inflateEnd(&strm);
		return CreateError("Decompression failed; could not allocate enough memory. Requested %u bytes.", expectedUncompressedSize);
	}
#else
	std::unique_ptr<unsigned char[]> output_buffer = std::make_unique<unsigned char[]>(expectedUncompressedSize);
#endif

	strm.next_in = (unsigned char *)inputData.data();
	strm.avail_in = (std::uint32_t)inputData.size();
	strm.avail_out = expectedUncompressedSize;
	strm.next_out = output_buffer.get();
	ret = inflate(&strm, Z_FINISH);
	if (ret < Z_OK)
	{
		const char *strmMsg = strm.msg ? strm.msg : "(no description)";
		inflateEnd(&strm);
		return CreateError("Error with decompression, inflate() returned error %i. Zlib description: %s.", ret, strmMsg);
	}

	inflateEnd(&strm);

	// Used to assign all exts in a questionable way, but threadData is now std::shared_ptr, so no need.
	threadData->receivedMsg.content.assign((char *)output_buffer.get(), expectedUncompressedSize);
	threadData->receivedMsg.cursor = 0;
}
void Extension::RecvMsg_MoveCursor(int position)
{
	if (position < 0)
		return CreateError("Cannot move cursor; Position %d is less than 0.", position);
	if (threadData->receivedMsg.content.size() - position <= 0)
		return CreateError("Cannot move cursor to position %d; message indexes are 0 to %zu.", position, threadData->receivedMsg.content.size());

	threadData->receivedMsg.cursor = position;
}
void Extension::LoopListedChannelsWithLoopName(const TCHAR * passedLoopName)
{
	if (passedLoopName[0] == _T('\0'))
		return CreateError("Cannot loop listed channels: invalid loop name \"\" supplied.");

	const std::tstring_view loopNameDup(passedLoopName);
	const auto origLoopName = loopName;
	const auto origChannelList = threadData->channelListing;

	std::vector<decltype(threadData->channelListing)> channelListingDup;
	{
		auto cliReadLock = Cli.lock.createReadLock();
		channelListingDup = Cli.getchannellisting();
	}

	for (const auto &chLst : channelListingDup)
	{
		threadData->channelListing = chLst;
		loopName = loopNameDup;
		Runtime.GenerateEvent(59);
	}

	threadData->channelListing = nullptr;
	loopName = loopNameDup;
	Runtime.GenerateEvent(60);

	loopName = origLoopName;
}
void Extension::LoopClientChannelsWithLoopName(const TCHAR * passedLoopName)
{
	if (passedLoopName[0] == _T('\0'))
		return CreateError("Cannot loop client channels: invalid loop name \"\" supplied.");

	const std::tstring_view loopNameDup(passedLoopName);
	const auto origLoopName = loopName;
	const auto origSelChannel = selChannel;
	const auto origSelPeer = selPeer;

	std::vector<decltype(selChannel)> channelListDup;
	{
		auto cliReadLock = Cli.lock.createReadLock();
		channelListDup = Cli.getchannels();
	}

	for (const auto &ch : channelListDup)
	{
		selChannel = ch;
		selPeer = nullptr;
		loopName = loopNameDup;
		Runtime.GenerateEvent(63);
	}

	selChannel = origSelChannel;
	selPeer = origSelPeer;
	loopName = loopNameDup;
	Runtime.GenerateEvent(64);

	loopName = origLoopName;
}
void Extension::LoopPeersOnChannelWithLoopName(const TCHAR * passedLoopName)
{
	if (passedLoopName[0] == _T('\0'))
		return CreateError("Cannot loop peers on channel: invalid loop name \"\" supplied.");
	if (!selChannel)
		return CreateError("Cannot loop peers on channel: no channel currently selected.");

	const std::tstring_view loopNameDup(passedLoopName);
	const auto origSelChannel = selChannel;
	const auto origSelPeer = selPeer;
	const auto origLoopName = loopName;
	std::vector<decltype(selPeer)> peerListDup;
	{
		auto channelReadLock = selChannel->lock.createReadLock();
		peerListDup = selChannel->getpeers();
	}

	for (const auto &peer : peerListDup)
	{
		selChannel = origSelChannel;
		selPeer = peer;
		loopName = loopNameDup;
		Runtime.GenerateEvent(61);
	}

	selChannel = origSelChannel;
	selPeer = origSelPeer;
	loopName = loopNameDup;
	Runtime.GenerateEvent(62);

	loopName = origLoopName;
}
void Extension::Connect(const TCHAR * hostname)
{
	if (hostname[0] == _T('\0'))
		return CreateError("Cannot connect to server: invalid hostname supplied.");

	int Port = 6121;
	std::string hostnameU8(DarkEdif::TStringToUTF8(hostname));
	const std::size_t colonCount = std::count(hostnameU8.cbegin(), hostnameU8.cend(), ':');
	const bool isIPv6Box = hostnameU8.find('[') != std::string::npos;
	const TCHAR* portPtr = (colonCount == 1 || isIPv6Box) ? _tcsrchr(hostname, _T(':')) : NULL;
	// IPv4/hostname, or IPv6 [ip]:port layout - with check for : not being index 0
	if (portPtr && (colonCount == 1 || (isIPv6Box && portPtr > hostname && *(portPtr - 1) == _T(']'))))
	{
		Port = _ttoi(portPtr + 1);

		if (Port <= 0 || Port > 0xFFFF)
			return CreateError("Invalid port in hostname (%s). Ports are limited from 1 to 65535.", DarkEdif::TStringToUTF8(portPtr + 1).c_str());
	}
	Cli.connect(hostnameU8.c_str(), Port);
}
void Extension::SendMsg_Resize(int newSize)
{
	if (newSize < 0)
		return CreateError("Cannot change size of binary to send: new size is under 0 bytes.");

	char * NewMsg = (char *)realloc(SendMsg, newSize);
	if (!NewMsg)
	{
		return CreateError("Cannot change size of binary to send: reallocation of memory failed. Size has not been modified.");
	}
	// Clear new bytes to 0
	if ((size_t)newSize > SendMsgSize)
		memset(NewMsg + SendMsgSize, 0, newSize - SendMsgSize);

	SendMsg = NewMsg;
	SendMsgSize = newSize;
}
void Extension::SetDestroySetting(int enabled)
{
	if (enabled > 1 || enabled < 0)
		return CreateError("Invalid setting passed to SetDestroySetting, expecting 0 or 1.");
	globals->fullDeleteEnabled = enabled != 0;
}
void Extension::SetLocalPortForHolePunch(int port)
{
	if (port < 1 || port > std::numeric_limits<unsigned short>::max())
		return CreateError("Invalid local port passed, expecting 1 through 65535, got %d.", port);
	globals->_client.setlocalport(globals->localPort = (unsigned short)port);
}
