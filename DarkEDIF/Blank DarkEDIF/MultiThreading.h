// DarkEDIF extension: allows safe multithreading returns.
#include "Common.h"

#ifdef MULTI_THREADING
	/*
		These three #pragmas specify alignment for the structs defined between them.
		Without this, the SaveExtInfo structure will be larger than the variables contained.
		For example, on the default 4-byte alignment, if you have 5 bytes' worth of variables,
		you will use 8 bytes. This is a waste of memory and might cause issues with memory copying.
		By setting it to 1 until the "pop" command, we tell the compiler not to round up in that manner.
	*/
	#pragma pack (push, align_to_one_multithreading)
	#pragma pack (1) 
	struct SaveExtInfo
	{
		// Required for DarkEDIF
		unsigned char		NumEvents;
		unsigned short *	CondTrig;

		// Your variables?

		SaveExtInfo()
		{
			// Initialise your variables here or use the initialiser list.
		}
		~SaveExtInfo()
		{
			// Free your pointers here, if you have allocated memory with malloc() or new or whatever.
			// Otherwise you will cause a memory leak.

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