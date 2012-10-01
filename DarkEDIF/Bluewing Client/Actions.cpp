
#include "Common.h"

#define Remake(name) MessageBoxA(NULL, "Your "#name" actions need to be recreated.\r\n" \
									   "This is probably due to parameter changes.", "Lacewing Relay Client - DarkEDIF", MB_OK)
void Extension::Replaced_Connect(char * Hostname, int Port)
{
	Remake("Connect");
}
void Extension::Disconnect()
{
	Cli.Disconnect();
}
void Extension::SetName(char * Name)
{
	if (!Name)
		CreateError("Error: SetName() was called with a null parameter.");
	else if (Name[0] == '\0')
		CreateError("Error: SetName() was called with \"\".");
	else
		Cli.Name(Name);
}
void Extension::Replaced_JoinChannel(char * ChannelName, int HideChannel)
{
	Remake("Join channel");
}
void Extension::LeaveChannel()
{
	if (ThreadData.Channel)
		ThreadData.Channel->Leave();
}
void Extension::SendTextToServer(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Text to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Send Text to Server was called with a null parameter.");
	else
		Cli.SendServer(Subchannel, TextToSend, strlen(TextToSend)+1, 0);
}
void Extension::SendTextToChannel(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Text to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Send Text to Channel was called with a null parameter.");
	else if (!ThreadData.Channel)
		CreateError("Error: Send Text to Channel was called without a channel being selected.");
	else
		ThreadData.Channel->Send(Subchannel, TextToSend, strlen(TextToSend)+1, 0);
}
void Extension::SendTextToPeer(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Text to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Send Text to Peer was called with a null parameter.");
	else if (!ThreadData.Peer)
		CreateError("Error: Send Text to Peer was called without a peer being selected.");
	else
		ThreadData.Peer->Send(Subchannel, TextToSend, strlen(TextToSend)+1, 0);
}
void Extension::SendNumberToServer(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Number to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else
		Cli.SendServer(Subchannel, (char *)&NumToSend, sizeof(int), 1);
}
void Extension::SendNumberToChannel(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Send Number to Channel was called without a channel being selected.");
	else
		ThreadData.Channel->Send(Subchannel, (char *)&NumToSend, sizeof(int), 1);
}
void Extension::SendNumberToPeer(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Send Number to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Peer)
		CreateError("Error: Send Number to Peer was called without a peer being selected.");
	else
		ThreadData.Peer->Send(Subchannel, (char *)&NumToSend, sizeof(int), 1);
}
void Extension::BlastTextToServer(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Text to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Blast Text to Server was called with a null parameter.");
	else
		ThreadData.Peer->Blast(Subchannel, TextToSend, strlen(TextToSend)+1, 0);
}
void Extension::BlastTextToChannel(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Text to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Blast Text to Channel was called with a null parameter.");
	else if (!ThreadData.Channel)
		CreateError("Error: Blast Text to Channel was called without a channel being selected.");
	else
		ThreadData.Channel->Blast(Subchannel, TextToSend, strlen(TextToSend)+1, 0);
}
void Extension::BlastTextToPeer(int Subchannel, char * TextToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Text to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!TextToSend)
		CreateError("Error: Blast Text to Peer was called with a null parameter.");
	else if (!ThreadData.Peer)
		CreateError("Error: Blast Text to Peer was called without a peer being selected.");
	else
		ThreadData.Peer->Blast(Subchannel, TextToSend, strlen(TextToSend)+1, 0);
}
void Extension::BlastNumberToServer(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Number to Server was called with an invalid subchannel; it must be between 0 and 255.");
	else
		Cli.BlastServer(Subchannel, (char *)&NumToSend, sizeof(int), 1);
}
void Extension::BlastNumberToChannel(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Number to Channel was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Blast Number to Channel was called without a channel being selected.");
	else
		ThreadData.Channel->Blast(Subchannel, (char *) &NumToSend, 4, 1);
}
void Extension::BlastNumberToPeer(int Subchannel, int NumToSend)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Blast Number to Peer was called with an invalid subchannel; it must be between 0 and 255.");
	else if (!ThreadData.Peer)
		CreateError("Error: Blast Number to Peer was called without a peer being selected.");
	else
		ThreadData.Peer->Blast(Subchannel, (char *) &NumToSend, 4, 1);
}
void Extension::SelectChannelWithName(char * ChannelName)
{
	Lacewing::RelayClient::Channel * Selected = Cli.FirstChannel();
	while (Selected)
	{
		if (!_stricmp(Selected->Name(), ChannelName))
			break;
		Selected = Selected->Next();
	}
	
	// Only modify ThreadData.Channel if we found it
	if (Selected)
		ThreadData.Channel = (Lacewing::RelayClient::Channel *)Selected->Tag;
	else
	{
		std::string Error = "Could not selected channel, not found:\r\n";
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
	// Store selected channel
	Lacewing::RelayClient::Channel * Stored = ThreadData.Channel,
								   * Selected = Cli.FirstChannel();
	while (Selected)
	{
		SaveExtInfo &S = AddEvent(14);
		S.Channel = (Lacewing::RelayClient::Channel *)Selected->Tag;
		Selected = Selected->Next();
	}
	AddEvent(18);
}
void Extension::SelectPeerOnChannelByName(char * PeerName)
{
	if (!PeerName || PeerName[0] == '\0')
		CreateError("Error: Select Peer On Channel By Name was called with a null parameter/blank name.");
	else if (!ThreadData.Channel)
		CreateError("Error: Select Peer On Channel By Name was called without a channel being selected.");
	else
	{
		Lacewing::RelayClient::Channel::Peer * Selected = ThreadData.Channel->FirstPeer();
		while (Selected)
		{
			if (!_stricmp(Selected->Name(), PeerName))
				break;
			Selected = Selected->Next();
		}

		// Only modify ThreadData.Peer if we found it
		if (Selected)
			ThreadData.Peer = Selected;
		else
		{
			std::string Error = "Peer not found:\r\n";
			Error += PeerName;
			Error += "\r\nOn channel:\r\n";
			Error += ThreadData.Channel->Name();
			CreateError(Error.c_str());
		}
	}
}
void Extension::SelectPeerOnChannelByID(int PeerID)
{
	if (PeerID < 0)
		CreateError("Could not select peer on channel, ID is below 0.");
	else if (!ThreadData.Channel)
		CreateError("Error: Select Peer On Channel By ID was called without a channel being selected.");
	else
	{
		Lacewing::RelayClient::Channel::Peer * Selected = ThreadData.Channel->FirstPeer();
		while (Selected)
		{
			if (Selected->ID() == PeerID)
				break;
			Selected = Selected->Next();
		}

		// Only modify ThreadData.Peer if we found it
		if (Selected)
			ThreadData.Peer = Selected;
		else
		{
			char num[20];
			std::string Error = "Peer ID ";
			if (_itoa_s(PeerID, num, 20, 10))
				Error += "> could not be copied <";
			else
				Error += &num[0];
			Error += " not found on channel:";
			Error += ThreadData.Channel->Name();
			CreateError(Error.c_str());
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
		Lacewing::RelayClient::Channel::Peer * Stored = ThreadData.Peer,
											 * Selected = ThreadData.Channel->FirstPeer();
		while (Selected)
		{
			SaveExtInfo &S = AddEvent(13);
			S.Peer = (Lacewing::RelayClient::Channel::Peer *)Selected->Tag;
			Selected = Selected->Next();
		}
		SaveExtInfo &S = AddEvent(17);
		S.Peer = Stored;
	}
}
void Extension::RequestChannelList()
{
	Cli.ListChannels();
}
void Extension::LoopListedChannels()
{
	Lacewing::RelayClient::ChannelListing * Selected = Cli.FirstChannelListing();
	while (Selected)
	{
		SaveExtInfo &S = AddEvent(27); // Catch first listing by this being first
		S.ChannelListing = Selected;
		Selected = Selected->Next();
	}
	AddEvent(28);
}
void Extension::SendBinaryToServer(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else
		Cli.SendServer(Subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToChannel(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Send Binary to Channel was called without a channel being selected.");
	else
		ThreadData.Channel->Send(Subchannel, SendMsg, SendMsgSize, 2);

	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::SendBinaryToPeer(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Peer)
		CreateError("Error: Send Binary to Peer was called without a peer being selected.");
	else
		ThreadData.Peer->Send(Subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToServer(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else
		Cli.BlastServer(Subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToChannel(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Channel)
		CreateError("Error: Blast Binary to Channel was called without a channel being selected.");
	else
		ThreadData.Channel->Blast(Subchannel, SendMsg, SendMsgSize, 2);
	
	if (AutomaticallyClearBinary)
		ClearBinaryToSend();
}
void Extension::BlastBinaryToPeer(int Subchannel)
{
	if (Subchannel > 255 || Subchannel < 0)
		CreateError("Error: Subchannel invalid; it must be between 0 and 255.");
	else if (!ThreadData.Peer)
		CreateError("Error: Blast Binary to Peer was called without a peer being selected.");
	else
		ThreadData.Peer->Blast(Subchannel, SendMsg, SendMsgSize, 2);
	
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
		CreateError("Error: Byte out of scale.");
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
	else if (ThreadData.ReceivedMsg.Size-Size <= 0)
		CreateError("Cannot save received binary; Message is too small.");
	else
	{
		FILE * File = NULL;
		if (fopen_s(&File, Filename, "wb") || !File)
		{
			char errorval [20];
			SaveExtInfo &S = AddEvent(0);
			std::string Error = "Cannot save received binary to file, error ";
			if (_itoa_s(*_errno(), &errorval[0], 20, 10))
			{
				Error += " with opening the file, and with converting error number.";
			}
			else
			{
				Error += "number [";
				Error += &errorval[0];
				Error += "] occured with opening the file.";
			}
			Error += "\r\nThe message has not been modified.";
			S.Error.Text = _strdup(Error.c_str());
			return;
		}
		// Jump to end
		fseek(File, 0, SEEK_END);

		// Read current position as file size
		long long filesize = _ftelli64(File);

		// Go back to start
		fseek(File, 0, SEEK_SET);
		long l;
		if ((l = fwrite(ThreadData.ReceivedMsg.Content+Position, 1, Size, File)) != Size)
		{
			char sizeastext [20];
			SaveExtInfo &S = AddEvent(0);
			std::string Error = "Couldn't save the received binary to file, ";
			if (_itoa_s(errno, &sizeastext[0], 20, 10))
			{
				Error += " and error copying size.";
			}
			else
			{
				Error += &sizeastext[0];
				Error += " bytes managed to be written.";
			}
			S.Error.Text = _strdup(Error.c_str());
		}
		fclose(File);
	}
}
void Extension::AppendReceivedBinaryToFile(int Position, int Size, char * Filename)
{
	if (Position < 0)
		CreateError("Cannot append received binary; Position less than 0.");
	else if (Size <= 0)
		CreateError("Cannot append received binary; Size equal or less than 0.");
	else if (!Filename || Filename[0] == '\0')
		CreateError("Cannot append received binary; filename is invalid.");
	else if (ThreadData.ReceivedMsg.Size-Size <= 0)
		CreateError("Cannot append received binary; Message is too small.");
	else
	{
		// Open while denying write of other programs
		FILE * File = _fsopen(Filename, "ab", SH_DENYWR);
		if (!File)
		{
			char errorval [20];
			SaveExtInfo &S = AddEvent(0);
			std::string Error = "Cannot append received binary to file, error ";
			if (_itoa_s(*_errno(), &errorval[0], 20, 10))
			{
				Error += " with opening the file, and with converting error number.";
			}
			else
			{
				Error += "number [";
				Error += &errorval[0];
				Error += "] occured with opening the file.";
			}
			Error += "\r\nThe message has not been modified.";

			S.Error.Text = _strdup(Error.c_str());
			return;
		}

		long l;
		if ((l = fwrite(ThreadData.ReceivedMsg.Content+Position, 1, Size, File)) != Size)
		{
			char sizeastext [20];
			SaveExtInfo &S = AddEvent(0);
			std::string Error = "Couldn't append the received binary to file, ";
			if (_itoa_s(errno, &sizeastext[0], 20, 10))
			{
				Error += " and error copying size.";
			}
			else
			{
				Error += &sizeastext[0];
				Error += " bytes managed to be append.";
			}
			S.Error.Text = _strdup(Error.c_str());
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
		FILE * File = NULL;
		
		// Open and deny other programs write priviledges
		if (!(File = _fsopen(Filename, "wb", _SH_DENYWR)))
		{
			char errorval [20];
			SaveExtInfo &S = AddEvent(0);
			std::string Error = "Cannot save binary to file, error ";
			if (_itoa_s(*_errno(), &errorval[0], 20, 10))
			{
				Error += " with opening the file, and with converting error number.";
			}
			else
			{
				Error += "number [";
				Error += &errorval[0];
				Error += "] occured with opening the file.";
			}
			Error += "\r\nThe message has not been modified.";

			S.Error.Text = _strdup(Error.c_str());
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
			CreateError("Couldn't reserve enough memory to add file into message.");
		else
		{
			size_t s;
			if ((s = fread_s(buffer, filesize, 1, filesize, File)) != filesize)
			{
				char sizeastext [20];
				SaveExtInfo &S = AddEvent(0);
				std::string Error = "Couldn't write full buffer to file, ";
				if (_itoa_s(s, &sizeastext[0], 20, 10))
				{
					Error += " and error copying size.";
				}
				else
				{
					Error += &sizeastext[0];
					Error += " bytes managed to be written.";
				}
				S.Error.Text = _strdup(Error.c_str());
			}
			AddToSend(buffer, s);

			free(buffer);
		}
		fclose(File);
	} 
}
void Extension::SelectChannelMaster()
{
	if (!ThreadData.Channel)
		CreateError("Could not select channel master: No channel selected.");
	else
	{
		Lacewing::RelayClient::Channel::Peer * Stored = ThreadData.Peer;
	
		ThreadData.Peer = ThreadData.Channel->FirstPeer();
		while (ThreadData.Peer)
		{
			if (ThreadData.Peer->IsChannelMaster())
			{
				ThreadData.Peer = (Lacewing::RelayClient::Channel::Peer *)ThreadData.Peer->Tag;
				return; // Selected the correct peer
			}
			ThreadData.Peer = ThreadData.Peer->Next();
		}

		// Restore if no channel master found
		ThreadData.Peer = Stored;
	}
}
void Extension::JoinChannel(char * ChannelName, int Hidden, int CloseAutomatically)
{
	if (!ChannelName || ChannelName[0] == '\0')
		CreateError("Cannot join channel: invalid channel name supplied.");
	else
		Cli.Join(ChannelName, Hidden != 0, CloseAutomatically != 0);
}
void Extension::CompressSendBinary()
{
	if (SendMsgSize <= 0)
		CreateError("Cannot compress; Message is too small.");
	else
	{
		int ret;
		z_stream strm = {0};

		ret = deflateInit_(&strm, 9, ZLIB_VERSION, sizeof(z_stream));
		if (ret)
		{
			std::stringstream error;
			error << "Error " << ret << "occured with initiating compression.";
			CreateError(error.str().c_str());
			return;
		}
	
		unsigned char * output_buffer = (unsigned char *)malloc(SendMsgSize + 256);
		if (!output_buffer)
		{
			std::stringstream error;
			error << "Error, could not allocate enough memory. Desired " << SendMsgSize + 256 << "bytes.";
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
			error << "Error with compression, deflate() returned " << ret << "! Text: " << strm.msg ? strm.msg : "";
			free(output_buffer);
			deflateEnd(&strm);
			CreateError(error.str().c_str());
			return;
		}

		deflateEnd(&strm);

		void * v = realloc(output_buffer, strm.total_out);
		if (!v)
		{
			free(output_buffer);
			CreateError("Error with compression, reallocating memory failed.");
			return;
		}
		free(SendMsg);
	
		SendMsg = (char *)v;
		SendMsgSize = strm.total_out;
	}
}
void Extension::DecompressReceivedBinary()
{
	if (ThreadData.ReceivedMsg.Size <= 0)
		CreateError("Cannot decompress; Message is too small.");
	else
	{
		int ret;
		z_stream strm = {0};

		ret = inflateInit_(&strm, ZLIB_VERSION, sizeof(z_stream));
		if (ret)
		{
			std::stringstream error;
			error << "Error " << ret << "occured with initiating decompression.";
			CreateError(error.str().c_str());
			return;
		}
	
		unsigned char * output_buffer = NULL, * output_buffer_pointer = NULL;

		strm.next_in = (unsigned char *)ThreadData.ReceivedMsg.Content;
		strm.avail_in = ThreadData.ReceivedMsg.Size;

		// run inflate() on input until output buffer not full, finish
		// compression if all of source has been read in
		do {
			// Allocate memory for compression
			output_buffer_pointer = (unsigned char *)realloc(output_buffer, (output_buffer ? _msize(output_buffer) : 0)+1024);
			if (!output_buffer_pointer)
			{
				std::stringstream error;
				error << "Error, could not allocate enough memory. Desired " << (output_buffer ? _msize(output_buffer) : 0)+1024 << "bytes.";
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
				error << "Error with decompression, inflate() returned " << ret << "! Text: " << strm.msg ? strm.msg : "";
				free(output_buffer);
				CreateError(error.str().c_str());
				inflateEnd(&strm);
				return;
			}
			
		} while (strm.avail_in != 0);
		
		if (ret < 0)
		{
			std::stringstream error;
			error << "Error with decompression: " << ret << "! Text: " << strm.msg ? strm.msg : "";
			CreateError(error.str().c_str());
			inflateEnd(&strm);
			return;
		}
		inflateEnd(&strm);

		char * ThisMsg = ThreadData.ReceivedMsg.Content;
		free(ThreadData.ReceivedMsg.Content);
		ThreadData.ReceivedMsg.Content = (char *)output_buffer;
		ThreadData.ReceivedMsg.Cursor = 0;
		ThreadData.ReceivedMsg.Size = _msize(output_buffer);
		for (std::vector<SaveExtInfo *>::iterator i = Saved.begin(); i != Saved.end(); ++i)
		{
			if (ThisMsg != (*i)->ReceivedMsg.Content)
				break;
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
		Lacewing::RelayClient::ChannelListing * Selected = Cli.FirstChannelListing();
		while (Selected)
		{
			SaveExtInfo &S = AddEvent(59); // Catch first listing by this being first
			S.ChannelListing = Selected;
			S.Loop.Name = _strdup(LoopName);
			Selected = Selected->Next();
		}
		
		SaveExtInfo &S = AddEvent(60);
		S.Loop.Name = _strdup(LoopName);
	}
}
void Extension::LoopClientChannelsWithLoopName(char * LoopName)
{
	if (LoopName[0] == '\0')
		CreateError("Cannot loop listed channels: invalid loop name supplied.");
	else
	{
		Lacewing::RelayClient::Channel * Stored = ThreadData.Channel,
									   * LoopChannel = Cli.FirstChannel();
		while (LoopChannel)
		{
			SaveExtInfo &S = AddEvent(63);
			S.Channel = (Lacewing::RelayClient::Channel *)LoopChannel->Tag;
			S.Loop.Name = _strdup(LoopName);
			LoopChannel = LoopChannel->Next();
		}
		SaveExtInfo &S = AddEvent(64);
	}
}
void Extension::LoopPeersOnChannelWithLoopName(char * LoopName)
{
	if (LoopName[0] == '\0')
		CreateError("Cannot loop peers on channel: invalid loop name supplied.");
	else if (!ThreadData.Channel)
		CreateError("Cannot loop peers on channel: No channel currently selected.");
	else
	{
		Lacewing::RelayClient::Channel::Peer * Stored = ThreadData.Peer,
											 * LoopPeer = ThreadData.Channel->FirstPeer();
		while (LoopPeer)
		{
			SaveExtInfo &S = AddEvent(61);
			S.Peer = (Lacewing::RelayClient::Channel::Peer *)LoopPeer->Tag;
			S.Loop.Name = _strdup(LoopName);
			LoopPeer = LoopPeer->Next();
		}
		SaveExtInfo &S = AddEvent(62);
		S.Peer = Stored;
		S.Loop.Name = _strdup(LoopName);
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
		
			if (Port < 0 || Port > 35565)
				return CreateError("Invalid port in hostname: too many numbers. Ports are limited from 0 to 35565.");
		}
		Cli.Connect(Hostname, Port);
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
			return CreateError("Cannot change size of binary to send: reallocation of memory failed.\r\n"
							   "Size has not been modified.");
		}
		// Clear new bytes to 0
		memset(NewMsg + SendMsgSize, 0, NewSize - SendMsgSize);
			
		SendMsg = NewMsg;
		SendMsgSize = NewSize;
	}
}
