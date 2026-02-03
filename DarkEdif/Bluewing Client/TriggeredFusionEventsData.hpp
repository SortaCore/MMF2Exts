#pragma once
#include "Common.hpp"
#include "Lacewing.h"

// When you receive a message
struct RecvMsg final {
			  std::string  content;
				   lw_ui8  subchannel;
				   lw_ui8  variant;
	mutable std::uint32_t  cursor = 0;
};

enum class EventType
{
	Unset = -1,
	NoContext,
	Error,
	Disconnect,
	// Channel or client name denied
	NameDenied,
	// Join or leave success
	ChannelJoinOrLeaveSuccess,
	ChannelLeaveDenied,
	// Msg events must be grouped, they're referred to as a set
	ServerMsg,
	ServerChannelMsg,
	ChannelMsg,
	PeerMsg,
	PeerJoinOrLeave,
	PeerNameChanged,
	ChannelListing,
	NetScanReply
};

/* Make sure any pointers in ExtVariables are free'd in ~EventToRun(). */
struct EventToRun
{
	EventType type = EventType::Unset;
	std::uint8_t	numEvents = 0;
	std::uint16_t	condTrig[2] = { 35353, 35353 }; // dummy values
	virtual void PreRun(Extension* const ext) {}
	virtual void PostRun(Extension* const ext) {}
	EventToRun() = default;
	EventToRun(std::uint16_t evt1) {
		numEvents = 1;
		condTrig[0] = evt1;
	}
	EventToRun(std::uint16_t evt1, std::uint16_t evt2) {
		numEvents = 2;
		condTrig[0] = evt1;
		condTrig[0] = evt2;
	}

	inline RecvMsg& GetRecvMsg();

	// An arguably nicer dynamic_cast
	template<typename T> bool Is() const {
		assert(type != EventType::Unset);
		return T::typeCode == this->type;
	}
	bool IsRecvMsg() const {
		assert(type != EventType::Unset);
		return type >= EventType::ServerMsg && type <= EventType::PeerMsg;
	}
	template<typename T> const T& AsC() const {
		assert(type != EventType::Unset);
		assert(T::typeCode == this->type);
		return *reinterpret_cast<const T*>(this);
	}
	template<typename T> T* As() {
		assert(type != EventType::Unset);
		assert(T::typeCode == this->type);
		return reinterpret_cast<T*>(this);
	}
protected:
	virtual ~EventToRun() = default;
};
// Used for events that have no specific context
struct NoContextEvent : EventToRun {
	static constexpr EventType typeCode = EventType::NoContext;
	NoContextEvent() = default;
	NoContextEvent(NoContextEvent&) = delete;
	NoContextEvent(const NoContextEvent&) = delete;
	NoContextEvent(NoContextEvent&&) = delete;
	NoContextEvent(const NoContextEvent&&) = delete;
	~NoContextEvent() override = default;
};
struct DisconnectEvent : EventToRun {
	static constexpr EventType typeCode = EventType::Disconnect;
	DisconnectEvent() = default;
	DisconnectEvent(DisconnectEvent&) = delete;
	DisconnectEvent(const DisconnectEvent&) = delete;
	DisconnectEvent(DisconnectEvent&&) = delete;
	DisconnectEvent(const DisconnectEvent&&) = delete;
	void PostRun(Extension* const ext) final;
	~DisconnectEvent() override = default;
};
struct ServerMsgEvent : EventToRun {
	static constexpr EventType typeCode = EventType::ServerMsg;
	RecvMsg msg;
	inline ServerMsgEvent(std::string_view msg, lw_ui8 subchannel, lw_ui8 variant)
		: msg{ std::string(msg), subchannel, variant } { }
	NO_DEFAULT_CTORS(ServerMsgEvent);
	~ServerMsgEvent() override = default;
};
struct ServerChannelMsgEvent : EventToRun {
	static constexpr EventType typeCode = EventType::ServerChannelMsg;
	RecvMsg msg;
	std::shared_ptr<lacewing::relayclient::channel> channel;
	inline ServerChannelMsgEvent(std::string_view msg, lw_ui8 subchannel, lw_ui8 variant,
		std::shared_ptr<lacewing::relayclient::channel> c)
		: msg { std::string(msg), subchannel, variant }, channel(c) { }
	void PreRun(Extension* const ext) final;

	NO_DEFAULT_CTORS(ServerChannelMsgEvent);
	~ServerChannelMsgEvent() override = default;
};
struct ChannelMsgEvent : EventToRun {
	static constexpr EventType typeCode = EventType::ServerMsg;
	RecvMsg msg;
	std::shared_ptr<lacewing::relayclient::channel> channel;
	std::shared_ptr<lacewing::relayclient::channel::peer> peer;
	inline ChannelMsgEvent(std::string_view msg, lw_ui8 subchannel, lw_ui8 variant,
		std::shared_ptr<lacewing::relayclient::channel> c, std::shared_ptr<lacewing::relayclient::channel::peer> pr)
		: msg { std::string(msg), subchannel, variant }, channel(c), peer(pr) { }
	NO_DEFAULT_CTORS(ChannelMsgEvent);
	void PreRun(Extension* const ext) final;
	~ChannelMsgEvent() override = default;
};
struct ChannelJoinOrLeaveSuccessEvent : EventToRun {
	static constexpr EventType typeCode = EventType::ChannelJoinOrLeaveSuccess;
	std::shared_ptr<lacewing::relayclient::channel> channel;
	inline ChannelJoinOrLeaveSuccessEvent(std::shared_ptr<lacewing::relayclient::channel> c)
		: channel(c) { }
	NO_DEFAULT_CTORS(ChannelJoinOrLeaveSuccessEvent);
	void PreRun(Extension* const ext) final;
	void PostRun(Extension* const ext) final;
	~ChannelJoinOrLeaveSuccessEvent() override = default;
};
struct PeerMsgEvent : EventToRun {
	static constexpr EventType typeCode = EventType::PeerMsg;
	RecvMsg msg;
	std::shared_ptr<lacewing::relayclient::channel> channel;
	std::shared_ptr<lacewing::relayclient::channel::peer> peer;
	inline PeerMsgEvent(std::string_view msg, lw_ui8 subchannel, lw_ui8 variant,
		std::shared_ptr<lacewing::relayclient::channel> c, std::shared_ptr<lacewing::relayclient::channel::peer> pr)
		: msg { std::string(msg), subchannel, variant }, channel(c), peer(pr) { }
	NO_DEFAULT_CTORS(PeerMsgEvent);
	void PreRun(Extension* const ext) final;
	~PeerMsgEvent() override = default;
};
struct PeerJoinOrLeaveEvent : EventToRun {
	static constexpr EventType typeCode = EventType::PeerJoinOrLeave;
	std::shared_ptr<lacewing::relayclient::channel> channel;
	std::shared_ptr<lacewing::relayclient::channel::peer> peer;
	inline PeerJoinOrLeaveEvent(std::shared_ptr<lacewing::relayclient::channel> c, std::shared_ptr<lacewing::relayclient::channel::peer> pr)
		: channel(c), peer(pr) { }
	NO_DEFAULT_CTORS(PeerJoinOrLeaveEvent);
	void PreRun(Extension* const ext) final;
	void PostRun(Extension* const ext) final;
	~PeerJoinOrLeaveEvent() override = default;
};
struct PeerNameChangedEvent : EventToRun {
	static constexpr EventType typeCode = EventType::PeerNameChanged;
	std::shared_ptr<lacewing::relayclient::channel> channel;
	std::shared_ptr<lacewing::relayclient::channel::peer> peer;
	std::string oldName;
	inline PeerNameChangedEvent(std::shared_ptr<lacewing::relayclient::channel> c, std::shared_ptr<lacewing::relayclient::channel::peer> pr,
		std::string_view oldName)
		: channel(c), peer(pr), oldName(oldName) { }
	NO_DEFAULT_CTORS(PeerNameChangedEvent);
	void PreRun(Extension* const ext) final;
	~PeerNameChangedEvent() override = default;
};
struct ChannelLeaveDeniedEvent : EventToRun {
	static constexpr EventType typeCode = EventType::ChannelLeaveDenied;
	std::shared_ptr<lacewing::relayclient::channel> channel;
	std::string denyReason;
	inline ChannelLeaveDeniedEvent(std::shared_ptr<lacewing::relayclient::channel> c, std::string_view denyReason) :
		channel(c), denyReason(denyReason) {}
	NO_DEFAULT_CTORS(ChannelLeaveDeniedEvent);
	void PreRun(Extension* const ext) final;
	~ChannelLeaveDeniedEvent() override = default;
};

// When an error occurs
struct ErrorEvent : EventToRun {
	static constexpr EventType typeCode = EventType::Error;
	std::string	text;

	NO_DEFAULT_CTORS(ErrorEvent);
	ErrorEvent(std::string text) : text(text) {}
	~ErrorEvent() override = default;
};
// When channel join/name set is denied
struct NameDeniedEvent : EventToRun {
	static constexpr EventType typeCode = EventType::NameDenied;
	std::string	name, denyReason;
	NO_DEFAULT_CTORS(NameDeniedEvent);
	NameDeniedEvent(std::string_view name, std::string_view denyReason) : name(name), denyReason(denyReason) {}
	~NameDeniedEvent() override = default;
};

// When a network scan entry replies
struct NetScanReplyEvent : EventToRun {
	static constexpr EventType typeCode = EventType::NetScanReply;
	std::string	remoteIP;
	std::string serverVersion;
	std::string welcomeMessage;
	lw_ui8 minClientBuild, curClientBuild, serverBuild;
	std::chrono::steady_clock::duration responseTime;
	NetScanReplyEvent(std::string remoteIP, std::string serverVersion, std::string welcomeMessage,
		lw_ui8 minClientBuild, lw_ui8 curClientBuild, lw_ui8 serverBuild, std::chrono::steady_clock::duration responseTime)
		: remoteIP(remoteIP), serverVersion(serverVersion), welcomeMessage(welcomeMessage),
		minClientBuild(minClientBuild), curClientBuild(curClientBuild), serverBuild(serverBuild), responseTime(responseTime){ }
	NO_DEFAULT_CTORS(NetScanReplyEvent);
	~NetScanReplyEvent() override = default;
};

inline RecvMsg& EventToRun::GetRecvMsg() {
	assert(IsRecvMsg());
	// same offset
	return reinterpret_cast<ServerMsgEvent*>(this)->msg;
}
