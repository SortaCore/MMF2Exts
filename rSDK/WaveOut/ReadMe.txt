Audio streaming extension example by Looki
Based on rSDK template

Modified files:

Information.h

Common.h 
	Stores NUM_BUFFERS constant

Main.h
	Stores streaming-related variables
	Modified structs: RUNDATA
	
Runtime.h 
	Includes complete audio streaming code
	Modified functions: CreateRunObject, DestroyRunObject
	Added functions: audioThreadProc 