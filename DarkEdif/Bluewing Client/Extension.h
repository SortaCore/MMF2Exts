
#include "Edif.h"
#include <functional>
void NewEvent(SaveExtInfo *);

struct GlobalInfo;
class Extension
{
public:
	// Hide stuff requiring other headers
	std::shared_ptr<SaveExtInfo> threadData; // Must be first variable in Extension class
	std::string_view loopName;

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

	void CreateError(const char *);

	void AddToSend(void *, size_t);
	void ClearThreadData();


	// Because Bluewing is multithreaded, and uses a second queue, once we move the variables outside of its
	// functions into DarkEdif, the data may be overwritten, causing crashes and other such nasties.
	
	// To work around this, we duplicate all the variables, and provide a special event number which will remove
	// all the pointers in SaveExtInfo after they should no longer be valid.
	// In this way, when a peer sends message then leaves channel, on liblacewing's side that happens instantly,
	// and liblacewing cleans up its peer variable instantly too.
	// But since these events are queued, the "on peer disconnect" may be called later, after liblacewing has deleted.
	// So we have to store a copy of the peer so we can look up name when it's disconnected.
	// See the _channels variable in GlobalInfo for our copy.

	// int MyVariable;




	/*  Add your actions, conditions and expressions as real class member
		functions here. The arguments (and return type for expressions) must
		match EXACTLY what you defined in the JSON.

		Remember to link the actions, conditions and expressions to their
		numeric IDs in the class constructor (Extension.cpp)
	*/



	/// Actions

		void Replaced_Connect(char * Hostname, int Port);
		void Disconnect();
		void SetName(char * name);
		void Replaced_JoinChannel(char * ChannelName, int HideChannel);
		void LeaveChannel();
		void SendTextToServer(int subchannel, char * TextToSend);
		void SendTextToChannel(int subchannel, char * TextToSend);
		void SendTextToPeer(int subchannel, char * TextToSend);
		void SendNumberToServer(int subchannel, int NumToSend);
		void SendNumberToChannel(int subchannel, int NumToSend);
		void SendNumberToPeer(int subchannel, int NumToSend);
		void BlastTextToServer(int subchannel, char * TextToSend);
		void BlastTextToChannel(int subchannel, char * TextToSend);
		void BlastTextToPeer(int subchannel, char * TextToSend);
		void BlastNumberToServer(int subchannel, int NumToSend);
		void BlastNumberToChannel(int subchannel, int NumToSend);
		void BlastNumberToPeer(int subchannel, int NumToSend);
		void SelectChannelWithName(char * ChannelName);
		void ReplacedNoParams();
		void LoopClientChannels();
		void SelectPeerOnChannelByName(char * PeerName);
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
		void AddByteText(char * Byte);
		void AddByteInt(int Byte);
		void AddShort(int Short);
		void AddInt(int Int);
		void AddFloat(float Float);
		void AddStringWithoutNull(char * String);
		void AddString(char * String);
		void AddBinary(unsigned int Address, int size);
		void ClearBinaryToSend();
		void SaveReceivedBinaryToFile(int Position, int size, char * Filename);
		void AppendReceivedBinaryToFile(int Position, int size, char * Filename);
		void AddFileToBinary(char * File);
		// ReplacedNoParams, x11
		void SelectChannelMaster();
		void JoinChannel(char * ChannelName, int Hidden, int CloseAutomatically);
		void CompressSendBinary();
		void DecompressReceivedBinary();
		void MoveReceivedBinaryCursor(int Position);
		void LoopListedChannelsWithLoopName(char * LoopName);
		void LoopClientChannelsWithLoopName(char * LoopName);
		void LoopPeersOnChannelWithLoopName(char * LoopName);
		// ReplacedNoParams
		void Connect(char * Hostname);
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
		bool OnChannelListLoopWithName(char * LoopName);
		bool OnChannelListLoopWithNameFinished(char * LoopName);
		bool OnPeerLoopWithName(char * LoopName);
		bool OnPeerLoopWithNameFinished(char * LoopName);
		bool OnClientChannelLoopWithName(char * LoopName);
		bool OnClientChannelLoopWithNameFinished(char * LoopName);
		bool OnSentTextChannelMessageFromServer(int subchannel);
		bool OnSentNumberChannelMessageFromServer(int subchannel);
		bool OnSentBinaryChannelMessageFromServer(int subchannel);
		bool OnAnySentChannelMessageFromServer(int subchannel);
		bool OnBlastedTextChannelMessageFromServer(int subchannel);
		bool OnBlastedNumberChannelMessageFromServer(int subchannel);
		bool OnBlastedBinaryChannelMessageFromServer(int subchannel);
		bool OnAnyBlastedChannelMessageFromServer(int subchannel);
		bool IsJoinedToChannel(char * ChannelName);
		bool IsPeerOnChannel_Name(char * PeerName, char * ChannelName);
		bool IsPeerOnChannel_ID(int ID, char * ChannelName);

	/// Expressions

		const char * Error();
		const char * ReplacedExprNoParams();
		const char * Self_Name();
		unsigned int Self_ChannelCount();
		const char * Peer_Name();
		const char * ReceivedStr();
		int ReceivedInt();
		unsigned int Subchannel();
		int Peer_ID();
		const char * Channel_Name();
		int Channel_PeerCount();
		// ReplacedExprNoParams
		const char * ChannelListing_Name();
		int ChannelListing_PeerCount();
		int Self_ID();
		// ReplacedExprNoParams, x5
		const char * StrByte(int Index);
		unsigned int UnsignedByte(int Index);
		int SignedByte(int Index);
		unsigned int UnsignedShort(int Index);
		int SignedShort(int Index);
		unsigned int UnsignedInteger(int Index);
		int SignedInteger(int Index);
		float Float(int Index);
		const char * StringWithSize(int Index, int size);
		const char * String(int Index);
		unsigned int ReceivedBinarySize();
		const char * Lacewing_Version();
		unsigned int SendBinarySize();
		const char * Self_PreviousName();
		const char * Peer_PreviousName();
		// ReplacedExprNoParams, x2
		const char * DenyReason();
		const char * Host_IP();
		unsigned int HostPort();
		// ReplacedExprNoParams
		const char * WelcomeMessage();
		unsigned int ReceivedBinaryAddress();
		const char * CursorStrByte();
		unsigned int CursorUnsignedByte();
		int CursorSignedByte();
		unsigned int CursorUnsignedShort();
		int CursorSignedShort();
		unsigned int CursorUnsignedInteger();
		int CursorSignedInteger();
		float CursorFloat();
		const char * CursorStringWithSize(int size);
		const char * CursorString();
		// ReplacedExprNoParams
		unsigned int SendBinaryAddress();
		const char * DumpMessage(int Index, const char * Format);
		unsigned int ChannelListing_ChannelCount();

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
	void CreateError(const char * errorText);

	GlobalInfo(Extension * e, EDITDATA * edPtr);
	~GlobalInfo() noexcept(false);
};
