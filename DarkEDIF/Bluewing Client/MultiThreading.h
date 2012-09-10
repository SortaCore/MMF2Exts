// DarkEDIF extension: allows safe multithreading returns.
#include "Common.h"

#ifdef MULTI_THREADING
	/* Make sure any pointers in ExtVariables are free'd in ~SaveExtInfo(). */
	#pragma pack( push, align_to_one_multithreading)
	#pragma pack(1) 
	struct SaveExtInfo
	{
		// Required for DarkEDIF
		unsigned char		NumEvents;
		unsigned short *	CondTrig;

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
				char *		Text;
			} Error;

			// When a selection/loop is called
			struct  {
				char *		Name;
			} Loop;
		};
		union {
			Lacewing::RelayClient::Channel * Channel;
			Lacewing::RelayClient::ChannelListing * ChannelListing;
		};
		Lacewing::RelayClient::Channel::Peer * Peer;

		SaveExtInfo() : NumEvents(0), CondTrig(NULL), Channel(NULL), Peer(NULL)
		{
			ReceivedMsg.Content = NULL;
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
				ReceivedMsg.Content = NULL;
			}
			
			// Required for DarkEDIF
			if (CondTrig)
			{
				free(CondTrig);
				CondTrig = NULL;
			}
		}
	};
#pragma pack (pop, align_to_one_multithreading)
#endif // MULTI_THREADING