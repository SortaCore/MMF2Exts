#pragma once
#include "DarkEdif.hpp"
#include "MultiThreading.hpp"
#include <functional>

static constexpr std::uint16_t CLEAR_EVTNUM = 0xFFFF;
static constexpr std::uint16_t DUMMY_EVTNUM = 35353;

class Extension final
{
public:
	// ======================================
	// Required variables + functions
	// Variables here must not be moved or swapped around or it can cause future issues
	// ======================================
	RunHeader* rhPtr;
	RunObjectMultiPlatPtr rdPtr;
#ifdef __ANDROID__
	global<jobject> javaExtPtr;
#elif defined(__APPLE__)
	void* const objCExtPtr;
#endif

	Edif::Runtime Runtime;

	static const int MinimumBuild = 251;
	static const int Version = lacewing::relayserver::buildnum;

	static constexpr OEFLAGS OEFLAGS = OEFLAGS::NEVER_KILL | OEFLAGS::NEVER_SLEEP;
	static constexpr OEPREFS OEPREFS = OEPREFS::NONE;

#ifdef _WIN32
	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
#elif defined(__ANDROID__)
	Extension(const EDITDATA* const edPtr, const jobject javaExtPtr, const CreateObjectInfo* const cobPtr);
#else
	Extension(const EDITDATA* const edPtr, void* const objCExtPtr, const CreateObjectInfo* const cobPtr);
#endif
	~Extension();

	// ======================================
	// Extension data
	// ======================================

	DarkEdif::FusionDebugger FusionDebugger;

	std::shared_ptr<EventToRun> threadData;
	static std::atomic<bool> AppWasClosed;
	static void eventpumpdeleter(lacewing::eventpump);
	static void LacewingLoopThread(Extension* ThisExt);

	bool isGlobal;
	struct GlobalInfo;
	GlobalInfo * globals;
	static void ObjectDestroyTimeoutFunc(GlobalInfo* G);


	// This allows prettier and more readable access while maintaining global variables.
	#define ObjEventPump				globals->_objEventPump
	#define Srv							globals->_server
	#define FlashSrv					globals->_server.flash
	#define SendMsg						globals->_sendMsg
	#define DenyReason					globals->_denyReason
	#define SendMsgSize					globals->_sendMsgSize
	#define AutomaticallyClearBinary	globals->_automaticallyClearBinary
	#define GlobalID					globals->_globalID
	#define NewChannelName				globals->_newChannelName
	#define NewClientName				globals->_newClientName
	#define DropMessage					globals->_dropMessage
	#define InteractivePending			globals->_interactivePending
	#define GThread						globals->_thread
	std::tstring_view loopName;

	// Selections local to this extension. When outside of events, these are used.
	// When events are run by Handle(), this selection is copied out, then altered to match event.
	// Following the event, the selection is restored.


	// While these selXX are shared_ptr, they are considered not true owners.
	// Due to that, make sure if server is being destroyed, these are reset *before* server close.
	// They are kept as shared_ptr and not weak_ptr simply for more readable code,
	// although this may be changed later.
	std::shared_ptr<lacewing::relayserver::channel> selChannel;
	std::shared_ptr<lacewing::relayserver::client> selClient;
	bool isOverloadWarningQueued = false;


	void ClearThreadData();
	void CreateError(PrintFHintInside const char * errU8, ...) PrintFHintAfter(2, 3);

	// Called as a subfunction by actions to add to the message-to-send
	void SendMsg_Sub_AddData(const void *, size_t);
	// Checks the pointer against known bad addresses. It's a quick check, not a perfect one.
	bool IsValidPtr(const void * ptr);

	// called from Handle() when a Lacewing object is being destroyed (e.g. client disconnect, channel leave)
	void DeselectIfDestroyed(std::shared_ptr<EventToRun> s);
	// called from Handle() when an interactive event needs to be responded to
	void HandleInteractiveEvent(std::shared_ptr<EventToRun> s);

	// Runs DarkEdif::TStringToUTF8() then lw_u8str_simplify() on input.
	std::string TStringToUTF8Simplified(std::tstring_view);

	// Returns 0 if OK. -1 if cut off UTF-8 at front, 1 if cut off at end
	int CheckForUTF8Cutoff(std::string_view sv);

	// UTF-8 uses a bitmask to determine how many bytes are in the item.
	// Note that this does not verify the ending characters other than a size check; but any TString converter will throw them out
	int GetNumBytesInUTF8Char(std::string_view sv);

	// Reads string at given position of received binary. If sizeInCodePoints is -1, will expect a null byte.
	// isCursorExpression is used for error messages.
	std::tstring RecvMsg_Sub_ReadString(size_t index, int sizeInCodePoints, bool isCursorExpression);

	// To work around this, we use a special event number which will deselect
	// all the pointers in EventToRun after they should no longer be valid.
	// In this way, when a client sends message then disconnects, on liblacewing's side that happens instantly,
	// and liblacewing cleans up its client variable instantly too.
	// We can't "deny" a disconnect, so we must accept it immediately.

	/// Actions
	void RemovedActionNoParams();

	void RelayServer_Host(int port);
	void RelayServer_StopHosting();
	void FlashServer_Host(const TCHAR * path);
	void FlashServer_StopHosting();
	void WebSocketServer_LoadHostCertificate_FromFile(const TCHAR* chainFile, const TCHAR* privkeyFile, const TCHAR* password);
	void WebSocketServer_LoadHostCertificate_FromSystemStore(const TCHAR* store, const TCHAR* name, const TCHAR* password);
	void WebSocketServer_EnableHosting(int insecurePort, int securePort);
	void WebSocketServer_DisableHosting(const TCHAR * which);
	void ChannelListing_Enable();
	void ChannelListing_Disable();
	void SetWelcomeMessage(const TCHAR * message);
	void SetUnicodeAllowList(const TCHAR * listToSet, const TCHAR * allowListContents);
	void EnableCondition_OnConnectRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason);
	void EnableCondition_OnNameSetRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason);
	void EnableCondition_OnJoinChannelRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason);
	void EnableCondition_OnLeaveChannelRequest(int informFusion, int immediateRespondWith, const TCHAR * autoDenyReason);
	void EnableCondition_OnMessageToChannel(int informFusion, int immediateRespondWith);
	void EnableCondition_OnMessageToPeer(int informFusion, int immediateRespondWith);
	void EnableCondition_OnMessageToServer(int informFusion);
	void OnInteractive_Deny(const TCHAR * reason);
	void OnInteractive_ChangeClientName(const TCHAR * newName);
	void OnInteractive_ChangeChannelName(const TCHAR * newName);
	void OnInteractive_DropMessage();
	void OnInteractive_ReplaceMessageWithText(const TCHAR * newText);
	void OnInteractive_ReplaceMessageWithNumber(int newNumber);
	void OnInteractive_ReplaceMessageWithSendBinary();

	void Channel_SelectByName(const TCHAR * name);
	void Channel_SelectByID(int id);
	void Channel_Close();
	void Channel_SelectMaster();
	void Channel_LoopClients();
	void Channel_LoopClientsWithName(const TCHAR * loopName);
	void Channel_SetLocalData(const TCHAR * key, const TCHAR * value);
	void LoopAllChannels();
	void LoopAllChannelsWithName(const TCHAR * loopName);
	void Channel_CreateChannelWithMasterByName(const TCHAR * channelName, int hidden, int autoclose, const TCHAR * masterClientName);
	void Channel_CreateChannelWithMasterByID(const TCHAR * channelName, int hidden, int autoclose, int masterClientID);
	void Channel_KickClientByName(const TCHAR * clientName);
	void Channel_KickClientByID(int id);
	void Channel_JoinClientByName(const TCHAR * clientName);
	void Channel_JoinClientByID(int id);
	void Client_Disconnect();
	void Client_SetLocalData(const TCHAR * key, const TCHAR * value);
	void Client_LoopJoinedChannels();
	void Client_LoopJoinedChannelsWithName(const TCHAR * loopName);
	void Client_SelectByName(const TCHAR * clientName);
	void Client_SelectByID(int clientID);
	void Client_SelectSender();
	void Client_SelectReceiver();
	void Client_JoinToChannel(const TCHAR * channelNamePtr);
	void LoopAllClients();
	void LoopAllClientsWithName(const TCHAR * loopName);

	void SendTextToChannel(int subchannel, const TCHAR * textToSend);
	void SendTextToClient(int subchannel, const TCHAR * textToSend);
	void SendNumberToChannel(int subchannel, int numToSend);
	void SendNumberToClient(int subchannel, int numToSend);
	void SendBinaryToChannel(int subchannel);
	void SendBinaryToClient(int subchannel);

	void BlastTextToChannel(int subchannel, const TCHAR * TextToBlast);
	void BlastTextToClient(int subchannel, const TCHAR * TextToBlast);
	void BlastNumberToChannel(int subchannel, int NumToBlast);
	void BlastNumberToClient(int subchannel, int NumToBlast);
	void BlastBinaryToChannel(int subchannel);
	void BlastBinaryToClient(int subchannel);

	void SendMsg_AddASCIIByte(const TCHAR * Byte);
	void SendMsg_AddByteInt(int Byte);
	void SendMsg_AddShort(int Short);
	void SendMsg_AddInt(int Int);
	void SendMsg_AddFloat(float Float);
	void SendMsg_AddStringWithoutNull(const TCHAR * string);
	void SendMsg_AddString(const TCHAR * string);
	void SendMsg_AddBinaryFromAddress(unsigned int address, int size);
	void SendMsg_AddFileToBinary(const TCHAR * file);
	void SendMsg_Resize(int newSize);
	void SendMsg_CompressBinary();
	void SendMsg_Clear();

	void RecvMsg_DecompressBinary();
	void RecvMsg_MoveCursor(int Position);
	void RecvMsg_SaveToFile(int Position, int size, const TCHAR * Filename);
	void RecvMsg_AppendToFile(int Position, int size, const TCHAR * Filename);

	void Relay_DoHolePunchToFutureClient(const TCHAR* clientIP, int localPort);


	/// Conditions

	const bool AlwaysTrue() { return true; }
	const bool AlwaysFalse() { return false; }
	const bool AlwaysFalseWithTextParam(const TCHAR * s) { return false; }
	bool SubchannelMatches(int subchannel);
	// AlwaysTrue:	bool OnError();
	// AlwaysTrue:	bool OnConnectRequest();
	// AlwaysTrue:	bool OnDisconnect();
	// AlwaysTrue:	bool OnChannel_JoinRequest();
	// AlwaysTrue:	bool OnChannel_LeaveRequest();
	// AlwaysTrue:	bool AllChannelsLoop();
	// AlwaysTrue:	bool OnClient_JoinedChannelsLoop();
	// AlwaysTrue:	bool AllClientsLoop();
	// AlwaysTrue:	bool OnChannel_ClientLoop();
	bool Client_IsChannelMaster();
	// AlwaysTrue:	bool OnClient_NameSetRequest();
	// MessageMatches:	bool OnSentTextMessageToServer(int subchannel);
	// MessageMatches:	bool OnSentNumberMessageToServer(int subchannel);
	// MessageMatches:	bool OnSentBinaryMessageToServer(int subchannel);
	// MessageMatches:	bool OnAnySentMessageToServer(int subchannel);
	// MessageMatches:	bool OnSentTextMessageToChannel(int subchannel);
	// MessageMatches:	bool OnSentNumberMessageToChannel(int subchannel);
	// MessageMatches:	bool OnSentBinaryMessageToChannel(int subchannel);
	// MessageMatches:	bool OnAnySentMessageToChannel(int subchannel);
	// MessageMatches:	bool OnSentTextMessageToPeer(int subchannel);
	// MessageMatches:	bool OnSentNumberMessageToPeer(int subchannel);
	// MessageMatches:	bool OnSentBinaryMessageToPeer(int subchannel);
	// MessageMatches:	bool OnAnySentMessageToPeer(int subchannel);
	// MessageMatches:	bool OnBlastedTextMessageToServer(int subchannel);
	// MessageMatches:	bool OnBlastedNumberMessageToServer(int subchannel);
	// MessageMatches:	bool OnBlastedBinaryMessageToServer(int subchannel);
	// MessageMatches:	bool OnAnyBlastedMessageToServer(int subchannel);
	// MessageMatches:	bool OnBlastedTextMessageToChannel(int subchannel);
	// MessageMatches:	bool OnBlastedNumberMessageToChannel(int subchannel);
	// MessageMatches:	bool OnBlastedBinaryMessageToChannel(int subchannel);
	// MessageMatches:	bool OnAnyBlastedMessageToChannel(int subchannel);
	// MessageMatches:	bool OnBlastedTextMessageToPeer(int subchannel);
	// MessageMatches:	bool OnBlastedNumberMessageToPeer(int subchannel);
	// MessageMatches:	bool OnBlastedBinaryMessageToPeer(int subchannel);
	// MessageMatches:	bool OnAnyBlastedMessageToPeer(int subchannel);
	bool OnAllChannelsLoopWithName(const TCHAR * loopName);
	bool OnClientsJoinedChannelLoopWithName(const TCHAR * loopName);
	bool OnAllClientsLoopWithName(const TCHAR * loopName);
	bool OnChannelClientsLoopWithName(const TCHAR * loopName);
	bool OnChannelClientsLoopWithNameFinished(const TCHAR * loopName);
	bool OnAllChannelsLoopWithNameFinished(const TCHAR * loopName);
	bool OnClientsJoinedChannelLoopWithNameFinished(const TCHAR * loopName);
	// AlwaysTrue	bool OnChannelsClientLoopFinished(const TCHAR * loopName);
	// AlwaysTrue	bool OnAllChannelsLoopFinished();
	// AlwaysTrue	bool OnAllClientsLoopFinished();
	// AlwaysTrue	bool OnClientsJoinedChannelLoopFinished();
	bool IsLacewingServerHosting();
	bool IsFlashPolicyServerHosting();
	bool ChannelIsHiddenFromChannelList();
	bool ChannelIsSetToCloseAutomatically();
	bool OnAllClientsLoopWithNameFinished(const TCHAR * loopName);
	// Added conditions:
	bool IsClientOnChannel_ByChannelNameClientID(int clientID, const TCHAR * channelName);
	bool IsClientOnChannel_ByChannelNameClientName(const TCHAR * clientName, const TCHAR * channelName);
	bool IsClientOnChannel_ByChannelIDClientID(int clientID, int channelID);
	bool IsClientOnChannel_ByChannelIDClientName(const TCHAR* clientName, int channelID);
	bool DoesChannelNameExist(const TCHAR * channelName);
	bool DoesChannelIDExist(int channelID);
	bool DoesClientNameExist(const TCHAR * clientName);
	bool DoesClientIDExist(int clientID);
	bool IsWebSocketHosting(const TCHAR * serverType);

	/// Expressions

	const TCHAR * Error();
	const TCHAR * Lacewing_Version();
	unsigned int SendBinaryMsg_Size();
	const TCHAR * RequestedClientName();
	const TCHAR * RequestedChannelName();
	const TCHAR * Channel_Name();
	unsigned int Channel_ClientCount();
	const TCHAR * Client_Name();
	unsigned int Client_ID();
	const TCHAR * Client_IP();
	unsigned int Client_ConnectionTime(); // NB: was removed in Lacewing, kept in Bluewing
	unsigned int Client_ChannelCount();
	const TCHAR * RecvMsg_ReadAsString();
	int RecvMsg_ReadAsInteger();
	unsigned int RecvMsg_SizeInBytes();
	unsigned int RecvMsg_MemoryAddress();
	const TCHAR * RecvMsg_StrASCIIByte(int index);
	unsigned int RecvMsg_UnsignedByte(int index);
	int RecvMsg_SignedByte(int index);
	unsigned int RecvMsg_UnsignedShort(int index);
	int RecvMsg_SignedShort(int index);
	unsigned int RecvMsg_UnsignedInteger(int index);
	int RecvMsg_SignedInteger(int index);
	float RecvMsg_Float(int index);
	const TCHAR * RecvMsg_StringWithSize(int index, int size);
	const TCHAR * RecvMsg_String(int index);
	unsigned int RecvMsg_Subchannel();
	unsigned int Channel_Count();
	const TCHAR * Client_GetLocalData(const TCHAR * key);
	const TCHAR * RecvMsg_Cursor_StrASCIIByte();
	unsigned int RecvMsg_Cursor_UnsignedByte();
	int RecvMsg_Cursor_SignedByte();
	unsigned int RecvMsg_Cursor_UnsignedShort();
	int RecvMsg_Cursor_SignedShort();
	unsigned int RecvMsg_Cursor_UnsignedInteger();
	int RecvMsg_Cursor_SignedInteger();
	float RecvMsg_Cursor_Float();
	const TCHAR * RecvMsg_Cursor_StringWithSize(int size);
	const TCHAR * RecvMsg_Cursor_String();
	const TCHAR * Client_ProtocolImplementation();
	const TCHAR * Channel_GetLocalData(const TCHAR * key);
	unsigned int Port();
	unsigned int SendBinaryMsg_MemoryAddress();
	const TCHAR * Welcome_Message();
	// Added expressions:
	const TCHAR * RecvMsg_DumpToString(int index, const TCHAR * format);
	unsigned int AllClientCount();
	const TCHAR * GetDenyReason();
	int ConvToUTF8_GetVisibleCharCount(const TCHAR * tStr);
	int ConvToUTF8_GetCompleteCodePointCount(const TCHAR * tStr);
	int ConvToUTF8_GetByteCount(const TCHAR * tStr);
	const TCHAR * ConvToUTF8_TestAllowList(const TCHAR * toTest, const TCHAR * allowList);
	int Channel_ID();
	int WebSocket_Insecure_Port();
	int WebSocket_Secure_Port();
	const TCHAR* WebSocket_Cert_ExpiryTime(int useUTC, const TCHAR * format);

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

	// These are called if there's no function linked to an ID

	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);
};


enum class AutoResponse : std::int8_t
{
	Invalid = (std::int8_t)-1,
	Approve_Quiet = 0,
	Deny_Quiet,
	Approve_TellFusion,
	Deny_TellFusion,
	WaitForFusion
};

struct Extension::GlobalInfo final
{
	std::unique_ptr<lacewing::_eventpump, std::function<decltype(eventpumpdeleter)>>	_objEventPump;
	lacewing::relayserver		_server;

	// Binary message to send
	char * _sendMsg = nullptr;
	// Number of bytes in binary message to send (sendMsg)
	size_t _sendMsgSize = 0U;

	// Current handler's name set/channel join/etc deny reason.
	// Can be set by Lacewing itself before name set request is submitted, e.g. if name is already set to what was requested.
	// Deny reason only be set once.
	std::string _denyReason;
	// Current "on join channel request" handler's new channel name, normally empty unless server Fusion events overrides name
	std::string _newChannelName;
	// Current "on name set request" handler's new client name, normally empty unless server Fusion events overrides name
	std::string _newClientName;
	// Type of triggered event that server is running right now
	InteractiveType _interactivePending = InteractiveType::None;
	// Drop the current client -> peer/channel message? Only relevant during those message handlers
	bool _dropMessage = false;
	// Clear binary when a binary message is sent/blasted?
	bool _automaticallyClearBinary = true;

	// This GlobalInfo global ID of extension, in UTF-8
	std::string _globalID;
	// If in multithreading mode, the Lacewing message handler thread
	std::thread _thread;
	// Current "owner" extension used to run events. Can be null, e.g. during frame switches.
	Extension * _ext = nullptr;
	// Deepest level named loop in use at the moment, checked against in "on loop" triggered conditions
	std::string_view _loopName;
	// Thread handle; Thread checking whether a server extension has not regained control of connection in a reasonable time, i.e. slow frame transition
	std::thread timeoutThread;
	// Event; counterpart to AppWasClosed, but used for cancelling the timeout thread from Ext ctor when a new ext is taking over
	std::atomic<bool> cancelTimeoutThread;
	// Enables or disables the inactivity timer built into liblacewing. See relayserver::setinactivitytimer().
	bool enableInactivityTimer = true;
	// If single-threaded, indicates if Lacewing is being ticked by Handle(). Used for error message location.
	bool lacewingTicking = false;

	// Max size of a UDP message - good values are 1400 bytes for Ethernet MTU,
	// and 576 bytes for minimum IPv4 packet transmissible without fragmentation.
	// Another size of note is a bit under 16KiB, due to SSL record size + Lacewing headers.
	unsigned short maxUDPSize = lacewing::relay_max_udp_payload;

	// Used to keep Fusion selection across frames
	std::weak_ptr<lacewing::relayserver::channel> lastDestroyedExtSelectedChannel;
	std::weak_ptr<lacewing::relayserver::client> lastDestroyedExtSelectedClient;

	// Queued conditions to trigger, with selected client/channel
	std::vector<std::shared_ptr<EventToRun>> _eventsToRun;
	// Used to determine if an error event happened in a Fusion event, e.g. user put in bad parameter.
	// Fusion code always runs in main thread, but errors can occur outside of user input.
	std::thread::id	mainThreadID;

	// Clang complains about the LocalData constructor, for some reason. Might be LLVM bug 20296.
#ifdef __clang__
#pragma clang diagnostic ignored "-Wundefined-internal"
#pragma clang diagnostic push
#endif

	// Used to store local data for clients/channels
	template<typename T> struct LocalData
	{
		std::shared_ptr<T> ptr;
		std::string keyU8Simplified; // key, as UTF-8, simplified destructively
		std::tstring val;

		LocalData(std::shared_ptr<T> ptr, std::string keyU8Simplified, std::tstring val)
			: ptr(ptr), keyU8Simplified(keyU8Simplified), val(val)
		{
			/* Yes, it is defined, clang. */
		}
	};

#ifdef __clang__
#pragma clang diagnostic ignored "-Wundefined-internal"
#pragma clang diagnostic pop
#endif


	std::vector<LocalData<lacewing::relayserver::client>> clientLocal;
	std::vector<LocalData<lacewing::relayserver::channel>> channelLocal;

	const std::tstring& GetLocalData(std::shared_ptr<lacewing::relayserver::client> client, std::tstring key);
	const std::tstring& GetLocalData(std::shared_ptr<lacewing::relayserver::channel> channel, std::tstring key);
	void SetLocalData(std::shared_ptr<lacewing::relayserver::client> client, std::tstring key, std::tstring value);
	void SetLocalData(std::shared_ptr<lacewing::relayserver::channel> channel, std::tstring key, std::tstring value);
	void ClearLocalData(std::shared_ptr<lacewing::relayserver::client> client);
	void ClearLocalData(std::shared_ptr<lacewing::relayserver::channel> channel);

	// Current triggered event's or user-selected channel/client
	std::shared_ptr<lacewing::relayserver::channel> selChannel = nullptr;
	std::shared_ptr<lacewing::relayserver::client> selClient = nullptr;

	// Lock to protect GlobalInfo contents, initialized to zeroes.
	Edif::recursive_mutex lock;
	// List of all extensions holding this Global ID
	std::vector<Extension *> extsHoldingGlobals;
	// If no Bluewing exists, fuss after a preset time period
	bool timeoutWarningEnabled = true;
	// If no Bluewing exists after DestroyRunObject, clean up this GlobalInfo
	bool fullDeleteEnabled = true;
	// Suppresses the channel close events during unhost actions. When server is deleted, handlers are all nulled anyway.
	bool unhostingInProgress = false;

	// Locks and queues an EventToRun with 1 condition ID to trigger
	void AddEvent1(int event1,
		std::shared_ptr<lacewing::relayserver::channel> channel = nullptr,
		std::shared_ptr<lacewing::relayserver::client> senderClient = nullptr,
		std::string_view messageOrErrorText = std::string_view(),
		lw_ui8 subchannel = 255,
		std::shared_ptr<lacewing::relayserver::client> receivingClient = nullptr,
		InteractiveType interactiveType = InteractiveType::None,
		lw_ui8 variant = 255,
		bool blasted = false,
		bool channelCreate_Hidden = false, bool channelCreate_AutoClose = false);
	// Locks and queues an EventToRun with 2 condition IDs to trigger (e.g. on any message + on text message)
	void AddEvent2(int event1, int event2,
		std::shared_ptr<lacewing::relayserver::channel> channel = nullptr,
		std::shared_ptr<lacewing::relayserver::client> senderClient = nullptr,
		std::string_view messageOrErrorText = std::string_view(),
		lw_ui8 subchannel = 255,
		std::shared_ptr<lacewing::relayserver::client> receivingClient = nullptr,
		InteractiveType interactiveType = InteractiveType::None,
		lw_ui8 variant = 255,
		bool blasted = false,
		bool channelCreate_Hidden = false, bool channelCreate_AutoClose = false);
	// Locks and queues an EventToRun with either 1 or 2 condition IDs to trigger (used by AddEvent1 & 2)
	void AddEventF(bool twoEvents, int Event1, int Event2,
		std::shared_ptr<lacewing::relayserver::channel> channel = nullptr,
		std::shared_ptr<lacewing::relayserver::client> senderClient = nullptr,
		std::string_view MessageOrErrorText = std::string_view(),
		lw_ui8 subchannel = 255,
		std::shared_ptr<lacewing::relayserver::client> receivingClient = nullptr,
		InteractiveType interactiveType = InteractiveType::None,
		lw_ui8 variant = 255,
		bool blasted = false,
		bool channelCreate_Hidden = false,
		bool channelCreate_AutoClose = false
	);
	// Queues an error event, accepts printf-like formatting e.g. printf("number is %d", number);
	void CreateError(PrintFHintInside const char * errorTextU8, ...) PrintFHintAfter(2, 3);
	void CreateError(PrintFHintInside const char * errorTextU8, va_list);

	// Handling manner for messages. Not waiting for Fusion to handle it in main thread causes a blisteringly fast automatic
	// handling system, but sometimes it is necessary to have something more complex.
	// Approve_Quiet = sends an OK message immediately, does not notify Fusion
	// Deny_Quiet = sends a Denied message immediately, does not notify Fusion
	// Approve_TellFusion = approves the event immediately, queues Fusion On [...] request event
	// Deny_TellFusion = sends a Denied message immediately, queues Fusion On [...] request event with preset deny reason
	// WaitForFusion = sends no message, queues Fusion On [...] request event, and after all triggered events finish
	//				   on Fusion's side, sends an OK or Denied message, based on whether a Deny action was run or not.
	//
	// The deny reasons here are only used in Deny_Quiet/Deny_TellFusion.
	// It is not possible to set anything except WaitForFusion and Deny_Quiet for messages to server.
	// Some handlers will not even be set in liblacewing side, but some are required for Fusion to stay in sync,
	// e.g. channel local data cannot be freed unless channel close handler is run.

	// As of b29, defaults match what Relay Server uses, to avoid confusion
	// Prior builds would only run server message events by default, for efficiency
	AutoResponse autoResponse_Connect = AutoResponse::Approve_TellFusion;
	std::string autoResponse_Connect_DenyReason;
	AutoResponse autoResponse_NameSet = AutoResponse::Approve_TellFusion;
	std::string autoResponse_NameSet_DenyReason;
	AutoResponse autoResponse_ChannelJoin = AutoResponse::Approve_TellFusion;
	std::string autoResponse_ChannelJoin_DenyReason;
	AutoResponse autoResponse_ChannelLeave = AutoResponse::Approve_TellFusion;
	std::string autoResponse_ChannelLeave_DenyReason;
	AutoResponse autoResponse_MessageClient = AutoResponse::Approve_Quiet;
	AutoResponse autoResponse_MessageChannel = AutoResponse::Approve_Quiet;
	AutoResponse autoResponse_MessageServer = AutoResponse::WaitForFusion;


	// Constructor and destructor

	GlobalInfo(Extension * e, const EDITDATA * const edPtr);

	// Due to Runtime.WriteGlobal() not working if there's no Extension,
	// or not working mid-frame transition, we'll have to just fake its deletion,
	// and not free memory until later.
	bool pendingDelete = false;
	void MarkAsPendingDelete();
	~GlobalInfo() noexcept(false);
};
