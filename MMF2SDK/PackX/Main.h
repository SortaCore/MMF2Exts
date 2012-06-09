// Object identifier "SAM2"

#define IDENTIFIER	0xA7832BA9

//Routine count definitions

#define	CND_LAST	1
#define	ACT_LAST	1
#define	EXP_LAST        4


// --------------------------------
// EDITION OF OBJECT DATA STRUCTURE
// --------------------------------
typedef struct tagEDATA_V1
{
	extHeader		eHeader;
	short			sx;
	short			sy;
	short			swidth;
	short			sheight;
	char			key[100];

} EDITDATA;
typedef EDITDATA _far *			LPEDATA;

// Object versions
#define	KCX_CURRENT_VERSION			1

// --------------------------------
// RUNNING OBJECT DATA STRUCTURE
// --------------------------------
typedef struct tagRDATA
{
	headerObject	rHo;					// Header

	char			key[100];


} RUNDATA;
typedef	RUNDATA	_far *			LPRDATA;

// Size when editing the object under level editor
// -----------------------------------------------
#define	MAX_EDITSIZE			sizeof(EDITDATA)

// Default flags
// -------------
#define	OEFLAGS      			0
#define	OEPREFS      			0

// If to handle message, specify the priority of the handling procedure
// 0= low, 255= very high. You should use 100 as normal.                                                
// --------------------------------------------------------------------
#define	WINDOWPROC_PRIORITY		100

 