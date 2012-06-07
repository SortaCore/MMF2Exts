// The code in this file is executed in CreateRunObject

// Make rRd
rdPtr->rRd=new rRundata(rdPtr);

// Runtime popup if needed
#ifdef BETA
	#ifdef POPUP_ON_EXE
		MessageBox(0,POPUP_MESSAGE,ObjectName,MB_OK|MB_ICONINFORMATION);
	#endif
#endif