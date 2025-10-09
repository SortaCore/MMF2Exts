#pragma once
#include "DarkEdif.hpp"
#include <functional>
#include "MultiThreading.hpp"
void NewEvent(EventToRun *);

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
	static const int Version = lacewing::relayclient::buildnum;

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

	std::shared_ptr<EventToRun> threadData;
	std::tstring_view loopName;
	static std::atomic<bool> AppWasClosed;

	bool isGlobal;

	struct GlobalInfo;
	GlobalInfo* globals;

	// This allows prettier and more readable access while maintaining global variables.
#define ObjEventPump				globals->_objEventPump
#define Cli							globals->_client
#define PreviousName				globals->_previousName
#define SendMsg						globals->_sendMsg
#define DenyReasonBuffer			globals->_denyReasonBuffer
#define SendMsgSize					globals->_sendMsgSize
#define AutomaticallyClearBinary	globals->_automaticallyClearBinary
#define GlobalID					globals->_globalID
#define HostIP						globals->_hostIP
#define HostPort					globals->_hostPort

	std::shared_ptr<lacewing::relayclient::channel> selChannel;
	std::shared_ptr<lacewing::relayclient::channel::peer> selPeer; // make sure it's one inside selChannel!
	bool isOverloadWarningQueued = false;

	void CreateError(PrintFHintInside const char* errU8, ...) PrintFHintAfter(2, 3);

	void SendMsg_Sub_AddData(const void*, size_t);
	bool IsValidPtr(const void*);
	void ClearThreadData();

	std::string TStringToUTF8Simplified(std::tstring);

	// Returns 0 if OK. -1 if cut off UTF-8 at front, 1 if cut off at end
	int CheckForUTF8Cutoff(std::string_view sv);

	// UTF-8 uses a bitmask to determine how many bytes are in the item.
	// Note that this does not verify the ending characters other than a size check; but any TString converter will throw them out
	int GetNumBytesInUTF8Char(std::string_view sv);

	// Reads string at given position of received binary. If size is -1, will expect a null.
	// isCursorExpression is used for error messages.
	std::tstring RecvMsg_Sub_ReadString(size_t index, int size, bool isCursorExpression);

	static void eventpumpdeleter(lacewing::eventpump);
	static void LacewingLoopThread(void* ThisExt);
	static void ObjectDestroyTimeoutFunc(void* ThisGlobalInfo);

	DarkEdif::FusionDebugger FusionDebugger;

	// Because Bluewing is multithreaded, and uses a second queue, once we move the variables outside of its
	// functions into DarkEdif, the data may be overwritten, causing crashes and other such nasties.
	// All read-write changes to clients, channel, and the main client are protected by read/write locks,
	// even scenarios like disconnects are handled with lock safety.

	/// Actions

	void Replaced_Connect(const TCHAR* Hostname, int Port);
	void Disconnect();
	void SetName(const TCHAR* name);
	void Replaced_JoinChannel(const TCHAR* ChannelName, int HideChannel);
	void LeaveChannel();
	void SendTextToServer(int subchannel, const TCHAR* TextToSend);
	void SendTextToChannel(int subchannel, const TCHAR* TextToSend);
	void SendTextToPeer(int subchannel, const TCHAR* TextToSend);
	void SendNumberToServer(int subchannel, int NumToSend);
	void SendNumberToChannel(int subchannel, int NumToSend);
	void SendNumberToPeer(int subchannel, int NumToSend);
	void BlastTextToServer(int subchannel, const TCHAR* TextToSend);
	void BlastTextToChannel(int subchannel, const TCHAR* TextToSend);
	void BlastTextToPeer(int subchannel, const TCHAR* TextToSend);
	void BlastNumberToServer(int subchannel, int NumToSend);
	void BlastNumberToChannel(int subchannel, int NumToSend);
	void BlastNumberToPeer(int subchannel, int NumToSend);
	void SelectChannelWithName(const TCHAR* ChannelName);
	void ReplacedNoParams();
	void LoopClientChannels();
	void SelectPeerOnChannelByName(const TCHAR* PeerName);
	void SelectPeerOnChannelByID(int PeerID);
	void LoopPeersOnChannel();
	// ReplacedNoParams, x7
	void RequestChannelList();
	void LoopListedChannels();
	// ReplacedNoParams, x3
	void SendBinaryToServer(int subchannel);
	void SendBinaryToChannel(int subchannel);
	void SendBinaryToPeer(int subchannel);
	void BlastBinaryToServer(int subchannel);
	void BlastBinaryToChannel(int subchannel);
	void BlastBinaryToPeer(int subchannel);
	void SendMsg_AddASCIIByte(const TCHAR* Byte);
	void SendMsg_AddByteInt(int Byte);
	void SendMsg_AddShort(int Short);
	void SendMsg_AddInt(int Int);
	void SendMsg_AddFloat(float Float);
	void SendMsg_AddStringWithoutNull(const TCHAR* String);
	void SendMsg_AddString(const TCHAR* String);
	void SendMsg_AddBinaryFromAddress(unsigned int Address, int size);
	void SendMsg_Clear();
	void RecvMsg_SaveToFile(int Position, int size, const TCHAR* Filename);
	void RecvMsg_AppendToFile(int Position, int size, const TCHAR* Filename);
	void SendMsg_AddFileToBinary(const TCHAR* File);
	// ReplacedNoParams, x11
	void SelectChannelMaster();
	void JoinChannel(const TCHAR* ChannelName, int Hidden, int CloseAutomatically);
	void SendMsg_CompressBinary();
	void RecvMsg_DecompressBinary();
	void RecvMsg_MoveCursor(int Position);
	void LoopListedChannelsWithLoopName(const TCHAR* LoopName);
	void LoopClientChannelsWithLoopName(const TCHAR* LoopName);
	void LoopPeersOnChannelWithLoopName(const TCHAR* LoopName);
	// ReplacedNoParams
	void Connect(const TCHAR* Hostname);
	void SendMsg_Resize(int NewSize);
	void SetDestroySetting(int enabled);
	void SetLocalPortForHolePunch(int port);

	/// Conditions

	const bool AlwaysTrue() { return true; }
	const bool AlwaysFalse() { return false; }
	// Used for triggered events;
	bool MandatoryTriggeredEvent();

	// CheckedEvent
	// AlwaysTrue:	bool OnConnect();
	// AlwaysTrue:	bool OnConnectDenied();
	// AlwaysTrue:	bool OnDisconnect();
	// AlwaysTrue:	bool OnChannelJoin();
	// AlwaysTrue:	bool OnChannelJoinDenied();
	// AlwaysTrue:	bool OnNameSet();
	// AlwaysTrue:	bool OnNameDenied();
	bool OnSentTextMessageFromServer(int subchannel);
	bool OnSentTextMessageFromChannel(int subchannel);
	// AlwaysTrue:	bool OnPeerConnect();
	// AlwaysTrue:	bool OnPeerDisonnect();
	// AlwaysFalse:	bool Replaced_OnChannelJoin();
	// AlwaysTrue:	bool OnChannelPeerLoop();
	// AlwaysTrue:	bool OnClientChannelLoop();
	bool OnSentNumberMessageFromServer(int subchannel);
	bool OnSentNumberMessageFromChannel(int subchannel);
	// AlwaysTrue:	bool OnChannelPeerLoopFinished();
	// AlwaysTrue:	bool OnClientChannelLoopFinished();
	// AlwaysFalse:	bool ReplacedCondNoParams();
	bool OnBlastedTextMessageFromServer(int subchannel);
	bool OnBlastedNumberMessageFromServer(int subchannel);
	bool OnBlastedTextMessageFromChannel(int subchannel);
	bool OnBlastedNumberMessageFromChannel(int subchannel);
	// ReplacedCondNoParams, x3
	// AlwaysTrue:	bool OnChannelListReceived();
	// AlwaysTrue:	bool OnChannelListLoop();
	// AlwaysTrue:	bool OnChannelListLoopFinished();
	// ReplacedCondNoParams, x3
	bool OnSentBinaryMessageFromServer(int subchannel);
	bool OnSentBinaryMessageFromChannel(int subchannel);
	bool OnBlastedBinaryMessageFromServer(int subchannel);
	bool OnBlastedBinaryMessageFromChannel(int subchannel);
	bool OnSentTextMessageFromPeer(int subchannel);
	bool OnSentNumberMessageFromPeer(int subchannel);
	bool OnSentBinaryMessageFromPeer(int subchannel);
	bool OnBlastedTextMessageFromPeer(int subchannel);
	bool OnBlastedNumberMessageFromPeer(int subchannel);
	bool OnBlastedBinaryMessageFromPeer(int subchannel);
	bool IsConnected();
	// AlwaysTrue:	bool OnChannelLeave();
	// AlwaysTrue:	bool OnChannelLeaveDenied();
	// AlwaysTrue:	bool OnPeerChangedName();
	// ReplacedCondNoParams
	bool OnAnySentMessageFromServer(int subchannel);
	bool OnAnySentMessageFromChannel(int subchannel);
	bool OnAnySentMessageFromPeer(int subchannel);
	bool OnAnyBlastedMessageFromServer(int subchannel);
	bool OnAnyBlastedMessageFromChannel(int subchannel);
	bool OnAnyBlastedMessageFromPeer(int subchannel);
	// AlwaysTrue:	bool OnNameChanged();
	bool ClientHasAName();
	// ReplacedCondNoParams, x2
	bool SelectedPeerIsChannelMaster();
	bool YouAreChannelMaster();
	bool OnChannelListLoopWithName(const TCHAR* LoopName);
	bool OnChannelListLoopWithNameFinished(const TCHAR* LoopName);
	bool OnPeerLoopWithName(const TCHAR* LoopName);
	bool OnPeerLoopWithNameFinished(const TCHAR* LoopName);
	bool OnClientChannelLoopWithName(const TCHAR* LoopName);
	bool OnClientChannelLoopWithNameFinished(const TCHAR* LoopName);
	bool OnSentTextChannelMessageFromServer(int subchannel);
	bool OnSentNumberChannelMessageFromServer(int subchannel);
	bool OnSentBinaryChannelMessageFromServer(int subchannel);
	bool OnAnySentChannelMessageFromServer(int subchannel);
	bool OnBlastedTextChannelMessageFromServer(int subchannel);
	bool OnBlastedNumberChannelMessageFromServer(int subchannel);
	bool OnBlastedBinaryChannelMessageFromServer(int subchannel);
	bool OnAnyBlastedChannelMessageFromServer(int subchannel);
	bool IsJoinedToChannel(const TCHAR* ChannelName);
	bool IsPeerOnChannel_Name(const TCHAR* PeerName, const TCHAR* ChannelName);
	bool IsPeerOnChannel_ID(int ID, const TCHAR* ChannelName);

	/// Expressions

	const TCHAR* Error();
	const TCHAR* ReplacedExprNoParams();
	const TCHAR* Self_Name();
	unsigned int Self_ChannelCount();
	const TCHAR* Peer_Name();
	const TCHAR* RecvMsg_ReadAsString();
	int RecvMsg_ReadAsInteger();
	unsigned int RecvMsg_Subchannel();
	int Peer_ID();
	const TCHAR* Channel_Name();
	int Channel_PeerCount();
	// ReplacedExprNoParams
	const TCHAR* ChannelListing_Name();
	int ChannelListing_PeerCount();
	int Self_ID();
	// ReplacedExprNoParams, x5
	const TCHAR* RecvMsg_StrASCIIByte(int Index);
	unsigned int RecvMsg_UnsignedByte(int Index);
	int RecvMsg_SignedByte(int Index);
	unsigned int RecvMsg_UnsignedShort(int Index);
	int RecvMsg_SignedShort(int Index);
	unsigned int RecvMsg_UnsignedInteger(int Index);
	int RecvMsg_SignedInteger(int Index);
	float RecvMsg_Float(int Index);
	const TCHAR* RecvMsg_StringWithSize(int Index, int size);
	const TCHAR* RecvMsg_String(int Index);
	unsigned int RecvMsg_SizeInBytes();
	const TCHAR* Lacewing_Version();
	unsigned int SendBinaryMsg_Size();
	const TCHAR* Self_PreviousName();
	const TCHAR* Peer_PreviousName();
	// ReplacedExprNoParams, x2
	const TCHAR* DenyReason();
	const TCHAR* Host_IP();
	unsigned int Host_Port();
	// ReplacedExprNoParams
	const TCHAR* WelcomeMessage();
	unsigned int RecvMsg_MemoryAddress();
	const TCHAR* RecvMsg_Cursor_StrASCIIByte();
	unsigned int RecvMsg_Cursor_UnsignedByte();
	int RecvMsg_Cursor_SignedByte();
	unsigned int RecvMsg_Cursor_UnsignedShort();
	int RecvMsg_Cursor_SignedShort();
	unsigned int RecvMsg_Cursor_UnsignedInteger();
	int RecvMsg_Cursor_SignedInteger();
	float RecvMsg_Cursor_Float();
	const TCHAR* RecvMsg_Cursor_StringWithSize(int size);
	const TCHAR* RecvMsg_Cursor_String();
	// ReplacedExprNoParams
	unsigned int SendBinaryMsg_MemoryAddress();
	const TCHAR* RecvMsg_DumpToString(int Index, const TCHAR* Format);
	unsigned int ChannelListing_ChannelCount();
	int ConvToUTF8_GetVisibleCharCount(const TCHAR* tStr);
	int ConvToUTF8_GetCompleteCodePointCount(const TCHAR* tStr);
	int ConvToUTF8_GetByteCount(const TCHAR* tStr);
	const TCHAR* ConvToUTF8_TestAllowList(const TCHAR* tStr, const TCHAR* charset);

	struct GlobalInfo final
	{
		// Lacewing event queue and ticker
		std::unique_ptr<lacewing::_eventpump, std::function<decltype(eventpumpdeleter)>> _objEventPump;
		lacewing::relayclient _client;

		// Check to make sure current non-ignorable triggered event was processed by Fusion events
		bool lastMandatoryEventWasChecked = true;

		// Server's IP address, set during connect; no port
		std::string _hostIP;
		unsigned int _hostPort = UINT32_MAX;

		// Binary message to send
		char* _sendMsg;
		// Number of bytes in binary message to send (sendMsg)
		std::size_t _sendMsgSize;

		// Previous name of this client, as UTF-8
		std::string _previousName;
		// Last deny reason, set during Handle() when running deny events
		std::string _denyReasonBuffer;
		// Clear binary when a binary message is sent/blasted?
		bool _automaticallyClearBinary;

		// This GlobalInfo global ID of extension, in UTF-8
		std::string _globalID;
		// If in multithreading mode, the Lacewing message handler thread
		std::thread _thread;
		// Current "owner" extension used to run events. Can be null, e.g. during frame switches
		Extension* _ext;
		// Thread handle; Thread checking whether a client extension has not regained control of connection in a reasonable time, i.e. slow frame transition
		std::thread timeoutThread;
		// Event; counterpart to AppWasClosed, but used for cancelling the timeout thread from Ext ctor when a new ext is taking over
		std::atomic<bool> cancelTimeoutThread;


		// Used to keep Fusion selection across frames
		std::weak_ptr<lacewing::relayclient::channel> lastDestroyedExtSelectedChannel;
		std::weak_ptr<lacewing::relayclient::channel::peer> lastDestroyedExtSelectedPeer;

		// Queued conditions to trigger, with selected client/channel
		std::vector<std::shared_ptr<EventToRun>> _eventsToRun;

		// Lock to protect GlobalInfo contents, initialized to zeroes.
		Edif::recursive_mutex lock;
		// List of all extensions holding this Global ID
		std::vector<Extension*> extsHoldingGlobals;
		// If no Bluewing exists, fuss after a preset time period
		bool timeoutWarningEnabled;
		// If no Bluewing exists after DestroyRunObject, clean up this GlobalInfo
		bool fullDeleteEnabled;
		// Used to determine if an error event happened in a Fusion event, e.g. user put in bad parameter.
		// Fusion code always runs in main thread, but errors can occur outside of user input.
		std::thread::id mainThreadID;
		// If single-threaded, indicates if Lacewing is being ticked by Handle(). Used for error message location.
		bool lacewingTicking = false;
		// Local port used when doing a connection. If 0, unset and OS picks.
		// @remarks Firewall hole punching works by having both sides connect to each other at the same time,
		// using pre-supplied local ports - which can be random, but must be known.
		// Since outgoing connections are allowed, the routers generate an exception for that connection;
		// and the incoming connection's matching tuple of 
		//		(local IP, local port, remote IP, remote port)
		// plus exception specifically for that, creates a two-way exception through the firewall.
		// However, when NAT is involved (network address translation, like port forwarding), it can get
		// a mismatch and this will fail.
		// Since uPnP is an IPv4 tech mostly, hole punching is useful for IPv6 clients behind rigid firewalls;
		// that aside, uPnP is not always on in routers, and IPv4 hole punching can work for them.
		unsigned short localPort = 0;

		// Max size of a UDP message - good values are 1400 bytes for Ethernet MTU,
		// and 576 bytes for minimum IPv4 packet transmissible without fragmentation.
		// Another size of note is a bit under 16KiB, due to SSL record size + Lacewing headers.
		unsigned short maxUDPSize = lacewing::relay_max_udp_payload;

		// Locks and queues an EventToRun with 1 condition ID to trigger
		void AddEvent1(std::uint16_t event1ID,
			std::shared_ptr<lacewing::relayclient::channel> channel = nullptr,
			std::shared_ptr<lacewing::relayclient::channellisting> channelListing = nullptr,
			std::shared_ptr<lacewing::relayclient::channel::peer> peer = nullptr,
			std::string_view messageOrErrorText = std::string_view(),
			lw_ui8 subchannel = 255, lw_ui8 variant = 255);
		// Locks and queues an EventToRun with 2 condition IDs to trigger (e.g. on any message + on text message)
		void AddEvent2(std::uint16_t event1ID, std::uint16_t event2ID,
			std::shared_ptr<lacewing::relayclient::channel> channel = nullptr,
			std::shared_ptr<lacewing::relayclient::channellisting> channelListing = nullptr,
			std::shared_ptr<lacewing::relayclient::channel::peer> peer = nullptr,
			std::string_view messageOrErrorText = std::string_view(),
			lw_ui8 subchannel = 255, lw_ui8 variant = 255);
	private:
		// Locks and queues an EventToRun with either 1 or 2 condition IDs to trigger (used by AddEvent1 & 2)
		void AddEventF(bool twoEvents, std::uint16_t event1ID, std::uint16_t event2ID,
			std::shared_ptr<lacewing::relayclient::channel> channel,
			std::shared_ptr<lacewing::relayclient::channellisting> channelListing,
			std::shared_ptr<lacewing::relayclient::channel::peer> peer,
			std::string_view messageOrErrorText,
			lw_ui8 subchannel, lw_ui8 variant);
	public:
		// Queues an error event, accepts printf-like formatting e.g. printf("number is %d", number);
		void CreateError(PrintFHintInside const char* errorText, ...) PrintFHintAfter(2, 3);
		void CreateError(PrintFHintInside const char* errorText, va_list v) PrintFHintAfter(2, 0);

		// Constructor and destructor
		GlobalInfo(Extension* e, const EDITDATA* const edPtr);

		// Due to Runtime.WriteGlobal() not working if there's no Extension,
		// or not working mid-frame transition, we'll have to just fake its deletion,
		// and not free memory until later.
		bool pendingDelete = false;
		void MarkAsPendingDelete();
		~GlobalInfo() noexcept(false);
	};

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

	// These are called if there's no function linked to an ID
	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);
};
