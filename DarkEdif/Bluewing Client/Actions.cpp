#include "Common.h"

#ifdef _DEBUG
std::stringstream CriticalSection;
#endif

static char errtext[1024];
void ErrNoToErrText()
{
	int error = errno; // strerror_s may change errno
	if (strerror_s(errtext, error))
	{
		strcpy_s(errtext, "errno failed to convert");
		_set_errno(error);
	}
}

#define Remake(name) MessageBoxA(NULL, "Your "#name" actions need to be recreated.\r\n" \
										"This is probably due to parameter changes.", "Lacewing Blue Client", MB_OK)
#define Saved (globals->_saved)

void Extension::Replaced_Connect(const TCHAR * hostname, int port)
{
	Remake("Connect");
}
void Extension::Disconnect()
{
	Cli.disconnect(); // calls OnDisconnect so clear-all 0xFFFF is done there
}
void Extension::SetName(const TCHAR * name)
{
	if (name[0] == _T('\0'))
		return CreateError("Error: Set Name was called with name \"\".");

	std::string nameU8(TStringToUTF8(name));
	if (!lw_u8str_normalise(nameU8))
		return CreateError("Error: Set Name was called with malformed name \"%s\".", nameU8.c_str());

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
		CreateError("Error: Send Text to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else
		Cli.sendserver(subchannel, TStringToUTF8(textToSend));
}
void Extension::SendTextToChannel(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Text to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Error: Send Text to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Send Text to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else
		selChannel->send(subchannel, TStringToUTF8(textToSend));
}
void Extension::SendTextToPeer(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Text to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Error: Send Text to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Error: Send Text to Peer was called with read-only peer \"%s\".", selPeer->name().c_str());
	else
		selPeer->send(subchannel, TStringToUTF8(textToSend));
}
void Extension::SendNumberToServer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Number to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else
		Cli.sendserver(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::SendNumberToChannel(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Number to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Error: Send Number to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Send Number to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else
		selChannel->send(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::SendNumberToPeer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Number to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Error: Send Number to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Error: Send Number to Peer was called with a read-only peer \"%s\".", selPeer->name().c_str());
	else
		selPeer->send(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::BlastTextToServer(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Text to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else
		Cli.blastserver(subchannel, TStringToUTF8(textToSend));
}
void Extension::BlastTextToChannel(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Text to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Error: Blast Text to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Blast Text to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else
		selChannel->blast(subchannel, TStringToUTF8(textToSend));
}
void Extension::BlastTextToPeer(int subchannel, const TCHAR * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Text to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Error: Blast Text to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Error: Blast Text to Peer was called with read-only peer \"%s\".", selPeer->name().c_str());
	else
		selPeer->blast(subchannel, TStringToUTF8(textToSend));
}
void Extension::BlastNumberToServer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Number to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else
		Cli.blastserver(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::BlastNumberToChannel(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Number to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Error: Blast Number to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Blast Number to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else
		selChannel->blast(subchannel, std::string_view((char *)&numToSend, 4), 1);
}
void Extension::BlastNumberToPeer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Number to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Error: Blast Number to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Error: Blast Number to Peer was called with read-only peer \"%s\".", selPeer->name().c_str());
	else
		selPeer->blast(subchannel, std::string_view((char *)&numToSend, 4), 1);
}
void Extension::SelectChannelWithName(const TCHAR * channelName)
{
	std::string channelNameU8 = TStringToUTF8(channelName);
	selChannel = nullptr;
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
	auto origSelChannel = selChannel;
	auto origSelPeer = selPeer;
	auto origLoopName = loopName;

	std::vector<decltype(selChannel)> channelListDup;
	{
		auto cliReadLock = Cli.lock.createReadLock();
		channelListDup = Cli.getchannels(); // duplicate list
	}
	// size_t peerID = selPeer ? selPeer->id() : MAXSIZE_T;

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
		return CreateError("Error: Select Peer On Channel By Name was called with a blank name.");
	if (!selChannel)
		return CreateError("Error: Select Peer On Channel By Name was called without a channel being selected.");

	selPeer = nullptr;
	{
		const std::string peerNameU8Simplified = TStringToUTF8Stripped(peerName);
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

	CreateError("Peer with name %s not found on channel %s.", TStringToUTF8(peerName).c_str(), selChannel->name().c_str());
}
void Extension::SelectPeerOnChannelByID(int peerID)
{
	if (peerID < 0 || peerID > 0xFFFE)
		return CreateError("Error: Select Peer On Channel By ID was called with ID %i, which is not in valid range of 0 - 65535.", peerID);
	if (!selChannel)
		return CreateError("Error: Select Peer On Channel By ID was called without a channel being selected.");

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
		return CreateError("Error: Loop Peers On Channel was called without a channel being selected.");

	auto origSelChannel = selChannel;
	auto origSelPeer = selPeer;
	auto origLoopName = loopName;
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
	auto origLoopName = loopName;
	auto origChannelList = threadData->channelListing;
	std::vector<decltype(threadData->channelListing)> channelListingDup;
	{
		auto channelReadLock = selChannel->lock.createReadLock();
		channelListingDup = Cli.getchannellisting();
	}
	for (const auto &chLst : channelListingDup)
	{
		threadData->channelListing = chLst;
		loopName = std::tstring_view();
		Runtime.GenerateEvent(27);
	}

	threadData->channelListing = nullptr;
	loopName = std::tstring_view();
	Runtime.GenerateEvent(28);
	threadData->channelListing = origChannelList;
	loopName = origLoopName;
}
void Extension::SendBinaryToServer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Binary to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else
		Cli.sendserver(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Binary to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Error: Send Binary to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Send Binary to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else
		selChannel->send(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToPeer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Binary to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Error: Send Binary to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Error: Send Binary to Peer was called with a read-only peer.");
	else
		selPeer->send(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToServer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Binary to Server was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else
		Cli.blastserver(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Binary to Channel was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selChannel)
		CreateError("Error: Blast Binary to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Blast Binary to Channel was called with read-only channel \"%s\".", selChannel->name().c_str());
	else
		selChannel->blast(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToPeer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Binary to Peer was called with invalid subchannel %i; it must be between 0 and 255.", subchannel);
	else if (!selPeer)
		CreateError("Error: Blast Binary to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Error: Blast Binary to Peer was called with a read-only peer.");
	else
		selPeer->blast(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::AddByteText(const TCHAR * byte)
{
	if (_tcsnlen(byte, 2) != 1)
		return CreateError("Adding byte to stack failed: byte \"%s\" supplied was part of a string, not a single byte.", TStringToUTF8(byte).c_str());
	std::string u8Str(TStringToUTF8(byte));
	AddToSend(u8Str.c_str(), u8Str.size());
}
void Extension::AddByteInt(int byte)
{
	if (byte > 255 || byte < -128)
		return CreateError("Error: Byte out of bounds.");
	if (byte < 0)
	{
		char RealByte = (char)byte;
		AddToSend(&RealByte, 1);
	}
	else
	{
		unsigned char RealByte = (unsigned char)byte;
		AddToSend(&RealByte, 1);
	}
}
void Extension::AddShort(int _short)
{
	if (_short > MAXUINT16 || _short < MININT16)
		return CreateError("Error: Short out of bounds.");
	if (_short < 0)
	{
		short RealShort = (short)_short;
		AddToSend(&RealShort, 2);
	}
	else
	{
		unsigned short RealShort = (unsigned short)_short;
		AddToSend(&RealShort, 2);
	}
}
void Extension::AddInt(int _int)
{
	AddToSend(&_int, 4);
}
void Extension::AddFloat(float _float)
{
	AddToSend(&_float, 4);
}
void Extension::AddStringWithoutNull(const TCHAR * string)
{
	std::string stringU8(TStringToUTF8(string));
	if (string)
		AddToSend(stringU8.c_str(), stringU8.size());
	else
		CreateError("Adding string without null failed: pointer was null.");
}
void Extension::AddString(const TCHAR *string)
{
	std::string stringU8(TStringToUTF8(string));
	if (string)
		AddToSend(stringU8.c_str(), stringU8.size() + 1);
	else
		CreateError("Adding string failed: pointer was null.");
}
void Extension::AddBinary(unsigned int address, int size)
{
	if (size < 0)
		return CreateError("Add binary failed: Size less than 0.");

	if (size != 0)
		AddToSend((void *)(long)address, size);
	// else do nothing
}
void Extension::ClearBinaryToSend()
{
	if (SendMsg)
	{
		free(SendMsg);
		SendMsg = NULL;
	}
	SendMsgSize = 0;
}
void Extension::SaveReceivedBinaryToFile(int position, int size, const TCHAR * filename)
{
	if (position < 0)
		return CreateError("Cannot save received binary to file; supplied position %i is less than 0.", position);
	if (size <= 0)
		return CreateError("Cannot save received binary to file; supplied size %i is equal or less than 0.", size);
	if (filename[0] == _T('\0'))
		return CreateError("Cannot save received binary to file; supplied filename \"\" is invalid.");

	if (((unsigned int)position) + size > threadData->receivedMsg.content.size())
		return CreateError("Cannot save received binary to file; message doesn't the supplied position range %i to %i.", position, position + size);

	FILE * File = _tfsopen(filename, _T("wb"), SH_DENYWR);
	if (!File)
	{
		ErrNoToErrText();
		return CreateError("Cannot save received binary to file \"%s\", error number %i \"%hs\" occurred with opening the file.", TStringToUTF8(filename).c_str(), errno, errtext);
	}

	size_t amountWritten;
	if ((amountWritten = fwrite(threadData->receivedMsg.content.data() + position, 1, size, File)) != size)
	{
		ErrNoToErrText();
		CreateError("Cannot save received binary to file, error %i \"%hs\" occurred with writing the file. Wrote %zu"
			" bytes total. The message has not been modified.", errno, errtext, amountWritten);
	}

	if (fclose(File))
	{
		ErrNoToErrText();
		CreateError("Cannot save received binary to file, error %i \"%hs\" occurred with writing the last part of the file."
			" The message has not been modified.", errno, errtext);
	}
}
void Extension::AppendReceivedBinaryToFile(int position, int size, const TCHAR * filename)
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
	FILE * File = _tfsopen(filename, _T("ab"), SH_DENYWR);
	if (!File)
	{
		ErrNoToErrText();
		return CreateError("Cannot append received binary to file, error %i \"%s\" occurred with opening file \"%s\".", errno, errtext, TStringToUTF8(filename).c_str());
	}

	size_t amountWritten;
	if ((amountWritten = fwrite(threadData->receivedMsg.content.data() + position, 1, size, File)) != size)
	{
		fseek(File, 0, SEEK_END);
		std::int64_t filesize = _ftelli64(File);
		ErrNoToErrText();
		CreateError("Cannot append received binary to file \"%s\", error %i \"%s\" occurred with writing the file. "
			"Wrote %zu bytes, leaving file at size %lld bytes.", TStringToUTF8(filename).c_str(), errno, errtext, amountWritten, filesize);
	}

	if (fclose(File))
		CreateError("Cannot append received binary to file \"%s\", error number %i occurred with writing last part of the file.", TStringToUTF8(filename).c_str(), errno);
}
void Extension::AddFileToBinary(const TCHAR * filename)
{
	if (filename[0] == _T('\0'))
		return CreateError("Cannot add file to send binary; supplied filename \"\" is invalid.");

	// Open and deny other programs write privileges
	FILE * File = _tfsopen(filename, _T("rb"), _SH_DENYWR);
	if (!File)
	{
		ErrNoToErrText();
		return CreateError("Cannot add file \"%s\" to send binary, error number %i \"%s\" occurred with opening the file.", TStringToUTF8(filename).c_str(), errno, errtext);
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
		CreateError("Couldn't read file \"%s\" into binary to send; couldn't reserve %i bytes of memory to add file into message.",
			TStringToUTF8(filename).c_str(), filesize);
	}
	else
	{
		size_t amountRead;
		if ((amountRead = fread_s(buffer, filesize, 1, filesize, File)) != filesize)
			CreateError("Couldn't read file \"%s\" into binary to send; reading file caused error %i \"%s\".", TStringToUTF8(filename).c_str(), errno, errtext);
		else
			AddToSend(buffer, amountRead);

		free(buffer);
	}
	fclose(File);
}
void Extension::SelectChannelMaster()
{
	if (!selChannel)
		return CreateError("Could not select channel master: no channel selected.");
	if (selChannel->readonly())
		return CreateError("Could not select channel master: channel \"%s\" is read-only.", selChannel->name().c_str());

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
	const std::string channelNameU8(TStringToUTF8(channelName));
	if (channelName[0] == _T('\0'))
		return CreateError("Cannot join channel: invalid channel name %s supplied.", channelNameU8.c_str());
	Cli.join(channelNameU8, hidden != 0, closeAutomatically != 0);
}
void Extension::CompressSendBinary()
{
	if (SendMsgSize <= 0)
		return CreateError("Cannot compress send binary; message is too small.");
	int ret;
	z_stream strm = {};
	ret = deflateInit(&strm, 9); // 9 is maximum compression level
	if (ret)
		return CreateError("Zlib error %i: %hs occurred with initiating compression.", ret, strm.msg ? "No details" : strm.msg);

	// 4: precursor lw_ui32 with uncompressed size, required by Relay
	// 256: if compression results in larger message, it shouldn't be *that* much larger.

	unsigned char * output_buffer = (unsigned char *)malloc(4 + SendMsgSize +  256);
	if (!output_buffer)
	{
		deflateEnd(&strm);
		return CreateError("Error with compressing send binary, could not allocate %u bytes of memory.", 4U + SendMsgSize + 256U);
	}

	// Store size as precursor - required by Relay
	*(lw_ui32 *)output_buffer = SendMsgSize;

	strm.next_in = (unsigned char *)SendMsg;
	strm.avail_in = SendMsgSize;

	// Allocate memory for compression
	strm.avail_out = SendMsgSize - 4;
	strm.next_out = output_buffer + 4;

	ret = deflate(&strm, Z_FINISH);
	if (ret != Z_STREAM_END)
	{
		const char *strmMsg = strm.msg ? strm.msg : "(no description)";
		free(output_buffer);
		deflateEnd(&strm);
		return CreateError("Error with compressing send binary, deflate() returned %i. Zlib error: %hs.", ret, strmMsg);
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
void Extension::DecompressReceivedBinary()
{
	if (threadData->receivedMsg.content.size() <= 4)
		return CreateError("Cannot decompress received binary; message is too small.");

	z_stream strm = { };
	int ret = inflateInit(&strm);
	if (ret)
	{
		const char * strmMsg = strm.msg ? strm.msg : "(no description)";
		return CreateError("Decompression failed; error %d: %hs with initiating decompression.", ret, strmMsg);
	}

	// Lacewing provides a precursor to the compressed data, with uncompressed size.
	lw_ui32 expectedUncompressedSize = *(lw_ui32 *)threadData->receivedMsg.content.data();
	std::string_view inputData(threadData->receivedMsg.content.data() + 4, threadData->receivedMsg.content.size() - 4);
	if (expectedUncompressedSize > 0x0F000000U)
		return CreateError("Decompression failed; message anticipated to be too large. Expected %u byte output.", expectedUncompressedSize);

	std::unique_ptr<unsigned char[]> output_buffer;
	try {
		output_buffer = std::make_unique<unsigned char[]>(expectedUncompressedSize);
	}
	catch (std::bad_alloc)
	{
		inflateEnd(&strm);
		return CreateError("Decompression failed; could not allocate enough memory. Requested %u bytes.", expectedUncompressedSize);
	}

	strm.next_in = (unsigned char *)inputData.data();
	strm.avail_in = inputData.size();
	strm.avail_out = expectedUncompressedSize;
	strm.next_out = output_buffer.get();
	ret = inflate(&strm, Z_FINISH);
	if (ret < Z_OK)
	{
		const char *strmMsg = strm.msg ? strm.msg : "(no description)";
		inflateEnd(&strm);
		return CreateError("Error with decompression, inflate() returned error %i. Zlib description: %hs.", ret, strmMsg);
	}

	inflateEnd(&strm);

	// Update all extensions with new message content.
	threadData->receivedMsg.content.assign((char *)output_buffer.get(), expectedUncompressedSize);
	threadData->receivedMsg.cursor = 0;

	for (auto& i : Saved)
	{
		if (threadData == i)
			continue;
		(*i).receivedMsg.content.assign((char *)output_buffer.get(), expectedUncompressedSize);
		(*i).receivedMsg.cursor = 0;
	}
}
void Extension::MoveReceivedBinaryCursor(int position)
{
	if (position < 0)
		return CreateError("Cannot move cursor; Position less than 0.");
	if (threadData->receivedMsg.content.size() - position <= 0)
		return CreateError("Cannot move cursor; Message is too small.");

	threadData->receivedMsg.cursor = position;
}
void Extension::LoopListedChannelsWithLoopName(const TCHAR * passedLoopName)
{
	if (loopName[0] == _T('\0'))
		return CreateError("Cannot loop listed channels: invalid loop name supplied.");

	auto origLoopName = loopName;
	std::tstring_view loopNameDup(passedLoopName);
	std::vector<decltype(threadData->channelListing)> channelListingDup;
	{
		auto cliReadLock = Cli.lock.createReadLock();
		channelListingDup = Cli.getchannellisting();
	}
	auto origChannelList = threadData->channelListing;

	for (const auto &chLst : channelListingDup)
	{
		threadData->channelListing = chLst;
		loopName = loopNameDup;
		Runtime.GenerateEvent(59);
	}

	threadData->channelListing = nullptr;
	loopName = loopNameDup;
	Runtime.GenerateEvent(60);

	threadData->channelListing = origChannelList;
	loopName = origLoopName;
}
void Extension::LoopClientChannelsWithLoopName(const TCHAR * passedLoopName)
{
	if (passedLoopName[0] == _T('\0'))
		return CreateError("Cannot loop client channels: invalid loop name supplied.");

	auto origLoopName = loopName;
	std::tstring_view loopNameDup(passedLoopName);
	std::vector<decltype(selChannel)> channelListDup;
	{
		auto cliReadLock = Cli.lock.createReadLock();
		channelListDup = Cli.getchannels();
	}
	for (const auto &ch : channelListDup)
	{
		selChannel = ch;
		loopName = loopNameDup;
		Runtime.GenerateEvent(63);
	}

	selChannel = nullptr;
	loopName = loopNameDup;
	Runtime.GenerateEvent(64);

	loopName = origLoopName;
}
void Extension::LoopPeersOnChannelWithLoopName(const TCHAR * passedLoopName)
{
	if (loopName[0] == _T('\0'))
		return CreateError("Cannot loop peers on channel: invalid loop name \"\" supplied.");
	if (!selChannel)
		return CreateError("Cannot loop peers on channel: no channel currently selected.");

	std::tstring_view loopNameDup(passedLoopName);
	auto origSelChannel = selChannel;
	auto origSelPeer = selPeer;
	auto origLoopName = loopName;
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
	const TCHAR * portPtr = _tcsrchr(hostname, _T(':'));
	if (portPtr)
	{
		Port = _ttoi(portPtr + 1);

		if (Port <= 0 || Port > 0xFFFF)
			return CreateError("Invalid port in hostname: too many numbers. Ports are limited from 1 to 65535.");
	}
	std::string hostnameU8(TStringToUTF8(hostname));
	Cli.connect(hostnameU8.c_str(), Port);
}
void Extension::ResizeBinaryToSend(int newSize)
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
