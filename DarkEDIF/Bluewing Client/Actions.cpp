#include "Common.h"

#ifdef _DEBUG
std::stringstream CriticalSection;
#endif

#define Remake(name) MessageBoxA(NULL, "Your "#name" actions need to be recreated.\r\n" \
									   "This is probably due to parameter changes.", "Lacewing Blue Client - DarkEDIF", MB_OK)
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
	if (threadData.channel && threadData.channel->isclosed)
		threadData.channel->orig().leave(); // don't set isclosed, leave not approved by server yet
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
	else if (!threadData.channel)
		CreateError("Error: Send Text to Channel was called without a channel being selected.");
	else if (threadData.channel->isclosed)
		CreateError("Error: Send Text to Channel was called with a closed channel.");
	else 
		threadData.channel->send(subchannel, textToSend);
}
void Extension::SendTextToPeer(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Text to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.peer)
		CreateError("Error: Send Text to Peer was called without a peer being selected.");
	else if (threadData.peer->isclosed)
		CreateError("Error: Send Text to Peer was called with a closed peer.");
	else
		threadData.peer->send(subchannel, textToSend);
}
void Extension::SendNumberToServer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Number to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else
		Cli.sendserver(subchannel, (char *)&numToSend, sizeof(int), 1);
}
void Extension::SendNumberToChannel(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.channel)
		CreateError("Error: Send Number to Channel was called without a channel being selected.");
	else if (threadData.channel->isclosed)
		CreateError("Error: Send Number to Channel was called with a closed channel.");
	else
		threadData.channel->send(subchannel, (char *)&numToSend, sizeof(int), 1);
}
void Extension::SendNumberToPeer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Send Number to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.peer)
		CreateError("Error: Send Number to Peer was called without a peer being selected.");
	else if (threadData.peer->isclosed)
		CreateError("Error: Send Number to Peer was called with a closed peer.");
	else
		threadData.peer->send(subchannel, (char *)&numToSend, sizeof(int), 1);
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
	else if (!threadData.channel)
		CreateError("Error: Blast Text to Channel was called without a channel being selected.");
	else if (threadData.channel->isclosed)
		CreateError("Error: Blast Text to Channel was called with a closed channel.");
	else
		threadData.channel->blast(subchannel, textToSend);
}
void Extension::BlastTextToPeer(int subchannel, char * textToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Text to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.peer)
		CreateError("Error: Blast Text to Peer was called without a peer being selected.");
	else if (threadData.peer->isclosed)
		CreateError("Error: Blast Text to Peer was called with a closed peer.");
	else
		threadData.peer->blast(subchannel, textToSend);
}
void Extension::BlastNumberToServer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Number to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else
		Cli.blastserver(subchannel, (char *)&numToSend, sizeof(int), 1);
}
void Extension::BlastNumberToChannel(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.channel)
		CreateError("Error: Blast Number to Channel was called without a channel being selected.");
	else
		threadData.channel->blast(subchannel, (char *) &numToSend, 4, 1);
}
void Extension::BlastNumberToPeer(int subchannel, int numToSend)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Blast Number to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!threadData.peer)
		CreateError("Error: Blast Number to Peer was called without a peer being selected.");
	else if (threadData.peer->isclosed)
		CreateError("Error: Blast Number to Peer was called with a closed peer.");
	else
		threadData.peer->blast(subchannel, (char *) &numToSend, 4, 1);
}
void Extension::SelectChannelWithName(char * channelName)
{
	auto SelectedI = std::find_if(Channels.begin(), Channels.end(),
		[=](ChannelCopy * c) { return !_stricmp(c->name(), channelName); });
	auto Selected = SelectedI == Channels.end() ? nullptr : *SelectedI;

	// Only modify threadData.channel if we found it
	if (Selected && !Selected->isclosed)
		threadData.channel = Selected;
	else
	{
		threadData.channel = nullptr;
		std::string error = "Could not select channel, name not found: ";
		error += channelName;
		CreateError(error.c_str());
	}
}
void Extension::ReplacedNoParams()
{
	Remake("Unknown action");
}
void Extension::LoopClientChannels()
{
	char * Stored = (char *)malloc(sizeof(SaveExtInfo));
	memcpy_s(Stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));

	for (auto Selected : Channels)
	{
		if (!Selected->isclosed)
		{
			ClearThreadData();
			threadData.channel = Selected;
			Runtime.GenerateEvent(14);
		}
	}

	memcpy_s(&threadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
	free(Stored);

	Runtime.GenerateEvent(18);
}
void Extension::SelectPeerOnChannelByName(char * peerName)
{
	if (peerName[0] == '\0')
		CreateError("Error: Select Peer On Channel By Name was called with a blank name.");
	else if (!threadData.channel)
		CreateError("Error: Select Peer On Channel By Name was called without a channel being selected.");
	else
	{
		auto &peers = threadData.channel->getpeers();
		auto Selected = std::find_if(peers.begin(), peers.end(),
			[=](PeerCopy * const P) {
				return !_stricmp(P->name(), peerName);
		});

		// Only modify threadData.peer if we found it
		if (Selected != peers.cend())
			threadData.peer = *Selected;
		else
		{
			threadData.peer = nullptr;
			std::stringstream error;
			error << "Peer with name " << peerName 
				  << " not found on channel " << threadData.channel->name() << ".";
			CreateError(error.str().c_str());
		}
	}
}
void Extension::SelectPeerOnChannelByID(int peerID)
{
	if (peerID < 0)
		CreateError("Could not select peer on channel, ID is below 0.");
	else if (!threadData.channel)
		CreateError("Error: Select Peer On Channel By ID was called without a channel being selected.");
	else if (threadData.channel->isclosed)
		CreateError("Error: Select Peer On Channel By ID was called with a closed channel.");
	else
	{
		auto &peers = threadData.channel->getpeers();
		auto Selected = std::find_if(peers.cbegin(), peers.cend(),
			[=](PeerCopy * const &P) {
				return P->id() == peerID;
		});
		
		// Only modify threadData.peer if we found it
		if (Selected != peers.cend())
			threadData.peer = *Selected;
		else
		{
			threadData.peer = nullptr;
			std::stringstream error;
			error << "Peer with ID " << peerID
				  << " not found on channel " << threadData.channel->name() << ".";
			CreateError(error.str().c_str());
		}
	}
}
void Extension::LoopPeersOnChannel()
{
	// Store selected channel
	if (!threadData.channel)
		CreateError("Error: Loop Peers On Channel was called without a channel being selected.");
	else
	{
		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));

		auto StoredChannel = threadData.channel;
		auto& peers = threadData.channel->getpeers();
		for (auto Selected : peers)
		{
			if (!Selected->isclosed)
			{
				ClearThreadData();
				threadData.channel = StoredChannel;
				threadData.peer = Selected;
				Runtime.GenerateEvent(13);
			}
		}
		memcpy_s(&threadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
		free(Stored);

		Runtime.GenerateEvent(17);
	}
}
void Extension::RequestChannelList()
{
	Cli.listchannels();
}
void Extension::LoopListedChannels()
{
	char * Stored = (char *)malloc(sizeof(SaveExtInfo));
	memcpy_s(Stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));
	
	for (auto Selected = Cli.firstchannellisting(); Selected; Selected = Selected->next())
	{
		ClearThreadData();
		threadData.channelListing = Selected;
		Runtime.GenerateEvent(27);
	}

	memcpy_s(&threadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
	free(Stored);

	Runtime.GenerateEvent(28);
}
void Extension::SendBinaryToServer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else
		Cli.sendserver(subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!threadData.channel)
		CreateError("Error: Send Binary to Channel was called without a channel being selected.");
	else if (threadData.channel->isclosed)
		CreateError("Error: Send Binary to Channel was called with a closed channel.");
	else
		threadData.channel->send(subchannel, SendMsg, SendMsgSize, 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToPeer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!threadData.peer)
		CreateError("Error: Send Binary to Peer was called without a peer being selected.");
	else if (threadData.peer->isclosed)
		CreateError("Error: Send Binary to Peer was called with a closed peer.");
	else
		threadData.peer->send(subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToServer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else
		Cli.blastserver(subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToChannel(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!threadData.channel)
		CreateError("Error: Blast Binary to Channel was called without a channel being selected.");
	else if (threadData.channel->isclosed)
		CreateError("Error: Blast Binary to Channel was called with a closed channel.");
	else
		threadData.channel->blast(subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToPeer(int subchannel)
{
	if (subchannel > 255 || subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!threadData.peer)
		CreateError("Error: Blast Binary to Peer was called without a peer being selected.");
	else if (threadData.peer->isclosed)
		CreateError("Error: Blast Binary to Peer was called with a closed peer.");
	else
		threadData.peer->blast(subchannel, SendMsg, SendMsgSize, 2);
	
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
		CreateError("Cannot save received binary; Position less than 0.");
	else if (size <= 0)
		CreateError("Cannot save received binary; Size equal or less than 0.");
	else if (filename[0] == '\0')
		CreateError("Cannot save received binary; filename is invalid.");
	else if (((unsigned int)position) + size > threadData.receivedMsg.size)
	{
		std::stringstream error;
		error << "Cannot save received binary to file; message doesn't have " << size
			<< " bytes at position " << position << " onwards.";
		CreateError(error.str().c_str());
		return;
	}
	else
	{
		FILE * File = _fsopen(filename, "wb", SH_DENYWR);
		if (!File)
		{
			std::stringstream error;
			error << "Cannot save received binary to file, error number " << errno
				<< " occured with opening the file. The message has not been modified.";
			CreateError(error.str().c_str());
			return;
		}

		size_t amountWritten;
		if ((amountWritten = fwrite(threadData.receivedMsg.content+position, 1, size, File)) != size)
		{
			std::stringstream error;
			error << "Cannot save received binary to file, error number " << errno 
				<< " occured with writing the file. Wrote " << amountWritten
				<< " bytes total. The message has not been modified.";
			CreateError(error.str().c_str());
		}

		fclose(File);
	}
}
void Extension::AppendReceivedBinaryToFile(int position, int size, char * filename)
{
	if (position < 0)
		CreateError("Cannot append received binary to file; position less than 0.");
	else if (size <= 0)
		CreateError("Cannot append received binary to file; size equal or less than 0.");
	else if (filename[0] == '\0')
		CreateError("Cannot append received binary to file; filename is invalid.");
	else if (((unsigned int)position) + size > threadData.receivedMsg.size)
	{
		std::stringstream error;
		error << "Cannot append received binary to file; message doesn't have " << size
			<< " bytes at position " << position << " onwards.";
		CreateError(error.str().c_str());
		return;
	}
	else
	{
		// Open while denying write of other programs
		FILE * File = _fsopen(filename, "ab", SH_DENYWR);
		if (!File)
		{
			std::stringstream error;
			error << "Cannot append received binary to file, error number " << errno
				<< " occured with opening the file. The binary message has not been modified.";
			CreateError(error.str().c_str());
			return;
		}

		size_t amountWritten;
		if ((amountWritten = fwrite(threadData.receivedMsg.content+position, 1, size, File)) != size)
		{
			fseek(File, 0, SEEK_END);
			long long filesize = _ftelli64(File);

			std::stringstream error;
			error << "Cannot append received binary to file, error number " << errno
				<< " occured with writing the file. Wrote " << amountWritten
				<< " bytes, leaving file at size " << filesize << " total. The binary message has not been modified.";
			CreateError(error.str().c_str());
		}

		fclose(File);
	}
}
void Extension::AddFileToBinary(char * Filename)
{
	if (Filename[0] == '\0')
		CreateError("Cannot add file to send binary; filename is invalid.");
	else
	{
		// Open and deny other programs write priviledges
		FILE * File = _fsopen(Filename, "rb", _SH_DENYWR);
		if (!File)
		{
			std::stringstream error;
			error << "Cannot add file to send binary, error number " << errno 
				<< " occured with opening the file. The send binary has not been modified.";
			CreateError(error.str().c_str());
			return;
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
					<< " occured with opening the file. The send binary has not been modified.";
				CreateError(error.str().c_str());
			}
			else
				AddToSend(buffer, amountRead);

			free(buffer);
		}
		fclose(File);
	}
}
void Extension::SelectChannelMaster()
{
	if (!threadData.channel)
		CreateError("Could not select channel master: No channel selected.");
	else if (threadData.channel->isclosed)
		CreateError("Could not select channel master: Channel is closed.");
	else
	{
		PeerCopy * Stored = threadData.peer;
		threadData.peer = threadData.channel->channelmaster();

		// Restore if no channel master found
		if (!threadData.peer || threadData.peer->isclosed)
			threadData.peer = Stored;
	}
}
void Extension::JoinChannel(char * channelName, int hidden, int closeAutomatically)
{
	if (!channelName || channelName[0] == '\0')
		CreateError("Cannot join channel: invalid channel name supplied.");
	else if (!Cli.name())
		CreateError("Cannot join channel: client name not set.");
	else
		Cli.join(channelName, hidden != 0, closeAutomatically != 0);
}
void Extension::CompressSendBinary()
{
	if (SendMsgSize <= 0)
		CreateError("Cannot compress send binary; Message is too small.");
	else
	{
		int ret;
		z_stream strm = {0};

		ret = deflateInit(&strm, 9); // 9 is maximum compression level
		if (ret)
		{
			std::stringstream error;
			error << "Error " << ret << " occured with initiating compression.";
			CreateError(error.str().c_str());
			return;
		}
	
		unsigned char * output_buffer = (unsigned char *)malloc(SendMsgSize + 256);
		if (!output_buffer)
		{
			std::stringstream error;
			error << "Error with compressing send binary, could not allocate enough memory. Desired " << SendMsgSize + 256 << " bytes.";
			CreateError(error.str().c_str());
			deflateEnd(&strm);
			return;
		}

		strm.next_in = (unsigned char *)SendMsg;
		strm.avail_in = SendMsgSize;

		// Allocate memory for compression
		strm.avail_out = _msize(output_buffer);
		strm.next_out = output_buffer;

		ret = deflate(&strm, Z_FINISH);
		if (ret != Z_STREAM_END)
		{
			std::stringstream error;
			error << "Error with compressing send binary, deflate() returned " << ret << ". Zlib error: " << (strm.msg ? strm.msg : "");
			free(output_buffer);
			deflateEnd(&strm);
			CreateError(error.str().c_str());
			return;
		}

		deflateEnd(&strm);

		void * output_bufferResize = realloc(output_buffer, strm.total_out);
		if (!output_bufferResize)
		{
			free(output_buffer); // realloc will not free on error
			CreateError("Error with compressing send binary, reallocating memory to remove excess space after compression failed.");
			return;
		}
		free(SendMsg);
	
		SendMsg = (char *)output_bufferResize;
		SendMsgSize = strm.total_out;
	}
}
void Extension::DecompressReceivedBinary()
{
	if (threadData.receivedMsg.size <= 0)
		CreateError("Cannot decompress received binary; message is too small.");
	else
	{
		z_stream strm = { 0 };
		int ret = inflateInit_(&strm, ZLIB_VERSION, sizeof(z_stream));
		if (ret)
		{
			std::stringstream error;
			error << "Error " << ret << " occured with initiating decompression.";
			CreateError(error.str().c_str());
			return;
		}

		unsigned char * output_buffer = NULL, *output_buffer_pointer = NULL;
		strm.next_in = (unsigned char *)threadData.receivedMsg.content;
		strm.avail_in = threadData.receivedMsg.size;
		// run inflate() on input until output buffer not full, finish
		// compression if all of source has been read in
		do {
			// Allocate memory for compression
			output_buffer_pointer = (unsigned char *)realloc(output_buffer, (output_buffer ? _msize(output_buffer) : 0) + 1024);
			if (!output_buffer_pointer)
			{
				std::stringstream error;
				error << "Error with decompression, could not allocate enough memory. Desired "
					<< (output_buffer ? _msize(output_buffer) : 0) + 1024 << " bytes.";
				if (output_buffer)
					free(output_buffer);

				CreateError(error.str().c_str());
				inflateEnd(&strm);
				return;
			}

			output_buffer = output_buffer_pointer;
			output_buffer_pointer += _msize(output_buffer) - 1024;
			strm.avail_out = 1024;
			strm.next_out = output_buffer_pointer;
			ret = inflate(&strm, Z_FINISH);
			if (ret < Z_OK)
			{
				std::stringstream error;
				error << "Error with decompression, inflate() returned error " << ret
					<< ". Zlib error: " << (strm.msg ? strm.msg : "");
				free(output_buffer);
				inflateEnd(&strm);
				CreateError(error.str().c_str());
				return;
			}

		} while (strm.avail_in != 0);

		if (ret < 0)
		{
			std::stringstream error;
			error << "Error with decompression: " << ret << ". Zlib error: " << (strm.msg ? strm.msg : "");
			inflateEnd(&strm);
			CreateError(error.str().c_str());
			return;
		}
		inflateEnd(&strm);

		// Update all extensions with new message content.
		char * ThisMsg = threadData.receivedMsg.content;
		free(threadData.receivedMsg.content);

		for (std::vector<SaveExtInfo *>::iterator i = Saved.begin(); i != Saved.end(); ++i)
		{
			if ((char *)output_buffer == (*i)->receivedMsg.content)
				continue;
			(*i)->receivedMsg.content = (char *)output_buffer;
			(*i)->receivedMsg.cursor = 0;
			(*i)->receivedMsg.size = _msize(output_buffer);
		}
	}
}
void Extension::MoveReceivedBinaryCursor(int position)
{
	if (position < 0)
		CreateError("Cannot move cursor; Position less than 0.");
	else if (threadData.receivedMsg.size - position <= 0)
		CreateError("Cannot move cursor; Message is too small.");
	else
		threadData.receivedMsg.cursor = position;
}
void Extension::LoopListedChannelsWithLoopName(char * loopName)
{
	if (loopName[0] == '\0')
		CreateError("Cannot loop listed channels: invalid loop name supplied.");
	else
	{
		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));
		
		for (auto Selected = Cli.firstchannellisting(); Selected; Selected = Selected->next())
		{
			ClearThreadData();
			threadData.channelListing = Selected;
			threadData.loop.name = _strdup(loopName);
			Runtime.GenerateEvent(59);
		}

		memcpy_s(&threadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
		free(Stored);

		threadData.loop.name = _strdup(loopName);
		Runtime.GenerateEvent(60);
	}
}
void Extension::LoopClientChannelsWithLoopName(char * loopName)
{
	if (loopName[0] == '\0')
		CreateError("Cannot loop client channels: invalid loop name supplied.");
	else
	{
		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));
		
		for (auto LoopChannel : Channels)
		{
			if (!LoopChannel->isclosed)
			{
				ClearThreadData();
				threadData.channel = LoopChannel;
				threadData.loop.name = _strdup(loopName);
				Runtime.GenerateEvent(63);
			}
		}

		memcpy_s(&threadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
		free(Stored);

		threadData.loop.name = _strdup(loopName);
		Runtime.GenerateEvent(64);
	}
}
void Extension::LoopPeersOnChannelWithLoopName(char * loopName)
{
	if (loopName[0] == '\0')
		CreateError("Cannot loop peers on channel: invalid loop name supplied.");
	else if (!threadData.channel)
		CreateError("Cannot loop peers on channel: No channel currently selected.");
	else if (threadData.channel->isclosed)
		CreateError("Cannot loop peers on channel: Channel is closed.");
	else
	{
		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &threadData, sizeof(SaveExtInfo));

		ChannelCopy * StoredChannel = threadData.channel;
		auto& peers = threadData.channel->getpeers();

		for (auto LoopPeer : peers)
		{
			if (!LoopPeer->isclosed)
			{
				ClearThreadData();
				threadData.channel = StoredChannel;
				threadData.peer = LoopPeer;
				threadData.loop.name = _strdup(loopName);
				Runtime.GenerateEvent(61);
			}
		}

		memcpy_s(&threadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
		free(Stored);

		threadData.loop.name = _strdup(loopName);
		Runtime.GenerateEvent(62);
	}
}
void Extension::Connect(char * hostname)
{
	if (hostname[0] == '\0')
		CreateError("Cannot connect to server: invalid hostname supplied.");
	else 
	{
		int Port = 6121;
		if (strchr(hostname, ':'))
		{
			Port = atoi(strchr(hostname, ':')+1);
		
			if (Port < 0 || Port > 0xFFFF)
				return CreateError("Invalid port in hostname: too many numbers. Ports are limited from 0 to 65535.");
		}
		Cli.connect(hostname, Port);
	}
}
void Extension::ResizeBinaryToSend(int newSize)
{
	if (newSize < 0)
		CreateError("Cannot change size of binary to send: new size is under 0 bytes.");
	else
	{
		char * NewMsg = (char *)realloc(SendMsg, newSize);
		if (!NewMsg)
		{
			return CreateError("Cannot change size of binary to send: reallocation of memory failed. "
							   "Size has not been modified.");
		}
		// Clear new bytes to 0
		memset(NewMsg + SendMsgSize, 0, newSize - SendMsgSize);
			
		SendMsg = NewMsg;
		SendMsgSize = newSize;
	}
}
void Extension::SetDestroySetting(int enabled)
{
	if (enabled > 1 || enabled < 0)
		return CreateError("Invalid setting passed to SetDestroySetting, expecting 0 or 1.");
	globals->fullDeleteEnabled = enabled != 0;
}
