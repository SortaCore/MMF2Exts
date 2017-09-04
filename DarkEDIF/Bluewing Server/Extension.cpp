#include "Common.h"
#include <assert.h>

///
/// EXTENSION CONSTRUCTOR/DESTRUCTOR
///

#define Ext (*Globals->_Ext)
#define Saved Globals->_Saved

HANDLE AppWasClosed = NULL;
Extension::Extension(RUNDATA * _rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr) :
	rdPtr(_rdPtr), rhPtr(_rdPtr->rHo.AdRunHeader), Runtime(_rdPtr)
{
	// Nullify the thread-specific data
	ClearThreadData();

	/*
        Link all your action/condition/expression functions to their IDs to match the
        IDs in the JSON here
    */
	{
		LinkAction(0, RelayServer_Host);
		LinkAction(1, RelayServer_StopHosting);
		LinkAction(2, SetWelcomeMessage);
		LinkAction(3, RemovedActionNoParams);
		LinkAction(4, RemovedActionNoParams);
		LinkAction(5, RemovedActionNoParams);
		LinkAction(6, RemovedActionNoParams);
		LinkAction(7, RemovedActionNoParams);
		LinkAction(8, RemovedActionNoParams);
		LinkAction(9, RemovedActionNoParams /* EnableCondition_OnMessageToChannel */);
		LinkAction(10, RemovedActionNoParams);
		LinkAction(11, RemovedActionNoParams /*EnableCondition_OnMessageToPeer */);
		LinkAction(12, RemovedActionNoParams);
		LinkAction(13, RemovedActionNoParams);
		LinkAction(14, RemovedActionNoParams);
		LinkAction(15, RemovedActionNoParams);
		LinkAction(16, RemovedActionNoParams);
		LinkAction(17, RemovedActionNoParams);
		LinkAction(18, RemovedActionNoParams);
		LinkAction(19, RemovedActionNoParams /* old deny action without reason */);
		LinkAction(20, OnInteractive_ChangeClientName);
		LinkAction(21, OnInteractive_ChangeChannelName);
		LinkAction(22, OnInteractive_ReplaceMessageWithText);
		LinkAction(23, OnInteractive_ReplaceMessageWithNumber);
		LinkAction(24, OnInteractive_ReplaceMessageWithSendBinary);
		LinkAction(25, Channel_Close);
		LinkAction(26, Channel_SelectMaster);
		LinkAction(27, Channel_SelectByName);
		LinkAction(28, LoopAllChannels);
		LinkAction(29, Client_Disconnect);
		LinkAction(30, Client_LoopJoinedChannels);
		LinkAction(31, Client_SelectByName);
		LinkAction(32, Client_SelectByID);
		LinkAction(33, LoopAllClients);
		LinkAction(34, SendTextToClient);
		LinkAction(35, SendTextToChannel);
		LinkAction(36, SendNumberToClient);
		LinkAction(37, SendNumberToChannel);
		LinkAction(38, SendBinaryToClient);
		LinkAction(39, SendBinaryToChannel);
		LinkAction(40, BlastTextToClient);
		LinkAction(41, BlastTextToChannel);
		LinkAction(42, BlastNumberToClient);
		LinkAction(43, BlastNumberToChannel);
		LinkAction(44, BlastBinaryToClient);
		LinkAction(45, BlastBinaryToChannel);
		LinkAction(46, AddByteText);
		LinkAction(47, AddByteInt);
		LinkAction(48, AddShort);
		LinkAction(49, AddInt);
		LinkAction(50, AddFloat);
		LinkAction(51, AddStringWithoutNull);
		LinkAction(52, AddString);
		LinkAction(53, AddBinary);
		LinkAction(54, AddFileToBinary);
		LinkAction(55, CompressSendBinary);
		LinkAction(56, ClearBinaryToSend);
		LinkAction(57, SaveReceivedBinaryToFile);
		LinkAction(58, AppendReceivedBinaryToFile);
		LinkAction(59, DecompressReceivedBinary);
		LinkAction(60, Channel_LoopClients);
		LinkAction(61, OnInteractive_DropMessage);
		LinkAction(62, Client_SelectSender);
		LinkAction(63, Client_SelectReceiver);
		LinkAction(64, LoopAllChannelsWithName);
		LinkAction(65, LoopAllClientsWithName);
		LinkAction(66, Client_LoopJoinedChannelsWithName);
		LinkAction(67, Channel_LoopClientsWithName);
		LinkAction(68, FlashServer_Host);
		LinkAction(69, FlashServer_StopHosting);
		LinkAction(70, Client_SetLocalData);
		LinkAction(71, MoveReceivedBinaryCursor);
		LinkAction(72, Channel_SetLocalData);
		LinkAction(73, ResizeBinaryToSend);
		// Added actions:
		LinkAction(74, EnableCondition_OnConnectRequest);
		LinkAction(75, EnableCondition_OnJoinChannelRequest);
		LinkAction(76, EnableCondition_OnLeaveChannelRequest);
		LinkAction(77, EnableCondition_OnMessageToChannel);
		LinkAction(78, EnableCondition_OnMessageToPeer);
		LinkAction(79, EnableCondition_OnMessageToServer);
		LinkAction(80, OnInteractive_Deny);
		LinkAction(81, EnableCondition_OnNameSetRequest);
		//LinkAction(82, HTML5Server_EnableHosting);
		//LinkAction(83, HTML5Server_DisableHosting);
	}
	{
		LinkCondition(0, AlwaysTrue /* OnError */);
		LinkCondition(1, AlwaysTrue /* OnClientConnectRequest */);
		LinkCondition(2, AlwaysTrue /* OnDisconnect */);
		LinkCondition(3, AlwaysTrue /* OnChannel_JoinRequest */);
		LinkCondition(4, AlwaysTrue /* OnChannel_LeaveRequest */);
		LinkCondition(5, AlwaysTrue /* AllChannelsLoop */);
		LinkCondition(6, AlwaysTrue /* OnClient_JoinedChannelsLoop */);
		LinkCondition(7, AlwaysTrue /* AllClientsLoop */);
		LinkCondition(8, AlwaysTrue /* OnChannel_ClientLoop */);
		LinkCondition(9, Client_IsChannelMaster);
		LinkCondition(10, AlwaysTrue /* OnClient_NameSetRequest */);
		LinkCondition(11, AlwaysFalse /* NOT USED: OnClient_NameChangeRequest */);
		LinkCondition(12, SubchannelMatches /* OnSentTextMessageToServer */);
		LinkCondition(13, SubchannelMatches /* OnSentNumberMessageToServer */);
		LinkCondition(14, SubchannelMatches /* OnSentBinaryMessageToServer */);
		LinkCondition(15, SubchannelMatches /* OnAnySentMessageToServer */);
		LinkCondition(16, SubchannelMatches /* OnSentTextMessageToChannel */);
		LinkCondition(17, SubchannelMatches /* OnSentNumberMessageToChannel */);
		LinkCondition(18, SubchannelMatches /* OnSentBinaryMessageToChannel */);
		LinkCondition(19, SubchannelMatches /* OnAnySentMessageToChannel */);
		LinkCondition(20, SubchannelMatches /* OnSentTextMessageToPeer */);
		LinkCondition(21, SubchannelMatches /* OnSentNumberMessageToPeer */);
		LinkCondition(22, SubchannelMatches /* OnSentBinaryMessageToPeer */);
		LinkCondition(23, SubchannelMatches /* OnAnySentMessageToPeer */);
		LinkCondition(24, SubchannelMatches /* OnBlastedTextMessageToPeer */);
		LinkCondition(25, SubchannelMatches /* OnBlastedNumberMessageToServer */);
		LinkCondition(26, SubchannelMatches /* OnBlastedBinaryMessageToServer */);
		LinkCondition(27, SubchannelMatches /* OnAnyBlastedMessageToServer */);
		LinkCondition(28, SubchannelMatches /* OnBlastedTextMessageToChannel */);
		LinkCondition(29, SubchannelMatches /* OnBlastedNumberMessageToChannel */);
		LinkCondition(30, SubchannelMatches /* OnBlastedBinaryMessageToChannel */);
		LinkCondition(31, SubchannelMatches /* OnAnyBlastedMessageToChannel */);
		LinkCondition(32, SubchannelMatches /* OnBlastedTextMessageToPeer */);
		LinkCondition(33, SubchannelMatches /* OnBlastedNumberMessageToPeer */);
		LinkCondition(34, SubchannelMatches /* OnBlastedBinaryMessageToPeer */);
		LinkCondition(35, SubchannelMatches /* OnAnyBlastedMessageToPeer */);
		LinkCondition(36, OnAllChannelsLoopWithName);
		LinkCondition(37, OnClientsJoinedChannelLoopWithName);
		LinkCondition(38, OnAllClientsLoopWithName);
		LinkCondition(39, OnChannelClientsLoopWithName);
		LinkCondition(40, OnChannelClientsLoopWithNameFinished);
		LinkCondition(41, OnAllChannelsLoopWithNameFinished);
		LinkCondition(42, OnAllClientsLoopWithNameFinished);
		LinkCondition(43, OnClientsJoinedChannelLoopWithNameFinished);
		LinkCondition(44, AlwaysTrue /* OnChannelsClientLoopFinished */);
		LinkCondition(45, AlwaysTrue /* OnAllChannelsLoopFinished */);
		LinkCondition(46, AlwaysTrue /* OnAllClientsLoopFinished */);
		LinkCondition(47, AlwaysTrue /* OnClientJoinedChannelLoopFinished */);
		LinkCondition(48, IsLacewingServerHosting);
		LinkCondition(49, IsFlashPolicyServerHosting);
		LinkCondition(50, ChannelIsHiddenFromChannelList);
		LinkCondition(51, ChannelIsSetToCloseAutomatically);
		LinkCondition(52, AlwaysFalse /* DUPLICATE IN ORIG, NOT USED IN BLUE: OnAllClientsLoopWithNameFinished */);
		// Added conditions:
		LinkCondition(53, IsClientOnChannel_ID);
		LinkCondition(54, IsClientOnChannel_Name);
	//	LinkCondition(55, IsHTML5Hosting);
	}
	{
		LinkExpression(0, Error);
		LinkExpression(1, Lacewing_Version);
		LinkExpression(2, BinaryToSend_Size);
		LinkExpression(3, RequestedClientName);
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
		LinkExpression(44, AllClientCount);
		LinkExpression(45, GetDenyReason);
	}
    
	/*
        This is where you'd do anything you'd do in CreateRunObject in the original SDK

        It's the only place you'll get access to edPtr at runtime, so you should transfer
        anything from edPtr to the extension class here.
    */

	// This is signalled by EndApp in General.cpp. It's used to unhost the server
	// when the application closes, from the timeout thread - assuming events
	// haven't done that already.
	if (!AppWasClosed)
		AppWasClosed = CreateEvent(NULL, TRUE, FALSE, NULL);

	IsGlobal = edPtr->Global;
	char msgBuff[500];
	sprintf_s(msgBuff, "Extension create: IsGlobal=%i.\n", IsGlobal ? 1 : 0);
	OutputDebugStringA(msgBuff);
	if (IsGlobal)
	{
		char * GlobalIDCopy = _strdup(edPtr->edGlobalID);
		void * v = Runtime.ReadGlobal(std::string(std::string("LacewingRelayServer") + GlobalIDCopy).c_str());
		if (!v)
		{
			Globals = new GlobalInfo(this, edPtr);
			Runtime.WriteGlobal(std::string(std::string("LacewingRelayServer") + GlobalIDCopy).c_str(), Globals);
			OutputDebugStringA("Created new Globals.");
		}
		else
		{
			// Add this Extension to Refs
			Globals = (GlobalInfo *)v;
			
			// If switching frames, the old ext will store selection here.
			// We'll keep it across frames for simplicity.
			ThreadData.Channel = Globals->LastDestroyedExtSelectedChannel;
			ThreadData.Client = Globals->LastDestroyedExtSelectedClient;

			Globals->Refs.push_back(this);
			if (!Globals->_Ext)
				Globals->_Ext = this;
			OutputDebugStringA("Globals exists: added to Refs.");
		}
	}
	else
	{
		OutputDebugStringA("Non-Global object; creating Globals, not submitting to WriteGlobal.");
		Globals = new GlobalInfo(this, edPtr);

		Globals->_ObjEventPump->tick();
	}

#ifndef _DEBUG
	if (edPtr->MultiThreading)
	{
		MessageBoxA(NULL, "For stability reasons, Bluewing Server does not support multithreading. "
			"Please use the \"Enable conditions\" menu appropriately for optimum speed. See the help file.",
			"Lacewing Blue Server - Startup warning", MB_OK | MB_ICONWARNING);
		edPtr->MultiThreading = false;
		Runtime.Rehandle();
	}
#endif


	// Try to boot the Lacewing thread if multithreading and not already running
	if (edPtr->MultiThreading && !Globals->_Thread && !(Globals->_Thread = CreateThread(NULL, NULL, LacewingLoopThread, this, NULL, NULL)))
	{
		CreateError("Error: failed to boot thread. Falling back to single-threaded interface.");
		Runtime.Rehandle();
	}
	else if (!edPtr->MultiThreading)
		Runtime.Rehandle();
}



DWORD WINAPI LacewingLoopThread(void * ThisExt)
{
	// If the loop thread is terminated, very few bytes of memory will be leaked.
	// However, it is better to use PostEventLoopExit().

	GlobalInfo * G = ((Extension *)ThisExt)->Globals;
	try {
		lacewing::error Error = G->_ObjEventPump->start_eventloop();

		// Can't error report if there's no extension to error-report to.
		// Worst case scenario CreateError calls Runtime.Rehandle which breaks because ext is gone.
		if (!Error)
			OutputDebugStringA("LacewingLoopThread closing gracefully.");
		else if (G->_Ext)
		{
			std::string Text = "Error returned by StartEventLoop(): ";
			Text += Error->tostring();
			G->CreateError(Text.c_str());
		}

	}
	catch (...)
	{
		OutputDebugStringA("LacewingLoopThread got an exception.");
		if (G->_Ext)
		{
			std::string Text = "StartEventLoop() killed by exception. Switching to single-threaded.";
			G->CreateError(Text.c_str());
		}
	}
	G->_Thread = NULL;
	OutputDebugStringA("LacewingLoopThread has exited.");
	return 0;
}

GlobalInfo::GlobalInfo(Extension * e, EDITDATA * edPtr)
	: _ObjEventPump((lacewing::eventpump)lacewing::eventpump_new()),
	_Server(_ObjEventPump),
	_SendMsg(nullptr), _SendMsgSize(0), _DenyReason(nullptr), _NewChannelName(nullptr),
	_NewClientName(nullptr), _InteractivePending(InteractiveType::None),
	_AutomaticallyClearBinary(edPtr->AutomaticClear), _GlobalID(nullptr), _Thread(nullptr),
	LastDestroyedExtSelectedChannel(nullptr), LastDestroyedExtSelectedClient(nullptr),
	AutoResponse_Connect(Approve_Quiet), AutoResponse_Connect_DenyReason(nullptr),
	AutoResponse_NameSet(Approve_Quiet), AutoResponse_NameSet_DenyReason(nullptr),
	AutoResponse_ChannelJoin(Approve_Quiet), AutoResponse_ChannelJoin_DenyReason(nullptr),
	AutoResponse_ChannelLeave(Approve_Quiet), AutoResponse_ChannelLeave_DenyReason(nullptr),
	AutoResponse_MessageServer(WaitForFusion),
	AutoResponse_MessageChannel(Approve_Quiet), AutoResponse_MessageClient(Approve_Quiet)
{
	_Ext = e;
	Refs.push_back(e);
	if (edPtr->Global)
		_GlobalID = _strdup(edPtr->edGlobalID);
	TimeoutWarningEnabled = edPtr->TimeoutWarningEnabled;
	FullDeleteEnabled = edPtr->FullDeleteEnabled;

	_Server.onerror(::OnError);
	_Server.onconnect(::OnClientConnectRequest);
	_Server.onnameset(::OnNameSetRequest);
	_Server.onchannel_join(::OnJoinChannelRequest);
	_Server.onchannel_leave(::OnLeaveChannelRequest);
	_Server.onmessage_server(::OnServerMessage);
	_Server.ondisconnect(::OnClientDisconnect);

	// Approve quiet needs no handler
	//_Server.onmessage_channel(::OnChannelMessage);
	//_Server.onmessage_peer(::OnPeerMessage);
	
	// RelayServer combines flash policy errors with regular errors.
	// So this is handled by Srv.onerror
	//FlashSrv->on_error(::OnFlashError);

	InitializeCriticalSection(&Lock);

	// Useful so Lacewing callbacks can access Extension
	_Server.tag = this;
}
GlobalInfo::~GlobalInfo() noexcept(false)
{
	if (!Refs.empty())
		throw std::exception("GlobalInfo dtor called prematurely.");
	free(_GlobalID);
	DeleteCriticalSection(&Lock);

	// We're no longer responding to these events
	_Server.onerror(nullptr);
	_Server.ondisconnect(nullptr);
	_Server.onconnect(nullptr);
	_Server.onnameset(nullptr);
	_Server.onchannel_join(nullptr);
	_Server.onchannel_leave(nullptr);
	_Server.onmessage_server(nullptr);
	_Server.onmessage_channel(nullptr);
	_Server.onmessage_peer(nullptr);

	// Cleanup all usages of GlobalInfo
	if (!_Thread)
		_ObjEventPump->tick();

	_Server.tag = nullptr; // was == this, now this is not usable

	if (_Server.hosting())
	{
		_Server.unhost();

		if (!_Thread)
			_ObjEventPump->tick();
		Sleep(0U);
	}
	_ObjEventPump->post_eventloop_exit();

	if (_Thread)
	{
		Sleep(0U);
		if (_Thread)
			Sleep(50U);
		if (_Thread) {
			TerminateThread(_Thread, 0U);
			_Thread = NULL;
		}
	}
	else
	{
		_ObjEventPump->tick();
		Sleep(0U);
	}
}

#ifdef MULTI_THREADING

void GlobalInfo::AddEvent1(int Event1,
	ChannelCopy * Channel /* = nullptr */,
	ClientCopy * Client /* = nullptr */,
	char * MessageOrErrorText /* = nullptr */,
	size_t MessageSize /* = 0 */,
	unsigned char Subchannel /* = 255 */,
	ClientCopy * ReceivingClient /* = nullptr */,
	InteractiveType InteractiveType  /* = None */,
	unsigned char Variant /* = 255 */,
	bool Blasted /* = false */,
	bool ChannelCreate_Hidden /* = false */,
	bool ChannelCreate_AutoClose /* = false */
	)
{
	return AddEventF(false, Event1, 35353,
		Channel, Client, MessageOrErrorText, MessageSize, Subchannel,
		ReceivingClient, InteractiveType, Variant, Blasted, ChannelCreate_Hidden, ChannelCreate_AutoClose);
}
void GlobalInfo::AddEvent2(int Event1, int Event2,
	ChannelCopy * Channel /* = nullptr */,
	ClientCopy * Client /* = nullptr */,
	char * MessageOrErrorText /* = nullptr */,
	size_t MessageSize /* = 0 */,
	unsigned char Subchannel /* = 255 */,
	ClientCopy * ReceivingClient /* = nullptr */,
	InteractiveType InteractiveType  /* = None */,
	unsigned char Variant /* = 255 */,
	bool Blasted /* = false */,
	bool ChannelCreate_Hidden /* = false */,
	bool ChannelCreate_AutoClose /* = false */)
{
	return AddEventF(true, Event1, Event2,
		Channel, Client, MessageOrErrorText, MessageSize, Subchannel,
		ReceivingClient, InteractiveType, Variant, Blasted, ChannelCreate_Hidden, ChannelCreate_AutoClose);
}
void GlobalInfo::AddEventF(bool twoEvents, int Event1, int Event2,
	ChannelCopy * Channel /* = nullptr */,
	ClientCopy * Client /* = nullptr */,
	char * MessageOrErrorText /* = nullptr */,
	size_t MessageSize /* = 0 */,
	unsigned char Subchannel /* = 255 */,
	ClientCopy * ReceivingClient /* = nullptr */,
	InteractiveType InteractiveType  /* = None */,
	unsigned char Variant /* = 255 */,
	bool Blasted /* = false */,
	bool ChannelCreate_Hidden /* = false */,
	bool ChannelCreate_AutoClose /* = false */)
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

	SaveExtInfo * NewEvent = new SaveExtInfo();
	SaveExtInfo &NewEvent2 = *NewEvent;

	// Initialise
	NewEvent2.NumEvents = twoEvents ? 2 : 1;
	NewEvent2.CondTrig[0] = (unsigned short)Event1;
	NewEvent2.CondTrig[1] = (unsigned short)Event2;
	NewEvent2.Channel = (ChannelCopy *)Channel;
	// Client and SenderClient overlap
	NewEvent2.Client = Client;
	NewEvent2.ReceivingClient = ReceivingClient;
	// Message Content, Error Text, and Loop Name overlap
	NewEvent2.ReceivedMsg.Content = MessageOrErrorText;
	NewEvent2.ReceivedMsg.Size = MessageSize;
	NewEvent2.ReceivedMsg.Subchannel = Subchannel;
	NewEvent2.ReceivedMsg.Variant = Variant;
	NewEvent2.ReceivedMsg.Blasted = Blasted;
	NewEvent2.InteractiveType = InteractiveType;
	NewEvent2.ChannelCreate_Hidden = ChannelCreate_Hidden;
	NewEvent2.ChannelCreate_AutoClose = ChannelCreate_AutoClose;

	EnterCriticalSectionDerpy(&Lock); // Needed before we access Extension

	_Saved.push_back(NewEvent);

	LeaveCriticalSectionDerpy(&Lock); // We're done accessing Extension

	// Cause Handle() to be triggered, allowing Saved to be parsed
	if (_Ext != nullptr)
		_Ext->Runtime.Rehandle();
}

#endif // MULTI_THREADING


void Extension::ClearThreadData()
{
	memset(&ThreadData, 0, sizeof(SaveExtInfo));
}

void Extension::CreateError(const char * Error)
{
	OutputDebugStringA(Error);
	Globals->AddEvent1(0, nullptr, nullptr, _strdup(Error));
	//__asm int 3;
}

void GlobalInfo::CreateError(const char * Error)
{
	OutputDebugStringA(Error);
	AddEvent1(0, nullptr, nullptr, _strdup(Error));
	//__asm int 3;
}

void Extension::AddToSend(void * Data, size_t Size)
{
	if (!Data)
	{
		CreateError("Error adding to send binary: pointer supplied is invalid. "
			"The message has not been modified.");
		return;
	}
	if (!Size)
		return;
	char * newptr = (char *)realloc(SendMsg, SendMsgSize + Size);

	// Failed to reallocate memory
	if (!newptr)
	{
		std::stringstream Error;
		Error << "Received error number " << errno << " with reallocating memory to append to binary message. "
			<< "The message has not been modified.";
		CreateError(Error.str().c_str());
		return;
	}
	SendMsg = newptr;
	SendMsgSize += Size;

	// memcpy_s does not allow copying from what's already inside SendMsg; memmove_s does.
	// If we failed to copy memory.
	if (memmove_s(newptr + SendMsgSize - Size, Size, Data, Size))
	{
		std::stringstream Error;
		Error << "Received error number " << errno << " with reallocating memory to append to binary message. "
			<< "The message has not been modified.";
		CreateError(Error.str().c_str());
		return;
	}
}

namespace lacewing { struct channelinternal; struct peerinternal; }
short Extension::Handle()
{
	// If thread is not working, use Tick functionality. This may add events, so do it before the event-loop check.
	if (!Globals->_Thread)
	{
		lacewing::error e = ObjEventPump->tick();
		if (e != nullptr)
		{
			CreateError(e->tostring());
			return 0; // Run next loop
		}
	}

	// AddEvent() was called and not yet handled
	// (note all code that accesses Saved must have ownership of Lock)

	// If Thread is not available, we have to tick() on Handle(), so
	// we have to run next loop even if there's no events in Saved() to deal with.
	bool RunNextLoop = !Globals->_Thread;

	for (size_t maxTrig = 0; maxTrig < 5; maxTrig++)
	{
		// Attempt to Enter, break if we can't get it instantly
		if (!TryEnterCriticalSection(&Globals->Lock))
		{
			RunNextLoop = true;
			break; // Lock already occupied; leave it and run next event loop
		}
		// At this point we have effectively run EnterCriticalSection
#ifdef _DEBUG
		::CriticalSection << "Thread " << GetCurrentThreadId() << " : Entered on "
			<< __FILE__ << ", line " << __LINE__ << ".\r\n";
#endif

		if (Saved.empty())
		{
			LeaveCriticalSectionDerpy(&Globals->Lock);
			break;
		}
		SaveExtInfo * S = Saved.front();
		Saved.erase(Saved.begin());

		#define DenyReasonSwitch(a) { free(DenyReason); DenyReason = a ? _strdup(a) : nullptr; }

		InteractivePending = S->InteractiveType;
		if (S->InteractiveType == InteractiveType::ConnectRequest)
			DenyReasonSwitch(Globals->AutoResponse_Connect_DenyReason)
		else if (S->InteractiveType == InteractiveType::ChannelLeave)
			DenyReasonSwitch(Globals->AutoResponse_ChannelLeave_DenyReason)
		else if (S->InteractiveType == InteractiveType::ClientNameSet)
		{
			NewClientName = _strdup(S->Requested.Name);
			DenyReasonSwitch(Globals->AutoResponse_NameSet_DenyReason)
		}
		else if (S->InteractiveType == InteractiveType::ChannelJoin)
		{
			NewChannelName = _strdup(S->Requested.Name);
			DenyReasonSwitch(Globals->AutoResponse_ChannelJoin_DenyReason)
		}
		else if (S->InteractiveType == InteractiveType::ChannelMessageIntercept)
		{
			DropMessage = Globals->AutoResponse_MessageChannel == GlobalInfo::Deny_Quiet ||
				Globals->AutoResponse_MessageChannel == GlobalInfo::Deny_TellFusion;
		}
		else if (S->InteractiveType == InteractiveType::ClientMessageIntercept)
		{
			DropMessage = Globals->AutoResponse_MessageClient == GlobalInfo::Deny_Quiet ||
				Globals->AutoResponse_MessageClient == GlobalInfo::Deny_TellFusion;
		}

		#undef DenyReasonSwitch

		LeaveCriticalSectionDerpy(&Globals->Lock);

		// Trigger all stored events (more than one may be stored by calling AddEvent(***, true) )
		for (unsigned char u = 0; u < S->NumEvents; ++u)
		{
			if (S->CondTrig[u] != 0xFFFF)
			{
				for (auto i : Globals->Refs)
				{
					if (S->ReceivedMsg.Content != nullptr)
					{
						i->ThreadData.ReceivedMsg.Content = S->ReceivedMsg.Content;
						i->ThreadData.ReceivedMsg.Size = S->ReceivedMsg.Size;
						i->ThreadData.ReceivedMsg.Cursor = S->ReceivedMsg.Cursor;
						i->ThreadData.ReceivedMsg.Subchannel = S->ReceivedMsg.Subchannel;
						// We only need these two for message Intercept ability
						i->ThreadData.ReceivedMsg.Blasted = S->ReceivedMsg.Blasted;
						i->ThreadData.ReceivedMsg.Variant = S->ReceivedMsg.Variant;
					}
					if (S->Channel != nullptr)
						i->ThreadData.Channel = S->Channel;
					if (S->Client != nullptr)
						i->ThreadData.Client = S->Client;

					i->ThreadData.ReceivingClient = S->ReceivingClient;
					i->ThreadData.InteractiveType = S->InteractiveType;
					i->ThreadData.ChannelCreate_Hidden = S->ChannelCreate_Hidden;
					i->ThreadData.ChannelCreate_AutoClose = S->ChannelCreate_AutoClose;
					i->Runtime.GenerateEvent((int)S->CondTrig[u]);
				}

				// We need to access these locks in multithreaded in case the user prevents
				// us from handling the interactive event properly by their actions,
				// e.g. don't want to allow the user to close midway through a send.
				// In single threaded the tick runs before this triggers, so it's not a problem.
				
				#define EnterSectionIfMultiThread(x) if (GThread) EnterCriticalSectionDerpy(x); Sleep(0);
				#define LeaveSectionIfMultiThread(x) if (GThread) LeaveCriticalSectionDerpy(x)

				// If multiple events are triggering, only do this on the last one
				if (u == S->NumEvents - 1)
				{
					// If interactive event, check for responses.
					if (S->InteractiveType != InteractiveType::None)
					{
						if (S->InteractiveType == InteractiveType::ConnectRequest)
						{
							if (Globals->AutoResponse_Connect == GlobalInfo::WaitForFusion
								&& !S->Client->isclosed)
							{
								EnterSectionIfMultiThread(&Globals->Lock);
								Srv.connect_response(S->Client->orig(), DenyReason);
								// Disconnected by Fusion: reflect in copy
								if (DenyReason)
								{
									// Mark as disconnected
									S->Client->disconnect(false);

									// Trick this for loop into running a second event to cleanup immediately
									assert(S->NumEvents == 1); // connect req must be one event for this to work
									S->CondTrig[1] = 0xFFFF;
									S->NumEvents = 2;
								}
								LeaveSectionIfMultiThread(&Globals->Lock);
							}
						}
						else if (S->InteractiveType == InteractiveType::ClientNameSet)
						{
							if (Globals->AutoResponse_NameSet == GlobalInfo::WaitForFusion
								&& !S->Client->isclosed)
							{
								EnterSectionIfMultiThread(&Globals->Lock);
								Srv.nameset_response(S->Client->orig(), NewClientName, DenyReason);

								if (!DenyReason)
									S->Client->name(NewClientName);
								LeaveSectionIfMultiThread(&Globals->Lock);
							}
							free(NewClientName);
							NewClientName = nullptr;
						}
						else if (S->InteractiveType == InteractiveType::ChannelJoin)
						{
							if (Globals->AutoResponse_ChannelJoin == GlobalInfo::WaitForFusion
								&& !S->Channel->isclosed && !S->Client->isclosed)
							{
								EnterSectionIfMultiThread(&Globals->Lock);
								const char * passedNewChannelName = NewChannelName;
								char * denyReason = DenyReason;
								bool channelmasterset = false;

								if (!denyReason)
								{
									// Server owner ran "Change requested name" and passed the same name.
									// Reeplace channel name with null.
									const char * newChannelName = NewChannelName;
									if (newChannelName && !strcmp(S->Channel->name(), passedNewChannelName))
										newChannelName = nullptr;

									// Channel name was changed in join request
									if (newChannelName)
									{
										// New name sucks
										if (newChannelName[0] == 0 || strnlen(newChannelName, 256) == 256)
										{
											std::stringstream str;
											str << "Replacement channel name \"" << newChannelName <<
												"\" was blank or too long. Maximum channel name length is 255 bytes. "
												"Denying join channel request.";
											CreateError(str.str().c_str());
											denyReason = "Server replaced joining channel name to something invalid.";
										}
										// If this new name does not exist, we're going to create it.
										// If it does exist, we're going to join client to it.
										// Which is not the one we're joining this client to.
										else
										{
											auto newNameCh = std::find_if(Channels.cbegin(), Channels.cend(),
												[=](ChannelCopy * const c) { return c != S->Channel && !_stricmp(c->name(), newChannelName);
											});

											// New name exists. Switch channel.
											if (newNameCh != Channels.cend())
											{
												// If the pre-name-change channel is not in server list, it should be deleted as this
												// is the last access point for the memory.
												// If it is in the list, the server will handle deleting it when needed.

												// If user is creating a channel, it won't be in server channel list until it's
												// create approved. (The creating user won't be in the channel's client list until
												// create approved either.)
												// If user had created it with autoclose off, and then left, the channel will exist
												// when a second client goes to join it.							

												// If the channel has clients, we're joining, in which case we don't need to delete
												bool foundPreNameChangeChannel = (*newNameCh)->clientcount() != 0U;
												if (!foundPreNameChangeChannel)
												{
													for (auto col = Srv.firstchannel(); col; col = col->next())
													{
														if (col == &S->Channel->orig())
														{
															foundPreNameChangeChannel = true;
															break;
														}
													}
												}

												if (!foundPreNameChangeChannel)
												{
													S->Channel->close();
													delete S->Channel;
												}

												S->Channel = *newNameCh;
											}
											else // new name not found.
											{
												// Old channel was being created by this user: rename it to new name
												if (S->Channel->newlycreated())
													S->Channel->name(newChannelName);
												else // Old channel was being joined (new name created); so we won't do anything with old channel.
													S->Channel = new ChannelCopy(Srv.createchannel(NewChannelName, S->Client->orig(), !S->ChannelCreate_Hidden, S->ChannelCreate_AutoClose));

												channelmasterset = true;
											}
										}
									}
									// Created new channel, now it's approved, add it to Fusion list
									// It'll be added to Relay Server's list when joinchannel_response is run.
									else if (S->Channel->newlycreated())
										channelmasterset = true;
								} // a deny reason was passed by server

								// Deny, create or join channel
								//  called by joinchannel_response
								Srv.joinchannel_response(S->Channel->orig(), S->Client->orig(), denyReason);

								if (denyReason)
								{
									// Renamed and then denied a channel; delete it
									// TODO: This check may not be necessary.
									if (S->Channel->newlycreated())
									{
										S->Channel->close();
										delete S->Channel;
									}
								}
								else
								{
									S->Channel->addclient(S->Client, false);
									if (channelmasterset)
										S->Channel->setchannelmaster(S->Client);
								}
								LeaveSectionIfMultiThread(&Globals->Lock);
							} // is any closed

							free(NewChannelName);
							NewChannelName = nullptr;
						}
						else if (S->InteractiveType == InteractiveType::ChannelLeave)
						{
							if (Globals->AutoResponse_ChannelLeave == GlobalInfo::WaitForFusion
								&& !S->Channel->isclosed && !S->Client->isclosed)
							{
								EnterSectionIfMultiThread(&Globals->Lock);
								Srv.leavechannel_response(S->Channel->orig(), S->Client->orig(), DenyReason);

								if (!DenyReason)
								{
									// Left channel, approved by Fusion: reflect in copy
									S->Channel->removeclient(S->Client, false);

									// Trick this for loop into running a second event to cleanup immediately
									assert(S->NumEvents == 1); // leave channel req must be one event for this to work
									S->CondTrig[1] = 0xFFFF;
									S->NumEvents = 2;

									// Valid channel + client with 0xFFFF is seen as "client leaving channel".
									// If the channel closed, we need to do close channel, which is channel + no client with 0xFFFF.
									if (S->Channel->isclosed)
										S->Client = nullptr;
								}
								LeaveSectionIfMultiThread(&Globals->Lock);
							}
						}
						else if (S->InteractiveType == InteractiveType::ChannelMessageIntercept)
						{
							if (Globals->AutoResponse_MessageChannel == GlobalInfo::WaitForFusion)
							{
								if (!DropMessage && (S->Channel->isclosed || S->SenderClient->isclosed))
									DropMessage = true;

								// ThreadData.SenderClient may not be accurate, since user selection will alter Client,
								// and it overlaps with Client. Refer to original event S.
								EnterSectionIfMultiThread(&Globals->Lock);
								Srv.channelmessage_permit(S->SenderClient->orig(), S->Channel->orig(),
									ThreadData.ReceivedMsg.Blasted, ThreadData.ReceivedMsg.Subchannel,
									ThreadData.ReceivedMsg.Content, ThreadData.ReceivedMsg.Size,
									ThreadData.ReceivedMsg.Variant, !DropMessage);
								LeaveSectionIfMultiThread(&Globals->Lock);
							}

							// Replace messages not programmed.
							// Look like Lacewing Relay was going to use Send Binary, though, but if they replace with
							// text/number the send binary should stay intact.
							// We may have to use a 3rd message struct as we don't want to use the SendMsg data.
							//Srv.channelmessage_permit(*ThreadData.Channel, *ThreadData.SenderClient,
							//	ThreadData.ReceivedMsg.Content, ThreadData.MsgToSend.Size, ThreadData.ReceivedMsg.Subchannel);
						}
						else if (S->InteractiveType == InteractiveType::ClientMessageIntercept)
						{
							if (Globals->AutoResponse_MessageClient == GlobalInfo::WaitForFusion)
							{
								if (!DropMessage && (ThreadData.Channel->isclosed ||
									ThreadData.SenderClient->isclosed || ThreadData.ReceivingClient->isclosed))
									DropMessage = true;

								EnterSectionIfMultiThread(&Globals->Lock);

								Srv.clientmessage_permit(ThreadData.SenderClient->orig(), ThreadData.Channel->orig(),
									ThreadData.ReceivingClient->orig(),
									ThreadData.ReceivedMsg.Blasted, ThreadData.ReceivedMsg.Subchannel,
									ThreadData.ReceivedMsg.Content, ThreadData.ReceivedMsg.Size,
									ThreadData.ReceivedMsg.Variant, !DropMessage);

								LeaveSectionIfMultiThread(&Globals->Lock);
							}
						}
						else
							Globals->CreateError("Interactive type not recognised!");
					}

					// Free memory for received message
					ThreadData.Free();
				}

				#undef EnterSectionIfMultiThread
				#undef LeaveSectionIfMultiThread
			}
			// Remove copies if this particular event number is used
			else
			{
				// If channel, it's a channel leave or peer leaving channel
				if (S->Channel)
				{
					// Channel, no client: channel closing.
					// Worth noting this is not called for non-autoclose channels.
					if (!S->Client)
					{
						assert(S->Channel->isclosed);

						for (auto u = Channels.begin(); u != Channels.end(); ++u)
						{
							if (*u == S->Channel)
							{
								Channels.erase(u);
								delete S->Channel;
								break;
							}
						}

						for (auto dropExt : Globals->Refs)
							if (dropExt->ThreadData.Channel == S->Channel)
								dropExt->ThreadData.Channel = nullptr;
					}
					// Channel, client: peer leaving channel.
					else
					{
						// Client and channel are already fully updated.
						// Unlike RelayClient, clients aren't copied per channel, so we
						// don't need to delete.
						
						// We just make sure user doesn't have them selected.
						// At least, not within the channel they just left.
						for (auto dropExt : Globals->Refs)
						{
							if (dropExt->ThreadData.Channel == S->Channel &&
								dropExt->ThreadData.Client == S->Client)
								dropExt->ThreadData.Client = nullptr;
						}
					}
				}
				// No channel, client: client is disconnecting.
				else if (S->Client)
				{
					assert(S->Client->isclosed);

					// Drop client
					for (auto u = Clients.begin(); u != Clients.end(); ++u)
					{
						if (*u == S->Client)
						{
							Clients.erase(u);
							delete S->Client;
							break;
						}
					}

					// Make sure user doesn't have them selected.
					for (auto dropExt : Globals->Refs)
					{
						if (dropExt->ThreadData.Client == S->Client)
							dropExt->ThreadData.Client = nullptr;
					}
				}
				// No channel, no peer: server has unhosted, clear everything
				else 
				{
					for (auto i : Channels)
					{
						assert(i->isclosed);
						i->close();
						delete i;
					}
					for (auto i : Clients)
					{
						assert(i->isclosed);
						delete i;
					}
					for (auto dropExt : Globals->Refs)
					{
						dropExt->ThreadData.Channel = nullptr;
						dropExt->ThreadData.Client = nullptr;
						dropExt->ThreadData.ReceivingClient = nullptr;
					}

					Channels.clear();
					Clients.clear();
				}
			}
		}
	}

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


DWORD WINAPI ObjectDestroyTimeoutFunc(void * ThisGlobalsInfo)
{
	GlobalInfo& G = *(GlobalInfo *)ThisGlobalsInfo;

	// If the user has created a new object which is receiving events from Bluewing
	// it's cool, just close silently
	if (!G.Refs.empty())
		return 0U;

	// If not hosting, no clients to worry about dropping
	if (!G._Server.hosting())
		return 0U;

	// App closed within next 3 seconds: unhost by default
	if (WaitForSingleObject(AppWasClosed, 3000U) == WAIT_OBJECT_0)
		return 0U;

	// 3 seconds have passed: if we now have an ext, or server was unhosted, we're good
	if (!G.Refs.empty())
		return 0U;

	if (!G._Server.hosting())
		return 0U;

	if (G.TimeoutWarningEnabled)
	{
		// Otherwise, fuss at them.
		MessageBoxA(NULL, "Bluewing Server Warning!\r\n"
			"All Bluewing Server objects have been destroyed and some time has passed; but "
			"the server is still hosting in the background, unused, but still available.\r\n"
			"If this is intended behaviour, disable the Timeout warning in the object properties.\r\n"
			"If you want to unhost if no instances remain open, use the FullCleanup action on the Bluewing object.",
			"Bluewing Server Warning",
			MB_OK | MB_DEFBUTTON1 | MB_ICONEXCLAMATION | MB_TOPMOST);
	}
	delete &G; // Cleanup!
	return 0U;
}

Extension::~Extension()
{
	char msgBuff[500];
	sprintf_s(msgBuff, "~Extension called; Refs count is %u.\n", Globals->Refs.size());
	OutputDebugStringA(msgBuff);

	EnterCriticalSectionDerpy(&Globals->Lock);
	// Remove this Extension from liblacewing usage.
	auto i = std::find(Globals->Refs.cbegin(), Globals->Refs.cend(), this);
	bool wasBegin = i == Globals->Refs.cbegin();
	Globals->Refs.erase(i);

	// Shift secondary event management to other Extension, if any
	if (!Globals->Refs.empty())
	{
		OutputDebugStringA("Note: Switched Lacewing instances.\n");

		// Switch Handle ticking over to next Extension visible.
		if (wasBegin)
		{
			Globals->_Ext = Globals->Refs.front();
			LeaveCriticalSectionDerpy(&Globals->Lock);

			Globals->_Ext->Runtime.Rehandle();
		}
		else // This extension wasn't even the main event handler (for Handle()/Globals).
		{
			LeaveCriticalSectionDerpy(&Globals->Lock);
		}
	}
	// Last instance of this object; if global and not full-delete-enabled, do not cleanup.
	// In single-threaded server instances, this will cause a dirty timeout from the client side;
	// the lower-level protocols, e.g. TCP, will close the connection after a certain amount of
	// not-responding.
	// In multi-threaded instances, messages and events will continue to be handled, and no visible
	// difference is made to the clients, other than the events explicitly handled by Fusion events.
	else
	{
		if (!Globals->_Server.hosting())
			OutputDebugStringA("Note: Not hosting, nothing important to retain, closing Globals info.\n");
		else if (!IsGlobal)
			OutputDebugStringA("Note: Not global, closing Globals info.\n");
		else if (Globals->FullDeleteEnabled)
			OutputDebugStringA("Note: Full delete enabled, closing Globals info.\n");
		// Wait for 0ms returns immediately as per spec
		else if (WaitForSingleObject(AppWasClosed, 0U) == WAIT_OBJECT_0)
			OutputDebugStringA("Note: App was closed, closing Globals info.\n"); 
		else // !Globals->FullDeleteEnabled
		{
			OutputDebugStringA("Note: Last instance dropped, and currently hosting - "
				"Globals will be retained until a Unhost is called.\n");
			Globals->_Ext = nullptr;
			Globals->LastDestroyedExtSelectedChannel = ThreadData.Channel;
			Globals->LastDestroyedExtSelectedClient = ThreadData.Client;
			LeaveCriticalSectionDerpy(&Globals->Lock);

			sprintf_s(msgBuff, "Timeout thread started. If no instance has reclaimed ownership in 3 seconds,%s.\n",
				Globals->TimeoutWarningEnabled
				? "a warning message will be shown"
				: "the hosting will terminate and all pending messages will be discarded");
			OutputDebugStringA(msgBuff);

			CreateThread(NULL, 0, ObjectDestroyTimeoutFunc, Globals, NULL, NULL);
			ClearThreadData();
			return;
		}

		std::string id = std::string(std::string("LacewingRelayServer") + (Globals->_GlobalID ? Globals->_GlobalID : ""));
		Runtime.WriteGlobal(id.c_str(), nullptr);
		LeaveCriticalSectionDerpy(&Globals->Lock);
		delete Globals; // Unhosts and closes event pump, deletes Lock
		Globals = nullptr;
	}

	ClearThreadData();
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
