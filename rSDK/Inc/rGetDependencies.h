// The code in this file is executed in GetDependencies

// Build popup if needed
#ifdef BETA
	#ifdef POPUP_ON_BUILD
		MessageBox(0,POPUP_MESSAGE,ObjectName,MB_OK|MB_ICONINFORMATION);
	#endif
#endif