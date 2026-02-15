#pragma once
#include <map>
#include "Common.hpp"
#include "Structures.hpp"

class Extension final
{
public:
	// ======================================
	// Required variables
	// Variables here must not be moved or swapped around or it can cause future issues
	// ======================================
	RunHeader* rhPtr;
	RunObjectMultiPlatPtr rdPtr;

	Edif::Runtime Runtime;

	static const int MinimumBuild = 252;
	static const int Version = 7;

	static constexpr OEFLAGS OEFLAGS = OEFLAGS::VALUES;
	static constexpr OEPREFS OEPREFS = OEPREFS::NONE;

	Extension(RunObject* const rdPtr, const EDITDATA* const edPtr, const CreateObjectInfo* const cobPtr);
	~Extension();

	// ======================================
	// Extension data
	// ======================================

#define MsgBox(text) ::DarkEdif::MsgBox::Info(_T("Debug info"), _T("%s"), DarkEdif::UTF8ToTString(text))
#define MakeDelim() const char delim [] = "|" // Used for delimiters in tokenizing
#define FatalBox()	::DarkEdif::MsgBox::Error(_T("Bypass notification"), _T("Fatal error has not been repaired; bypassing erroneous code."));

	std::shared_ptr<EventToRun> curEvent;

	GlobalInfo * globals = nullptr;
	bool isGlobal = false;
	// Not an actual string, contains raw binary
	std::string packetBeingBuilt;

	bool Internal_GetTextWithEncoding(const std::string_view encoding, const std::string_view packetBytes, std::tstring &msgTo, int socketID);
	bool Internal_SetTextWithEncoding(const std::string_view encoding, const std::tstring_view userPacketText, std::string &msgTo, int socketID);

	// If packetBeingBuiltOffset == -1, sets it to packetBeingBuilt.size()
	void Internal_BuiltPacketSizeSet(int& packetBeingBuiltOffset);
	// Converts sockaddr to IPv6. Optionally unmaps IPv4-mapped-IPv6 to IPv4. Includes port.
	std::tstring Internal_GetIPFromSockaddr(sockaddr_storage* sockadd, size_t size);

	/// Actions

	// Got rid of these:
	// they only needed testing because I was a poor coder and not sure why things didn't work
	void DEPRECATED_TestReportAndErrors();
	// You can use a popup message object to equate it, and a popup would generally interrupt socket handling anyway
	void DEPRECATED_UsePopupMessages(int onOrOff);

	// Initial text was more trouble than what it was worth, let the events handle it
	void DEPRECATED_ClientInitialize_Basic(const TCHAR * Hostname, int Port, const TCHAR * Protocol, const TCHAR * InitialText);
	void DEPRECATED_ClientInitialize_Advanced(const TCHAR * Hostname, int Port, const TCHAR * Protocol, const TCHAR * AddressFamily, const TCHAR * SocketType, const TCHAR * InitialText);
	// Text encoding was confusing, so gonna make the user pass it explicitly
	void DEPRECATED_ClientSend(int socketID, const TCHAR * packet);
	// Client identification was meant to be IP or a comma-separated list
	void DEPRECATED_ServerSend(int socketID, const TCHAR * packet, const TCHAR * clientID);
	// Just an absolutely awful idea to detach a thread from control
	void REMOVED_ClientGoIndependent(int socketID);
	void REMOVED_ServerGoIndependent(int socketID);
	// Also an awful idea to make a thread do its own thing - the servers/clients had no functionality of their own!
	void REMOVED_ClientFusionReport(int socketID, int onOrOff);
	void REMOVED_ServerFusionReport(int socketID, int onOrOff);
	// Long makes no sense; it's stored either as signed int32, for which we have SetInteger action,
	// or it's stored as int64, in which case "long" is ambiguous
	void DEPRECATED_PacketBeingBuilt_SetLong(int longParam, int whereTo, int runHTONL);
	// Text encoding should not depend on the runtime's Unicode compatibility and user awareness!
	void DEPRECATED_PacketBeingBuilt_SetString(const TCHAR* text, int whereTo, int SizeOfString);
	void DEPRECATED_PacketBeingBuilt_SetWString(const TCHAR* text, int whereTo, int SizeOfStringInChar);
	// Removed these; it quickly spiralled DarkSocket into file and binary manipulation, which is feature creep
	void DEPRECATED_ClientLinkFileOutput(int socketID, const TCHAR* file);
	void DEPRECATED_ServerLinkFileOutput(int socketID, const TCHAR* file);
	void DEPRECATED_ClientUnlinkFileOutput(int socketID);
	void DEPRECATED_ServerUnlinkFileOutput(int socketID);

	// Text encoding was ignored or assumed to be equal to runtime, which is useless
	// as most protocols use UTF-8 and neither of Fusion's runtime uses that natively
	const TCHAR * DEPRECATED_GetLastMessageText();
	const TCHAR * DEPRECATED_PendingData_GetString(int readFromIndex, int sizeOfString);
	const TCHAR * DEPRECATED_PendingData_GetWString(int readFromIndex, int sizeOfString);
	// Memory addresses can be 64-bit, which can't be represented by a 32-bit integer address.
	void DEPRECATED_PacketBeingBuilt_SetBuffer(int memoryAddress, int indexToWriteTo, int numBytes);
	// Long is ambiguous
	int DEPRECATED_PendingData_GetLong(int readFromIndex);
	// Will only work with 32-bit systems, which makes cross-platform sockets impossible.
	unsigned int DEPRECATED_PacketBeingBuilt_GetAddress();
	unsigned int DEPRECATED_GetLastMessageAddress();


	void ClientInitialize_Basic(const TCHAR * hostname, int port, const TCHAR * protocol);
	void ClientInitialize_Advanced(const TCHAR * hostname, int port, const TCHAR * protocol, const TCHAR * addressFamily, const TCHAR * socketType);
	void ClientSend(int socketID, const TCHAR * packet, const TCHAR * encoding, int flags);
	void ClientCloseSocket(int socketID);
	void ClientReceiveOnly(int socketID);

	void ServerInitialize_Basic(const TCHAR * protocol, int port);
	void ServerInitialize_Advanced(const TCHAR * protocol, const TCHAR * addressFamily, const TCHAR * socketType, int port, const TCHAR * inAddr);
	void ServerShutdown(int socketID);
	void ServerShutdownPeerSocket(int socketID, int peerSocketID, int immediate);
	void ServerSend(int socketID, int peerSocketID, const TCHAR * message, const TCHAR * encoding, int flags);
	void ServerAutoAccept(int socketID, int onOrOff);

	void PacketBeingBuilt_NewPacket(int size);
	void PacketBeingBuilt_ResizePacket(int size);
	void PacketBeingBuilt_SetByte(int Byte, int whereTo);
	void PacketBeingBuilt_SetShort(int Short, int whereTo, int RunHTON);
	void PacketBeingBuilt_SetInteger(int Integer, int whereTo, int RunHTOL);
	void PacketBeingBuilt_SetFloat(float Float, int whereTo);
	void PacketBeingBuilt_SetDouble(float Double, int whereTo);
	void PacketBeingBuilt_SetBuffer(const TCHAR * readFromAddress, int indexToWriteTo, int numBytes);
	void PacketBeingBuilt_SetInt64(const TCHAR * int64AsText, int indexToWriteTo);
	void PacketBeingBuilt_SetString(const TCHAR * text, const TCHAR * encoding, int whereTo, int includeNull);

	// Some protocols glue multiple messages in one packet, but will cut off messages to do so.
	// In order to ensure we have full messages, the data is sent from receiving thread into an EventToRun.
	// We then append it to the pendingData for that socket. To show the user has used the data, this action is run.
	// The first parameter is in case the messages are processed in a non-FIFO arrangement.
	void PendingData_DiscardBytes(int fromPosition, int numBytes);


	/// Conditions

	bool AlwaysTrue() const;
	bool SocketIDCondition(int socketIDOrMinusOne) const;
	bool ServerPeerDisconnected(int socketID);

	/// Expressions

	const TCHAR * DEPRECATED_GetErrors(int clear);
	const TCHAR * DEPRECATED_GetReports(int clear);
	const TCHAR* GetErrorOrInfo();

	int GetCurrent_SocketID();
	int GetCurrent_PeerSocketID();
	const TCHAR * PendingData_GetAddress();
	unsigned int GetLastReceivedData_Size();

	int GetNewSocketID();
	int DEPRECATED_GetSocketIDForLastEvent();
	int GetPortFromType(const TCHAR * type);

	const TCHAR * PacketBeingBuilt_GetAddress();
	unsigned int PacketBeingBuilt_GetSize();
	int PacketBeingBuilt_ICMPChecksum(int readFromIndex, int SizeOfBank);
	int PendingData_GetByte(int readFromIndex);
	int PendingData_GetUnsignedByte(int readFromIndex);
	int PendingData_GetShort(int readFromIndex);
	int PendingData_GetUnsignedShort(int readFromIndex);
	int PendingData_GetInteger(int readFromIndex);
	const TCHAR * PendingData_GetInt64(int readFromIndex);
	const TCHAR* PendingData_GetUnsignedInt64(int readFromIndex);
	float PendingData_GetFloat(int readFromIndex);
	float PendingData_GetDouble(int readFromIndex); // Reads as double, casts to float
	const TCHAR * PendingData_GetString(const TCHAR * encoding, int readFromIndex, int sizeOfStringOrMinusOne);
	int PendingData_FindIndexOfChar(int charToFind, int numBytesInChar, int searchStartIndex, int textEndChars);
	int PendingData_ReverseFindIndexOfChar(int charToFind, int numBytesInChar, int searchStartIndex, int textEndChars);

	int GetLastReceivedData_Offset();
	const TCHAR * GetCurrent_RemoteAddress();
	int Statistics_BytesIn(int socketID, int peerSocketID);
	int Statistics_BytesOut(int socketID, int peerSocketID);
	int Statistics_PacketsIn(int socketID, int peerSocketID);
	int Statistics_PacketsOut(int socketID, int peerSocketID);

	// Runs every tick of Fusion's runtime, can be toggled off and back on
	REFLAG Handle();

	// These are called if there's no function linked to an ID
	void UnlinkedAction(int ID);
	long UnlinkedCondition(int ID);
	long UnlinkedExpression(int ID);
};
