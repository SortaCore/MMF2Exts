//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// 																														    //
// 																														    //
//		KLIK & CREATE : external object																						//
// 																														    //
// 																														    //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////





// ------------------------------
// DEFINITION OF CONDITIONS CODES
// ------------------------------
#define	CND_CMPOSINTRACK			0
#define	CND_CMPOSITION				1
#define	CND_LAST					2





// ---------------------------
// DEFINITION OF ACTIONS CODES
// ---------------------------
#define	ACT_PLAY					0
#define	ACT_STOP                    1
#define	ACT_PAUSE                   2
#define	ACT_RESUME                  3
#define	ACT_PREVIOUS                4
#define	ACT_NEXT                    5
#define	ACT_TIMEPLAY                6
#define	ACT_OPENDOOR                7
#define	ACT_CLOSEDOOR               8
#define	ACT_PLAYTRACK				9
#define	ACT_LAST					10




	
// -------------------------------
// DEFINITION OF EXPRESSIONS CODES
// -------------------------------
#define	EXP_TRACK    				0
#define	EXP_MAXTRACK                1
#define	EXP_TRACKPOS                2
#define	EXP_TRACKLEN                3
#define	EXP_POS                     4
#define	EXP_LEN                     5
#define	EXP_LASTERROR               6
#define	EXP_LAST                    7



// --------------------------------
// EDITION OF OBJECT DATA STRUCTURE
// --------------------------------

#pragma pack(push,_PACK_CNCS_EXT)  
#pragma pack(2)

typedef struct tagEDATA
{
	extHeader	eHeader;

	short		ecdaIcon;			// Object icon under level editor
	short		ecdaFlags;
	short		ecdaTrackOnStart;
	short		ecdaSecu;

} editData;

typedef editData	_far *			fpedata;

#pragma pack(pop,_PACK_CNCS_EXT)

// Object flags
#define	CD_OPENONSTART			0x0001
#define	CD_PLAYONSTART			0x0002
#define	CD_STOPONCLOSE			0x0004
#define CD_PLAYWHENMINIMIZED	0x0008


// Object versions
#define	KCX_VERSION_0				0
#define	KCX_CURRENT_VERSION			0





// -----------------------------
// RUNNING OBJECT DATA STRUCTURE
// -----------------------------
typedef struct tagRDATA {

	headerObject 	rHo;		  		
	
	short			rcdDevice;
	short			rcdFlags;
	short			rcdStatus;
	short			rcdStopAtTrackEnd;
	long			rcdLastPosition;
	short			rcdError;
	
	} runData;

typedef	runData		_far *		fprdata;



// Size when editing the object under level editor
#define	MAX_EDITSIZE	sizeof(editData)



// Default flags
#define	OEFLAGS      			(OEFLAG_DISPLAYINFRONT)
#define	OEPREFS      			0

