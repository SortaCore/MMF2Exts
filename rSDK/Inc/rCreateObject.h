// The code in this file is executed in CreateObject

// Drop popup if needed
#ifdef BETA
	#ifdef POPUP_ON_DROP
		MessageBoxA(0,POPUP_MESSAGE,ObjectName,MB_OK|MB_ICONINFORMATION);
	#endif
#endif

// Warn about the identifier if it has not been changed
if (IDENTIFIER==MAKEID(N,O,N,E))
	MessageBoxA(0,"Do not forget to change the identifier!","SDK warning",0);