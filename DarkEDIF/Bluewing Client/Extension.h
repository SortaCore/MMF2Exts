#include "Edif.h"
class Extension
{
public:
	#ifdef MULTI_THREADING
		SaveExtInfo ThreadData; // Must be first variable in Extension class
		std::vector<SaveExtInfo *> Saved;
		SaveExtInfo &AddEvent(int Event, bool UseLastData = false);
		void NewEvent(SaveExtInfo *);
		CRITICAL_SECTION Lock;
	#endif

    RUNDATA * rdPtr;
    RunHeader * rhPtr;

    Edif::Runtime Runtime;

    static const int MinimumBuild = 251;
    static const int Version = 39;

    static const int OEFLAGS = 0; // Use OEFLAGS namespace
    static const int OEPREFS = 0; // Use OEPREFS namespace
    
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
	struct GlobalInfo {
		Lacewing::EventPump		_ObjEventPump;
		Lacewing::RelayClient	_Client;
		char *					_PreviousName,
			 *					_SendMsg,
			 *					_DenyReasonBuffer;
		size_t					_SendMsgSize;
		bool					_AutomaticallyClearBinary;
		char *					_GlobalID;
		HANDLE					_Thread;
		
		GlobalInfo() : _Client(_ObjEventPump), _PreviousName(NULL),
		_SendMsg(NULL), _DenyReasonBuffer(NULL), _SendMsgSize(0),
		_AutomaticallyClearBinary(false), _GlobalID(NULL), _Thread(NULL) {}
	} * Globals;

	// This allows prettier and more readable access while maintaining global variables.
	#define ObjEventPump				Globals->_ObjEventPump
	#define Cli							Globals->_Client
	#define PreviousName				Globals->_PreviousName
	#define SendMsg						Globals->_SendMsg
	#define DenyReasonBuffer			Globals->_DenyReasonBuffer
	#define SendMsgSize					Globals->_SendMsgSize
	#define AutomaticallyClearBinary	Globals->_AutomaticallyClearBinary
	#define GlobalID					Globals->_GlobalID

	

	void CreateError(const char *);

	void AddToSend(void *, size_t);
	

	// Because Lacewing is singlethreaded, once we move the variables outside of its functions into DarkEDIF,
	// the data may be overwritten, causing crashes and other such nasties.
	// To work around this, we duplicate all the variables, and provide a special event number which will remove
	// all the pointers in SaveExtInfo after they should no longer be valid.
	// In this way, when a peer is disconnected then a channel left, both can be queried properly while as far as
	// Lacewing is concerned they no longer exist.

	std::vector<Lacewing::RelayClient::Channel *> Channels;
	std::vector<Lacewing::RelayClient::Channel::Peer *> Peers;
	Lacewing::RelayClient::Channel * DuplicateChannel(Lacewing::RelayClient::Channel &);
	Lacewing::RelayClient::Channel::Peer * DuplicatePeer(Lacewing::RelayClient::Channel::Peer &);

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
		void SetName(char * Name);
		void Replaced_JoinChannel(char * ChannelName, int HideChannel);
		void LeaveChannel();
		void SendTextToServer(int Subchannel, char * TextToSend);
		void SendTextToChannel(int Subchannel, char * TextToSend);
		void SendTextToPeer(int Subchannel, char * TextToSend);
		void SendNumberToServer(int Subchannel, int NumToSend);
		void SendNumberToChannel(int Subchannel, int NumToSend);
		void SendNumberToPeer(int Subchannel, int NumToSend);
		void BlastTextToServer(int Subchannel, char * TextToSend);
		void BlastTextToChannel(int Subchannel, char * TextToSend);
		void BlastTextToPeer(int Subchannel, char * TextToSend);
		void BlastNumberToServer(int Subchannel, int NumToSend);
		void BlastNumberToChannel(int Subchannel, int NumToSend);
		void BlastNumberToPeer(int Subchannel, int NumToSend);
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
		void SendBinaryToServer(int Subchannel);
		void SendBinaryToChannel(int Subchannel);
		void SendBinaryToPeer(int Subchannel);
		void BlastBinaryToServer(int Subchannel);
		void BlastBinaryToChannel(int Subchannel);
		void BlastBinaryToPeer(int Subchannel);
		void AddByteText(char * Byte);
		void AddByteInt(int Byte);
		void AddShort(int Short);
		void AddInt(int Int);
		void AddFloat(float Float);
		void AddStringWithoutNull(char * String);
		void AddString(char * String);
		void AddBinary(void * Address, int Size);
		void ClearBinaryToSend();
		void SaveReceivedBinaryToFile(int Position, int Size, char * Filename);
		void AppendReceivedBinaryToFile(int Position, int Size, char * Filename);
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
		bool OnSentTextMessageFromServer(int Subchannel);
		bool OnSentTextMessageFromChannel(int Subchannel);
		// AlwaysTrue:	bool OnPeerConnect();
		// AlwaysTrue:	bool OnPeerDisonnect();
		// AlwaysFalse:	bool Replaced_OnChannelJoin();
		// AlwaysTrue:	bool OnChannelPeerLoop();
		// AlwaysTrue:	bool OnClientChannelLoop();
		bool OnSentNumberMessageFromServer(int Subchannel);
		bool OnSentNumberMessageFromChannel(int Subchannel);
		// AlwaysTrue:	bool OnChannelPeerLoopFinished();
		// AlwaysTrue:	bool OnClientChannelLoopFinished();
		// AlwaysFalse:	bool ReplacedCondNoParams();
		bool OnBlastedTextMessageFromServer(int Subchannel);
		bool OnBlastedNumberMessageFromServer(int Subchannel);
		bool OnBlastedTextMessageFromChannel(int Subchannel);
		bool OnBlastedNumberMessageFromChannel(int Subchannel);
		// ReplacedCondNoParams, x3
		// AlwaysTrue:	bool OnChannelListReceived();
		// AlwaysTrue:	bool OnChannelListLoop();
		// AlwaysTrue:	bool OnChannelListLoopFinished();
		// ReplacedCondNoParams, x3
		bool OnSentBinaryMessageFromServer(int Subchannel);
		bool OnSentBinaryMessageFromChannel(int Subchannel);
		bool OnBlastedBinaryMessageFromServer(int Subchannel);
		bool OnBlastedBinaryMessageFromChannel(int Subchannel);
		bool OnSentTextMessageFromPeer(int Subchannel);
		bool OnSentNumberMessageFromPeer(int Subchannel);
		bool OnSentBinaryMessageFromPeer(int Subchannel);
		bool OnBlastedTextMessageFromPeer(int Subchannel);
		bool OnBlastedNumberMessageFromPeer(int Subchannel);
		bool OnBlastedBinaryMessageFromPeer(int Subchannel);
		bool IsConnected();
		// AlwaysTrue:	bool OnChannelLeave();
		// AlwaysTrue:	bool OnChannelLeaveDenied();
		// AlwaysTrue:	bool OnPeerChangedName();
		// ReplacedCondNoParams
		bool OnAnySentMessageFromServer(int Subchannel);
		bool OnAnySentMessageFromChannel(int Subchannel);
		bool OnAnySentMessageFromPeer(int Subchannel);
		bool OnAnyBlastedMessageFromServer(int Subchannel);
		bool OnAnyBlastedMessageFromChannel(int Subchannel);
		bool OnAnyBlastedMessageFromPeer(int Subchannel);
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
		bool OnSentTextChannelMessageFromServer(int Subchannel);
		bool OnSentNumberChannelMessageFromServer(int Subchannel);
		bool OnSentBinaryChannelMessageFromServer(int Subchannel);
		bool OnAnySentChannelMessageFromServer(int Subchannel);
		bool OnBlastedTextChannelMessageFromServer(int Subchannel);
		bool OnBlastedNumberChannelMessageFromServer(int Subchannel);
		bool OnBlastedBinaryChannelMessageFromServer(int Subchannel);
		bool OnAnyBlastedChannelMessageFromServer(int Subchannel);
		bool IsJoinedToChannel(char * ChannelName);
		bool IsPeerOnChannel_Name(char * PeerName, char * ChannelName);
		bool IsPeerOnChannel_ID(int ID, char * ChannelName);

    /// Expressions

		const char * Error();
		const char * ReplacedExprNoParams();
		const char * Self_Name();
		int Self_ChannelCount();
		const char * Peer_Name();
		const char * ReceivedStr();
		int ReceivedInt();
		int Subchannel();
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
		const char * StringWithSize(int Index, int Size);
		const char * String(int Index);
		int ReceivedBinarySize();
		const char * Lacewing_Version();
		int SendBinarySize();
		const char * Self_PreviousName();
		const char * Peer_PreviousName();
		// ReplacedExprNoParams, x2
		const char * DenyReason();
		const char * HostIP();
		int HostPort();
		// ReplacedExprNoParams
		const char * WelcomeMessage();
		long ReceivedBinaryAddress();
		const char * CursorStrByte();
		unsigned int CursorUnsignedByte();
		int CursorSignedByte();
		unsigned int CursorUnsignedShort();
		int CursorSignedShort();
		unsigned int CursorUnsignedInteger();
		int CursorSignedInteger();
		float CursorFloat();
		const char * CursorStringWithSize(int Size);
		const char * CursorString();
		// ReplacedExprNoParams
		long SendBinaryAddress();
		const char * DumpMessage(int Index, const char * Format);

    /* These are called if there's no function linked to an ID */

    void Action(int ID, RUNDATA * rdPtr, long param1, long param2);
    long Condition(int ID, RUNDATA * rdPtr, long param1, long param2);
    long Expression(int ID, RUNDATA * rdPtr, long param);

	

    /*  These replace the functions like HandleRunObject that used to be
        implemented in Runtime.cpp. They work exactly the same, but they're
        inside the extension class.
    */

    short Handle();
    short Display();

    short Pause();
    short Continue();

    bool Save(HANDLE File);
    bool Load(HANDLE File);

};
