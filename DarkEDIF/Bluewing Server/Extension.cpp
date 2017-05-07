#include "Common.h"


///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(_rdPtr)
{
	// Create the thread lock
	InitializeCriticalSection(&Lock);

	// Nullify the thread-specific data
	memset(&ThreadData, 0, sizeof(SaveExtInfo));

	/*
        Link all your action/condition/expression functions to their IDs to match the
        IDs in the JSON here
    */
	{
		LinkAction(0, RelayServer_Host);
		LinkAction(1, RelayServer_StopHosting);
		LinkAction(2, SetWelcomeMessage);
		LinkAction(3, EnableCondition_OnMessageToChannel);
		LinkAction(4, EnableCondition_OnMessageToPeer);
		LinkAction(5, OnInteractive_Deny);
		LinkAction(6, OnInteractive_ChangePeerName);
		LinkAction(7, OnInteractive_ChangeChannelName);
		LinkAction(8, Channel_Close);
		LinkAction(9, Channel_SelectMaster);
		LinkAction(10, Channel_SelectByName);
		LinkAction(11, LoopAllChannels);
		LinkAction(12, Client_Disconnect);
		LinkAction(13, Client_LoopJoinedChannels);
		LinkAction(14, Client_SelectByName);
		LinkAction(15, Client_SelectByID);
		LinkAction(16, LoopAllClients);
		LinkAction(17, SendTextToClient);
		LinkAction(18, SendTextToChannel);
		LinkAction(19, SendNumberToClient);
		LinkAction(20, SendNumberToChannel);
		LinkAction(21, SendBinaryToClient);
		LinkAction(22, SendBinaryToChannel);
		LinkAction(23, BlastTextToClient);
		LinkAction(24, BlastTextToChannel);
		LinkAction(25, BlastNumberToClient);
		LinkAction(26, BlastNumberToChannel);
		LinkAction(27, BlastBinaryToClient);
		LinkAction(28, BlastBinaryToChannel);
		LinkAction(29, AddByteText);
		LinkAction(30, AddByteInt);
		LinkAction(31, AddShort);
		LinkAction(32, AddInt);
		LinkAction(33, AddFloat);
		LinkAction(34, AddStringWithoutNull);
		LinkAction(35, AddString);
		LinkAction(36, AddBinary);
		LinkAction(37, AddFileToBinary);
		LinkAction(38, CompressSendBinary);
		LinkAction(39, ClearBinaryToSend);
		LinkAction(40, SaveReceivedBinaryToFile);
		LinkAction(41, AppendReceivedBinaryToFile);
		LinkAction(42, DecompressReceivedBinary);
		LinkAction(43, Channel_LoopClients);
		LinkAction(44, OnInteractive_DropMessage);
		LinkAction(45, Client_SelectSender);
		LinkAction(46, Client_SelectReceiver);
		LinkAction(47, LoopAllChannelsWithName);
		LinkAction(48, LoopAllClientsWithName);
		LinkAction(49, Client_LoopJoinedChannelsWithName);
		LinkAction(50, FlashServer_Host);
		LinkAction(51, FlashServer_StopHosting);
		LinkAction(52, Client_SetLocalData);
		LinkAction(53, Channel_SetLocalData);
		LinkAction(54, ResizeBinaryToSend);
		// Added actions:
	}
	{
		LinkCondition(0, AlwaysTrue /* OnError */);
		LinkCondition(1, AlwaysTrue /* OnConnectRequest */);
		LinkCondition(2, AlwaysTrue /* OnDisconnect */);
		LinkCondition(3, AlwaysTrue /* OnChannel_JoinRequest */);
		LinkCondition(4, AlwaysTrue /* OnChannel_LeaveRequest */);
		LinkCondition(5, AlwaysTrue /* AllChannelsLoop */);
		LinkCondition(6, AlwaysTrue /* OnClient_JoinedChannelsLoop */);
		LinkCondition(7, AlwaysTrue /* AllClientsLoop */);
		LinkCondition(8, AlwaysTrue /* OnChannel_ClientLoop */);
		LinkCondition(9, Client_IsChannelMaster);
		LinkCondition(10, AlwaysTrue /* OnClient_NameSetRequest */);
		LinkCondition(11, SubchannelMatches /* OnSentTextMessageToServer */);
		LinkCondition(12, SubchannelMatches /* OnSentNumberMessageToServer */);
		LinkCondition(13, SubchannelMatches /* OnSentBinaryMessageToServer */);
		LinkCondition(14, SubchannelMatches /* OnAnySentMessageToServer */);
		LinkCondition(15, SubchannelMatches /* OnSentTextMessageToChannel */);
		LinkCondition(16, SubchannelMatches /* OnSentNumberMessageToChannel */);
		LinkCondition(17, SubchannelMatches /* OnSentBinaryMessageToChannel */);
		LinkCondition(18, SubchannelMatches /* OnAnySentMessageToChannel */);
		LinkCondition(19, SubchannelMatches /* OnSentTextMessageToPeer */);
		LinkCondition(20, SubchannelMatches /* OnSentNumberMessageToPeer */);
		LinkCondition(21, SubchannelMatches /* OnSentBinaryMessageToPeer */);
		LinkCondition(22, SubchannelMatches /* OnAnySentMessageToPeer */);
		LinkCondition(23, SubchannelMatches /* OnBlastedTextMessageToPeer */);
		LinkCondition(24, SubchannelMatches /* OnBlastedNumberMessageToServer */);
		LinkCondition(25, SubchannelMatches /* OnBlastedBinaryMessageToServer */);
		LinkCondition(26, SubchannelMatches /* OnAnyBlastedMessageToServer */);
		LinkCondition(27, SubchannelMatches /* OnBlastedTextMessageToChannel */);
		LinkCondition(28, SubchannelMatches /* OnBlastedNumberMessageToChannel */);
		LinkCondition(29, SubchannelMatches /* OnBlastedBinaryMessageToChannel */);
		LinkCondition(30, SubchannelMatches /* OnAnyBlastedMessageToChannel */);
		LinkCondition(31, SubchannelMatches /* OnBlastedTextMessageToPeer */);
		LinkCondition(32, SubchannelMatches /* OnBlastedNumberMessageToPeer */);
		LinkCondition(33, SubchannelMatches /* OnBlastedBinaryMessageToPeer */);
		LinkCondition(34, SubchannelMatches /* OnAnyBlastedMessageToPeer */);
		LinkCondition(35, OnAllChannelsLoopWithName);
		LinkCondition(36, OnClientsJoinedChannelLoopWithName);
		LinkCondition(37, OnAllClientsLoopWithName);
		LinkCondition(38, OnChannelClientsLoopWithName);
		LinkCondition(39, OnChannelClientsLoopWithNameFinished);
		LinkCondition(40, OnAllChannelsLoopWithNameFinished);
		LinkCondition(41, OnClientsJoinedChannelLoopWithNameFinished);
		LinkCondition(42, OnChannelsClientLoopFinished);
		LinkCondition(43, AlwaysTrue /* OnAllChannelsLoopFinished */);
		LinkCondition(44, AlwaysTrue /* OnAllClientsLoopFinished */);
		LinkCondition(45, AlwaysTrue /* OnClientJoinedChannelLoopFinished */);
		LinkCondition(46, IsLacewingServerHosting);
		LinkCondition(47, IsFlashPolicyServerHosting);
		LinkCondition(48, ChannelIsHiddenFromChannelList);
		LinkCondition(49, ChannelIsSetToCloseAutomatically);
		LinkCondition(50, OnAllClientsLoopWithNameFinished);
		// Added conditions:
		LinkCondition(51, IsClientOnChannel_ID);
		LinkCondition(52, IsClientOnChannel_Name);
		LinkCondition(53, IsHTML5Hosting);
	}
	{
		LinkExpression(0, Error);
		LinkExpression(1, Lacewing_Version);
		LinkExpression(2, BinaryToSend_Size);
		LinkExpression(3, RequestedPeerName);
		LinkExpression(4, RequestedChannelName);
		LinkExpression(5, Channel_Name);
		LinkExpression(6, Channel_ClientCount);
		LinkExpression(7, Client_Name);
		LinkExpression(8, Client_ID);
		LinkExpression(9, Client_IP);
		LinkExpression(10, Client_ConnectionTime);
		LinkExpression(11, Client_ChannelCount);
		LinkExpression(12, ReceivedStr);
		LinkExpression(13, ReceivedInt);
		LinkExpression(14, ReceivedBinarySize);
		LinkExpression(15, ReceivedBinaryAddress);
		LinkExpression(16, StrByte);
		LinkExpression(17, UnsignedByte);
		LinkExpression(18, SignedByte);
		LinkExpression(19, UnsignedShort);
		LinkExpression(20, SignedShort);
		LinkExpression(21, UnsignedInteger);
		LinkExpression(22, SignedInteger);
		LinkExpression(23, Float);
		LinkExpression(24, StringWithSize);
		LinkExpression(25, String);
		LinkExpression(26, Subchannel);
		LinkExpression(27, Channel_Count);
		LinkExpression(28, Client_GetLocalData);
		LinkExpression(29, CursorStrByte);
		LinkExpression(30, CursorUnsignedByte);
		LinkExpression(31, CursorSignedByte);
		LinkExpression(32, CursorUnsignedShort);
		LinkExpression(33, CursorSignedShort);
		LinkExpression(34, CursorUnsignedInteger);
		LinkExpression(35, CursorSignedInteger);
		LinkExpression(36, CursorFloat);
		LinkExpression(37, CursorStringWithSize);
		LinkExpression(38, CursorString);
		LinkExpression(39, Client_ProtocolImplementation);
		LinkExpression(40, Channel_GetLocalData);
		LinkExpression(41, Port);
		LinkExpression(42, BinaryToSend_Address);
		// Added expressions:
		LinkExpression(43, DumpMessage);
		LinkExpression(44, Client_Platform);
	}
    
	/*
        This is where you'd do anything you'd do in CreateRunObject in the original SDK

        It's the only place you'll get access to edPtr at runtime, so you should transfer
        anything from edPtr to the extension class here.
    */
	/*
	std::vector<Prop *> Prop;
	std::vector<char> PropChkbox;
	char * Current =  &edPtr->DarkEDIF_Props[0];
	for(unsigned int i = 0; i < ::SDK->json[CurLang]["Properties"].u.object.length; ++i)
	{
		Prop.push_back(GetProperty(edPtr, i));
		PropChkbox.push_back(*GetPropertyChbx(edPtr, i));
	}
	*/
	char * GlobalIDCopy = _strdup(edPtr->Global ? edPtr->edGlobalID : "");
	Globals = (struct GlobalInfo *)Runtime.ReadGlobal(std::string(std::string("LacewingRelayClient") + GlobalIDCopy).c_str());
	if (!Globals)
	{
		Globals = new Extension::GlobalInfo();
		Runtime.WriteGlobal(std::string(std::string("LacewingRelayClient") + GlobalIDCopy).c_str(), Globals);
	}

	AutomaticallyClearBinary = edPtr->AutomaticClear;
	GlobalID = GlobalIDCopy;

	// Try to boot the Lacewing thread if multithreading and not already running
	if (edPtr->MultiThreading && !Globals->_Thread && !(Globals->_Thread = CreateThread(NULL, NULL, LacewingLoopThread, this, NULL, NULL)))
		MessageBoxA(NULL, "Error: failed to boot thread. Falling back to single-threaded interface.", "DarkEDIF - runtime error", MB_OK);

	// Link all callbacks
	{
		Srv.onconnect(::OnClientConnectRequest);
		Srv.ondisconnect(::OnClientDisconnect);
		Srv.onerror(::OnError);
		Srv.onchannel_join(::OnJoinChannelRequest);
		Srv.onchannel_leave(::OnLeaveChannelRequest);
		Srv.onnameset(::OnNameSet);
		Srv.onmessage_server(::OnServerMessage);
		// Don't link message_peer/channel by default.

		// Handled by Srv.onerror
		//FlashSrv->on_error(::OnFlashError);
	}
	
	Srv.tag = this; // Useful so Lacewing callbacks can access Extension
}

DWORD WINAPI LacewingLoopThread(void * ThisExt)
{
	// If the loop thread is terminated, only 4 bytes of memory will be leaked.
	// However, it is better to use PostEventLoopExit().
	lacewing::error Error = ((Extension *)ThisExt)->ObjEventPump->start_eventloop();
	if (Error)
	{
		std::string Text = "Error returned by StartEventLoop(): ";
					Text += Error->tostring();
		((Extension *)ThisExt)->CreateError(Text.c_str());
	}
	((Extension *)ThisExt)->Globals->_Thread = NULL;
	return 0;
}


#ifdef MULTI_THREADING
SaveExtInfo &Extension::AddEvent(int Event, bool UseLastData /* = false */)
{
	/*
		Saves all variables returned by expressions in order to ensure two conditions, triggering simultaneously,
		do not cause reading of only the last condition's output. Only used in multi-threading.
		
		For example, if an error event changes an output variable, and immediately afterward a 
		success event changes the same variable, only the success event's output can be read since it overwrote
		the last event. However, using MMF2 in single-threaded mode, overwriting is impossible to do
		provided you use GenerateEvent(). Since PushEvent() is not immediate, you can reproduce this
		multi-threaded bug in single-thread mode. But PushEvent() isn't recommended for use anyway;
		it's designed so extensions who don't have focus of MMF2 (gained by MMF2 calling, say, Rehandle)
		can still create events. With GenerateEvent() + multithreading, this would cause serious issues.
		
		But in DarkEDIF, you'll note all the GenerateEvents() are handled on a queue, and the queue is
		iterated through in Rehandle, thus it is quite safe. But we still need to protect potentially several
		AddEvent() functions running at once and corrupting the memory at some point; so we need the
		CRITICAL_SECTION variable mentioned in Extension.h to ensure this will not happen.
	*/

	// Cause a new event
	if (UseLastData == false)
	{
		SaveExtInfo * NewEvent = new SaveExtInfo;
		
		// Initialise with one condition to be triggered
		NewEvent->NumEvents = 1;
		NewEvent->CondTrig = (unsigned short *)malloc(sizeof(unsigned short));
		
		// Failed to allocate memory
		if (!NewEvent->CondTrig)
		{
			delete NewEvent;
			return ThreadData;
		}
		NewEvent->CondTrig[0] = (unsigned short)Event;
		
		EnterCriticalSectionDerpy(&Lock); // Needed before we access Extension
		
		// Copy Extension's data to vector
		if (memcpy_s(((char *)NewEvent)+5, sizeof(SaveExtInfo)-5, ((char *)&ThreadData)+5, sizeof(SaveExtInfo)-5))
		{
			// Failed to copy memory (error in "errno")
			LeaveCriticalSectionDerpy(&Lock);
			return ThreadData;
		}

		Saved.push_back(NewEvent);

		LeaveCriticalSectionDerpy(&Lock); // We're done accessing Extension
	}
	else // New event is part of the last saved data (good for optimisation)
	{
		EnterCriticalSectionDerpy(&Lock);

		// Add current condition to saved expressions
		SaveExtInfo &S = (Saved.size() == 0) ? ThreadData : *Saved.back();
		
		unsigned short * CurrentCond = (unsigned short *)realloc(&S.CondTrig[0], S.NumEvents+1 * sizeof(short));
		
		if (!CurrentCond)
		{
			LeaveCriticalSectionDerpy(&Lock);
			return ThreadData;
		}

		CurrentCond[S.NumEvents++] = (unsigned short)Event;
		
		S.CondTrig = CurrentCond;

		LeaveCriticalSectionDerpy(&Lock);
	}

	// Cause Handle() to be triggered, allowing Saved to be parsed
	Runtime.Rehandle();
	return *Saved.back();
}
#endif // MULTI_THREADING


void Extension::ClearThreadData()
{
	memset(&ThreadData, 0, sizeof(SaveExtInfo));
}

void Extension::CreateError(const char * Error)
{
	SaveExtInfo &event = AddEvent(0, false);
	event.Error.Text = _strdup(Error);
	//__asm int 3;
}

void Extension::AddToSend(void * Data, size_t Size)
{
	if (!Data)
	{
		CreateError("Error adding to send binary: pointer supplied is invalid.\r\n"
					"The message has not been modified.");
		return;
	}
	if (!Size)
		return;
	char * newptr = (char *)realloc(SendMsg, SendMsgSize+Size);
		
	// Failed to reallocate memory
	if (!newptr)
	{
		char errorval [20];
		SaveExtInfo &S = AddEvent(0);
		std::string Error = "Received error ";
		if (_itoa_s(*_errno(), &errorval[0], 20, 10))
		{
			Error += "with reallocating memory to append to binary message, and with converting error number.";
		}
		else
		{
			Error += "number [";
			Error += &errorval[0];
			Error += "] with reallocating memory to append to binary message.";
		}
		Error += "\r\nThe message has not been modified.";
		S.Error.Text = _strdup(Error.c_str());
		return;
	}
	SendMsg = newptr;
	SendMsgSize += Size;
		
	// memcpy_s does not allow copying from what's already inside SendMsg; memmove_s does.
	// If we failed to copy memory.
	if (memmove_s(newptr+SendMsgSize-Size, Size, Data, Size))
	{
		char errorval [20];
		SaveExtInfo &S = AddEvent(0);
		std::string Error = "Received error ";
		if (_itoa_s(errno, &errorval[0], 20, 10))
		{
			Error += "with reallocating memory to append to binary message, and with converting error number.";
		}
		else
		{
			Error += "number [";
			Error += &errorval[0];
			Error += "] with copying memory to binary message.";
		}
		Error += "\r\nThe message has been resized but the data left uncopied.";
		S.Error.Text = _strdup(Error.c_str());
	}
}

Extension::~Extension()
{
	DeleteCriticalSection(&Lock);
}

short Extension::Handle()
{
	// If thread is not working, use Tick functionality. This may add events, so do it before the event-loop check.
	if (!Globals->_Thread)
	{
		ObjEventPump->tick();
		return 0;
	}

	#ifdef MULTI_THREADING

		// AddEvent() was called and not yet handled
		// (note all code that accesses Saved must have ownership of Lock)
		bool RunNextLoop = false;
		while (true)
		{
			if (!TryEnterCriticalSection(&Lock))
			{
				RunNextLoop = true;
				break; // Lock already occupied; leave it and run next event loop
			}
			sprintf_s(::Buffer, "Thread %u : Entered on %s, line %i.\r\n", GetCurrentThreadId(), __FILE__, __LINE__);
			::CriticalSection = ::Buffer + ::CriticalSection;

			if (Saved.size() == 0)
			{
				LeaveCriticalSectionDerpy(&Lock);
				break;
			}
			SaveExtInfo * S = Saved.front();
			
			if (S->ReceivedMsg.Content != nullptr)
			{
				ThreadData.ReceivedMsg.Content = S->ReceivedMsg.Content;
				ThreadData.ReceivedMsg.Size = S->ReceivedMsg.Size;
				ThreadData.ReceivedMsg.Cursor = S->ReceivedMsg.Cursor;
				ThreadData.ReceivedMsg.Subchannel = S->ReceivedMsg.Subchannel;
			}
			if (S->Channel != nullptr)
				ThreadData.Channel = S->Channel;
			if (S->Client != nullptr)
				ThreadData.Client = S->Client;
			LeaveCriticalSectionDerpy(&Lock);
				
			// Remove copies if this particular event number is used
			if (S->CondTrig[0] == 0xFFFF)
			{
				// If channel, it's either a channel leave or peer leave
				if (S->Channel)
				{
					// Channel leave
					if (!S->Client)
					{
						for (std::vector<lacewing::relayserver::channel *>::const_iterator u = Channels.begin(); u != Channels.end(); ++u)
						{
							if (*u == S->Channel)
							{
								if (!S->Channel->isclosed)
									CreateError("Channel being removed but not marked as closed!");

								Channels.erase(u);
								break;
							}
						}

						// delete ThreadData.Channel;
					}
					else // Peer leave
					{
						for (std::vector<lacewing::relayserver::channel *>::const_iterator u = Channels.begin(); u != Channels.end(); ++u)
						{
							if (*u == ThreadData.Channel)
							{
								for (std::vector<lacewing::relayserver::client *>::const_iterator v = Clients.begin(); v != Clients.end(); ++v)
								{
									if (*v == S->Client)
									{
										if (!S->Client->isclosed)
											CreateError("Peer being removed but not marked as closed!");

										Clients.erase(v);
										break;
									}
								}

								break;
							}
						}

						// delete ThreadData.Peer;
					}
				}
				else // On disconnect, clear everyting
				{
					// Thanks to the old liblacewing 2.7's shoddy header design, we cannot call the deconstructor here
					// and thus we have a small, but reoccuring, memory leak.
					// This should be fixed when this extension is updated to the more recent liblacewing version.

					// for (std::vector<Lacewing::RelayClient::Channel *>::const_iterator u = Channels.begin(); u != Channels.end(); ++u)
					//		delete (struct ::ChannelInternal *)((*u)->InternalTag);

					// Old username is stored in the tag and must be deleted separately from clear()
					for (std::vector<lacewing::relayserver::client *>::const_iterator u = Clients.begin(); u != Clients.end(); ++u)
					{
						if ((*u)->internaltag)
							free((*u)->internaltag);

						//	delete (struct ::PeerInternal *)((*u)->InternalTag);
					}

					// Delete main data of each struct (note larger Internal classes are not deleted, see above)
					Channels.clear();
					Clients.clear();
				}
			}
			else
			{
				// Trigger all stored events (more than one may be stored by calling AddEvent(***, true) )
				for (unsigned char u = 0; u < S->NumEvents; ++u)
					Runtime.GenerateEvent((int) S->CondTrig[u]);
			}

			EnterCriticalSectionDerpy(&Lock);
			Saved.erase(Saved.begin());
			LeaveCriticalSectionDerpy(&Lock);
		}
	#endif // MULTI_THREADING
	

    /*
       If your extension will draw to the MMF window you should first 
       check if anything about its display has changed :

           if (rdPtr->roc.rcChanged)
              return REFLAG_DISPLAY;
           else
              return 0;

       You will also need to make sure you change this flag yourself 
       to 1 whenever you want to redraw your object
     
       If your extension won't draw to the window, but it still needs 
       to do something every MMF loop use :

            return 0;

       If you don't need to do something every loop, use :

            return REFLAG::ONE_SHOT;

       This doesn't mean this function can never run again. If you want MMF
       to handle your object again (causing this code to run) use this function:

            Runtime.Rehandle();

       At the end of the loop this code will run

    */

	// Will not be called next loop if RunNextLoop is false
	return RunNextLoop ? 0 : REFLAG::ONE_SHOT;
}


short Extension::Display()
{
    /*
       If you return REFLAG::DISPLAY in Handle() this routine will run.
    */

    // Ok
    return 0;
}

short Extension::Pause()
{

    // Ok
    return 0;
}

short Extension::Continue()
{

    // Ok
    return 0;
}

bool Extension::Save(HANDLE File)
{
	bool OK = false;

    #ifndef VITALIZE

	    // Save the object's data here

	    OK = true;

    #endif

	return OK;
}

bool Extension::Load(HANDLE File)
{
	bool OK = false;

    #ifndef VITALIZE

	    // Load the object's data here

	    OK = true;

    #endif

	return OK;
}


// These are called if there's no function linked to an ID

void Extension::Action(int ID, RUNDATA * rdPtr, long param1, long param2)
{

}

long Extension::Condition(int ID, RUNDATA * rdPtr, long param1, long param2)
{
    return false;
}

long Extension::Expression(int ID, RUNDATA * rdPtr, long param)
{
    return 0;
}
