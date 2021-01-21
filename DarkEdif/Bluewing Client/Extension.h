
#include "Edif.h"
#include <functional>
void NewEvent(SaveExtInfo *);

struct GlobalInfo;
class Extension
{
public:
	// Hide stuff requiring other headers
	std::shared_ptr<SaveExtInfo> threadData; // Must be first variable in Extension class
	std::tstring_view loopName;

	RUNDATA * rdPtr;
	RunHeader * rhPtr;

	Edif::Runtime Runtime;

	static const int MinimumBuild = 251;
	static const int Version = lacewing::relayclient::buildnum;

	static const OEFLAGS OEFLAGS = OEFLAGS::NEVER_KILL | OEFLAGS::NEVER_SLEEP; // Use OEFLAGS namespace
	static const OEPREFS OEPREFS = OEPREFS::GLOBAL; // Use OEPREFS namespace

	static const int WindowProcPriority = 100;

	Extension(RUNDATA * rdPtr, EDITDATA * edPtr, CreateObjectInfo * cobPtr);
	~Extension();


	/*  Add any data you want to store in your extension to this class
		(eg. what you'd normally store in rdPtr).

		For those using multi-threading, any variables that are modified
		by the threads should be in SaveExtInfo.
		See MultiThreading.h.

		Unlike rdPtr, you can store real C++ objects with constructors
		and destructors, without having to call them manually or store
		a pointer.
	*/

	bool isGlobal;
	GlobalInfo * globals;

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

	std::shared_ptr<lacewing::relayclient::channel> selChannel;
	std::shared_ptr<lacewing::relayclient::channel::peer> selPeer; // make sure it's one inside selChannel!
	bool isOverloadWarningQueued = false;

	void CreateError(_Printf_format_string_ const char * errU8, ...);

	void AddToSend(const void *, size_t);
	void ClearThreadData();

	std::string TStringToUTF8Stripped(std::tstring);

	// Returns 0 if OK. -1 if cut off UTF-8 at front, 1 if cut off at end
	int CheckForUTF8Cutoff(std::string_view sv);

	// UTF-8 uses a bitmask to determine how many bytes are in the item.
	// Note that this does not verify the ending characters other than a size check; but any TString converter will throw them out
	int GetNumBytesInUTF8Char(std::string_view sv);

	// Reads string at given position of received binary. If size is -1, will expect a null.
	// isCursorExpression is used for error messages.
	std::tstring ReadStringFromRecvBinary(size_t index, int size, bool isCursorExpression);

	DarkEdif::FusionDebugger FusionDebugger;

	// Because Bluewing is multithreaded, and uses a second queue, once we move the variables outside of its
	// functions into DarkEdif, the data may be overwritten, causing crashes and other such nasties.
	// All read-write changes to clients, channel, and the main client are protected by read/write locks,
	// even scenarios like disconnects are handled with lock safety.

	/// Actions

		void Replaced_Connect(const TCHAR * Hostname, int Port);
		void Disconnect();
		void SetName(const TCHAR * name);
		void Replaced_JoinChannel(const TCHAR * ChannelName, int HideChannel);
		void LeaveChannel();
		void SendTextToServer(int subchannel, const TCHAR * TextToSend);
		void SendTextToChannel(int subchannel, const TCHAR * TextToSend);
		void SendTextToPeer(int subchannel, const TCHAR * TextToSend);
		void SendNumberToServer(int subchannel, int NumToSend);
		void SendNumberToChannel(int subchannel, int NumToSend);
		void SendNumberToPeer(int subchannel, int NumToSend);
		void BlastTextToServer(int subchannel, const TCHAR * TextToSend);
		void BlastTextToChannel(int subchannel, const TCHAR * TextToSend);
		void BlastTextToPeer(int subchannel, const TCHAR * TextToSend);
		void BlastNumberToServer(int subchannel, int NumToSend);
		void BlastNumberToChannel(int subchannel, int NumToSend);
		void BlastNumberToPeer(int subchannel, int NumToSend);
		void SelectChannelWithName(const TCHAR * ChannelName);
		void ReplacedNoParams();
		void LoopClientChannels();
		void SelectPeerOnChannelByName(const TCHAR * PeerName);
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
		void AddByteText(const TCHAR * Byte);
		void AddByteInt(int Byte);
		void AddShort(int Short);
		void AddInt(int Int);
		void AddFloat(float Float);
		void AddStringWithoutNull(const TCHAR * String);
		void AddString(const TCHAR * String);
		void AddBinary(unsigned int Address, int size);
		void ClearBinaryToSend();
		void SaveReceivedBinaryToFile(int Position, int size, const TCHAR * Filename);
		void AppendReceivedBinaryToFile(int Position, int size, const TCHAR * Filename);
		void AddFileToBinary(const TCHAR * File);
		// ReplacedNoParams, x11
		void SelectChannelMaster();
		void JoinChannel(const TCHAR * ChannelName, int Hidden, int CloseAutomatically);
		void CompressSendBinary();
		void DecompressReceivedBinary();
		void MoveReceivedBinaryCursor(int Position);
		void LoopListedChannelsWithLoopName(const TCHAR * LoopName);
		void LoopClientChannelsWithLoopName(const TCHAR * LoopName);
		void LoopPeersOnChannelWithLoopName(const TCHAR * LoopName);
		// ReplacedNoParams
		void Connect(const TCHAR * Hostname);
		void ResizeBinaryToSend(int NewSize);
		void SetDestroySetting(int enabled);

	/// Conditions

		const bool AlwaysTrue() { return true; }
		const bool AlwaysFalse() { return false; }
		// AlwaysTrue:	bool OnError();
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
		bool OnChannelListLoopWithName(const TCHAR * LoopName);
		bool OnChannelListLoopWithNameFinished(const TCHAR * LoopName);
		bool OnPeerLoopWithName(const TCHAR * LoopName);
		bool OnPeerLoopWithNameFinished(const TCHAR * LoopName);
		bool OnClientChannelLoopWithName(const TCHAR * LoopName);
		bool OnClientChannelLoopWithNameFinished(const TCHAR * LoopName);
		bool OnSentTextChannelMessageFromServer(int subchannel);
		bool OnSentNumberChannelMessageFromServer(int subchannel);
		bool OnSentBinaryChannelMessageFromServer(int subchannel);
		bool OnAnySentChannelMessageFromServer(int subchannel);
		bool OnBlastedTextChannelMessageFromServer(int subchannel);
		bool OnBlastedNumberChannelMessageFromServer(int subchannel);
		bool OnBlastedBinaryChannelMessageFromServer(int subchannel);
		bool OnAnyBlastedChannelMessageFromServer(int subchannel);
		bool IsJoinedToChannel(const TCHAR * ChannelName);
		bool IsPeerOnChannel_Name(const TCHAR * PeerName, const TCHAR * ChannelName);
		bool IsPeerOnChannel_ID(int ID, const TCHAR * ChannelName);

	/// Expressions

		const TCHAR * Error();
		const TCHAR * ReplacedExprNoParams();
		const TCHAR * Self_Name();
		unsigned int Self_ChannelCount();
		const TCHAR * Peer_Name();
		const TCHAR * ReceivedStr();
		int ReceivedInt();
		unsigned int Subchannel();
		int Peer_ID();
		const TCHAR * Channel_Name();
		int Channel_PeerCount();
		// ReplacedExprNoParams
		const TCHAR * ChannelListing_Name();
		int ChannelListing_PeerCount();
		int Self_ID();
		// ReplacedExprNoParams, x5
		const TCHAR * StrASCIIByte(int Index);
		unsigned int UnsignedByte(int Index);
		int SignedByte(int Index);
		unsigned int UnsignedShort(int Index);
		int SignedShort(int Index);
		unsigned int UnsignedInteger(int Index);
		int SignedInteger(int Index);
		float Float(int Index);
		const TCHAR * StringWithSize(int Index, int size);
		const TCHAR * String(int Index);
		unsigned int ReceivedBinarySize();
		const TCHAR * Lacewing_Version();
		unsigned int SendBinarySize();
		const TCHAR * Self_PreviousName();
		const TCHAR * Peer_PreviousName();
		// ReplacedExprNoParams, x2
		const TCHAR * DenyReason();
		const TCHAR * Host_IP();
		unsigned int HostPort();
		// ReplacedExprNoParams
		const TCHAR * WelcomeMessage();
		unsigned int ReceivedBinaryAddress();
		const TCHAR * CursorStrASCIIByte();
		unsigned int CursorUnsignedByte();
		int CursorSignedByte();
		unsigned int CursorUnsignedShort();
		int CursorSignedShort();
		unsigned int CursorUnsignedInteger();
		int CursorSignedInteger();
		float CursorFloat();
		const TCHAR * CursorStringWithSize(int size);
		const TCHAR * CursorString();
		// ReplacedExprNoParams
		unsigned int SendBinaryAddress();
		const TCHAR * DumpMessage(int Index, const TCHAR * Format);
		unsigned int ChannelListing_ChannelCount();
		int ConvToUTF8_GetVisibleCharCount(const TCHAR * tStr);
		int ConvToUTF8_GetCompleteCharCount(const TCHAR * tStr);
		int ConvToUTF8_GetByteCount(const TCHAR * tStr);

	/* These are called if there's no function linked to an ID */

	void Action(int ID, RUNDATA * rdPtr, long param1, long param2);
	long Condition(int ID, RUNDATA * rdPtr, long param1, long param2);
	long Expression(int ID, RUNDATA * rdPtr, long param);



	/*  These replace the functions like HandleRunObject that used to be
		implemented in Runtime.cpp. They work exactly the same, but they're
		inside the extension class.
	*/

	REFLAG Handle();
	REFLAG Display();

	short Pause();
	short Continue();

	bool Save(HANDLE File);
	bool Load(HANDLE File);

};

void eventpumpdeleter(lacewing::eventpump);

struct GlobalInfo
{
	std::unique_ptr<lacewing::_eventpump, std::function<decltype(eventpumpdeleter)>>	_objEventPump;
	lacewing::relayclient						_client;
	std::string									_previousName;
	char *										_sendMsg;
	std::string									_denyReasonBuffer;
	size_t										_sendMsgSize;
	bool										_automaticallyClearBinary;
	std::string									_globalID;
	HANDLE										_thread;
	Extension *									_ext;
	std::vector<std::shared_ptr<SaveExtInfo>>	_saved;
	std::string									_hostIP;
	HANDLE										timeoutThread = NULL;
	std::weak_ptr<lacewing::relayclient::channel>				lastDestroyedExtSelectedChannel;
	std::weak_ptr<lacewing::relayclient::channel::peer>		lastDestroyedExtSelectedPeer;

	CRITICAL_SECTION							lock;
	std::vector<Extension *>					refs;
	bool										timeoutWarningEnabled; // If no Bluewing exists, fuss after set time period
	bool										fullDeleteEnabled; // If no Bluewing exists after DestroyRunObject, clean up GlobalInfo

	void AddEvent1(std::uint16_t event1ID,
		std::shared_ptr<lacewing::relayclient::channel> channel = nullptr,
		std::shared_ptr<lacewing::relayclient::channellisting> channelListing = nullptr,
		std::shared_ptr<lacewing::relayclient::channel::peer> peer = nullptr,
		std::string_view messageOrErrorText = std::string_view(),
		lw_ui8 subchannel = 255);
	void AddEvent2(std::uint16_t event1ID, std::uint16_t event2ID,
		std::shared_ptr<lacewing::relayclient::channel> channel = nullptr,
		std::shared_ptr<lacewing::relayclient::channellisting> channelListing = nullptr,
		std::shared_ptr<lacewing::relayclient::channel::peer> peer = nullptr,
		std::string_view messageOrErrorText = std::string_view(),
		lw_ui8 subchannel = 255);
private:
	void AddEventF(bool twoEvents, std::uint16_t event1ID, std::uint16_t event2ID,
		std::shared_ptr<lacewing::relayclient::channel> channel = nullptr,
		std::shared_ptr<lacewing::relayclient::channellisting> channelListing = nullptr,
		std::shared_ptr<lacewing::relayclient::channel::peer> peer = nullptr,
		std::string_view messageOrErrorText = std::string_view(),
		lw_ui8 subchannel = 255
		);
public:
	void CreateError(_Printf_format_string_ const char * errorText, ...);
	void CreateError(_Printf_format_string_ const char *errorText, va_list v);

	GlobalInfo(Extension * e, EDITDATA * edPtr);
	~GlobalInfo() noexcept(false);
};
