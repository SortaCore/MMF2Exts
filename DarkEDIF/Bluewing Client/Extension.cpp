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
		LinkAction(0, Replaced_Connect);
		LinkAction(1, Disconnect);
		LinkAction(2, SetName);
		LinkAction(3, Replaced_JoinChannel);
		LinkAction(4, LeaveChannel);
		LinkAction(5, SendTextToServer);
		LinkAction(6, SendTextToChannel);
		LinkAction(7, SendTextToPeer);
		LinkAction(8, SendNumberToServer);
		LinkAction(9, SendNumberToChannel);
		LinkAction(10, SendNumberToPeer);
		LinkAction(11, BlastTextToServer);
		LinkAction(12, BlastTextToChannel);
		LinkAction(13, BlastTextToPeer);
		LinkAction(14, BlastNumberToServer);
		LinkAction(15, BlastNumberToChannel);
		LinkAction(16, BlastNumberToPeer);
		LinkAction(17, SelectChannelWithName);
		LinkAction(18, ReplacedNoParams);
		LinkAction(19, LoopClientChannels);
		LinkAction(20, SelectPeerOnChannelByName);
		LinkAction(21, SelectPeerOnChannelByID);
		LinkAction(22, LoopPeersOnChannel);
		LinkAction(23, ReplacedNoParams);
		LinkAction(24, ReplacedNoParams);
		LinkAction(25, ReplacedNoParams);
		LinkAction(26, ReplacedNoParams);
		LinkAction(27, ReplacedNoParams);
		LinkAction(28, ReplacedNoParams);
		LinkAction(29, ReplacedNoParams);
		LinkAction(30, RequestChannelList);
		LinkAction(31, LoopListedChannels);
		LinkAction(32, ReplacedNoParams);
		LinkAction(33, ReplacedNoParams);
		LinkAction(34, ReplacedNoParams);
		LinkAction(35, SendBinaryToServer);
		LinkAction(36, SendBinaryToChannel);
		LinkAction(37, SendBinaryToPeer);
		LinkAction(38, BlastBinaryToServer);
		LinkAction(39, BlastBinaryToChannel);
		LinkAction(40, BlastBinaryToPeer);
		LinkAction(41, AddByteText);
		LinkAction(42, AddByteInt);
		LinkAction(43, AddShort);
		LinkAction(44, AddInt);
		LinkAction(45, AddFloat);
		LinkAction(46, AddStringWithoutNull);
		LinkAction(47, AddString);
		LinkAction(48, AddBinary);
		LinkAction(49, ClearBinaryToSend);
		LinkAction(50, SaveReceivedBinaryToFile);
		LinkAction(51, AppendReceivedBinaryToFile);
		LinkAction(52, AddFileToBinary);
		LinkAction(53, ReplacedNoParams);
		LinkAction(54, ReplacedNoParams);
		LinkAction(55, ReplacedNoParams);
		LinkAction(56, ReplacedNoParams);
		LinkAction(57, ReplacedNoParams);
		LinkAction(58, ReplacedNoParams);
		LinkAction(59, ReplacedNoParams);
		LinkAction(60, ReplacedNoParams);
		LinkAction(61, ReplacedNoParams);
		LinkAction(62, ReplacedNoParams);
		LinkAction(63, ReplacedNoParams);
		LinkAction(64, SelectChannelMaster);
		LinkAction(65, JoinChannel);
		LinkAction(66, CompressSendBinary);
		LinkAction(67, DecompressReceivedBinary);
		LinkAction(68, MoveReceivedBinaryCursor);
		LinkAction(69, LoopListedChannelsWithLoopName);
		LinkAction(70, LoopClientChannelsWithLoopName);
		LinkAction(71, LoopPeersOnChannelWithLoopName);
		LinkAction(72, ReplacedNoParams);
		LinkAction(73, Connect);
		LinkAction(74, ResizeBinaryToSend);
	}
	{
		LinkCondition(0, AlwaysTrue /* OnError */);
		LinkCondition(1, AlwaysTrue /* OnConnect */);
		LinkCondition(2, AlwaysTrue /* OnConnectDenied */);
		LinkCondition(3, AlwaysTrue /* OnDisconnect */);
		LinkCondition(4, AlwaysTrue /* OnChannelJoin */);
		LinkCondition(5, AlwaysTrue /* OnChannelJoinDenied */);
		LinkCondition(6, AlwaysTrue /* OnNameSet */);
		LinkCondition(7, AlwaysTrue /* OnNameDenied */);
		LinkCondition(8, OnSentTextMessageFromServer);
		LinkCondition(9, OnSentTextMessageFromChannel);
		LinkCondition(10, AlwaysTrue /* OnPeerConnect */);
		LinkCondition(11, AlwaysTrue /* OnPeerDisonnect */);
		LinkCondition(12, AlwaysFalse /* Replaced_OnChannelJoin */);
		LinkCondition(13, AlwaysTrue /* OnChannelPeerLoop */);
		LinkCondition(14, AlwaysTrue /* OnClientChannelLoop */);
		LinkCondition(15, OnSentNumberMessageFromServer);
		LinkCondition(16, OnSentNumberMessageFromChannel);
		LinkCondition(17, AlwaysTrue /* OnChannelPeerLoopFinished */);
		LinkCondition(18, AlwaysTrue /* OnClientChannelLoopFinished */);
		LinkCondition(19, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(20, OnBlastedTextMessageFromServer);
		LinkCondition(21, OnBlastedNumberMessageFromServer);
		LinkCondition(22, OnBlastedTextMessageFromChannel);
		LinkCondition(23, OnBlastedNumberMessageFromChannel);
		LinkCondition(24, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(25, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(26, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(27, AlwaysTrue /* OnChannelListReceived */);
		LinkCondition(28, AlwaysTrue /* OnChannelListLoop */);
		LinkCondition(29, AlwaysTrue /* OnChannelListLoopFinished */);
		LinkCondition(30, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(31, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(32, OnSentBinaryMessageFromServer /* ReplacedCondNoParams */);
		LinkCondition(33, OnSentBinaryMessageFromChannel);
		LinkCondition(34, OnBlastedBinaryMessageFromServer);
		LinkCondition(35, OnBlastedBinaryMessageFromChannel);
		LinkCondition(36, OnSentTextMessageFromPeer);
		LinkCondition(37, OnSentNumberMessageFromPeer);
		LinkCondition(38, OnSentBinaryMessageFromPeer);
		LinkCondition(39, OnBlastedTextMessageFromPeer);
		LinkCondition(40, OnBlastedNumberMessageFromPeer);
		LinkCondition(41, OnBlastedBinaryMessageFromPeer);
		LinkCondition(42, IsConnected);
		LinkCondition(43, AlwaysTrue /* OnChannelLeave */);
		LinkCondition(44, AlwaysTrue /* OnChannelLeaveDenied */);
		LinkCondition(45, AlwaysTrue /* OnPeerChangedName */);
		LinkCondition(46, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(47, OnAnySentMessageFromServer);
		LinkCondition(48, OnAnySentMessageFromChannel);
		LinkCondition(49, OnAnySentMessageFromPeer);
		LinkCondition(50, OnAnyBlastedMessageFromServer);
		LinkCondition(51, OnAnyBlastedMessageFromChannel);
		LinkCondition(52, OnAnyBlastedMessageFromPeer);
		LinkCondition(53, AlwaysTrue /* OnNameChanged */);
		LinkCondition(54, ClientHasAName);
		LinkCondition(55, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(56, AlwaysFalse /* ReplacedCondNoParams */);
		LinkCondition(57, SelectedPeerIsChannelMaster);
		LinkCondition(58, YouAreChannelMaster);
		LinkCondition(59, OnChannelListLoopWithName);
		LinkCondition(60, OnChannelListLoopWithNameFinished);
		LinkCondition(61, OnPeerLoopWithName);
		LinkCondition(62, OnPeerLoopWithNameFinished);
		LinkCondition(63, OnClientChannelLoopWithName);
		LinkCondition(64, OnClientChannelLoopWithNameFinished);
		LinkCondition(65, OnSentTextChannelMessageFromServer);
		LinkCondition(66, OnSentNumberChannelMessageFromServer);
		LinkCondition(67, OnSentBinaryChannelMessageFromServer);
		LinkCondition(68, OnAnySentChannelMessageFromServer);
		LinkCondition(69, OnBlastedTextChannelMessageFromServer);
		LinkCondition(70, OnBlastedNumberChannelMessageFromServer);
		LinkCondition(71, OnBlastedBinaryChannelMessageFromServer);
		LinkCondition(72, OnAnyBlastedChannelMessageFromServer);
		LinkCondition(73, IsJoinedToChannel);
		LinkCondition(74, IsPeerOnChannel_Name);
		LinkCondition(75, IsPeerOnChannel_ID);
	}
	{
		LinkExpression(0, Error);
		LinkExpression(1, ReplacedExprNoParams);
		LinkExpression(2, Self_Name);
		LinkExpression(3, Self_ChannelCount);
		LinkExpression(4, Peer_Name);
		LinkExpression(5, ReceivedStr);
		LinkExpression(6, ReceivedInt);
		LinkExpression(7, Subchannel);
		LinkExpression(8, Peer_ID);
		LinkExpression(9, Channel_Name);
		LinkExpression(10, Channel_PeerCount);
		LinkExpression(11, ReplacedExprNoParams);
		LinkExpression(12, ChannelListing_Name);
		LinkExpression(13, ChannelListing_PeerCount);
		LinkExpression(14, Self_ID);
		LinkExpression(15, ReplacedExprNoParams);
		LinkExpression(16, ReplacedExprNoParams);
		LinkExpression(17, ReplacedExprNoParams);
		LinkExpression(18, ReplacedExprNoParams);
		LinkExpression(19, ReplacedExprNoParams);
		LinkExpression(20, StrByte);
		LinkExpression(21, UnsignedByte);
		LinkExpression(22, SignedByte);
		LinkExpression(23, UnsignedShort);
		LinkExpression(24, SignedShort);
		LinkExpression(25, UnsignedInteger);
		LinkExpression(26, SignedInteger);
		LinkExpression(27, Float);
		LinkExpression(28, StringWithSize);
		LinkExpression(29, String);
		LinkExpression(30, ReceivedBinarySize);
		LinkExpression(31, Lacewing_Version);
		LinkExpression(32, SendBinarySize);
		LinkExpression(33, Self_PreviousName);
		LinkExpression(34, Peer_PreviousName);
		LinkExpression(35, ReplacedExprNoParams);
		LinkExpression(36, ReplacedExprNoParams);
		LinkExpression(37, DenyReason);
		LinkExpression(38, HostIP);
		LinkExpression(39, HostPort);
		LinkExpression(40, ReplacedExprNoParams);
		LinkExpression(41, WelcomeMessage);
		LinkExpression(42, ReceivedBinaryAddress);
		LinkExpression(43, CursorStrByte);
		LinkExpression(44, CursorUnsignedByte);
		LinkExpression(45, CursorSignedByte);
		LinkExpression(46, CursorUnsignedShort);
		LinkExpression(47, CursorSignedShort);
		LinkExpression(48, CursorUnsignedInteger);
		LinkExpression(49, CursorSignedInteger);
		LinkExpression(50, CursorFloat);
		LinkExpression(51, CursorStringWithSize);
		LinkExpression(52, CursorString);
		LinkExpression(53, ReplacedExprNoParams);
		LinkExpression(54, SendBinaryAddress);
		LinkExpression(55, DumpMessage);
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
		Globals = new struct Extension::GlobalInfo;
		Runtime.WriteGlobal(std::string(std::string("LacewingRelayClient") + GlobalIDCopy).c_str(), Globals);
	}

	AutomaticallyClearBinary = edPtr->AutomaticClear;
	GlobalID = GlobalIDCopy;

	// Try to boot the Lacewing thread if multithreading and not already running
	if (edPtr->MultiThreading && !Globals->_Thread && !(Globals->_Thread = CreateThread(NULL, NULL, LacewingLoopThread, this, NULL, NULL)))
		MessageBoxA(NULL, "Error: failed to boot thread. Falling back to single-threaded interface.", "DarkEDIF - runtime error", MB_OK);

	// Link all callbacks
	{
		Cli.onChannelListReceived(::OnChannelListReceived);
		Cli.onChannelMessage(::OnChannelMessage);
		Cli.onConnect(::OnConnect);
		Cli.onConnectionDenied(::OnConnectDenied);
		Cli.onDisconnect(::OnDisconnect);
		Cli.onError(::OnError);
		Cli.onJoin(::OnJoinChannel);
		Cli.onJoinDenied(::OnJoinChannelDenied);
		Cli.onLeave(::OnLeaveChannel);
		Cli.onLeaveDenied(::OnLeaveChannelDenied);
		Cli.onNameChanged(::OnNameChanged);
		Cli.onNameDenied(::OnNameDenied);
		Cli.onNameSet(::OnNameSet);
		Cli.onPeerChangeName(::OnPeerNameChanged);
		Cli.onPeerConnect(::OnPeerConnect);
		Cli.onPeerDisconnect(::OnPeerDisconnect);
		Cli.onPeerMessage(::OnPeerMessage);
		Cli.onServerChannelMessage(::OnServerChannelMessage);
		Cli.onServerMessage(::OnServerMessage);
	}
	
	Cli.Tag = this; // Useful so Lacewing callbacks can access Extension
}

DWORD WINAPI LacewingLoopThread(void * ThisExt)
{
	// If the loop thread is terminated, only 4 bytes of memory will be leaked.
	// However, it is better to use PostEventLoopExit().
	Lacewing::Error * Error = ((Extension *)ThisExt)->ObjEventPump.StartEventLoop();
	if (Error)
	{
		std::string Text = "Error returned by StartEventLoop(): ";
					Text += Error->ToString();
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
			return ThreadData;
		NewEvent->CondTrig[0] = (unsigned short)Event;
		
		EnterCriticalSection(&Lock); // Needed before we access Extension
		
		// Copy Extension's data to vector
		if (memcpy_s(((char *)NewEvent)+5, sizeof(SaveExtInfo)-5, ((char *)&ThreadData)+5, sizeof(SaveExtInfo)-5))
		{
			// Failed to copy memory (error in "errno")
			LeaveCriticalSection(&Lock);
			return ThreadData;
		}

		Saved.push_back(NewEvent);

		LeaveCriticalSection(&Lock); // We're done accessing Extension
	}
	else // New event is part of the last saved data (good for optimisation)
	{
		EnterCriticalSection(&Lock);

		// Add current condition to saved expressions
		SaveExtInfo &S = (Saved.size() == 0) ? ThreadData : *Saved.back();
		
		unsigned short * CurrentCond = (unsigned short *)realloc(&S.CondTrig[0], S.NumEvents+1 * sizeof(short));
		
		if (!CurrentCond)
		{
			LeaveCriticalSection(&Lock);
			return ThreadData;
		}

		CurrentCond[S.NumEvents++] = (unsigned short)Event;
		
		S.CondTrig = CurrentCond;

		LeaveCriticalSection(&Lock);
	}

	// Cause Handle() to be triggered, allowing Saved to be parsed
	Runtime.Rehandle();
	return *Saved.back();
}
#endif // MULTI_THREADING


void Extension::CreateError(const char * Error)
{
	SaveExtInfo &event = AddEvent(0, false);
	event.Error.Text = _strdup(Error);
	// __asm int 3;
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
	#ifdef MULTI_THREADING

		// AddEvent() was called and not yet handled
		// (note all code that accesses Saved must have ownership of Lock)
		while (true)
		{
			EnterCriticalSection(&Lock);
			if (Saved.size() == 0)
			{
				LeaveCriticalSection(&Lock);
				break;
			}
			// Copy from saved list of events to current extension
			if (memcpy_s(&ThreadData, sizeof(SaveExtInfo), Saved.front(), sizeof(SaveExtInfo)))
			{
				LeaveCriticalSection(&Lock);
				break; // Failed; leave until next Extension::Handle()
			}
			LeaveCriticalSection(&Lock);
				
			// Remove copies if this particular event number is used
			if (ThreadData.CondTrig[0] == 0xFFFF)
			{
				if (ThreadData.Channel)
				{
					if (!ThreadData.Channel)
					{
						for (std::vector<Lacewing::RelayClient::Channel *>::const_iterator u = Channels.begin(); u != Channels.end(); ++u)
						{
							if (*u == ThreadData.Channel)
							{
								Channels.erase(u);
								break;
							}
						}
						delete ThreadData.Channel;
					}
					else
					{
						for (std::vector<Lacewing::RelayClient::Channel::Peer *>::const_iterator u = Peers.begin(); u != Peers.end(); ++u)
						{
							if (*u == ThreadData.Peer)
							{
								Peers.erase(u);
								break;
							}
						}
						delete ThreadData.Peer;
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
					for (std::vector<Lacewing::RelayClient::Channel::Peer *>::const_iterator u = Peers.begin(); u != Peers.end(); ++u)
					{
						if ((*u)->InternalTag)
							free((*u)->InternalTag);

						//	delete (struct ::PeerInternal *)((*u)->InternalTag);
					}

					// Delete main data of each struct (note larger Internal classes are not deleted, see above)
					Channels.clear();
					Peers.clear();
				}
			}
			else
			{
				// Trigger all stored events (more than one may be stored by calling AddEvent(***, true) )
				for (unsigned char u = 0; u < ThreadData.NumEvents; ++u)
					Runtime.GenerateEvent((int) ThreadData.CondTrig[u]);
			}

			EnterCriticalSection(&Lock);
			Saved.erase(Saved.begin());
			LeaveCriticalSection(&Lock);
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

	// If thread is not working, use Tick functionality.
	if (!Globals->_Thread)
	{
		ObjEventPump.Tick();
		return 0;
	}

	// Will not be called next loop	
	return REFLAG::ONE_SHOT;
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
