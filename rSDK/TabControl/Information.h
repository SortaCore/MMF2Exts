// --------------------
// Object information
// --------------------
#include <tchar.h>
// Basic object details
#define	ObjectName			_T("Tab Control")
#define	ObjectAuthor		_T("AquadaSoft/Looki")
#define	ObjectCopyright		_T("Copyright © 2008-2010 AquadaSoft")
#define	ObjectComment		_T("Add tabbing to your applications, e.g. for file managing. Icon made by Ganryu.")
#define	ObjectURL			_T("http://www.aquadasoft.com")
#define	ObjectHelp			_T("")

// If you register your object with Clickteam, change this to the ID you were given
#define ObjectRegID			REGID_PRIVATE

// Change N,O,N,E to 4 unique characters (MMF currently still uses this to keep track)
#define	IDENTIFIER			MAKEID(T,A,B,C)

// --------------------
// Version information
// --------------------

// PRODUCT_VERSION_TGF or PRODUCT_VERSION_DEVELOPER
#define ForVersion			PRODUCT_VERSION_STANDARD

// Set this to the latest MMF build out when you build the object
#define	MinimumBuild		251

// --------------------
// Beta information
// --------------------

// #define BETA
// #define POPUP_ON_DROP
// #define POPUP_ON_EXE
// #define POPUP_ON_BUILD
// #define POPUP_MESSAGE	"This is a beta extension- use with caution!"

// --------------------
// Handling priority
// --------------------
// If this extension will handle windows messages, specify the priority
// of the handling procedure, 0 = low and 255 = very high

#define	WINDOWPROC_PRIORITY 100
