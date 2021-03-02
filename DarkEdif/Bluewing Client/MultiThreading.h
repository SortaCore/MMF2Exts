// DarkEdif extension: allows safe multithreading returns.
#include "Common.h"

#if defined(MULTI_THREADING) && !defined(ALREADY_DEFINED_MT)
#define ALREADY_DEFINED_MT
	/* Make sure any pointers in ExtVariables are free'd in ~EventToRun(). */
	#pragma pack (push, align_to_one_multithreading)
	#pragma pack (1)
	struct EventToRun
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
		};
		std::shared_ptr<lacewing::relayclient::channel> channel;
		std::shared_ptr<lacewing::relayclient::channellisting> channelListing;
		std::shared_ptr<lacewing::relayclient::channel::peer> peer;

		EventToRun() : numEvents(0), condTrig{ 35353, 35353 }
		{
			new(&receivedMsg.content)std::string();
			receivedMsg.cursor = 0;
			receivedMsg.subchannel = 0;
			receivedMsg.variant = 0;
		}
		~EventToRun()
		{
			receivedMsg.content.~basic_string();
			peer = nullptr;
			channel = nullptr;
		}
	};
#pragma pack (pop, align_to_one_multithreading)
#endif // MULTI_THREADING
