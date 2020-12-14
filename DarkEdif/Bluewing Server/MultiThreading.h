// DarkEdif extension: allows safe multithreading returns.
#include "Common.h"

enum InteractiveType : unsigned char
{
	None = 0,
	ConnectRequest = 1,
	ClientNameSet = 2,
	ChannelJoin = 3,
	ChannelLeave = 4,
	ChannelMessageIntercept = 5,
	ClientMessageIntercept = 6
};

#ifdef MULTI_THREADING
	/* Make sure any pointers in ExtVariables are free'd in ~SaveExtInfo(). */
	#pragma pack( push, align_to_one_multithreading)
	#pragma pack(1)
	struct SaveExtInfo
	{
		// Required for DarkEdif
		std::uint8_t		numEvents;
		std::uint16_t		CondTrig[2];

		// Lacewing code
		union {
			// When you receive a message
			struct {
				std::string		content;
				std::uint32_t	cursor;
				lw_ui8			subchannel;
				bool			blasted;
				lw_ui8			variant;
			} receivedMsg;

			// When an error occurs
			struct {
				std::string text;
			} error;

			// When a selection/loop is called
			struct  {
				std::string name;
			} loop;

			// On name set request
			struct {
				std::string name;
			} requested;
		};
		std::shared_ptr<lacewing::relayserver::channel> channel;
		std::shared_ptr<lacewing::relayserver::client> senderClient; // By default the same as Client
		std::shared_ptr<lacewing::relayserver::client> receivingClient;
		InteractiveType InteractiveType;
		bool channelCreate_Hidden;
		bool channelCreate_AutoClose;

		SaveExtInfo() : numEvents(0), CondTrig { 0, 0 },
			InteractiveType(InteractiveType::None),
			channelCreate_Hidden(false),
			channelCreate_AutoClose(false)
		{
			new (&receivedMsg.content)std::string();
			receivedMsg.cursor = 0;
			receivedMsg.subchannel = 0;
			receivedMsg.blasted = false;
			receivedMsg.variant = 255;
		}
		~SaveExtInfo()
		{
			receivedMsg.content.~basic_string();
			receivedMsg.cursor = 0;
			receivedMsg.subchannel = 0;
			receivedMsg.blasted = false;
			receivedMsg.variant = 255;

			numEvents = 0;
			CondTrig[0] = 0; CondTrig[1] = 0;
			channel.reset();
			senderClient.reset();
			receivingClient.reset();
			InteractiveType = InteractiveType::None;
		}
	};
#pragma pack (pop, align_to_one_multithreading)
#endif // MULTI_THREADING
