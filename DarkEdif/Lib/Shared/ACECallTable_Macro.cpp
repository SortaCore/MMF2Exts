// This file is used by the AltPlatformPreBuildTool as an input template for the calltable generator.
// Do not modify or delete this file.
#if DARKEDIF_ACE_CALL_TABLE_INDEX==0
			#error ASM Condition Replacement Macro was not replaced!
#elif DARKEDIF_ACE_CALL_TABLE_INDEX==1
			#error ASM Action Replacement Macro was not replaced!
#elif DARKEDIF_ACE_CALL_TABLE_INDEX==2
		#error ASM Expression Replacement Macro was not replaced!
#else
	#error Incorrect DarkEdif ACE Call Table index
#endif
