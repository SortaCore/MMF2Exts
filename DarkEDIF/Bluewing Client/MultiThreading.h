// DarkEDIF extension: allows safe multithreading returns.
#include "Common.h"

#if defined(MULTI_THREADING) && !defined(ALREADY_DEFINED_MT)
#define ALREADY_DEFINED_MT
	/* Make sure any pointers in ExtVariables are free'd in ~SaveExtInfo(). */
	#pragma pack (push, align_to_one_multithreading)
	#pragma pack (1) 
	struct SaveExtInfo
	{
		// Required for DarkEDIF
		unsigned char	numEvents;
		unsigned short	condTrig[2];

		// Lacewing code
		union {
			// When you receive a message
			struct {
			char *				content;
			size_t				size,
								cursor;
			unsigned char		subchannel;
			} receivedMsg;

			// When an error occurs
			struct {
				const char *	text;
			} error;

			// When a selection/loop is called
			struct  {
				const char *	name;
			} loop;
		};
		union {
			ChannelCopy * channel;
			const lacewing::relayclient::channellisting * channelListing;
		};
		PeerCopy * peer;

		SaveExtInfo() : numEvents(0), condTrig{ 0, 0 }, channel(nullptr), peer(nullptr)
		{
			receivedMsg.content = nullptr;
			receivedMsg.cursor = 0;
			receivedMsg.size = 0;
			receivedMsg.subchannel = 0;
		}
		~SaveExtInfo()
		{
			// A char * is always the first member of each struct inside the union.
			// So they will be at the same memory address. Neat, huh?
			if (receivedMsg.content)
			{
				free(receivedMsg.content);
				receivedMsg.content = nullptr;
			}
			
			peer = nullptr;
			channel = nullptr;
		}
	};
#pragma pack (pop, align_to_one_multithreading)
#endif // MULTI_THREADING