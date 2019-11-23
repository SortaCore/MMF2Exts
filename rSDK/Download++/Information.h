// Wraps str into double-quotes, like "str". This function cannot be defined via /D.
#define sub_asStr(str) #str
#define asStr(str) sub_asStr(str)

// --------------------
// Object information
// --------------------

// Basic object details
#define	ObjectName			"Download++ object"
#define	ObjectAuthor		"Clickteam & Darkwire Software"
#define	ObjectCopyright		"Copyright © " asStr(YEAR) " Clickteam & Darkwire Software"
#define	ObjectComment		"Allows you to download files from the Internet with HTTP or HTTPS."
#define	ObjectURL			"https://dark-wire.com/"
#define	ObjectHelp			""

// If you register your object with Clickteam, change this to the ID you were given
#define ObjectRegID			REGID_PRIVATE

// Change N,O,N,E to 4 unique characters (MMF currently still uses this to keep track)
#define	IDENTIFIER			MAKEID(D,L,+,+)

// --------------------
// Version information
// --------------------

// PRODUCT_VERSION_TGF or PRODUCT_VERSION_DEVELOPER
#define ForVersion			PRODUCT_VERSION_STANDARD

// Set this to the latest MMF build out when you build the object
#define	MinimumBuild		250

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
