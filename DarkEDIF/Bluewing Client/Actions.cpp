#include "Common.h"

#ifdef _DEBUG
std::stringstream CriticalSection;
#endif

#define Remake(name) MessageBoxA(NULL, "Your "#name" actions need to be recreated.\r\n" \
									   "This is probably due to parameter changes.", "Lacewing Relay Client - DarkEDIF", MB_OK)
#define Saved (Globals->_Saved)

void Extension::Replaced_Connect(char * Hostname, int Port)
{
	Remake("Connect");
}
void Extension::Disconnect()
{
	Cli.disconnect();
}
void Extension::SetName(char * Name)
{
	if (!Name)
		CreateError("Error: SetName() was called with a null parameter.");
	else if (Name[0] == '\0')
		CreateError("Error: SetName() was called with \"\".");
	else
		Cli.name(Name);
}
void Extension::Replaced_JoinChannel(char * ChannelName, int HideChannel)
{
	Remake("Join channel");
}
void Extension::LeaveChannel()
{
	if (ThreadData.Channel && ThreadData.Channel->isclosed)
		ThreadData.Channel->orig().leave(); // don't set isclosed, leave not approved by server yet
}
void Extension::SendTextToServer(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Text to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Send Text to Server was called with a null parameter.");
	else
		Cli.sendserver(Subchannel, TextToSend, strlen(TextToSend)+1, 0);
}
void Extension::SendTextToChannel(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Text to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Send Text to Channel was called with a null parameter.");
	else if (!ThreadData.Channel)
		CreateError("Error: Send Text to Channel was called without a channel being selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Error: Send Text to Channel was called with a closed channel.");
	else 
		ThreadData.Channel->send(Subchannel, TextToSend);
}
void Extension::SendTextToPeer(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Text to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Send Text to Peer was called with a null parameter.");
	else if (!ThreadData.Peer)
		CreateError("Error: Send Text to Peer was called without a peer being selected.");
	else if (ThreadData.Peer->isclosed)
		CreateError("Error: Send Text to Peer was called with a closed peer.");
	else
		ThreadData.Peer->send(Subchannel, TextToSend);
}
void Extension::SendNumberToServer(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Number to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else
		Cli.sendserver(Subchannel, (char *)&NumToSend, sizeof(int), 1);
}
void Extension::SendNumberToChannel(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Send Number to Channel was called without a channel being selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Error: Send Number to Channel was called with a closed channel.");
	else
		ThreadData.Channel->send(Subchannel, (char *)&NumToSend, sizeof(int), 1);
}
void Extension::SendNumberToPeer(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Number to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Peer)
		CreateError("Error: Send Number to Peer was called without a peer being selected.");
	else if (ThreadData.Peer->isclosed)
		CreateError("Error: Send Number to Peer was called with a closed peer.");
	else
		ThreadData.Peer->send(Subchannel, (char *)&NumToSend, sizeof(int), 1);
}
void Extension::BlastTextToServer(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Text to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Blast Text to Server was called with a null parameter.");
	else
		Cli.blastserver(Subchannel, TextToSend);
}
void Extension::BlastTextToChannel(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Text to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Blast Text to Channel was called with a null parameter.");
	else if (!ThreadData.Channel)
		CreateError("Error: Blast Text to Channel was called without a channel being selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Error: Blast Text to Channel was called with a closed channel.");
	else
		ThreadData.Channel->blast(Subchannel, TextToSend);
}
void Extension::BlastTextToPeer(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Text to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Blast Text to Peer was called with a null parameter.");
	else if (!ThreadData.Peer)
		CreateError("Error: Blast Text to Peer was called without a peer being selected.");
	else if (ThreadData.Peer->isclosed)
		CreateError("Error: Blast Text to Peer was called with a closed peer.");
	else
		ThreadData.Peer->blast(Subchannel, TextToSend);
}
void Extension::BlastNumberToServer(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Number to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else
		Cli.blastserver(Subchannel, (char *)&NumToSend, sizeof(int), 1);
}
void Extension::BlastNumberToChannel(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Blast Number to Channel was called without a channel being selected.");
	else
		ThreadData.Channel->blast(Subchannel, (char *) &NumToSend, 4, 1);
}
void Extension::BlastNumberToPeer(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Number to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Peer)
		CreateError("Error: Blast Number to Peer was called without a peer being selected.");
	else if (ThreadData.Peer->isclosed)
		CreateError("Error: Blast Number to Peer was called with a closed peer.");
	else
		ThreadData.Peer->blast(Subchannel, (char *) &NumToSend, 4, 1);
}
void Extension::SelectChannelWithName(char * ChannelName)
{
	auto SelectedI = std::find_if(Channels.begin(), Channels.end(),
		[=](ChannelCopy * c) { return !_stricmp(c->name(), ChannelName); });
	auto Selected = SelectedI == Channels.end() ? nullptr : *SelectedI;

	// Only modify ThreadData.Channel if we found it
	if (Selected && !Selected->isclosed)
		ThreadData.Channel = Selected;
	else
	{
		std::string Error = "Could not select channel, name not found:\r\n";
		Error += ChannelName;
		CreateError(Error.c_str());
	}
}
void Extension::ReplacedNoParams()
{
	Remake("Unknown action");
}
void Extension::LoopClientChannels()
{
	char * Stored = (char *)malloc(sizeof(SaveExtInfo));
	memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));

	for (auto Selected : Channels)
	{
		if (!Selected->isclosed)
		{
			ClearThreadData();
			ThreadData.Channel = Selected;
			Runtime.GenerateEvent(14);
		}
	}

	memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
	free(Stored);

	Runtime.GenerateEvent(18);
}
void Extension::SelectPeerOnChannelByName(char * PeerName)
{
	if (!PeerName || PeerName[0] == '\0')
		CreateError("Error: Select Peer On Channel By Name was called with a null parameter/blank name.");
	else if (!ThreadData.Channel)
		CreateError("Error: Select Peer On Channel By Name was called without a channel being selected.");
	else
	{
		auto &peers = ThreadData.Channel->getpeers();
		auto Selected = std::find_if(peers.begin(), peers.end(), [=](PeerCopy * const P) {
			return !_stricmp(P->name(), PeerName);
		});

		// Only modify ThreadData.Peer if we found it
		if (Selected != peers.cend())
			ThreadData.Peer = *Selected;
		else
		{
			std::stringstream Error;
			Error << "Peer with name " << PeerName 
				<< " not found on channel " << ThreadData.Channel->name() << ".";
			CreateError(Error.str().c_str());
		}
	}
}
void Extension::SelectPeerOnChannelByID(int PeerID)
{
	if (PeerID < 0)
		CreateError("Could not select peer on channel, ID is below 0.");
	else if (!ThreadData.Channel)
		CreateError("Error: Select Peer On Channel By ID was called without a channel being selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Error: Select Peer On Channel By ID was called with a closed channel.");
	else
	{
		auto &peers = ThreadData.Channel->getpeers();
		auto Selected = std::find_if(peers.cbegin(), peers.cend(), [=](PeerCopy * const &P) {
			return P->id() == PeerID;
		});
		
		// Only modify ThreadData.Peer if we found it
		if (Selected != peers.cend())
			ThreadData.Peer = *Selected;
		else
		{
			std::stringstream Error;
			Error << "Peer with ID " << PeerID
				<< " not found on channel " << ThreadData.Channel->name() << ".";
			CreateError(Error.str().c_str());
		}
	}
}
void Extension::LoopPeersOnChannel()
{
	// Store selected channel
	if (!ThreadData.Channel)
		CreateError("Error: Loop Peers On Channel was called without a channel being selected.");
	else
	{
		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));

		auto StoredChannel = ThreadData.Channel;
		auto& peers = ThreadData.Channel->getpeers();
		for (auto Selected : peers)
		{
			if (!Selected->isclosed)
			{
				ClearThreadData();
				ThreadData.Channel = StoredChannel;
				ThreadData.Peer = Selected;
				Runtime.GenerateEvent(13);
			}
		}
		memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
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
	memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));
	
	for (auto Selected = Cli.firstchannellisting(); Selected; Selected = Selected->next())
	{
		ClearThreadData();
		ThreadData.ChannelListing = Selected;
		Runtime.GenerateEvent(27);
	}

	memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
	free(Stored);

	Runtime.GenerateEvent(28);
}
void Extension::SendBinaryToServer(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else
		Cli.sendserver(Subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToChannel(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Send Binary to Channel was called without a channel being selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Error: Send Binary to Channel was called with a closed channel.");
	else
		ThreadData.Channel->send(Subchannel, SendMsg, SendMsgSize, 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToPeer(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Peer)
		CreateError("Error: Send Binary to Peer was called without a peer being selected.");
	else if (ThreadData.Peer->isclosed)
		CreateError("Error: Send Binary to Peer was called with a closed peer.");
	else
		ThreadData.Peer->send(Subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToServer(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else
		Cli.blastserver(Subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToChannel(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Blast Binary to Channel was called without a channel being selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Error: Blast Binary to Channel was called with a closed channel.");
	else
		ThreadData.Channel->blast(Subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToPeer(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Peer)
		CreateError("Error: Blast Binary to Peer was called without a peer being selected.");
	else if (ThreadData.Peer->isclosed)
		CreateError("Error: Blast Binary to Peer was called with a closed peer.");
	else
		ThreadData.Peer->blast(Subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::AddByteText(char * Byte)
{
	if (!Byte || strnlen(Byte, 2) != 1)
		CreateError("Adding byte to stack failed: byte supplied was part of a string, not a single byte.");
	else
		AddToSend(Byte, 1);
}
void Extension::AddByteInt(int Byte)
{
	if (Byte > 255 || Byte < -128)
		CreateError("Error: Byte out of bounds.");
	else
	{
		if (Byte < 0)
		{
			char RealByte = (char)Byte;
			AddToSend(&RealByte, 1);
		}
		else
		{
			unsigned char RealByte = (unsigned char)Byte;
			AddToSend(&RealByte, 1);
		}
	}
}
void Extension::AddShort(int Short)
{
	if (Short > 65535 || Short < -32768)
		CreateError("Error: Short out of bounds.");
	else
	{
		if (Short < 0)
		{
			short RealShort = (short)Short;
			AddToSend(&RealShort, 2);
		}
		else
		{
			unsigned short RealShort = (unsigned short)Short;
			AddToSend(&RealShort, 2);
		}
	}
}
void Extension::AddInt(int Int)
{
	AddToSend(&Int, 4);
}
void Extension::AddFloat(float Float)
{
	AddToSend(&Float, 4);
}
void Extension::AddStringWithoutNull(char * String)
{
	if (String)
		AddToSend(String, strlen(String));
	else
		CreateError("Adding string without null failed: pointer was null.");
}
void Extension::AddString(char * String)
{
	if (String)
		AddToSend(String, strlen(String)+1);
	else
		CreateError("Adding string failed: pointer was null.");
}
void Extension::AddBinary(void * Address, int Size)
{
	if (Size < 0)
		CreateError("Add binary failed: Size < 0.");
	else 
	{
		if (Size != 0)
			AddToSend(Address, Size);
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
void Extension::SaveReceivedBinaryToFile(int Position, int Size, char * Filename)
{
	if (Position < 0)
		CreateError("Cannot save received binary; Position less than 0.");
	else if (Size <= 0)
		CreateError("Cannot save received binary; Size equal or less than 0.");
	else if (!Filename || Filename[0] == '\0')
		CreateError("Cannot save received binary; filename is invalid.");
	else if (((unsigned int)Position) + Size > ThreadData.ReceivedMsg.Size)
	{
		std::stringstream Error;
		Error << "Cannot save received binary to file; message doesn't have " << Size
			<< " bytes at position " << Position << " onwards.";
		CreateError(Error.str().c_str());
		return;
	}
	else
	{
		FILE * File = _fsopen(Filename, "wb", SH_DENYWR);
		if (!File)
		{
			std::stringstream Error;
			Error << "Cannot save received binary to file, error number " << errno
				<< " occured with opening the file. The message has not been modified.";
			CreateError(Error.str().c_str());
			return;
		}

		size_t amountWritten;
		if ((amountWritten = fwrite(ThreadData.ReceivedMsg.Content+Position, 1, Size, File)) != Size)
		{
			std::stringstream Error;
			Error << "Cannot save received binary to file, error number " << errno 
				<< " occured with writing the file. Wrote " << amountWritten
				<< " bytes total. The message has not been modified.";
			CreateError(Error.str().c_str());
		}

		fclose(File);
	}
}
void Extension::AppendReceivedBinaryToFile(int Position, int Size, char * Filename)
{
	if (Position < 0)
		CreateError("Cannot append received binary to file; position less than 0.");
	else if (Size <= 0)
		CreateError("Cannot append received binary to file; size equal or less than 0.");
	else if (!Filename || Filename[0] == '\0')
		CreateError("Cannot append received binary to file; filename is invalid.");
	else if (((unsigned int)Position) + Size > ThreadData.ReceivedMsg.Size)
	{
		std::stringstream Error;
		Error << "Cannot append received binary to file; message doesn't have " << Size
			<< " bytes at position " << Position << " onwards.";
		CreateError(Error.str().c_str());
		return;
	}
	else
	{
		// Open while denying write of other programs
		FILE * File = _fsopen(Filename, "ab", SH_DENYWR);
		if (!File)
		{
			std::stringstream Error;
			Error << "Cannot append received binary to file, error number " << errno
				<< " occured with opening the file. The binary message has not been modified.";
			CreateError(Error.str().c_str());
			return;
		}

		size_t amountWritten;
		if ((amountWritten = fwrite(ThreadData.ReceivedMsg.Content+Position, 1, Size, File)) != Size)
		{
			fseek(File, 0, SEEK_END);
			long long filesize = _ftelli64(File);

			std::stringstream Error;
			Error << "Cannot append received binary to file, error number " << errno
				<< " occured with writing the file. Wrote " << amountWritten
				<< " bytes, leaving file at size " << filesize << " total. The binary message has not been modified.";
			CreateError(Error.str().c_str());
		}

		fclose(File);
	}
}
void Extension::AddFileToBinary(char * Filename)
{
	if (!Filename || Filename[0] == '\0')
		CreateError("Cannot add file to send binary; filename is invalid.");
	else
	{
		// Open and deny other programs write priviledges
		FILE * File = _fsopen(Filename, "rb", _SH_DENYWR);
		if (!File)
		{
			std::stringstream Error;
			Error << "Cannot add file to send binary, error number " << errno 
				<< " occured with opening the file. The send binary has not been modified.";
			CreateError(Error.str().c_str());
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
			std::stringstream Error;
			Error << "Couldn't read file \"" << Filename << "\" into binary to send; couldn't reserve enough memory "
				"to add file into message. The send binary has not been modified.";
			CreateError(Error.str().c_str());
		}
		else
		{
			size_t amountRead;
			if ((amountRead = fread_s(buffer, filesize, 1, filesize, File)) != filesize)
			{
				std::stringstream Error;
				Error << "Couldn't read file \"" << Filename << "\" into binary to send, error number " << errno
					<< " occured with opening the file. The send binary has not been modified.";
				CreateError(Error.str().c_str());
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
	if (!ThreadData.Channel)
		CreateError("Could not select channel master: No channel selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Could not select channel master: Channel is closed.");
	else
	{
		PeerCopy * Stored = ThreadData.Peer;
		ThreadData.Peer = ThreadData.Channel->channelmaster();

		// Restore if no channel master found
		if (!ThreadData.Peer || ThreadData.Peer->isclosed)
			ThreadData.Peer = Stored;
	}
}
void Extension::JoinChannel(char * ChannelName, int Hidden, int CloseAutomatically)
{
	if (!ChannelName || ChannelName[0] == '\0')
		CreateError("Cannot join channel: invalid channel name supplied.");
	else if (!Cli.name())
		CreateError("Cannot join channel: client name not set.");
	else
		Cli.join(ChannelName, Hidden != 0, CloseAutomatically != 0);
}
void Extension::CompressSendBinary()
{
	if (SendMsgSize <= 0)
		CreateError("Cannot compress send binary; Message is too small.");
	else
	{
		int ret;
		z_stream strm = {0};

		ret = deflateInit(&strm, 9);
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
	if (ThreadData.ReceivedMsg.Size <= 0)
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
		strm.next_in = (unsigned char *)ThreadData.ReceivedMsg.Content;
		strm.avail_in = ThreadData.ReceivedMsg.Size;
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
		char * ThisMsg = ThreadData.ReceivedMsg.Content;
		free(ThreadData.ReceivedMsg.Content);

		for (std::vector<SaveExtInfo *>::iterator i = Saved.begin(); i != Saved.end(); ++i)
		{
			if ((char *)output_buffer == (*i)->ReceivedMsg.Content)
				continue;
			(*i)->ReceivedMsg.Content = (char *)output_buffer;
			(*i)->ReceivedMsg.Cursor = 0;
			(*i)->ReceivedMsg.Size = _msize(output_buffer);
		}
	}
}
void Extension::MoveReceivedBinaryCursor(int Position)
{
	if (Position < 0)
		CreateError("Cannot move cursor; Position less than 0.");
	else if (ThreadData.ReceivedMsg.Size - Position <= 0)
		CreateError("Cannot move cursor; Message is too small.");
	else
		ThreadData.ReceivedMsg.Cursor = Position;
}
void Extension::LoopListedChannelsWithLoopName(char * LoopName)
{
	if (LoopName[0] == '\0')
		CreateError("Cannot loop listed channels: invalid loop name supplied.");
	else
	{
		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));
		
		for (auto Selected = Cli.firstchannellisting(); Selected; Selected = Selected->next())
		{
			ClearThreadData();
			ThreadData.ChannelListing = Selected;
			ThreadData.Loop.Name = _strdup(LoopName);
			Runtime.GenerateEvent(59);
		}

		memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
		free(Stored);

		ThreadData.Loop.Name = _strdup(LoopName);
		Runtime.GenerateEvent(60);
	}
}
void Extension::LoopClientChannelsWithLoopName(char * LoopName)
{
	if (LoopName[0] == '\0')
		CreateError("Cannot loop client channels: invalid loop name supplied.");
	else
	{
		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));
		
		for (auto LoopChannel : Channels)
		{
			if (!LoopChannel->isclosed)
			{
				ClearThreadData();
				ThreadData.Channel = LoopChannel;
				ThreadData.Loop.Name = _strdup(LoopName);
				Runtime.GenerateEvent(63);
			}
		}

		memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
		free(Stored);

		ThreadData.Loop.Name = _strdup(LoopName);
		Runtime.GenerateEvent(64);
	}
}
void Extension::LoopPeersOnChannelWithLoopName(char * LoopName)
{
	if (LoopName[0] == '\0')
		CreateError("Cannot loop peers on channel: invalid loop name supplied.");
	else if (!ThreadData.Channel)
		CreateError("Cannot loop peers on channel: No channel currently selected.");
	else if (ThreadData.Channel->isclosed)
		CreateError("Cannot loop peers on channel: Channel is closed.");
	else
	{
		char * Stored = (char *)malloc(sizeof(SaveExtInfo));
		memcpy_s(Stored, sizeof(SaveExtInfo), &ThreadData, sizeof(SaveExtInfo));

		ChannelCopy * StoredChannel = ThreadData.Channel;
		auto& peers = ThreadData.Channel->getpeers();

		for (auto LoopPeer : peers)
		{
			if (!LoopPeer->isclosed)
			{
				ClearThreadData();
				ThreadData.Channel = StoredChannel;
				ThreadData.Peer = LoopPeer;
				ThreadData.Loop.Name = _strdup(LoopName);
				Runtime.GenerateEvent(61);
			}
		}

		memcpy_s(&ThreadData, sizeof(SaveExtInfo), Stored, sizeof(SaveExtInfo));
		free(Stored);

		ThreadData.Loop.Name = _strdup(LoopName);
		Runtime.GenerateEvent(62);
	}
}
void Extension::Connect(char * Hostname)
{
	if (!Hostname || Hostname[0] == '\0')
		CreateError("Cannot connect to server: invalid hostname supplied.");
	else 
	{
		int Port = 6121;
		if (strchr(Hostname, ':'))
		{
			Port = atoi(strchr(Hostname, ':')+1);
		
			if (Port < 0 || Port > 0xFFFF)
				return CreateError("Invalid port in hostname: too many numbers. Ports are limited from 0 to 65535.");
		}
		Cli.connect(Hostname, Port);
	}
}
void Extension::ResizeBinaryToSend(int NewSize)
{
	if (NewSize < 0)
		CreateError("Cannot change size of binary to send: new size is under 0 bytes.");
	else
	{
		char * NewMsg = (char *)realloc(SendMsg, NewSize);
		if (!NewMsg)
		{
			return CreateError("Cannot change size of binary to send: reallocation of memory failed. "
							   "Size has not been modified.");
		}
		// Clear new bytes to 0
		memset(NewMsg + SendMsgSize, 0, NewSize - SendMsgSize);
			
		SendMsg = NewMsg;
		SendMsgSize = NewSize;
	}
}
void Extension::SetDestroySetting(int enabled)
{
	if (enabled > 1 || enabled < 0)
		return CreateError("Illegal setting passed to SetDestroySetting, expecting 0 or 1.");
	Globals->FullDeleteEnabled = enabled != 0;
}
