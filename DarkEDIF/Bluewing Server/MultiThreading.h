// DarkEDIF extension: allows safe multithreading returns.
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

struct ChannelCopy;
struct ClientCopy;

#ifdef MULTI_THREADING
	/* Make sure any pointers in ExtVariables are free'd in ~SaveExtInfo(). */
	#pragma pack( push, align_to_one_multithreading)
	#pragma pack(1) 
	struct SaveExtInfo
	{
		// Required for DarkEDIF
		unsigned char		numEvents;
		unsigned short		CondTrig[2];

		// Lacewing code
		union {
			// When you receive a message
			struct {
			char *				content;
			size_t				size,
								cursor;
			unsigned char		subchannel;
			bool				blasted;
			unsigned char		variant;
			} receivedMsg;

			// When an error occurs
			struct {
				char *		text;
			} error;

			// When a selection/loop is called
			struct  {
				char *		name;
			} loop;

			// On name set request
			struct {
				char *		name;
			} Requested;
		};
		ChannelCopy * channel;
		union {
			ClientCopy * client;
			ClientCopy * senderClient; // By default the same as Client
		};
		ClientCopy * ReceivingClient;
		InteractiveType InteractiveType;
		bool channelCreate_Hidden;
		bool channelCreate_AutoClose;

		SaveExtInfo() : numEvents(0), CondTrig { 0 },
			channel(NULL), client(NULL), ReceivingClient(NULL), 
			InteractiveType(InteractiveType::None),
			channelCreate_Hidden(false),
			channelCreate_AutoClose(false)
		{
			receivedMsg.content = NULL;
			receivedMsg.cursor = 0;
			receivedMsg.size = 0;
			receivedMsg.subchannel = 0;
			receivedMsg.blasted = false;
			receivedMsg.variant = 255;
		}
		~SaveExtInfo()
		{
			Free();
		}

		/// <summary> Frees this object. </summary>
		void Free()
		{
			// A char * is always the first member of each struct inside the union.
			// So they will be at the same memory address. Neat, huh?
			if (receivedMsg.content)
			{
				free(receivedMsg.content);
				receivedMsg.content = NULL;
				receivedMsg.cursor = 0;
				receivedMsg.size = 0;
				receivedMsg.subchannel = 0;
				receivedMsg.blasted = false;
				receivedMsg.variant = 255;
			}
			numEvents = 0;
			CondTrig[0] = 0; CondTrig[1] = 0;
			channel = NULL;
			client = NULL;
			ReceivingClient = NULL;
			InteractiveType = InteractiveType::None;
		}
	};
#pragma pack (pop, align_to_one_multithreading)
#endif // MULTI_THREADING