// DarkEDIF extension: allows safe multithreading returns.
#include "Common.h"

#if defined(MULTI_THREADING) && !defined(ALREADY_DEFINED_MT)
#define ALREADY_DEFINED_MT
	/* Make sure any pointers in ExtVariables are free'd in ~SaveExtInfo(). */
	#pragma pack( push, align_to_one_multithreading)
	#pragma pack(1) 
	struct SaveExtInfo
	{
		// Required for DarkEDIF
		unsigned char		NumEvents;
		unsigned short	CondTrig[2];

		// Lacewing code
		union {
			// When you receive a message
			struct {
			char *				Content;
			size_t				Size,
								Cursor;
			unsigned char		Subchannel;
			} ReceivedMsg;

			// When an error occurs
			struct {
				const char *		Text;
			} Error;

			// When a selection/loop is called
			struct  {
				const char *		Name;
			} Loop;
		};
		union {
			ChannelCopy * Channel;
			const lacewing::relayclient::channellisting * ChannelListing;
		};
		PeerCopy * Peer;

		SaveExtInfo() : NumEvents(0), CondTrig{ 0 }, Channel(nullptr), Peer(nullptr)
		{
			ReceivedMsg.Content = nullptr;
			ReceivedMsg.Cursor = 0;
			ReceivedMsg.Size = 0;
			ReceivedMsg.Subchannel = 0;
		}
		~SaveExtInfo()
		{
			// A char * is always the first member of each struct inside the union.
			// So they will be at the same memory address. Neat, huh?
			if (ReceivedMsg.Content)
			{
				free(ReceivedMsg.Content);
				ReceivedMsg.Content = nullptr;
			}
			
			Peer = nullptr;
			Channel = nullptr;
		}
	};
#pragma pack (pop, align_to_one_multithreading)
#endif // MULTI_THREADING