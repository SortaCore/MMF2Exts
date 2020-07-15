#include "Common.h"

#ifdef _DEBUG
std::stringstream CriticalSection;
#endif

#define Remake(name) MessageBoxA(NULL, "Your "#name" actions need to be recreated.\r\n" \
										"This is probably due to parameter changes.", "Lacewing Blue Client", MB_OK)
#define Saved (globals->_saved)

void Extension::Replaced_Connect(char * hostname, int port)
{
	Remake("Connect");
}
void Extension::Disconnect()
{
	Cli.disconnect(); // calls OnDisconnect so clear-all 0xFFFF is done there
}
void Extension::SetName(char * name)
{
	if (name[0] == '\0')
		CreateError("Error: SetName() was called with \"\".");
	else
		Cli.name(name);
}
void Extension::Replaced_JoinChannel(char * channelName, int hideChannel)
{
	Remake("Join channel");
}
void Extension::LeaveChannel()
{
	bool isReadOnly = selChannel && selChannel->readonly();
	if (selChannel && !selChannel->readonly())
		selChannel->leave();
}
void Extension::SendTextToServer(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Text to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!textToSend)
		CreateError("Error: Send Text to Server was called with a null parameter.");
	else
		Cli.sendserver(subchannel, textToSend);
}
void Extension::SendTextToChannel(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Text to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!selChannel)
		CreateError("Error: Send Text to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Send Text to Channel was called with a read-only channel.");
	else
		selChannel->send(subchannel, textToSend);
}
void Extension::SendTextToPeer(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Text to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!selPeer)
		CreateError("Error: Send Text to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Error: Send Text to Peer was called with a read-only peer.");
	else
		selPeer->send(subchannel, textToSend);
}
void Extension::SendNumberToServer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Number to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else
		Cli.sendserver(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::SendNumberToChannel(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!selChannel)
		CreateError("Error: Send Number to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Send Number to Channel was called with a read-only channel.");
	else
		selChannel->send(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::SendNumberToPeer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Number to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!selPeer)
		CreateError("Error: Send Number to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Error: Send Number to Peer was called with a read-only peer.");
	else
		selPeer->send(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::BlastTextToServer(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Text to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else
		Cli.blastserver(subchannel, textToSend);
}
void Extension::BlastTextToChannel(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Text to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!selChannel)
		CreateError("Error: Blast Text to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Blast Text to Channel was called with a read-only channel.");
	else
		selChannel->blast(subchannel, textToSend);
}
void Extension::BlastTextToPeer(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Text to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!selPeer)
		CreateError("Error: Blast Text to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Error: Blast Text to Peer was called with a read-only peer.");
	else
		selPeer->blast(subchannel, textToSend);
}
void Extension::BlastNumberToServer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Number to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else
		Cli.blastserver(subchannel, std::string_view((char *)&numToSend, sizeof(int)), 1);
}
void Extension::BlastNumberToChannel(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!selChannel)
		CreateError("Error: Blast Number to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Blast Number to Channel was called with a read-only channel.");
	else
		selChannel->blast(subchannel, std::string_view((char *)&numToSend, 4), 1);
}
void Extension::BlastNumberToPeer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Number to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!selPeer)
		CreateError("Error: Blast Number to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Error: Blast Number to Peer was called with a read-only peer.");
	else
		selPeer->blast(subchannel, std::string_view((char *)&numToSend, 4), 1);
}
void Extension::SelectChannelWithName(char * channelName)
{
	selChannel = nullptr;
	{
		auto cliReadLock = Cli.lock.createReadLock();
		const auto &channels = Cli.getchannels();
		auto foundChIt = std::find_if(channels.cbegin(), channels.cend(),
			[=](const auto & c) {
				return lw_sv_icmp(c->name(), channelName); });

		if (foundChIt != channels.cend())
		{
			selChannel = *foundChIt;
			return;
		}
	}

	// Only modify selected channel if we found it
	std::string error = "Could not select channel, name not found: ";
	error += channelName;
	CreateError(error.c_str());
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
		loopName = std::string_view();
		Runtime.GenerateEvent(14);
	}

	selChannel = origSelChannel;
	selPeer = origSelPeer;
	loopName = std::string_view();
	Runtime.GenerateEvent(18);
	loopName = origLoopName;
}
void Extension::SelectPeerOnChannelByName(char * peerName)
{
	if (peerName[0] == '\0')
		return CreateError("Error: Select Peer On Channel By Name was called with a blank name.");
	if (!selChannel)
		return CreateError("Error: Select Peer On Channel By Name was called without a channel being selected.");

	selPeer = nullptr;
	{
		auto chReadLock = selChannel->lock.createReadLock();
		const auto & peers = selChannel->getpeers();
		auto foundPeerIt = std::find_if(peers.cbegin(), peers.cend(),
			[=](const auto & p) {
				return lw_sv_icmp(p->name(), peerName);
			});
		if (foundPeerIt != peers.cend())
		{
			selPeer = *foundPeerIt;
			return;
		}
	}

	std::stringstream error;
	error << "Peer with name " << peerName
		<< " not found on channel " << selChannel->name() << ".";
	CreateError(error.str().c_str());
}
void Extension::SelectPeerOnChannelByID(int peerID)
{
	if (peerID < 0)
		return CreateError("Could not select peer on channel, ID is below 0.");
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

	std::stringstream error;
	error << "Peer with ID " << peerID
			<< " not found on channel " << selChannel->name() << ".";
	CreateError(error.str().c_str());
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
		loopName = std::string_view();
		Runtime.GenerateEvent(13);
	}

	selChannel = origSelChannel;
	selPeer = origSelPeer;
	loopName = std::string_view();
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
		loopName = std::string_view();
		Runtime.GenerateEvent(27);
	}

	threadData->channelListing = nullptr;
	loopName = std::string_view();
	Runtime.GenerateEvent(28);
	threadData->channelListing = origChannelList;
	loopName = origLoopName;
}
void Extension::SendBinaryToServer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else
		Cli.sendserver(subchannel, std::string_view(SendMsg, SendMsgSize), 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!selChannel)
		CreateError("Error: Send Binary to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Send Binary to Channel was called with a read-only channel.");
	else
		selChannel->send(subchannel, std::string_view(SendMsg, SendMsgSize), 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToPeer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
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
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else
		Cli.blastserver(subchannel, std::string_view(SendMsg, SendMsgSize), 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!selChannel)
		CreateError("Error: Blast Binary to Channel was called without a channel being selected.");
	else if (selChannel->readonly())
		CreateError("Error: Blast Binary to Channel was called with a read-only channel.");
	else
		selChannel->blast(subchannel, std::string_view(SendMsg, SendMsgSize), 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToPeer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!selPeer)
		CreateError("Error: Blast Binary to Peer was called without a peer being selected.");
	else if (selPeer->readonly())
		CreateError("Error: Blast Binary to Peer was called with a read-only peer.");
	else
		selPeer->blast(subchannel, std::string_view(SendMsg, SendMsgSize), 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::AddByteText(char * byte)
{
	if (!byte || strnlen(byte, 2) != 1)
		CreateError("Adding byte to stack failed: byte supplied was part of a string, not a single byte.");
	else
		AddToSend(byte, 1);
}
void Extension::AddByteInt(int byte)
{
	if (byte > 255 || byte < -128)
		CreateError("Error: Byte out of bounds.");
	else
	{
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
}
void Extension::AddShort(int _short)
{
	if (_short > MAXUINT16 || _short < MININT16)
		CreateError("Error: Short out of bounds.");
	else
	{
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
}
void Extension::AddInt(int _int)
{
	AddToSend(&_int, 4);
}
void Extension::AddFloat(float _float)
{
	AddToSend(&_float, 4);
}
void Extension::AddStringWithoutNull(char * string)
{
	if (string)
		AddToSend(string, strlen(string));
	else
		CreateError("Adding string without null failed: pointer was null.");
}
void Extension::AddString(char * string)
{
	if (string)
		AddToSend(string, strlen(string) + 1U);
	else
		CreateError("Adding string failed: pointer was null.");
}
void Extension::AddBinary(unsigned int address, int size)
{
	if (size < 0)
		CreateError("Add binary failed: Size < 0.");
	else 
	{
		if (size != 0)
			AddToSend((void *)(long)address, size);
		// else do nothing
	}
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
void Extension::SaveReceivedBinaryToFile(int position, int size, char * filename)
{
	if (position < 0)
		return CreateError("Cannot save received binary; Position less than 0.");
	if (size <= 0)
		return CreateError("Cannot save received binary; Size equal or less than 0.");
	if (filename[0] == '\0')
		return CreateError("Cannot save received binary; filename is invalid.");

	if (((unsigned int)position) + size > threadData->receivedMsg.content.size())
	{
		std::stringstream error;
		error << "Cannot save received binary to file; message doesn't have " << size
			<< " bytes at position " << position << " onwards.";
		return CreateError(error.str().c_str());
	}
	FILE * File = _fsopen(filename, "wb", SH_DENYWR);
	if (!File)
	{
		std::stringstream error;
		error << "Cannot save received binary to file, error number " << errno
			<< " occurred with opening the file. The message has not been modified.";
		return CreateError(error.str().c_str());
	}

	size_t amountWritten;
	if ((amountWritten = fwrite(threadData->receivedMsg.content.data() + position, 1, size, File)) != size)
	{
		std::stringstream error;
		error << "Cannot save received binary to file, error number " << errno 
			<< " occurred with writing the file. Wrote " << amountWritten
			<< " bytes total. The message has not been modified.";
		CreateError(error.str().c_str());
	}

	if (fclose(File))
	{
		std::stringstream error;
		error << "Cannot save received binary to file, error number " << errno
			<< " occurred with writing last part of the file. The message has not been modified.";
		CreateError(error.str().c_str());
	}
}
void Extension::AppendReceivedBinaryToFile(int position, int size, char * filename)
{
	if (position < 0)
		return CreateError("Cannot append received binary to file; position less than 0.");
	if (size <= 0)
		return CreateError("Cannot append received binary to file; size equal or less than 0.");
	if (filename[0] == '\0')
		return CreateError("Cannot append received binary to file; filename is invalid.");

	if (((unsigned int)position) + size > threadData->receivedMsg.content.size())
	{
		std::stringstream error;
		error << "Cannot append received binary to file; message doesn't have " << size
			<< " bytes at position " << position << " onwards.";
		return CreateError(error.str().c_str());
	}

	// Open while denying write of other programs
	FILE * File = _fsopen(filename, "ab", SH_DENYWR);
	if (!File)
	{
		std::stringstream error;
		error << "Cannot append received binary to file, error number " << errno
			<< " occurred with opening the file. The binary message has not been modified.";
		return CreateError(error.str().c_str());
	}

	size_t amountWritten;
	if ((amountWritten = fwrite(threadData->receivedMsg.content.data() + position, 1, size, File)) != size)
	{
		fseek(File, 0, SEEK_END);
		long long filesize = _ftelli64(File);

		std::stringstream error;
		error << "Cannot append received binary to file, error number " << errno
			<< " occurred with writing the file. Wrote " << amountWritten
			<< " bytes, leaving file at size " << filesize << " total. The binary message has not been modified.";
		CreateError(error.str().c_str());
	}

	if (fclose(File))
	{
		std::stringstream error;
		error << "Cannot append received binary to file, error number " << errno
			<< " occurred with writing last part of the file. The message has not been modified.";
		CreateError(error.str().c_str());
	}
}
void Extension::AddFileToBinary(char * Filename)
{
	if (Filename[0] == '\0')
		return CreateError("Cannot add file to send binary; filename is invalid.");

	// Open and deny other programs write priviledges
	FILE * File = _fsopen(Filename, "rb", _SH_DENYWR);
	if (!File)
	{
		std::stringstream error;
		error << "Cannot add file to send binary, error number " << errno 
			<< " occurred with opening the file. The send binary has not been modified.";
		return CreateError(error.str().c_str());
	}

	// Jump to end
	fseek(File, 0, SEEK_END);

	// Read current position as file size
	long filesize = ftell(File);

	// Go back to start
	fseek(File, 0, SEEK_SET);

	char * buffer = (char *)malloc(filesize);
	if (!buffer)
	{
		std::stringstream error;
		error << "Couldn't read file \"" << Filename << "\" into binary to send; couldn't reserve enough memory "
			"to add file into message. The send binary has not been modified.";
		CreateError(error.str().c_str());
	}
	else
	{
		size_t amountRead;
		if ((amountRead = fread_s(buffer, filesize, 1, filesize, File)) != filesize)
		{
			std::stringstream error;
			error << "Couldn't read file \"" << Filename << "\" into binary to send, error number " << errno
				<< " occurred with opening the file. The send binary has not been modified.";
			CreateError(error.str().c_str());
		}
		else
			AddToSend(buffer, amountRead);

		free(buffer);
	}
	fclose(File);
}
void Extension::SelectChannelMaster()
{
	if (!selChannel)
		return CreateError("Could not select channel master: No channel selected.");
	if (selChannel->readonly())
		return CreateError("Could not select channel master: Channel is read-only.");

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
void Extension::JoinChannel(char * channelName, int hidden, int closeAutomatically)
{
	if (!channelName || channelName[0] == '\0')
		return CreateError("Cannot join channel: invalid channel name supplied.");
	Cli.join(channelName, hidden != 0, closeAutomatically != 0);
}
void Extension::CompressSendBinary()
{
	if (SendMsgSize <= 0)
		return CreateError("Cannot compress send binary; Message is too small.");
	int ret;
	z_stream strm = {};
	ret = deflateInit(&strm, 9); // 9 is maximum compression level
	if (ret)
	{
		std::stringstream error;
		error << "Error " << ret << " occurred with initiating compression.";
		return CreateError(error.str().c_str());
	}

	// 4: precursor lw_ui32 with uncompressed size, required by Relay
	// 256: if compression results in larger message, it shouldn't be *that* much larger.

	unsigned char * output_buffer = (unsigned char *)malloc(4 + SendMsgSize +  256);
	if (!output_buffer)
	{
		deflateEnd(&strm);
		std::stringstream error;
		error << "Error with compressing send binary, could not allocate enough memory. Desired " << 4 + SendMsgSize + 256 << " bytes.";
		return CreateError(error.str().c_str());
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
		std::stringstream error;
		error << "Error with compressing send binary, deflate() returned " << ret <<
			". Zlib error: " << (strm.msg ? strm.msg : "");
		free(output_buffer);
		deflateEnd(&strm);
		return CreateError(error.str().c_str());
	}

	deflateEnd(&strm);

	char * output_bufferResize = (char *)realloc(output_buffer, 4 + strm.total_out);
	if (!output_bufferResize)
	{
		free(output_buffer); // realloc will not free on error
		CreateError("Error with compressing send binary, reallocating memory to remove excess space after compression failed.");
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
		std::stringstream error;
		error << "Error " << ret << " occurred with initiating decompression.";
		return CreateError(error.str().c_str());
	}

	// Lacewing provides a precursor to the compressed data, with uncompressed size.
	lw_ui32 expectedUncompressedSize = *(lw_ui32 *)threadData->receivedMsg.content.data();
	std::string_view inputData(threadData->receivedMsg.content.data() + 4, threadData->receivedMsg.content.size() - 4);
	if (expectedUncompressedSize > 0x0F000000U)
	{
		std::stringstream error;
		error << "Decompression failed; message anticipated to be too large. Expected "
			<< expectedUncompressedSize << " byte output.";
		return CreateError(error.str().c_str());
	}

	std::unique_ptr<unsigned char[]> output_buffer;
	try {
		output_buffer = std::make_unique<unsigned char[]>(expectedUncompressedSize);
	}
	catch (std::bad_alloc)
	{
		std::stringstream error;
		error << "Error with decompression, could not allocate enough memory. Desired "
			<< expectedUncompressedSize << " bytes.";
		inflateEnd(&strm);
		return CreateError(error.str().c_str());
	}

	strm.next_in = (unsigned char *)inputData.data();
	strm.avail_in = inputData.size();
	strm.avail_out = expectedUncompressedSize;
	strm.next_out = output_buffer.get();
	ret = inflate(&strm, Z_FINISH);
	if (ret < Z_OK)
	{
		std::stringstream error;
		error << "Error with decompression, inflate() returned error " << ret
			<< ". Zlib error: " << (strm.msg ? strm.msg : "");
		inflateEnd(&strm);
		return CreateError(error.str().c_str());
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
void Extension::LoopListedChannelsWithLoopName(char * passedLoopName)
{
	if (passedLoopName[0] == '\0')
		return CreateError("Cannot loop listed channels: blank loop name supplied.");

	auto origLoopName = loopName;
	std::string_view loopNameDup(passedLoopName);
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
void Extension::LoopClientChannelsWithLoopName(char * passedLoopName)
{
	if (passedLoopName[0] == '\0')
		return CreateError("Cannot loop client channels: blank loop name supplied.");

	auto origLoopName = loopName;
	std::string_view loopNameDup(passedLoopName);
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
void Extension::LoopPeersOnChannelWithLoopName(char * passedLoopName)
{
	if (passedLoopName[0] == '\0')
		return CreateError("Cannot loop peers on channel: blank loop name supplied.");
	if (!selChannel)
		return CreateError("Cannot loop peers on channel: No channel currently selected.");

	std::string_view loopNameDup(passedLoopName);
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
void Extension::Connect(char * hostname)
{
	if (hostname[0] == '\0')
		return CreateError("Cannot connect to server: invalid hostname supplied.");

	int Port = 6121;
	const char * portPtr = strrchr(hostname, ':');
	if (portPtr)
	{
		Port = atoi(portPtr + 1);
		
		if (Port <= 0 || Port > 0xFFFF)
			return CreateError("Invalid port in hostname: too many numbers. Ports are limited from 1 to 65535.");
	}
	Cli.connect(hostname, Port);
}
void Extension::ResizeBinaryToSend(int newSize)
{
	if (newSize < 0)
		return CreateError("Cannot change size of binary to send: new size is under 0 bytes.");

	char * NewMsg = (char *)realloc(SendMsg, newSize);
	if (!NewMsg)
	{
		return CreateError("Cannot change size of binary to send: reallocation of memory failed. "
							"Size has not been modified.");
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
