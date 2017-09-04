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
		unsigned char		NumEvents;
		unsigned short		CondTrig[2];

		// Lacewing code
		union {
			// When you receive a message
			struct {
			char *				Content;
			size_t				Size,
								Cursor;
			unsigned char		Subchannel;
			bool				Blasted;
			unsigned char		Variant;
			} ReceivedMsg;

			// When an error occurs
			struct {
				char *		Text;
			} Error;

			// When a selection/loop is called
			struct  {
				char *		Name;
			} Loop;

			// On name set request
			struct {
				char *		Name;
			} Requested;
		};
		ChannelCopy * Channel;
		union {
			ClientCopy * Client;
			ClientCopy * SenderClient; // By default the same as Client
		};
		ClientCopy * ReceivingClient;
		InteractiveType InteractiveType;
		bool ChannelCreate_Hidden;
		bool ChannelCreate_AutoClose;

		SaveExtInfo() : NumEvents(0), CondTrig { 0 },
			Channel(NULL), Client(NULL), ReceivingClient(NULL), 
			InteractiveType(InteractiveType::None),
			ChannelCreate_Hidden(false),
			ChannelCreate_AutoClose(false)
		{
			ReceivedMsg.Content = NULL;
			ReceivedMsg.Cursor = 0;
			ReceivedMsg.Size = 0;
			ReceivedMsg.Subchannel = 0;
			ReceivedMsg.Blasted = false;
			ReceivedMsg.Variant = 255;
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
			if (ReceivedMsg.Content)
			{
				free(ReceivedMsg.Content);
				ReceivedMsg.Content = NULL;
				ReceivedMsg.Cursor = 0;
				ReceivedMsg.Size = 0;
				ReceivedMsg.Subchannel = 0;
				ReceivedMsg.Blasted = false;
				ReceivedMsg.Variant = 255;
			}
			NumEvents = 0;
			CondTrig[0] = 0; CondTrig[1] = 0;
			Channel = NULL;
			Client = NULL;
			ReceivingClient = NULL;
			InteractiveType = InteractiveType::None;
		}
	};
#pragma pack (pop, align_to_one_multithreading)
#endif // MULTI_THREADING