#pragma once
#include "Common.hpp"
#include "Lacewing.h"

/* Make sure any pointers in ExtVariables are free'd in ~EventToRun(). */
struct EventToRun final
{
	// Required for DarkEdif
	std::uint8_t	numEvents;
	std::uint16_t	condTrig[2];

	// Lacewing code
	union {
		// When you receive a message
		struct {
			std::string		content;
			std::uint32_t	cursor;
			lw_ui8			subchannel;
			lw_ui8			variant;
		} receivedMsg;

		// When an error occurs
		struct {
			std::string	text;
		} error;

		// When channel join/name set is denied
		struct {
			std::string	name;
		} denied;
	};
	std::shared_ptr<lacewing::relayclient::channel> channel;
	std::shared_ptr<lacewing::relayclient::channellisting> channelListing;
	std::shared_ptr<lacewing::relayclient::channel::peer> peer;

	EventToRun();
	~EventToRun();
};
