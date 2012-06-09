
#ifndef _CNCF_H_
#define _CNCF_H_

#include "cncy.h"
#ifdef STAND_ALONE
#include "CValue.h"
#else

// Definitions for extensions
#define TYPE_LONG	0x0000
#define TYPE_INT	TYPE_LONG
#define TYPE_STRING	0x0001
#define TYPE_FLOAT	0x0002				// Pour les extensions
#define TYPE_DOUBLE 0x0002

#ifdef __cplusplus
class CValue {
public:
#else
typedef struct CValue {
#endif
	UINT m_type;
	UINT m_paddle;
	union
	{
		long m_long;
		double m_double;
		LPSTR m_pString;
	};
#ifdef __cplusplus
};
#else
} CValue;
#endif
#endif // STAND_ALONE

// Changes the structure alignment
#ifndef	_H2INC  
#pragma pack( push, _pack_cncf_ )
#pragma pack(2) 
#endif
typedef short *				LPSHORT;

#ifdef __cplusplus
class headerObject;
class kpj;
class objInfoList;
class drawRoutine;
class LO;
class pev;
class qualToOi;
class CRunApp;
class CRunFrame;
class CRunMvt;
class CMvt;
class CDemoRecord;
//class CValue;
#else
#define headerObject VOID
#define kpj VOID
#define objInfoList VOID
#define drawRoutine VOID
#define LO VOID
#define pev VOID
#define qualToOi VOID
//#define CValue VOID
#define CRunApp void
#define CRunFrame void
#define CRunMvt void
#define CMvt void
#define CDemoRecord void
#endif

// Number of values
#define VALUES_NUMBEROF_ALTERABLE	26
#define VALUES_NUMBEROF_MAX			26
#define STRINGS_NUMBEROF_ALTERABLE	10
#define STRINGS_NUMBEROF_MAX		10

typedef short ITEMTYPE;
typedef short OINUM;
typedef short HFII;

// LOGFONT 16 structure for compatibility with old extensions
/////////////////////////////////////////////////////////////
typedef struct tagLOGFONTV1 {
	short	lfHeight;
	short	lfWidth;
	short	lfEscapement;
	short	lfOrientation;
	short	lfWeight;
	BYTE	lfItalic;
	BYTE	lfUnderline;
	BYTE	lfStrikeOut;
	BYTE	lfCharSet;
	BYTE	lfOutPrecision;
	BYTE	lfClipPrecision;
	BYTE	lfQuality;
	BYTE	lfPitchAndFamily;
	BYTE	lfFaceName[LF_FACESIZE];
} LOGFONTV1;

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////

//
// ObjectsCommon - Dynamic items
//
#define	OC_MAX_QUALIFIERS	8

#ifdef __cplusplus
class OC {
public:
#else
typedef struct OC {
#endif

	DWORD		ocDWSize;				// Total size of the structures

	WORD		ocMovements;			// Offset of the movements
	WORD		ocAnimations; 			// Offset of the animations
	WORD		ocVersion;				// For version versions > MOULI 
	WORD		ocCounter;				// Pointer to COUNTER structure
	WORD		ocData;					// Pointer to DATA structure
	WORD		ocFree;
	DWORD		ocOEFlags;				// New flags

	WORD		ocQualifiers[OC_MAX_QUALIFIERS];	// Qualifier list
	WORD		ocExtension;						// Extension structure 
	WORD		ocValues;							// Values structure
	WORD		ocStrings;							// String structure

	WORD		ocFlags2;				// New news flags, before was ocEvents
	WORD		ocOEPrefs;				// Automatically modifiable flags
	DWORD		ocIdentifier;			// Identifier d'objet

	COLORREF	ocBackColor;			// Background color
	DWORD		ocFadeIn;				// Offset fade in 
	DWORD		ocFadeOut;				// Offset fade out 
	DWORD		ocValueNames;			// For the debugger
	DWORD		ocStringNames;

#ifdef __cplusplus
};
#else
} OC;
#endif
typedef OC * LPOC;
typedef OC * fpoc;

typedef struct
{
	WORD	number;
	long	values[1];
} OCValues;
typedef	OCValues*			LPOCVALUES;

typedef struct
{
	WORD	number;
	char	str[2];

} OCStrings;
typedef	OCStrings*			LPOCSTRINGS;


typedef struct
{
	WORD	number;
	char	str[2];

} OCValueNames;
typedef	OCValueNames*		LPOCVALUENAMES;

#define	OCFLAGS2_DONTSAVEBKD		0x0001
#define	OCFLAGS2_SOLIDBKD			0x0002
#define	OCFLAGS2_COLBOX				0x0004
#define	OCFLAGS2_VISIBLEATSTART		0x0008
#define	OCFLAGS2_OBSTACLESHIFT		4
#define	OCFLAGS2_OBSTACLEMASK		0x0030
#define	OCFLAGS2_OBSTACLE_SOLID		0x0010
#define	OCFLAGS2_OBSTACLE_PLATFORM	0x0020
#define	OCFLAGS2_OBSTACLE_LADDER	0x0030
#define	OCFLAGS2_AUTOMATICROTATION	0x0040


//////////////////////////////////////////////////////////////////////////////
//
// Values
//
#define		MAX_VALUES			1024
#define		VALUENAME_LENGTH	20
typedef struct tabVAL {
	long	vValue;
	char	vName[VALUENAME_LENGTH];
	} oValue;
typedef struct tagVALUES {
	short	vaSize;
	DWORD	vaFlags;			// Flags of the object
	oValue	vaValues[3];		// valurs
	char	vaString[2];		// Character string
	} objectValues;
typedef	objectValues *		fpValues;

//////////////////////////////////////////////////////////////////////////////
//
// Counter
//
typedef	struct	tagCT {
	WORD	ctSize;
	long	ctInit;				// Initial value
	long	ctMini;				// Minimal value
	long	ctMaxi;				// Maximal value
} counter;
typedef counter	*      fpct;
typedef counter	*      fpCounter;

//////////////////////////////////////////////////////////////////////////////
//
// Animations
//
#define	ANIMID_MIN	16

// Anim header
#ifdef __cplusplus
class AnimHeader {
public:
#else
typedef	struct	AnimHeader {
#endif
	WORD		ahSize;
	WORD		ahAnimMax;					// New V2, number of entries in offset table
	short		ahOffsetToAnim[ANIMID_MIN];	// Minimum 16 animations, can be larger!
#ifdef __cplusplus
};
#else
} AnimHeader;
#endif
typedef AnimHeader * fpAnimHeader;
typedef AnimHeader * fpah;
typedef AnimHeader * LPAH;

#ifndef DIRID_MAX
#define DIRID_MAX	32
#endif

// Anim structure
typedef	struct	Anim {
	short	 		anOffsetToDir[DIRID_MAX];	// Offset to animations with a direction
} Anim;
typedef Anim *	fpAnim;
typedef Anim *	fpan;
typedef Anim *	LPAN;

// AnimDirection structure
typedef	struct	AnimDirection {
	BYTE		adMinSpeed;					// Minimum speed
	BYTE		adMaxSpeed;					// Maximum speed
	short		adRepeat;					// Number of loops
	short		adRepeatFrame;				// Where to loop
	short		adNumberOfFrame;			// Number of frames
	short		adFrame[1];					// Frames
} AnimDirection;
#define sizeof_AnimDirection	(sizeof(AnimDirection)-2)
typedef AnimDirection *	fpAnimDir;
typedef AnimDirection *	fpad;
typedef AnimDirection *	LPAD;

// Chunks de sauvegarde
#define EVTFILECHUNK_HEAD			B2L('E','R','>','>')
#define EVTFILECHUNK_EVTHEAD		B2L('E','R','e','s')
#define EVTFILECHUNK_EVENTS			B2L('E','R','e','v')
#define EVTFILECHUNK_NEWITEMS		B2L('>','N','I','<')
#define EVTFILECHUNK_END			B2L('<','<','E','R')

// Marquage des OI qualifiers
#define	OIFLAG_QUALIFIER			0x8000
#define	NDEF_QUALIFIERS				100
#define MAX_EVENTPROGRAMS			256

#define	GAMEON_EMPTY		0
#define	GAMEON_UNLOADED		1
#define	GAMEON_TIME0		2
#define	GAMEON_PAUSED		3
#define	GAMEON_RUNNING		4
#define	GAMEON_STEPPING		5

// COMMUNICATION STRUCTURE BETWEEN RUNTIME AND EDITOR
typedef struct
{
	DWORD		command;

	WORD		runMode;
	DWORD		timer;

	WORD		code;
	OINUM		oi;
	DWORD		param[2];
	char		path[_MAX_PATH];

} ComStructure;
enum
{
	RUNCOMMAND_INITCOM=1,
	RUNCOMMAND_NEWEVENT,
	RUNCOMMAND_NEWSOUND,
	RUNCOMMAND_NEWOBJECT,
	RUNCOMMAND_NEWEVENTS,
	RUNCOMMAND_SETRUNMODE,
	RUNCOMMAND_CONTINUE,
	RUNCOMMAND_RESTART,
	RUNCOMMAND_QUIT,
	HANDLEMESSAGE_WAITFORCONFIRM,
	HANDLEMESSAGE_WAITFORCOMMAND,
	HANDLEMESSAGE_OK,
	HANDLEMESSAGE_QUIT,
	HANDLEMESSAGE_TIMEOUT
};
// DIFFERENTS MODES OF RUN
enum
{
	RUNMODE_NONE,
	RUNMODE_STOPPED,
	RUNMODE_RUNNING,
	RUNMODE_PLAYING,
	RUNMODE_STEPPING,
	RUNMODE_PAUSED,
	RUNMODE_WAITING,
	RUNMODE_WAITINGQUIT,
};


// -------------------------------------------------------------------------
// MOVEMENT DEFINITIONS
// -------------------------------------------------------------------------

// Definition of animation codes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define  	ANIMID_STOP				0
#define  	ANIMID_WALK				1
#define  	ANIMID_RUN				2
#define  	ANIMID_APPEAR			3
#define  	ANIMID_DISAPPEAR		4
#define  	ANIMID_BOUNCE			5
#define  	ANIMID_SHOOT			6
#define  	ANIMID_JUMP				7
#define  	ANIMID_FALL				8
#define  	ANIMID_CLIMB			9
#define  	ANIMID_CROUCH			10
#define  	ANIMID_UNCROUCH			11
#define  	ANIMID_USER1			12
//#define  	ANIMID_MAX				16
#define		ANIMNAME_MAXCHAR		64

// Definition of direction codes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define		DIRF_ALL				0xFFFFFFFF
#define		DIRID_STATIC			32
#define		DIRID_MAX				32
#define		DIRID_SEE				30
#define		DIRID_SE				28
#define		DIRID_SSE				26
#define		DIRID_S					24
#define		DIRID_SSW				22
#define		DIRID_SW				20
#define		DIRID_SWW				18
#define		DIRID_W					16
#define		DIRID_NWW				14
#define		DIRID_NW				12
#define		DIRID_NNW				10
#define		DIRID_N					8
#define		DIRID_NNE				6
#define		DIRID_NE				4
#define		DIRID_NEE				2
#define		DIRID_E					0

// Definition of control types
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define		MVCONT_COMPUTER			0
#define		MVCONT_PLAYER1			1
#define		MVCONT_PLAYER2			2
#define		MVCONT_PLAYER3			3
#define		MVCONT_PLAYER4			4
#define		NB_MVCONT				5
#define		MAX_PLAYERS				4

// Definition of joystick controls
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define		JOYSTICK_UP				0b00000001
#define		JOYSTICK_DOWN			0b00000010
#define		JOYSTICK_LEFT			0b00000100
#define		JOYSTICK_RIGHT			0b00001000
#define		JOYSTICK_FIRE1			0b00010000
#define		JOYSTICK_FIRE2			0b00100000
#define		JOYSTICK_FIRE3			0b01000000
#define		JOYSTICK_FIRE4			0b10000000

// Definition of movement types
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define		MVTYPE_STATIC			0
#define		MVTYPE_MOUSE			1
#define		MVTYPE_RACE				2
#define		MVTYPE_GENERIC			3
#define		MVTYPE_BALL				4
#define		MVTYPE_TAPED			5
#define		MVTYPE_INTELLIGENT		6
#define		MVTYPE_PINBALL			7
#define		MVTYPE_LIST				8
#define		MVTYPE_PLATFORM			9
#define		MVTYPE_GOMOVEMENT		10
#define		MVTYPE_DISAPPEAR		11
#define		MVTYPE_APPEAR			12
#define		MVTYPE_BULLET			13
#define		MVTYPE_EXT				14
#define		MVTYPE_MAX				15

#define		MVTACTION_SAVEPOSITION	0x1010
#define		MVTACTION_LOADPOSITION	0x1011

////////////////////////////////////////////////////////////////////////////////
// EVENTS
////////////////////////////////////////////////////////////////////////////////

// Eventgroup structure, before conditions and actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct tagEVG {
	short	evgSize;				// 0 Size of the group (<=0)
	BYTE	evgNCond;				// 2 Number of conditions
	BYTE	evgNAct;				// 3 Number of actions
	WORD	evgFlags;				// 4 Flags 
	short	evgInhibit;				// 6 If the group is inhibited
	WORD	evgInhibitCpt;			// 8 Counter
	WORD	evgIdentifier;			// 10 Unique identifier
	short 	evgUndo;				// 12 Identifier for UNDO
	} eventGroup;
typedef		eventGroup	*		PEVG;
typedef		eventGroup	*		LPEVG;
#define		EVG_SIZE				14

// MACRO: next group
#define		EVGNEXT(evgPtr)		   	((LPEVG)((LPSTR)evgPtr-evgPtr->evgSize))
// MACRO: first event
#define		EVGFIRSTEVT(p)	   		((LPEVT)((LPSTR)p+sizeof(eventGroup)))
// MACRO: number of events in the group
#define		EVGNEVENTS(p)	 		(p->evgNCond+p->evgNAct)

// Internal flags of eventgroups
#define		EVGFLAGS_ONCE			0x0001
#define		EVGFLAGS_NOTALWAYS		0x0002
#define		EVGFLAGS_REPEAT			0x0004
#define		EVGFLAGS_NOMORE			0x0008
#define		EVGFLAGS_SHUFFLE		0x0010
#define		EVGFLAGS_EDITORMARK		0x0020
#define		EVGFLAGS_UNDOMARK		0x0040
#define		EVGFLAGS_COMPLEXGROUP	0x0080
#define		EVGFLAGS_BREAKPOINT		0x0100
#define		EVGFLAGS_ALWAYSCLEAN	0x0200
#define		EVGFLAGS_ORINGROUP		0x0400
//#define		EVGFLAGS_2MANYACTIONS	0x0400
#define		EVGFLAGS_STOPINGROUP	0x0800
//#define		EVGFLAGS_NOTASSEMBLED	0x1000
#define		EVGFLAGS_ORLOGICAL		0x1000
#define		EVGFLAGS_GROUPED		0x2000
#define		EVGFLAGS_INACTIVE		0x4000
#define		EVGFLAGS_NOGOOD			0x8000
#define		EVGFLAGS_LIMITED		(EVGFLAGS_SHUFFLE+EVGFLAGS_NOTALWAYS+EVGFLAGS_REPEAT+EVGFLAGS_NOMORE)
#define		EVGFLAGS_DEFAULTMASK	(EVGFLAGS_BREAKPOINT+EVGFLAGS_GROUPED)
//#define		EVGFLAGS_FADE			(EVGFLAGS_FADEIN|EVGFLAGS_FADEOUT)
	

// Condition ou action structure
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct tagEVTV1 {
	short	evtSize;				
	union
	{
		struct
		{
			short	evtCode;		
		};
		struct
		{
			char	evtType;		
			char	evtNum;			
		};
	};
	OINUM	evtOi;					
	short	evtOiList;				
	char	evtFlags;				
	char	evtFlags2;				
	char	evtNParams;				
	char   	evtDefType;				
// For the conditions
	short	evtIdentifier;	  		
	} eventV1;
typedef	eventV1	*	LPEVTV1;

#define	CND_SIZEV1					14
#define	ACT_SIZEV1					12
#define	EVGFIRSTEVTV1(p)	   		((LPEVTV1)((LPSTR)p+sizeof(eventGroup)))
#define	EVTPARAMSV1(p) 				((LPEVP)( p->evtCode<0 ? (LPBYTE)p+CND_SIZEV1 : (LPBYTE)p+ACT_SIZEV1) )
#define	EVTNEXTV1(p)		 		((LPEVTV1)((LPBYTE)p+p->evtSize))



typedef struct tagEVT {
	short	evtSize;				// 0 Size of the event
	union
	{
		struct
		{
			long	evtCode;		// 2 Code (hi:NUM lo:TYPE)
		};
		struct
		{
			short	evtType;		// 2 Type of object
			short	evtNum;			// 4 Number of action/condition
		};
	};
	OINUM	evtOi;					// 6 OI if normal object
	short	evtOiList;				// 8 Pointer
	char	evtFlags;				// 10 Flags
	char	evtFlags2;				// 11 Flags II
	char	evtNParams;				// 12 Number of parameters
	char   	evtDefType;				// 13 If default, type
// Pour les conditions
	short	evtIdentifier;	  		// 14 Event identifier
	} event;						// 16
typedef	event	*	PEVT;
typedef	event	*	LPEVT;

#define	CND_SIZE					16
#define	ACT_SIZE					14

// Definition of conditions / actions flags
#define		EVFLAGS_REPEAT			0x01
#define		EVFLAGS_DONE			0x02
#define		EVFLAGS_DEFAULT			0x04
#define		EVFLAGS_DONEBEFOREFADEIN 0x08
#define		EVFLAGS_NOTDONEINSTART	0x10
#define		EVFLAGS_ALWAYS			0x20
#define		EVFLAGS_BAD				0x40
#define		EVFLAGS_BADOBJECT		0x80
#define		EVFLAGS_DEFAULTMASK		(EVFLAGS_ALWAYS+EVFLAGS_REPEAT+EVFLAGS_DEFAULT+EVFLAGS_DONEBEFOREFADEIN+EVFLAGS_NOTDONEINSTART)
#define		ACTFLAGS_REPEAT			0x0001


// For flags II
// -------------                    
#define		EVFLAG2_NOT			0x0001
#define		EVFLAG2_NOTABLE		0x0002
#define		EVFLAGS_NOTABLE		(EVFLAG2_NOTABLE<<8)
#define		EVFLAGS_MONITORABLE	0x0004
#define		EVFLAGS_TODELETE	0x0008
#define		EVFLAGS_NEWSOUND	0x0010
#define		EVFLAG2_MASK		(EVFLAG2_NOT|EVFLAG2_NOTABLE|EVFLAGS_MONITORABLE)

// MACRO: Returns the code for an extension
#define		EXTCONDITIONNUM(i)		(-((short)(i>>16))-1)           
#define		EXTACTIONNUM(i)			((short)(i>>16))

// PARAM Structure
// ~~~~~~~~~~~~~~~
#ifdef __cplusplus
class eventParam { 
public:
#else
typedef struct eventParam {
#endif
	short		evpSize;
	short		evpCode;
	union	{
		struct	{
		short		evpW0;
		short		evpW1;
		short		evpW2;
		short		evpW3;
		short		evpW4;
		short		evpW5;
		short		evpW6;
		short		evpW7;
		} evpW;
		struct	{
		long	evpL0;
		long	evpL1;
		long	evpL2;
		long	evpL3;
		} evpL;
	} evp;
#ifdef __cplusplus
};
#else
} eventParam;
#endif
typedef	eventParam	*			PEVP;
typedef	eventParam	*			LPEVP;
typedef	eventParam	*			fpevp;

// MACRO: next parameter
#define		EVPNEXT(p)		   		((LPEVP)((LPSTR)p+p->evpSize))


// Operators / expressions parameters
// --------------------------------------------------
#define		MIN_LONG				(-10000000L)
#define		MAX_LONG				(10000000L)
#define		EXPPARAM_LONG			1
#define		EXPPARAM_VARGLO			2
#define		EXPPARAM_STRING			3
#define		EXPPARAM_ALTVALUE		4
#define		EXPPARAM_FLAG			5
#define		EXP_STOP				-1
#define		OPERATOR_START			0x00000000
#define		EXPL_END				0x00000000	
#define		EXPL_PLUS				0x00020000
#define		EXPL_MOINS				0x00040000
#define		EXPL_MULT				0x00060000
#define		EXPL_DIV				0x00080000                                         
#define		EXPL_MOD				0x000A0000
#define		EXPL_POW				0x000C0000
#define		EXPL_AND				0x000E0000
#define		EXPL_OR					0x00100000
#define		EXPL_XOR				0x00120000
#define		OPERATOR_END			0x00140000
typedef struct tagEXP {
	union
	{
		struct
		{
			long	expCode;		// 2 Code (hi:NUM lo:TYPE)
		};
		struct
		{
			short	expType;		// 2 Type of object
			short	expNum;			// 3 Expression number
		};
	};
	short	expSize;
	union	
	{
		struct {
			short	expOi;
			short	expOiList;
		} expo;
		struct {
			long	expLParam;
		} expl;
		struct {
			double	expDouble;
			float	expFloat;
		} expd;
		struct {
			short	expWParam0;
			short	expWParam1;
		} expw;
		struct {
			short	expOi;
			short	expOiList;
			short	expNum;
		} expv;
		struct {
			long	expExtCode;
			short	expExtNumber;
		} expc;
	} expu;
	} expression;
typedef	expression *	LPEXP;
typedef	expression *	PEXP;

#define		CMPOPE_EQU				0x0000
#define		CMPOPE_DIF				0x0001
#define		CMPOPE_LOWEQU			0x0002
#define		CMPOPE_LOW				0x0003
#define		CMPOPE_GREEQU			0x0004
#define		CMPOPE_GRE				0x0005
#define		MAX_CMPOPE				6
#define		EXPNEXT(expPtr)			((LPEXP)((LPSTR)expPtr+expPtr->expSize))
#define		EXPFLAG_STRING			0x0001
#define		EXPFLAG_DOUBLE			0x0002

typedef struct tagEXPV1 {
	union
	{
		struct
		{
			short	expCode;
		};
		struct
		{
			char	expType;
			char	expNum;	
		};
	};
	short	expSize;
	union	
	{
		struct {
			short	expOi;
			short	expOiList;
		} expo;
		struct {
			long	expLParam;
		} expl;
		struct {
			double	expDouble;
			float	expFloat;
		} expd;
		struct {
			short	expWParam0;
			short	expWParam1;
		} expw;
		struct {
			short	expOi;
			short	expOiList;
			short	expNum;
		} expv;
		struct {
			long	expExtCode;
			short	expExtNumber;
		} expc;
	} expu;
	}expressionV1;
typedef	expressionV1 *			LPEXPV1;

#define		EXPNEXTV1(expPtr)		((LPEXPV1)((LPSTR)expPtr+expPtr->expSize))

/*
#define		Q_SPR	0x0100
#define		Q_TXT	0x0200
#define		Q_QST	0x0400
#define		Q_ARE	0x0800
#define		Q_CNT	0x1000
#define		Q_PLA	0x2000
#define		Q_GAM	0x4000
#define		Q_TIM	0x8000
#define		Q_COL	0x0001
#define		Q_ZNE	0x0002
#define		Q_MVT	0x0004
#define		Q_ANI	0x0008
#define		Q_OBJ	0x0010
#define		Q_KEY	0x0020
#define		Q_SYS	0x0040
*/

// Information structure
// ----------------------------------------------
typedef struct tagEVO {                                  
	short		evoConditions;	  	// Conditions
	short		evoActions;			// Actions
	short		evoExpressions;		// Expressions
	short		evoMsgMenus;		// Menu strings for actions/conditions
	short		evoMsgDisplay;		// Display strings for actions/conditions
	short		evoMsgExpressions; 	// Menu/display strings for expressions
	short		evoMsgExpParams;	// Parameter string
	short		evoNConditions;		// Number of conditions
	} eventInfosOffsets;
typedef	eventInfosOffsets *		PEVO;
typedef	eventInfosOffsets *		LPEVO;

// Definition of the different system objects (TYPE<0)
// ----------------------------------------------------------
#define		TYPE_DIRECTION			-127
#define		TYPE_QUALIFIER			-126
#define		NUMBEROF_SYSTEMTYPES    7
#define		OBJ_PLAYER				-7
#define		OBJ_KEYBOARD			-6
#define		OBJ_CREATE				-5
#define		OBJ_TIMER		  		-4
#define		OBJ_GAME		  		-3
#define		OBJ_SPEAKER	  			-2
#define		OBJ_SYSTEM				-1
#define		OBJ_FIRST_C_OBJECT		8
#define		OBJ_LAST				NB_SYSOBJ








// ------------------------------------------------------------
// EXTENSION OBJECT DATA ZONE
// ------------------------------------------------------------

// Flags 
#define		OEFLAG_DISPLAYINFRONT		0x0001
#define		OEFLAG_BACKGROUND			0x0002
#define		OEFLAG_BACKSAVE				0x0004
#define		OEFLAG_RUNBEFOREFADEIN		0x0008
#define		OEFLAG_MOVEMENTS			0x0010
#define		OEFLAG_ANIMATIONS			0x0020
#define		OEFLAG_TABSTOP				0x0040
#define		OEFLAG_WINDOWPROC			0x0080
#define		OEFLAG_VALUES				0x0100
#define		OEFLAG_SPRITES				0x0200
#define		OEFLAG_INTERNALBACKSAVE		0x0400
#define		OEFLAG_SCROLLINGINDEPENDANT	0x0800
#define		OEFLAG_QUICKDISPLAY			0x1000
#define		OEFLAG_NEVERKILL			0x2000
#define		OEFLAG_NEVERSLEEP			0x4000
#define		OEFLAG_MANUALSLEEP			0x8000
#define		OEFLAG_TEXT					0x10000
#define		OEFLAG_DONTCREATEATSTART	0x20000

// Flags modifiable by the program
#define		OEPREFS_BACKSAVE				0x0001
#define		OEPREFS_SCROLLINGINDEPENDANT	0x0002
#define		OEPREFS_QUICKDISPLAY			0x0004
#define		OEPREFS_SLEEP					0x0008
#define		OEPREFS_LOADONCALL				0x0010
#define		OEPREFS_GLOBAL					0x0020
#define		OEPREFS_BACKEFFECTS				0x0040
#define		OEPREFS_KILL					0x0080
#define		OEPREFS_INKEFFECTS				0x0100
#define		OEPREFS_TRANSITIONS				0x0200
#define		OEPREFS_FINECOLLISIONS			0x0400

// Running flags
#define		REFLAG_ONESHOT				0x0001
#define		REFLAG_DISPLAY				0x0002
#define		REFLAG_MSGHANDLED			0x0004
#define		REFLAG_MSGCATCHED			0x0008
#define		REFLAG_MSGDEFPROC			0x0010

#define		REFLAG_MSGRETURNVALUE		0x0040

#define		OEFLAGS_DEFAULTSETUPS		(OEFLAG_MOVEMENTS|OEFLAG_ANIMATIONS|OEFLAG_SPRITES|OEFLAG_VALUES)



#ifndef			IN_KPX

///////////////////////////////////////////////////////////////////////
//
// DEFINITION OF EVENTS AND ACTIONS CODES
//
///////////////////////////////////////////////////////////////////////

// SYSTEM Conditions / Actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//#define		OBJ_SYSTEM				-1
//-1&255=255
#define CND_CHANCE			((-26<<8)|255)
#define CND_ORLOGICAL		((-25<<8)|255)
#define CNDL_ORLOGICAL		((-25<<16)|65535)
#define CND_OR				((-24<<8)|255)
#define CNDL_OR				((-24<<16)|65535)
#define CND_GROUPSTART		((-23<<8)|255)
#define CNDL_GROUPSTART		((-23<<16)|65535)
#define CND_CLIPBOARD		((-22<<8)|255)
#define CND_ONCLOSE			((-21<<8)|255)
#define CNDL_ONCLOSE		((-21<<16)|65535)
#define	CND_COMPAREGSTRING	((-20<<8)|255)
#define	CNDL_COMPAREGSTRING	((-20<<16)|65535)
#define CND_MENUVISIBLE		((-19<<8)|255)
#define CND_MENUENABLED		((-18<<8)|255)
#define CND_MENUCHECKED		((-17<<8)|255)
#define	CND_ONLOOP			((-16<<8)|255)
#define	CNDL_ONLOOP			((-16<<16)|65535)
#define CND_DROPFILES		((-15<<8)|255)
#define CNDL_DROPFILES		((-15<<16)|65535)
#define	CND_MENUSELECTED	((-14<<8)|255)
#define CNDL_MENUSELECTED	((-14<<16)|65535)
#define	CND_RECORDKEY		((-13<<8)|255)
#define CNDL_RECORDKEY		((-13<<16)|65535)
#define	CND_GROUPACTIVATED	((-12<<8)|255)
#define	CNDL_GROUPACTIVATED	((-12<<16)|65535)
#define	CND_ENDGROUP		((-11<<8)|255)
#define CNDL_ENDGROUP		((-11<<16)|65535)
#define	CND_GROUP			((-10<<8)|255)
#define CNDL_GROUP			((-10<<16)|65535)
#define	CND_REMARK			((-9<<8)|255)
#define CNDL_REMARK			((-9<<16)|65535)
#define	CND_COMPAREG		((-8<<8)|255)
#define	CNDL_COMPAREG		((-8<<16)|65535)
#define	CND_NOTALWAYS		((-7<<8)|255)
#define	CNDL_NOTALWAYS		((-7<<16)|65535)
#define	CND_ONCE			((-6<<8)|255)
#define CNDL_ONCE			((-6<<16)|65535)
#define	CND_REPEAT			((-5<<8)|255)
#define CNDL_REPEAT			((-5<<16)|65535)
#define	CND_NOMORE			((-4<<8)|255)
#define CNDL_NOMORE			((-4<<16)|65535)
#define	CND_COMPARE			((-3<<8)|255)
#define	CND_NEVER			((-2<<8)|255)
#define CNDL_NEVER			((-2<<16)|65535)
#define	CND_ALWAYS			((-1<<8)|255)
#define CNDL_ALWAYS			((-1<<16)|65535)

#define	ACT_SKIP			((0<<8)|255)
#define	ACTL_SKIP			((0<<16)|65535)
#define	ACT_SKIPMONITOR		((1<<8)|255)
#define	ACTL_SKIPMONITOR	((1<<16)|65535)
#define	ACT_EXECPROG		((2<<8)|255)
#define	ACT_SETVARG			((3<<8)|255)
#define	ACTL_SETVARG		((3<<16)|65535)
#define	ACT_SUBVARG			((4<<8)|255)
#define	ACTL_SUBVARG		((4<<16)|65535)
#define	ACT_ADDVARG			((5<<8)|255)
#define	ACTL_ADDVARG		((5<<16)|65535)
#define	ACT_GRPACTIVATE		((6<<8)|255)
#define	ACTL_GRPACTIVATE	((6<<16)|65535)
#define	ACT_GRPDEACTIVATE	((7<<8)|255)
#define	ACTL_GRPDEACTIVATE	((7<<16)|65535)
#define	ACT_MENUACTIVATE	((8<<8)|255)
#define	ACT_MENUDEACTIVATE	((9<<8)|255)
#define	ACT_MENUCHECK		((10<<8)|255)
#define	ACT_MENUNCHECK		((11<<8)|255)
#define	ACT_MENUSHOW		((12<<8)|255)
#define	ACTL_MENUSHOW		((12<<16)|65535)
#define	ACT_MENUHIDE		((13<<8)|255)
#define	ACTL_MENUHIDE		((13<<16)|65535)
#define	ACT_STARTLOOP		((14<<8)|255)
#define	ACT_STOPLOOP		((15<<8)|255)
#define	ACT_SETLOOPINDEX	((16<<8)|255)
#define	ACT_RANDOMIZE		((17<<8)|255)
#define ACT_SENDMENUCMD		((18<<8)|255)
#define ACT_SETGLOBALSTRING	((19<<8)|255)
#define	ACTL_SETGLOBALSTRING ((19<<16)|65535)
#define ACT_SENDCLIPBOARD	((20<<8)|255)
#define ACT_CLEARCLIPBOARD	((21<<8)|255)
#define ACT_EXECPROG2		((22<<8)|255)
#define ACT_OPENDEBUGGER	((23<<8)|255)
#define ACT_PAUSEDEBUGGER	((24<<8)|255)
#define ACT_EXTRACTBINFILE	((25<<8)|255)
#define ACT_RELEASEBINFILE	((26<<8)|255)

#define	EXP_LONG			((0<<8)|255)
#define	EXPL_LONG			((0<<16)|65535)
#define	EXP_RANDOM			((1<<8)|255)
#define	EXPL_RANDOM			((1<<16)|65535)
#define	EXP_VARGLO			((2<<8)|255)
#define	EXPL_VARGLO			((2<<16)|65535)
#define	EXP_STRING			((3<<8)|255)
#define	EXPL_STRING			((3<<16)|65535)
#define	EXP_STR				((4<<8)|255)
#define	EXPL_STR			((4<<16)|65535)
#define	EXP_VAL				((5<<8)|255)
#define	EXPL_VAL			((5<<16)|65535)
#define	EXP_DRIVE			((6<<8)|255)
#define	EXPL_DRIVE			((6<<16)|65535)
#define	EXP_DIRECTORY		((7<<8)|255)
#define	EXPL_DIRECTORY		((7<<16)|65535)
#define	EXP_PATH			((8<<8)|255)
#define	EXPL_PATH			((8<<16)|65535)
#define	EXP_APPNAME			((9<<8)|255)
#define	EXPL_APPNAME		((9<<16)|65535)
#define EXP_SIN				((10<<8)|255)
#define EXPL_SIN			((10<<16)|65535)
#define EXP_COS				((11<<8)|255)
#define EXPL_COS			((11<<16)|65535)
#define EXP_TAN				((12<<8)|255)
#define EXPL_TAN			((12<<16)|65535)
#define EXP_SQR				((13<<8)|255)
#define EXPL_SQR			((13<<16)|65535)
#define EXP_LOG				((14<<8)|255)
#define EXPL_LOG			((14<<16)|65535)
#define EXP_LN				((15<<8)|255)
#define EXPL_LN				((15<<16)|65535)
#define EXP_HEX				((16<<8)|255)
#define EXPL_HEX			((16<<16)|65535)
#define EXP_BIN				((17<<8)|255)
#define EXPL_BIN			((17<<16)|65535)
#define EXP_EXP				((18<<8)|255)
#define EXPL_EXP			((18<<16)|65535)
#define EXP_LEFT			((19<<8)|255)
#define EXPL_LEFT			((19<<16)|65535)
#define EXP_RIGHT			((20<<8)|255)
#define EXPL_RIGHT			((20<<16)|65535)
#define EXP_MID				((21<<8)|255)
#define EXPL_MID			((21<<16)|65535)
#define EXP_LEN				((22<<8)|255)
#define EXPL_LEN			((22<<16)|65535)
#define	EXP_DOUBLE			((23<<8)|255)
#define	EXPL_DOUBLE			((23<<16)|65535)
#define	EXP_VARGLONAMED		((24<<8)|255)
#define	EXPL_VARGLONAMED	((24<<16)|65535)
#define EXP_ENTERSTRINGHERE	((25<<8)|255)
#define EXPL_ENTERSTRINGHERE ((25<<16)|65535)
#define EXP_ENTERVALUEHERE	((26<<8)|255)
#define EXPL_ENTERVALUEHERE	((26<<16)|65535)
#define	EXP_FLOAT			((27<<8)|255)
#define	EXPL_FLOAT			((27<<16)|65535)
#define	EXP_INT				((28<<8)|255)
#define	EXPL_INT			((28<<16)|65535)
#define EXP_ABS				((29<<8)|255)
#define EXPL_ABS			((29<<16)|65535)
#define EXP_CEIL			((30<<8)|255)
#define EXPL_CEIL			((30<<16)|65535)
#define	EXP_FLOOR			((31<<8)|255)
#define	EXPL_FLOOR			((31<<16)|65535)
#define	EXP_ACOS			((32<<8)|255)
#define	EXPL_ACOS			((32<<16)|65535)
#define	EXP_ASIN			((33<<8)|255)
#define	EXPL_ASIN			((33<<16)|65535)
#define	EXP_ATAN			((34<<8)|255)
#define	EXPL_ATAN			((34<<16)|65535)
#define	EXP_NOT				((35<<8)|255)
#define	EXPL_NOT			((35<<16)|65535)
#define	EXP_NDROPFILES		((36<<8)|255)
#define	EXPL_NDROPFILES		((36<<16)|65535)
#define	EXP_DROPFILE		((37<<8)|255)
#define	EXPL_DROPFILE		((37<<16)|65535)
#define	EXP_GETCOMMANDLINE	((38<<8)|255)
#define	EXPL_GETCOMMANDLINE	((38<<16)|65535)
#define	EXP_GETCOMMANDITEM	((39<<8)|255)
#define	EXPL_GETCOMMANDITEM	((39<<16)|65535)
#define	EXP_MIN				((40<<8)|255)
#define	EXPL_MIN			((40<<16)|65535)
#define	EXP_MAX				((41<<8)|255)
#define	EXPL_MAX			((41<<16)|65535)
#define	EXP_GETRGB			((42<<8)|255)
#define	EXPL_GETRGB			((42<<16)|65535)
#define	EXP_GETRED			((43<<8)|255)
#define	EXPL_GETRED			((43<<16)|65535)
#define	EXP_GETGREEN		((44<<8)|255)
#define	EXPL_GETGREEN		((44<<16)|65535)
#define	EXP_GETBLUE			((45<<8)|255)
#define	EXPL_GETBLUE		((45<<16)|65535)
#define	EXP_LOOPINDEX		((46<<8)|255)
#define	EXPL_LOOPINDEX		((46<<16)|65535)
#define EXP_NEWLINE			((47<<8)|255)
#define EXPL_NEWLINE		((47<<16)|65535)
#define EXP_ROUND			((48<<8)|255)
#define EXPL_ROUND			((48<<16)|65535)
#define	EXP_STRINGGLO		((49<<8)|255)
#define	EXPL_STRINGGLO		((49<<16)|65535)
#define	EXP_STRINGGLONAMED	((50<<8)|255)
#define	EXPL_STRINGGLONAMED	((50<<16)|65535)
#define EXP_LOWER			((51<<8)|255)
#define EXPL_LOWER			((51<<16)|65535)
#define EXP_UPPER			((52<<8)|255)
#define EXPL_UPPER			((52<<16)|65535)
#define EXP_FIND			((53<<8)|255)
#define EXPL_FIND			((53<<16)|65535)
#define EXP_REVERSEFIND		((54<<8)|255)
#define EXPL_REVERSEFIND	((54<<16)|65535)
#define EXP_GETCLIPBOARD	((55<<8)|255)
#define	EXP_TEMPPATH		((56<<8)|255)
#define EXP_BINFILETEMPNAME	((57<<8)|255)

#define	EXP_PARENTH1		((-1<<8)|255)
#define	EXPL_PARENTH1		((-1<<16)|65535)
#define	EXP_PARENTH2		((-2<<8)|255)
#define	EXPL_PARENTH2		((-2<<16)|65535)
#define	EXP_VIRGULE			((-3<<8)|255)
#define	EXPL_VIRGULE		((-3<<16)|65535)

// SPEAKER Conditions / Actions 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// #define		TYPE_SPEAKER	  		-2
//(TYPE_SPEAKER&255)=254
#define CND_SPCHANNELPAUSED ((-9<<8)|254)
#define CND_NOSPCHANNELPLAYING ((-8<<8)|254)
#define	CND_MUSPAUSED		((-7<<8)|254)
#define	CND_SPSAMPAUSED		((-6<<8)|254)
#define	CND_MUSICENDS		((-5<<8)|254)
#define	CNDL_MUSICENDS		((-5<<16)|65534)
#define	CND_NOMUSPLAYING	((-4<<8)|254)
#define	CNDL_NOMUSPLAYING	((-4<<16)|65534)
#define	CND_NOSAMPLAYING	((-3<<8)|254)
#define	CND_NOSPMUSPLAYING	((-2<<8)|254)
#define	CND_NOSPSAMPLAYING	((-1<<8)|254)
#define	ACT_PLAYSAMPLE		((0<<8)|254)
#define	ACT_STOPSAMPLE		((1<<8)|254)
#define	ACT_PLAYMUSIC		((2<<8)|254)
#define	ACTL_PLAYMUSIC		((2<<16)|65534)
#define	ACT_STOPMUSIC		((3<<8)|254)
#define	ACTL_STOPMUSIC		((3<<16)|65534)
#define	ACT_PLAYLOOPSAMPLE 	((4<<8)|254)
#define	ACT_PLAYLOOPMUSIC 	((5<<8)|254)
#define	ACT_STOPSPESAMPLE	((6<<8)|254)
#define	ACT_PAUSESAMPLE		((7<<8)|254)
#define	ACT_RESUMESAMPLE	((8<<8)|254)
#define	ACT_PAUSEMUSIC		((9<<8)|254)
#define	ACT_RESUMEMUSIC		((10<<8)|254)
#define	ACT_PLAYCHANNEL		((11<<8)|254)
#define	ACT_PLAYLOOPCHANNEL	((12<<8)|254)
#define	ACT_PAUSECHANNEL	((13<<8)|254)
#define	ACT_RESUMECHANNEL	((14<<8)|254)
#define	ACT_STOPCHANNEL		((15<<8)|254)
#define	ACT_SETCHANNELPOS	((16<<8)|254)
#define	ACT_SETCHANNELVOL	((17<<8)|254)
#define	ACT_SETCHANNELPAN	((18<<8)|254)
#define	ACT_SETSAMPLEPOS	((19<<8)|254)
#define	ACT_SETSAMPLEMAINVOL	((20<<8)|254)
#define	ACT_SETSAMPLEVOL		((21<<8)|254)
#define	ACT_SETSAMPLEMALNPAN	((22<<8)|254)
#define	ACT_SETSAMPLEPAN		((23<<8)|254)
#define ACT_PAUSEALLCHANNELS	((24<<8)|254)
#define ACT_RESUMEALLCHANNELS	((25<<8)|254)
#define ACT_PLAYMUSICFILE		((26<<8)|254)
#define ACT_PLAYLOOPMUSICFILE	((27<<8)|254)
#define ACT_PLAYFILECHANNEL		((28<<8)|254)
#define ACT_PLAYLOOPFILECHANNEL	((29<<8)|254)
#define ACT_LOCKCHANNEL			((30<<8)|254)
#define ACT_UNLOCKCHANNEL		((31<<8)|254)
#define ACT_SETCHANNELFREQ		((32<<8)|254)
#define ACT_SETSAMPLEFREQ		((33<<8)|254)

#define	EXP_GETSAMPLEMAINVOL	((0<<8)|254)
#define	EXP_GETSAMPLEVOL		((1<<8)|254)
#define	EXP_GETCHANNELVOL		((2<<8)|254)
#define	EXP_GETSAMPLEMAINPAN	((3<<8)|254)
#define	EXP_GETSAMPLEPAN		((4<<8)|254)
#define	EXP_GETCHANNELPAN		((5<<8)|254)
#define	EXP_GETSAMPLEPOS		((6<<8)|254)
#define	EXP_GETCHANNELPOS		((7<<8)|254)
#define	EXP_GETSAMPLEDUR		((8<<8)|254)
#define	EXP_GETCHANNELDUR		((9<<8)|254)
#define	EXP_GETSAMPLEFREQ		((10<<8)|254)
#define	EXP_GETCHANNELFREQ		((11<<8)|254)


// GAME Conditions / Actions 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//#define		TYPE_GAME		  		-3
//(TYPE_GAME&255)=253

#define CND_FRAMELOADED		((-9<<8)|253)
#define	CNDL_ENDOFPAUSE		((-8<<16)|65533)
#define CND_ENDOFPAUSE		((-8<<8)|253)
#define CND_ISVSYNCON		((-7<<8)|253)
#define	CND_ISLADDER		((-6<<8)|253)
#define	CND_ISOBSTACLE		((-5<<8)|253)
#define	CND_QUITAPPLICATION	((-4<<8)|253)
#define	CNDL_QUITAPPLICATION	((-4<<16)|65533)
#define	CND_LEVEL			((-3<<8)|253)
#define	CND_END				((-2<<8)|253)
#define	CNDL_END			((-2<<16)|65533)
#define	CND_START			((-1<<8)|253)
#define	CNDL_START			((-1<<16)|65533)

#define	ACT_NEXTLEVEL       ((0<<8)|253)
#define	ACTL_NEXTLEVEL       ((0<<16)|65533)
#define	ACT_PREVLEVEL       ((1<<8)|253)
#define	ACTL_PREVLEVEL       ((1<<16)|65533)
#define	ACT_GOLEVEL     	((2<<8)|253)
#define	ACTL_GOLEVEL     	((2<<16)|65533)
#define	ACT_PAUSE           ((3<<8)|253)
#define	ACT_ENDGAME         ((4<<8)|253)
#define	ACTL_ENDGAME         ((4<<16)|65533)
#define	ACT_RESTARTGAME     ((5<<8)|253)
#define	ACTL_RESTARTGAME     ((5<<16)|65533)
#define	ACT_RESTARTLEVEL    ((6<<8)|253)
#define	ACT_CDISPLAY	    ((7<<8)|253)
#define	ACT_CDISPLAYX	    ((8<<8)|253)
#define	ACT_CDISPLAYY	    ((9<<8)|253)
#define	ACT_LOADGAME		((10<<8)|253)
#define	ACT_SAVEGAME		((11<<8)|253)
#define ACT_CLS				((12<<8)|253)
#define	ACT_CLEARZONE		((13<<8)|253)
#define ACT_FULLSCREENMODE	((14<<8)|253)
#define ACT_WINDOWEDMODE	((15<<8)|253)
#define ACT_SETFRAMERATE	((16<<8)|253)
#define ACT_PAUSEKEY		((17<<8)|253)
#define ACT_PAUSEANYKEY		((18<<8)|253)
#define	ACT_SETVSYNCON		((19<<8)|253)
#define	ACT_SETVSYNCOFF		((20<<8)|253)
#define	ACT_SETVIRTUALWIDTH	((21<<8)|253)
#define	ACT_SETVIRTUALHEIGHT ((22<<8)|253)
#define ACT_SETFRAMEBDKCOLOR ((23<<8)|253)
#define ACT_DELCREATEDBKDAT ((24<<8)|253)
#define ACT_DELALLCREATEDBKD ((25<<8)|253)
#define ACT_SETFRAMEWIDTH	((26<<8)|253)
#define ACT_SETFRAMEHEIGHT	((27<<8)|253)
#define ACT_SAVEFRAME		((28<<8)|253)
#define ACT_LOADFRAME		((29<<8)|253)
#define ACT_LOADAPPLICATION	((30<<8)|253)
#define ACT_PLAYDEMO		((31<<8)|253)

#define	EXP_GAMLEVEL		((0<<8)|253)
#define	EXP_GAMNPLAYER		((1<<8)|253)
#define	EXP_PLAYXLEFT		((2<<8)|253)
#define	EXP_PLAYXRIGHT		((3<<8)|253)
#define	EXP_PLAYYTOP		((4<<8)|253)
#define	EXP_PLAYYBOTTOM		((5<<8)|253)
#define	EXP_PLAYWIDTH		((6<<8)|253)
#define	EXP_PLAYHEIGHT		((7<<8)|253)
#define	EXP_GAMLEVELNEW		((8<<8)|253)
#define	EXP_GETCOLLISIONMASK ((9<<8)|253)
#define EXP_FRAMERATE		((10<<8)|253)
#define EXP_GETVIRTUALWIDTH	((11<<8)|253)
#define EXP_GETVIRTUALHEIGHT ((12<<8)|253)
#define EXP_GETFRAMEBKDCOLOR ((13<<8)|253)

// TIMER Conditions / Actions 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define	CND_TIMEOUT       	((-5<<8)|(OBJ_TIMER&255))
#define	CND_EVERY       	((-4<<8)|(OBJ_TIMER&255))
#define	CNDL_EVERY       	((-4<<16)|(OBJ_TIMER&0xFFFF))
#define	CND_TIMER       	((-3<<8)|(OBJ_TIMER&255))
#define	CNDL_TIMER       	((-3<<16)|(OBJ_TIMER&0xFFFF))
#define	CND_TIMERINF       	((-2<<8)|(OBJ_TIMER&255))
#define	CNDL_TIMERINF      	((-2<<16)|(OBJ_TIMER&0xFFFF))
#define	CND_TIMERSUP       	((-1<<8)|(OBJ_TIMER&255))
#define	CNDL_TIMERSUP      	((-1<<16)|(OBJ_TIMER&0xFFFF))
#define	ACT_SETTIMER        ((0<<8)|(OBJ_TIMER&255))
#define	EXP_TIMVALUE		((0<<8)|(OBJ_TIMER&255))
#define	EXP_TIMCENT			((1<<8)|(OBJ_TIMER&255))
#define	EXP_TIMSECONDS		((2<<8)|(OBJ_TIMER&255))
#define	EXP_TIMHOURS		((3<<8)|(OBJ_TIMER&255))
#define	EXP_TIMMINITS		((4<<8)|(OBJ_TIMER&255))
#define	NUM_EVERY       	-4
#define	NUM_TIMER       	-3
#define	NUM_TIMERINF       	-2
#define	NUM_TIMERSUP       	-1

// KEYBOARD Conditions / Actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define	CNDL_ONMOUSEWHEELDOWN  	((-12<<16)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_ONMOUSEWHEELDOWN   	((-12<<8)|(OBJ_KEYBOARD&255))
#define	CNDL_ONMOUSEWHEELUP  	((-11<<16)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_ONMOUSEWHEELUP   	((-11<<8)|(OBJ_KEYBOARD&255))
#define	CND_MOUSEON		   	((-10<<8)|(OBJ_KEYBOARD&255))
#define	CND_ANYKEY			((-9<<8)|(OBJ_KEYBOARD&255))
#define	CNDL_ANYKEY			((-9<<16)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_MKEYDEPRESSED	((-8<<8)|(OBJ_KEYBOARD&255))
#define	CND_MCLICKONOBJECT	((-7<<8)|(OBJ_KEYBOARD&255))
#define	CNDL_MCLICKONOBJECT	((-7<<16)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_MCLICKINZONE 	((-6<<8)|(OBJ_KEYBOARD&255))
#define	CNDL_MCLICKINZONE 	((-6<<16)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_MCLICK	 		((-5<<8)|(OBJ_KEYBOARD&255))
#define	CNDL_MCLICK	 		((-5<<16)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_MONOBJECT		((-4<<8)|(OBJ_KEYBOARD&255))
#define	CNDL_MONOBJECT		((-4<<16)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_MINZONE			((-3<<8)|(OBJ_KEYBOARD&255))
#define	CND_KBKEYDEPRESSED 	((-2<<8)|(OBJ_KEYBOARD&255))
#define	CND_KBPRESSKEY   	((-1<<8)|(OBJ_KEYBOARD&255))
#define CNDL_KBPRESSKEY		((-1<<16)|(OBJ_KEYBOARD&0xFFFF))
#define	ACT_HIDECURSOR		((0<<8)|(OBJ_KEYBOARD&255))
#define	ACT_SHOWCURSOR		((1<<8)|(OBJ_KEYBOARD&255))
#define	EXP_XMOUSE			((0<<8)|(OBJ_KEYBOARD&255))
#define	EXP_YMOUSE			((1<<8)|(OBJ_KEYBOARD&255))
#define	EXP_MOUSEWHEELDELTA	((2<<8)|(OBJ_KEYBOARD&255))


// PLAYERS Conditions / Actions 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define	CND_JOYPUSHED		((-6<<8)|(OBJ_PLAYER&255))
#define	CND_NOMORELIVE		((-5<<8)|(OBJ_PLAYER&255))
#define	CNDL_NOMORELIVE		((-5<<16)|(OBJ_PLAYER&0xFFFF))
#define	CND_JOYPRESSED		((-4<<8)|(OBJ_PLAYER&255))
#define	CNDL_JOYPRESSED		((-4<<16)|(OBJ_PLAYER&0xFFFF))
#define	CND_LIVE	        ((-3<<8)|(OBJ_PLAYER&255))
#define	CND_SCORE		    ((-2<<8)|(OBJ_PLAYER&255))
#define	CND_PLAYERPLAYING   ((-1<<8)|(OBJ_PLAYER&255))

#define	ACT_SETSCORE	  	((0<<8)|(OBJ_PLAYER&255))
#define	ACT_SETLIVES      	((1<<8)|(OBJ_PLAYER&255))
#define	ACT_NOINPUT      	((2<<8)|(OBJ_PLAYER&255))
#define	ACT_RESTINPUT      	((3<<8)|(OBJ_PLAYER&255))
#define	ACT_ADDSCORE	  	((4<<8)|(OBJ_PLAYER&255))
#define	ACT_ADDLIVES      	((5<<8)|(OBJ_PLAYER&255))
#define	ACT_SUBSCORE	  	((6<<8)|(OBJ_PLAYER&255))
#define	ACT_SUBLIVES      	((7<<8)|(OBJ_PLAYER&255))
#define	ACT_SETINPUT	  	((8<<8)|(OBJ_PLAYER&255))
#define	ACT_SETINPUTKEY    	((9<<8)|(OBJ_PLAYER&255))
#define	ACT_SETPLAYERNAME	((10<<8)|(OBJ_PLAYER&255))

#define	EXP_PLASCORE		((0<<8)|(OBJ_PLAYER&255))
#define	EXP_PLALIVES		((1<<8)|(OBJ_PLAYER&255))
#define	EXP_GETINPUT		((2<<8)|(OBJ_PLAYER&255))
#define	EXP_GETINPUTKEY		((3<<8)|(OBJ_PLAYER&255))
#define	EXP_GETPLAYERNAME	((4<<8)|(OBJ_PLAYER&255))
#define	NUM_JOYPRESSED		-4

// CREATE Conditions / Actions 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define	CND_CHOOSEALLINLINE	((-23<<8)|(OBJ_CREATE&255))
#define	CND_CHOOSEFLAGRESET	((-22<<8)|(OBJ_CREATE&255))
#define	CND_CHOOSEFLAGSET 	((-21<<8)|(OBJ_CREATE&255))
#define	CND_CHOOSEVALUE 	((-20<<8)|(OBJ_CREATE&255))
#define	CND_PICKFROMID		((-19<<8)|(OBJ_CREATE&255))
#define	CND_CHOOSEALLINZONE ((-18<<8)|(OBJ_CREATE&255))
#define	CND_CHOOSEALL       ((-17<<8)|(OBJ_CREATE&255))
#define	CND_CHOOSEZONE      ((-16<<8)|(OBJ_CREATE&255))
#define	CND_NUMOFALLOBJECT  ((-15<<8)|(OBJ_CREATE&255))
#define	CND_NUMOFALLZONE    ((-14<<8)|(OBJ_CREATE&255))
#define	CND_NOMOREALLZONE   ((-13<<8)|(OBJ_CREATE&255))
#define	CND_CHOOSEFLAGRESET_OLD	((-12<<8)|(OBJ_CREATE&255))
#define	CND_CHOOSEFLAGSET_OLD 	((-11<<8)|(OBJ_CREATE&255))
//...
#define	CND_CHOOSEVALUE_OLD 	((-8<<8)|(OBJ_CREATE&255))
#define	CND_PICKFROMID_OLD		((-7<<8)|(OBJ_CREATE&255))
#define	CND_CHOOSEALLINZONE_OLD ((-6<<8)|(OBJ_CREATE&255))
#define	CND_CHOOSEALL_OLD       ((-5<<8)|(OBJ_CREATE&255))
#define	CND_CHOOSEZONE_OLD      ((-4<<8)|(OBJ_CREATE&255))
#define	CND_NUMOFALLOBJECT_OLD  ((-3<<8)|(OBJ_CREATE&255))
#define	CND_NUMOFALLZONE_OLD    ((-2<<8)|(OBJ_CREATE&255))
#define	CND_NOMOREALLZONE_OLD   ((-1<<8)|(OBJ_CREATE&255))
#define	ACT_CREATE				((0<<8)|(OBJ_CREATE&255))
#define	EXP_CRENUMBERALL		((0<<8)|(OBJ_CREATE&255))
#define	NUM_END				-2
#define	NUM_START			-1

#endif
           
// BALL Movements
// ~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct	tagMB {
	WORD	mbSpeed;
	WORD	mbBounce;
	WORD	mbAngles;
	WORD	mbSecurity;
	WORD	mbDecelerate;
	WORD	mbFree;
	} MoveBall;
typedef	MoveBall * LPMOVEBALL;

// MOUSE movement
// ~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct	tagMM {
	short		mmDx;      				
	short		mmFx;
	short		mmDy;
	short		mmFy;
	short		mmFlags;
	} MoveMouse;
typedef	MoveMouse * LPMOVEMOUSE;

#define MOUSE_XCENTER		100
#define MOUSE_YCENTER		100
#define	MMMODE_NORMAL		0
#define	MMMODE_HIDDEN		1
#define	MMMODE_REMOVED		2

// 8 Direction movement
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct	tagMG {
	WORD	mgSpeed;				// Max speed
	WORD	mgAcc;					// Acceleration
	WORD	mgDec;					// Deceleration
	WORD	mgBounceMult;			// Bounce factor
	DWORD	mgDir;					// Allowed directions
	} MoveGeneric;
typedef	MoveGeneric	* LPMOVEGENERIC;

// RACE-CAR movement
// ~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct	tagMR {
	WORD	mrSpeed;				// Max speed
	WORD	mrAcc;					// Acceleration
	WORD	mrDec;					// Deceleration
	WORD	mrRot;					// Rotating speed
	WORD	mrBounceMult;			// Bounce factor
	WORD	mrAngles;				// Number of angles
	WORD	mrOkReverse;			// Allowed reverse speed?
	} MoveRace;
typedef	MoveRace	* LPMOVERACE;

// Path movement
// ~~~~~~~~~~~~~~~~~~~~~~~~~
// Structure for one move
typedef struct tagMDV1 {
	BYTE	mdSpeed;
	BYTE	mdDir;
	short	mdDx;
	short	mdDy;
	short	mdCosinus;
	short	mdSinus;
	short	mdLength;
	short	mdPause;
	} MoveDefV1;
typedef	MoveDefV1	* LPMOVEDEFV1;

typedef struct tagMD {
	BYTE	mdPrevious;
	BYTE	mdNext;
	BYTE	mdSpeed;
	BYTE	mdDir;
	short	mdDx;
	short	mdDy;
	short	mdCosinus;
	short	mdSinus;
	short	mdLength;
	short	mdPause;
	char	mdName[2];
	} MoveDef;
typedef	MoveDef	* LPMOVEDEF;

typedef struct tagMT {
	WORD	mtNumber;				// Number of movement 
	WORD	mtMinSpeed; 			// maxs and min speed in the movements 
	WORD	mtMaxSpeed;
	BYTE	mtLoop;					// Loop at end
	BYTE	mtRepos;				// Reposition at end
	BYTE	mtReverse;				// Pingpong?
	BYTE	mtFree;
	BYTE	mtMoves[2];				// Start of movement definition
	} MoveTaped;
typedef	MoveTaped	* LPMOVETAPED;

// Movement edition
typedef struct tagME {
	int		meSpeed;
	int		meDx;
	int		meDy;
	int		meFlag;
	int		meX;
	int		meY;
	int		meDelay;
	char	*meName;
	} MoveEdit;
typedef	MoveEdit * LPMOVEEDIT;

typedef struct tagMA {
	MoveEdit	me[1];
	}MoveArray;
#define	sizeof_MoveArray		(sizeof(MoveArray)-sizeof(MoveEdit))
typedef	MoveArray * LPMOVEARRAY;

// Bloc movement structure
typedef struct tagMBLOC {
	short		pbNMoves;
	short		pbData[2];
	} PathBloc;
typedef PathBloc  *  LPPATHBLOC;

// PLATFORM mouvement 
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct	tagMP {
	WORD	mpSpeed;				// Max speed
	WORD	mpAcc;					// Acceleration
	WORD	mpDec;					// Deceleration
	WORD	mpJumpControl;			// Bounce factor
	WORD	mpGravity;
	WORD	mpJump;
	} MovePlatform;
typedef	MovePlatform *	LPMOVEPLATFORM;

#define	MPJC_NOJUMP		0
#define	MPJC_DIAGO		1
#define	MPJC_BUTTON1	2
#define	MPJC_BUTTON2	3

// Movement Extension structure
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct	MoveExt {
	CMvt*	meMvt;				// Object
} MoveExt;
typedef	MoveExt *	LPMOVEEXT;

// Movement Structure
// ~~~~~~~~~~~~~~~~~~
typedef struct MvtHdr {
	DWORD	mvhModuleNameOffset;
	DWORD	mvhID;
	DWORD	mvhDataOffset;
	DWORD	mvhDataSize;
} MvtHdr;

typedef struct MovementTable
{
	DWORD	mvtNMvt;				// Number of movements
	MvtHdr	mvtHdr[1];
} MovementTable;
typedef MovementTable * LPMVTTABLE;

typedef struct 	tagMV {
	WORD	mvControl;				// Who controls the object
	WORD	mvType;					// Type of movement
	BYTE	mvMove;					// Move at start?
	BYTE	mvFree0;
	BYTE	mvFree1;
	BYTE	mvFree2;
	DWORD	mvDirAtStart;
	union	{
			MoveMouse		mm;
			MoveGeneric		mg;
			MoveBall	  	mb;
			MoveTaped		mt;
			MoveRace		mr;
			MovePlatform	mp;
			MoveExt			me;
	} mu;
} Movement;
typedef	Movement	*	LPMOVEMENT;

#ifndef IN_KPX
           
// COMMON CONDITIONS FOR NORMAL OBJECTS
//////////////////////////////////////////
#define	EVENTS_EXTBASE				80

#define	CND_EXTISSTRIKEOUT			(-40<<8)
#define CND_EXTISUNDERLINE			(-39<<8)
#define CND_EXTISITALIC				(-38<<8)
#define CND_EXTISBOLD				(-37<<8)
#define	CND_EXTCMPVARSTRING			(-36<<8)
#define	CNDL_EXTCMPVARSTRING		(-36<<16)
#define	CND_EXTPATHNODENAME			(-35<<8)
#define	CNDL_EXTPATHNODENAME		(-35<<16)
#define	CND_EXTCHOOSE				(-34<<8)
#define	CND_EXTNOMOREOBJECT			(-33<<8)
#define	CNDL_EXTNOMOREOBJECT		(-33<<16)
#define	CND_EXTNUMOFOBJECT			(-32<<8)
#define	CND_EXTNOMOREZONE			(-31<<8)
#define	CND_EXTNUMBERZONE			(-30<<8)
#define	CND_EXTSHOWN				(-29<<8)
#define	CND_EXTHIDDEN				(-28<<8)
#define	CND_EXTCMPVAR				(-27<<8)
#define	CNDL_EXTCMPVAR				(-27<<16)
#define	CND_EXTCMPVARFIXED			(-26<<8)
#define	CND_EXTFLAGSET				(-25<<8)
#define	CND_EXTFLAGRESET			(-24<<8)
#define	CND_EXTISCOLBACK	        (-23<<8)
#define	CND_EXTNEARBORDERS	        (-22<<8)
#define	CND_EXTENDPATH	  	        (-21<<8)
#define	CNDL_EXTENDPATH	  	        (-21<<16)
#define	CND_EXTPATHNODE    	        (-20<<8)
#define	CNDL_EXTPATHNODE    	    (-20<<16)
#define	CND_EXTCMPACC	            (-19<<8)
#define	CND_EXTCMPDEC	 	        (-18<<8)
#define	CND_EXTCMPX	 	  	        (-17<<8)
#define	CND_EXTCMPY   		        (-16<<8)
#define	CND_EXTCMPSPEED             (-15<<8)
#define	CND_EXTCOLLISION   	        (-14<<8)
#define	CNDL_EXTCOLLISION   	    (-14<<16)
#define	CND_EXTCOLBACK              (-13<<8)
#define	CNDL_EXTCOLBACK             (-13<<16)
#define	CND_EXTOUTPLAYFIELD         (-12<<8)
#define	CNDL_EXTOUTPLAYFIELD        (-12<<16)
#define	CND_EXTINPLAYFIELD          (-11<<8)
#define	CNDL_EXTINPLAYFIELD         (-11<<16)
#define	CND_EXTISOUT	            (-10<<8)
#define	CNDL_EXTISOUT	            (-10<<16)
#define	CND_EXTISIN                 (-9 <<8)
#define	CNDL_EXTISIN                (-9 <<16)
#define	CND_EXTFACING               (-8 <<8)
#define	CND_EXTSTOPPED              (-7 <<8)
#define	CND_EXTBOUNCING	            (-6 <<8)
#define	CND_EXTREVERSED             (-5 <<8)
#define	CND_EXTISCOLLIDING          (-4 <<8)
#define	CNDL_EXTISCOLLIDING         (-4 <<16)
#define	CND_EXTANIMPLAYING          (-3 <<8)
#define	CND_EXTANIMENDOF        	(-2 <<8)
#define	CNDL_EXTANIMENDOF        	(-2 <<16)
#define	CND_EXTCMPFRAME     		(-1 <<8)

#define	ACT_EXTSETPOS		        (1 <<8)
#define	ACT_EXTSETX			        (2 <<8)
#define	ACT_EXTSETY			        (3 <<8)
#define	ACT_EXTSTOP			        (4 <<8)
#define	ACTL_EXTSTOP		        (4 <<16)
#define	ACT_EXTSTART		        (5 <<8)
#define	ACT_EXTSPEED		        (6 <<8)
#define	ACT_EXTMAXSPEED		        (7 <<8)
#define	ACT_EXTWRAP			        (8 <<8)
#define	ACTL_EXTWRAP		        (8 <<16)
#define	ACT_EXTBOUNCE		        (9 <<8)
#define	ACTL_EXTBOUNCE		        (9 <<16)
#define	ACT_EXTREVERSE		        (10<<8)
#define	ACT_EXTNEXTMOVE		        (11<<8)
#define	ACT_EXTPREVMOVE		        (12<<8)
#define	ACT_EXTSELMOVE		        (13<<8)
#define	ACT_EXTLOOKAT		        (14<<8)
#define	ACT_EXTSTOPANIM		        (15<<8)
#define	ACT_EXTSTARTANIM	        (16<<8)
#define	ACT_EXTFORCEANIM	        (17<<8)
#define	ACT_EXTFORCEDIR		        (18<<8)
#define	ACT_EXTFORCESPEED	        (19<<8)
#define	ACT_EXTRESTANIM		        (20<<8)         
#define	ACT_EXTRESTDIR		        (21<<8)
#define	ACT_EXTRESTSPEED	        (22<<8)
#define	ACT_EXTSETDIR				(23<<8)
#define	ACT_EXTDESTROY				(24<<8)
#define	ACT_EXTSHUFFLE				(25<<8)
#define	ACTL_EXTSHUFFLE				(25<<16)
#define	ACT_EXTHIDE					(26<<8)
#define	ACT_EXTSHOW					(27<<8)
#define	ACT_EXTDISPLAYDURING		(28<<8)
#define	ACT_EXTSHOOT				(29<<8)
#define	ACTL_EXTSHOOT				(29<<16)
#define	ACT_EXTSHOOTTOWARD			(30<<8)
#define	ACTL_EXTSHOOTTOWARD			(30<<16)
#define	ACT_EXTSETVAR				(31<<8)
#define	ACTL_EXTSETVAR				(31<<16)
#define	ACT_EXTADDVAR				(32<<8)
#define	ACTL_EXTADDVAR				(32<<16)
#define	ACT_EXTSUBVAR				(33<<8)
#define	ACTL_EXTSUBVAR				(33<<16)
#define	ACT_EXTDISPATCHVAR			(34<<8)
#define	ACTL_EXTDISPATCHVAR			(34<<16)
#define	ACT_EXTSETFLAG				(35<<8)
#define	ACT_EXTCLRFLAG				(36<<8)
#define	ACT_EXTCHGFLAG				(37<<8)
#define	ACT_EXTINKEFFECT			(38<<8)
#define ACT_EXTSETSEMITRANSPARENCY	(39<<8)
#define ACT_EXTFORCEFRAME			(40<<8)
#define ACT_EXTRESTFRAME			(41<<8)
#define ACT_EXTSETACCELERATION		(42<<8)
#define ACT_EXTSETDECELERATION		(43<<8)
#define ACT_EXTSETROTATINGSPEED		(44<<8)
#define ACT_EXTSETDIRECTIONS		(45<<8)
#define ACT_EXTBRANCHNODE			(46<<8)
#define ACT_EXTSETGRAVITY			(47<<8)
#define ACT_EXTGOTONODE				(48<<8)
#define	ACT_EXTSETVARSTRING			(49<<8)
#define	ACTL_EXTSETVARSTRING		(49<<16)
#define ACT_EXTSETFONTNAME			(50<<8)
#define ACT_EXTSETFONTSIZE			(51<<8)
#define ACT_EXTSETBOLD				(52<<8)
#define ACT_EXTSETITALIC			(53<<8)
#define ACT_EXTSETUNDERLINE			(54<<8)
#define	ACT_EXTSETSRIKEOUT			(55<<8)
#define	ACT_EXTSETTEXTCOLOR			(56<<8)
#define ACT_EXTSPRFRONT				(57<<8)
#define ACT_EXTSPRBACK				(58<<8)
#define	ACT_EXTMOVEBEFORE			(59<<8)
#define	ACT_EXTMOVEAFTER			(60<<8)
#define ACT_EXTMOVETOLAYER			(61<<8)
#define ACT_EXTADDTODEBUGGER		(62<<8)

#define	EXP_EXTYSPR        		    ( 1<<8)
#define	EXP_EXTISPR        		    ( 2<<8)
#define	EXP_EXTSPEED       		    ( 3<<8)
#define	EXP_EXTACC         		    ( 4<<8)
#define	EXP_EXTDEC         		    ( 5<<8)
#define	EXP_EXTDIR					( 6<<8)
#define	EXP_EXTXLEFT				( 7<<8)
#define	EXP_EXTXRIGHT				( 8<<8)
#define	EXP_EXTYTOP					( 9<<8)
#define	EXP_EXTYBOTTOM				(10<<8)
#define	EXP_EXTXSPR					(11<<8)
#define	EXP_EXTIDENTIFIER			(12<<8)
#define	EXP_EXTFLAG					(13<<8)
#define EXP_EXTNANI					(14<<8)
#define	EXP_EXTNOBJECTS				(15<<8)
#define	EXP_EXTVAR					(16<<8)
#define	EXPL_EXTVAR					(16<<16)
#define EXP_EXTGETSEMITRANSPARENCY	(17<<8)
#define EXP_EXTNMOVE				(18<<8)
#define	EXP_EXTVARSTRING			(19<<8)
#define	EXPL_EXTVARSTRING			(19<<16)
#define EXP_EXTGETFONTNAME			(20<<8)
#define EXP_EXTGETFONTSIZE			(21<<8)
#define EXP_EXTGETFONTCOLOR			(22<<8)
#define EXP_EXTGETLAYER				(23<<8)
#define EXP_EXTGETGRAVITY			(24<<8)
#define EXP_EXTXAP					(25<<8)
#define EXP_EXTYAP					(26<<8)

// TEXT Conditions / Actions 
////////////////////////////////////////////
#define	ACT_STRDESTROY	  	 (( (EVENTS_EXTBASE+0)<<8)|3)
#define	ACT_STRDISPLAY		 (( (EVENTS_EXTBASE+1)<<8)|3)
#define	ACT_STRDISPLAYDURING (( (EVENTS_EXTBASE+2)<<8)|3)
#define	ACT_STRSETCOLOUR     (( (EVENTS_EXTBASE+3)<<8)|3)
#define	ACT_STRSET	   		 (( (EVENTS_EXTBASE+4)<<8)|3)
#define	ACT_STRPREV	   		 (( (EVENTS_EXTBASE+5)<<8)|3)
#define	ACT_STRNEXT	   		 (( (EVENTS_EXTBASE+6)<<8)|3)
#define ACT_STRDISPLAYSTRING (( (EVENTS_EXTBASE+7)<<8)|3)
#define	ACT_STRSETSTRING	 (( (EVENTS_EXTBASE+8)<<8)|3)
#define	EXP_STRNUMBER		 (( (EVENTS_EXTBASE+0)<<8)|3)
#define	EXP_STRGETCURRENT	 (( (EVENTS_EXTBASE+1)<<8)|3)
#define	EXP_STRGETNUMBER	 (( (EVENTS_EXTBASE+2)<<8)|3)
#define	EXP_STRGETNUMERIC	 (( (EVENTS_EXTBASE+3)<<8)|3)
#define EXP_STRGETNPARA		 (( (EVENTS_EXTBASE+4)<<8)|3)

// SPRITES Conditions / Actions 
/////////////////////////////////////////////
#define	CND_SPRCLICK	   	(((-EVENTS_EXTBASE-1)<<8)|2)
#define	ACT_SPRPASTE		(((EVENTS_EXTBASE+0)<<8)|2)
#define	ACT_SPRFRONT		(((EVENTS_EXTBASE+1)<<8)|2)
#define	ACT_SPRBACK			(((EVENTS_EXTBASE+2)<<8)|2)
#define	ACT_SPRADDBKD		(((EVENTS_EXTBASE+3)<<8)|2)
#define ACT_SPRREPLACECOLOR	(((EVENTS_EXTBASE+4)<<8)|2)
#define ACT_SPRSETSCALE		(((EVENTS_EXTBASE+5)<<8)|2)
#define ACT_SPRSETSCALEX	(((EVENTS_EXTBASE+6)<<8)|2)
#define ACT_SPRSETSCALEY	(((EVENTS_EXTBASE+7)<<8)|2)
#define ACT_SPRSETANGLE		(((EVENTS_EXTBASE+8)<<8)|2)
#define ACT_SPRLOADFRAME	(((EVENTS_EXTBASE+9)<<8)|2)
#define EXP_GETRGBAT		(((EVENTS_EXTBASE+0)<<8)|2)
#define EXP_GETSCALEX		(((EVENTS_EXTBASE+1)<<8)|2)
#define EXP_GETSCALEY		(((EVENTS_EXTBASE+2)<<8)|2)
#define EXP_GETANGLE		(((EVENTS_EXTBASE+3)<<8)|2)

// COUNTERS Conditions / Actions 
//////////////////////////////////////////////
#define	CND_CCOUNTER	   	(((-EVENTS_EXTBASE-1)<<8)|7)
#define	ACT_CSETVALUE		(( (EVENTS_EXTBASE+0)<<8)|7)
#define	ACT_CADDVALUE		(( (EVENTS_EXTBASE+1)<<8)|7)
#define	ACT_CSUBVALUE		(( (EVENTS_EXTBASE+2)<<8)|7)
#define	ACT_CSETMIN			(( (EVENTS_EXTBASE+3)<<8)|7)
#define	ACT_CSETMAX			(( (EVENTS_EXTBASE+4)<<8)|7)
#define	ACT_CSETCOLOR1		(( (EVENTS_EXTBASE+5)<<8)|7)
#define	ACT_CSETCOLOR2		(( (EVENTS_EXTBASE+6)<<8)|7)
#define	EXP_CVALUE			(( (EVENTS_EXTBASE+0)<<8)|7)
#define	EXP_CGETMIN			(( (EVENTS_EXTBASE+1)<<8)|7)
#define	EXP_CGETMAX			(( (EVENTS_EXTBASE+2)<<8)|7)
#define	EXP_CGETCOLOR1		(( (EVENTS_EXTBASE+3)<<8)|7)
#define	EXP_CGETCOLOR2		(( (EVENTS_EXTBASE+4)<<8)|7)

// QUESTIONS Conditions / Actions 
///////////////////////////////////////////////
#define	CND_QEQUAL			(((-EVENTS_EXTBASE-3)<<8)|4)
#define	CNDL_QEQUAL			(((-EVENTS_EXTBASE-3)<<16)|4)
#define	CND_QFALSE		    (((-EVENTS_EXTBASE-2)<<8)|4)
#define	CNDL_QFALSE		    (((-EVENTS_EXTBASE-2)<<16)|4)
#define	CND_QEXACT		    (((-EVENTS_EXTBASE-1)<<8)|4)
#define	CNDL_QEXACT		    (((-EVENTS_EXTBASE-1)<<16)|4)
#define	ACT_QASK		    (( (EVENTS_EXTBASE+0)<<8)|4)


// Formatted text Conditions / actions / expressions 
//////////////////////////////////////////////////////////////////
#define	ACT_RTFSETXPOS					(((EVENTS_EXTBASE+0)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSETYPOS					(((EVENTS_EXTBASE+1)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSETZOOM					(((EVENTS_EXTBASE+2)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_CLEAR				(((EVENTS_EXTBASE+3)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_WORDSTRONCE		(((EVENTS_EXTBASE+4)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_WORDSTRNEXT		(((EVENTS_EXTBASE+5)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_WORDSTRALL		(((EVENTS_EXTBASE+6)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_WORD				(((EVENTS_EXTBASE+7)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_LINE				(((EVENTS_EXTBASE+8)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_PARAGRAPH			(((EVENTS_EXTBASE+9)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_PAGE				(((EVENTS_EXTBASE+10)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_ALL				(((EVENTS_EXTBASE+11)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_RANGE				(((EVENTS_EXTBASE+12)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_BOOKMARK			(((EVENTS_EXTBASE+13)<<8)|(OBJ_RTF&0x00FF))
#define ACT_RTFSETFOCUSWORD				(((EVENTS_EXTBASE+14)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHT_OFF				(((EVENTS_EXTBASE+15)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTTEXT_COLOR  		(((EVENTS_EXTBASE+16)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTTEXT_BOLD	  		(((EVENTS_EXTBASE+17)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTTEXT_ITALIC 		(((EVENTS_EXTBASE+18)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTTEXT_UNDERL 		(((EVENTS_EXTBASE+19)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTTEXT_OUTL	  		(((EVENTS_EXTBASE+20)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTBACK_COLOR  		(((EVENTS_EXTBASE+21)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTBACK_RECT	  		(((EVENTS_EXTBASE+22)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTBACK_MARKER 		(((EVENTS_EXTBASE+23)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTBACK_HATCH  		(((EVENTS_EXTBASE+24)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTBACK_INVERSE		(((EVENTS_EXTBASE+25)<<8)|(OBJ_RTF&0x00FF))
#define	ACT_RTFDISPLAY					(((EVENTS_EXTBASE+26)<<8)|(OBJ_RTF&0x00FF))
#define ACT_RTFSETFOCUSPREV				(((EVENTS_EXTBASE+27)<<8)|(OBJ_RTF&0x00FF))
#define ACT_RTFSETFOCUSNEXT				(((EVENTS_EXTBASE+28)<<8)|(OBJ_RTF&0x00FF))
#define ACT_RTFREMOVEFOCUS				(((EVENTS_EXTBASE+29)<<8)|(OBJ_RTF&0x00FF))
#define ACT_RTFAUTOON					(((EVENTS_EXTBASE+30)<<8)|(OBJ_RTF&0x00FF))
#define ACT_RTFAUTOOFF					(((EVENTS_EXTBASE+31)<<8)|(OBJ_RTF&0x00FF))
#define ACT_RTFINSERTSTRING				(((EVENTS_EXTBASE+32)<<8)|(OBJ_RTF&0x00FF))
#define ACT_RTFLOADTEXT					(((EVENTS_EXTBASE+33)<<8)|(OBJ_RTF&0x00FF))
#define ACT_RTFINSERTTEXT				(((EVENTS_EXTBASE+34)<<8)|(OBJ_RTF&0x00FF))


#define	EXP_RTFXPOS						(((EVENTS_EXTBASE+0)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYPOS						(((EVENTS_EXTBASE+1)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSXPAGE					(((EVENTS_EXTBASE+2)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSYPAGE					(((EVENTS_EXTBASE+3)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFZOOM						(((EVENTS_EXTBASE+4)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFWORDMOUSE				(((EVENTS_EXTBASE+5)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFWORDXY					(((EVENTS_EXTBASE+6)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFWORD						(((EVENTS_EXTBASE+7)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXWORD					(((EVENTS_EXTBASE+8)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYWORD					(((EVENTS_EXTBASE+9)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSXWORD					(((EVENTS_EXTBASE+10)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSYWORD					(((EVENTS_EXTBASE+11)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFLINEMOUSE				(((EVENTS_EXTBASE+12)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFLINEXY					(((EVENTS_EXTBASE+13)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXLINE					(((EVENTS_EXTBASE+14)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYLINE					(((EVENTS_EXTBASE+15)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSXLINE					(((EVENTS_EXTBASE+16)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSYLINE					(((EVENTS_EXTBASE+17)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFPARAMOUSE				(((EVENTS_EXTBASE+18)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFPARAXY					(((EVENTS_EXTBASE+19)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXPARA					(((EVENTS_EXTBASE+20)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYPARA					(((EVENTS_EXTBASE+21)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSXPARA					(((EVENTS_EXTBASE+22)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSYPARA					(((EVENTS_EXTBASE+23)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXWORDTEXT				(((EVENTS_EXTBASE+24)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYWORDTEXT				(((EVENTS_EXTBASE+25)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXLINETEXT				(((EVENTS_EXTBASE+26)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYLINETEXT				(((EVENTS_EXTBASE+27)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXPARATEXT				(((EVENTS_EXTBASE+28)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYPARATEXT				(((EVENTS_EXTBASE+29)<<8)|(OBJ_RTF&0x00FF))
#define	EXP_RTFMEMSIZE					(((EVENTS_EXTBASE+30)<<8)|(OBJ_RTF&0x00FF))
#define EXP_RTFGETFOCUSWORD				(((EVENTS_EXTBASE+31)<<8)|(OBJ_RTF&0x00FF))
#define EXP_RTFGETHYPERLINK				(((EVENTS_EXTBASE+32)<<8)|(OBJ_RTF&0x00FF))

#define CND_CCAISPAUSED					(((-EVENTS_EXTBASE-4)<<8)|(OBJ_CCA&0x00FF))
#define CND_CCAISVISIBLE				(((-EVENTS_EXTBASE-3)<<8)|(OBJ_CCA&0x00FF))
#define CND_CCAAPPFINISHED				(((-EVENTS_EXTBASE-2)<<8)|(OBJ_CCA&0x00FF))
#define CNDL_CCAAPPFINISHED				(((-EVENTS_EXTBASE-2)<<16)|(OBJ_CCA&0xFFFF))
#define CND_CCAFRAMECHANGED				(((-EVENTS_EXTBASE-1)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCARESTARTAPP				(((EVENTS_EXTBASE+0)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCARESTARTFRAME				(((EVENTS_EXTBASE+1)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCANEXTFRAME				(((EVENTS_EXTBASE+2)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCAPREVIOUSFRAME			(((EVENTS_EXTBASE+3)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCAENDAPP					(((EVENTS_EXTBASE+4)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCANEWAPP					(((EVENTS_EXTBASE+5)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCAJUMPFRAME				(((EVENTS_EXTBASE+6)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCASETGLOBALVALUE			(((EVENTS_EXTBASE+7)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCASHOW						(((EVENTS_EXTBASE+8)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCAHIDE						(((EVENTS_EXTBASE+9)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCASETGLOBALSTRING			(((EVENTS_EXTBASE+10)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCAPAUSEAPP					(((EVENTS_EXTBASE+11)<<8)|(OBJ_CCA&0x00FF))
#define ACT_CCARESUMEAPP				(((EVENTS_EXTBASE+12)<<8)|(OBJ_CCA&0x00FF))
#define EXP_CCAGETFRAMENUMBER			(((EVENTS_EXTBASE+0)<<8)|(OBJ_CCA&0x00FF))
#define EXP_CCAGETGLOBALVALUE			(((EVENTS_EXTBASE+1)<<8)|(OBJ_CCA&0x00FF))
#define EXP_CCAGETGLOBALSTRING			(((EVENTS_EXTBASE+2)<<8)|(OBJ_CCA&0x00FF))

#endif // IN_KPX

///////////////////////////////////////////////////////////////////////
//
// DEFINITION OF THE DIFFERENT PARAMETERS
//
///////////////////////////////////////////////////////////////////////

// -------------------------------- Objects
// W-Offset list OI
// W-Number 
// W-TYPE *** Version > FVERSION_NEWOBJECTS
#define		PARAM_OBJECT			1
#define		PS_OBJ					6

// -------------------------------- Time
// L-Timer
// L-Equivalent loops
#define		PARAM_TIME				2
#define		PS_TIM					8

// -------------------------------- Border
// W-border
#define		PARAM_BORDER			3
#define		PS_BOR					2
#define		BORDER_LEFT				1
#define		BORDER_RIGHT			2
#define		BORDER_TOP				4
#define		BORDER_BOTTOM			8
#define		BORDER_ALL				15

// -------------------------------- Direction
// W-direction
#define		PARAM_DIRECTION			4
#define		PS_DIR					2

// -------------------------------- Integer
// W-shorteger (or base)
// W-nothing (or maximum)
#define		PARAM_INTEGER			5
#define		PS_INT					4

// -------------------------------- Sample
#define		MAX_SOUNDNAME				64
#define		PSOUNDFLAG_UNINTERRUPTABLE	0x0001
#define		PSOUNDFLAG_BAD				0x0002

typedef struct tagSSND {
	short	sndHandle;
	short	sndFlags;
	char	sndName[MAX_SOUNDNAME];
	} SoundParam;
typedef		SoundParam *		LPSND;

#define		PARAM_SAMPLE			6
#define		PS_SAM					sizeof(SoundParam)

// -------------------------------- Music
#define		PARAM_MUSIC				7
#define		PS_MUS					sizeof(SoundParam)


// POSITION PARAM Structure 
// ~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct tagPPA {
	short		posOINUMParent;			//0
	short		posFlags;
	short		posX;					//4
	short		posY;
	short		posSlope;				//8
	short		posAngle;
	long		posDir;					//12
	ITEMTYPE	posTypeParent;			//16
	short		posOiList;				//18
	short		posLayer;				//20
	} PositionParam;
typedef	PositionParam *			 	LPPOS;

// CREATE PARAM Structure 
// ~~~~~~~~~~~~~~~~~~~~~~
typedef struct tagCDP {
	PositionParam   cdpPos;			// Position structure
	HFII			cdpHFII;		// FrameItemInstance number
	OINUM			cdpOi;			// OI of the object to create
//	DWORD  			cdpFII;			
	DWORD			cdpFree;
	} CreateDuplicateParam;
typedef	CreateDuplicateParam * 		LPCDP;

// SHOOT PARAM Structure, should be identical to CREATE PARAM !
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct tagSHT {
	CreateDuplicateParam	shtCdp;
	short					shtSpeed;		// Speed
	} ShootParam;
typedef	ShootParam 	* 					LPSHT;

#define		CPF_DIRECTION			0x0001
#define		CPF_ACTION				0x0002
#define		CPF_INITIALDIR			0x0004
#define		CPF_DEFAULTDIR			0x0008


// -------------------------------- Create
#define		PARAM_CREATE			9
#define		PS_CRE					sizeof(CreateDuplicateParam)

// -------------------------------- Animation
// W-Number
// B-Name
#define		PARAM_ANIMATION			10
#define		PS_ANI					32

// -------------------------------- Nothing
// W-
#define		PARAM_NOP				11
#define		PS_NOP					2

// -------------------------------- Player
// W-
#define		PARAM_PLAYER			12
#define		PS_PLA					2

// -------------------------------- Every
// L-delay
// L-compteur
#define		PARAM_EVERY				13
#define		PS_EVE					8

// -------------------------------- Virtual Key
// W- VK_Code
#define		PARAM_KEY				14
#define		PS_KEY					2

// -------------------------------- Speed= integer
// Expression
#define		PARAM_SPEED				15

// -------------------------------- Position
// W- Direction
#define		PARAM_POSITION	  		16
#define		PS_POS				   	sizeof(PositionParam)

// -------------------------------- Joystick Direcion 
// W- Direction
#define		PARAM_JOYDIRECTION 		17
#define		PS_JOY				   	2

// -------------------------------- Shoot param

#define		PARAM_SHOOT				18
#define		PS_SHT				   	sizeof(ShootParam)

// -------------------------------- Playfield Zone 
#define		PARAM_ZONE				19
#define		PS_ZNE					8
                     
// -------------------------------- System object position
// W- Direction
#define		PARAM_SYSCREATE		   	21
#define		PS_SSC  			   	sizeof(CreateDuplicateParam)

// -------------------------------- Expression
// W0- Id parametre originel
// W1- 0 (au moins!)
#define		PARAM_EXPRESSION	   	22
#define		PS_EXP  			   	14

// -------------------------------- Comparaison
// W0- Id comparaison
// W1- 0 (au moins!)
#define		PARAM_COMPARAISON	   	23

// -------------------------------- Text color 
// L0- RGB
// L0- Identificateur!
#define		PARAM_COLOUR		   	24
#define		PS_COL  			   	8

// -------------------------------- Work buffer
// L0-
#define		PARAM_BUFFER4		   	25
#define		PS_BU4  			   	4

// -------------------------------- Storyboard frame number 
// W0-
#define		PARAM_FRAME			   	26
#define		PS_FRA  			   	2

// -------------------------------- Number of loops for a sample
#define		PARAM_SAMLOOP		   	27
#define		PS_SLOOP				2
// -------------------------------- Number of loops for a music 
#define		PARAM_MUSLOOP		   	28
#define		PS_MLOOP				2
// -------------------------------- Direction
#define		PARAM_NEWDIRECTION	   	29
#define		PS_NDIR					4

// -------------------------------- Text number
#define		PARAM_TEXTNUMBER	 	31
#define		PS_TXN					4

// -------------------------------- Click definition
#define		PARAM_CLICK			 	32
#define		PS_KLK					4
#define		PARAMCLICK_MASK			0x00FF
#define		PARAMCLICK_DOUBLE		0x0100

// -------------------------------- External program
#define		PARAM_PROGRAM			33
typedef struct tagPRG {
	short	prgFlags;				// Default flags
	char	prgPath[_MAX_PATH];		// Name of the program
	char	prgCommand[108];		// Command line
	} prgParam;
typedef		prgParam *			LPPRG;	

#define		PS_PRG					sizeof(prgParam)
#define		PRGFLAGS_WAIT			0x0001                                                    
#define		PRGFLAGS_HIDE			0x0002

// -------------------------------- Global variable number 
#define		OLDPARAM_VARGLO			34
//#define		PS_VGLO					4

// -------------------------------- Condition sample (no flags)
// W-Number
// W-Flags
// W-Loops
// B-Name
#define		PARAM_CNDSAMPLE			35
// -------------------------------- Condition Music (no flags)
// W-Number
// W-Flags
// W-Loops
// B-Name
#define		PARAM_CNDMUSIC			36
// -------------------------------- Event editor remark
#define		PARAM_REMARK			37
typedef struct tagREM {
	LOGFONTV1	remLogFont;				// Font 
	COLORREF	remColorFont;			// Text color
	COLORREF	remColorBack;			// Background color
	short		remAlign;				// Alignement flags
	WORD		remTextId;				// Text number in the buffer
	char		remStyle[40];			// Style
	} paramRemark;
#define		PS_REM				sizeof(paramRemark)
typedef		paramRemark *		LPRMK;	

// -------------------------------- Group title
#define		PARAM_GROUP				38
#define		GROUP_MAXTITLE			80
#define		GROUP_MAXPASSWORD		16
typedef struct tagGRP {
	short		grpFlags;					// Active / Unactive?
	short		grpId;						// Group identifier
	char		grpTitle[GROUP_MAXTITLE];	// Title
	char		grpPassword[GROUP_MAXPASSWORD];		// Protection 
	DWORD		grpChecksum;				// Checksum
	} paramGroup;
#define		PS_GRP				sizeof(paramGroup)
typedef struct tagOLDGRP {
	short		grpFlags;					
	short		grpId;					
	char		grpTitle[GROUP_MAXTITLE];
	} paramOldGroup;
#define		PS_OLDGRP			sizeof(paramOldGroup)
typedef		paramGroup *		LPGRP;	

#define		GRPFLAGS_INACTIVE		0x0001
#define		GRPFLAGS_CLOSED			0x0002
#define		GRPFLAGS_PARENTINACTIVE	0x0004
#define		GRPFLAGS_GROUPINACTIVE	0x0008
//#define		GRPFLAGS_FADEIN			0x0004
//#define		GRPFLAGS_FADEOUT		0x0008
#define		GRPFLAGS_GLOBAL			0x0010
// Pour l'assemblage
typedef struct tagGRPLIST {
	DWORD		glEvg;
	short		glId;
	} groupList;
typedef		groupList *		LPGL;	

// -------------------------------- A pointer to a group
#define		PARAM_GROUPOINTER		39
#define		PS_GPT					6
// L - Offset dans programme
// W - Identifier

// -------------------------------- A pointner to a filename
#define		PARAM_FILENAME			40
#define		PS_FILEN				_MAX_PATH
// B nom de fichier...

// -------------------------------- String pointer
#define		PARAM_STRING			41
#define		PS_STR					2
// B string

// -------------------------------- Time
// L-Timer
// L-Loops
// W-Comparaison
#define		PARAM_CMPTIME			42
#define		PS_CTIM					10

// --------------------------------- PASTE SPRITE
// W- Flags
// W- Security
#define		PARAM_PASTE				43
#define		PS_PASTE				4

// -------------------------------- MOUSEKEY VIRTUAL
// W- VK_Code
#define		PARAM_VMKEY				44
#define		PS_VMKEY				2

// -------------------------------- String expression
// W0- Id original parameter
// W1- 0 (at least!)
#define		PARAM_EXPSTRING		   	45

// -------------------------------- String comparaison
// W0- Id comparaison
// W1- 0 (at least!)
#define		PARAM_CMPSTRING		   	46

// -------------------------------- Ink effect
// W0- Id effect
// W1- Effet parameter
// L1- Free
#define		PARAM_INKEFFECT		   	47
#define		PS_INK					8

// -------------------------------- Menu
// L0- Identifier
// L1- Security
#define		PARAM_MENU		   		48
#define		PS_MENU					8

// Access to named variables
#define		PARAM_VARGLOBAL		   	49
#define		PS_VARGLOBAL  			4
#define		PARAM_ALTVALUE		   	50
#define		PS_ALTVALUE  			4
#define		PARAM_FLAG			   	51
#define		PS_FLAG  				4

// With expressions
#define		PARAM_VARGLOBAL_EXP		52
#define		PS_VARGLOBAL_EXP  		PS_EXP
#define		PARAM_ALTVALUE_EXP		53
#define		PS_ALTVALUE_EXP	 		PS_EXP
#define		PARAM_FLAG_EXP		   	54
#define		PS_FLAG_EXP  			PS_EXP

// -------------------------------- Extensions parameters
#define		PARAM_EXTENSION			55
#define		PARAM_EXTMAXSIZE		512
typedef struct 
{
	short pextSize;
	short pextType;
	short pextCode;
	char pextData[2];
} paramExt;
#define		PARAM_EXTSIZE			12
#define		PARAM_EXTBASE			1000

// -------------------------------- Direction
#define		PARAM_8DIRECTIONS	   	56
#define		PS_8DIR					4

// -------------------------------- Movement number
#define MAX_MVTNAME					32
typedef struct tagMvtParam {
	short	mvtNumber;
	char	mvtName[MAX_MVTNAME];
	} MvtParam;
typedef		MvtParam *			LPMVTP;

#define		PARAM_MVT			57
#define		PS_MVT				sizeof(MvtParam)

// Access to renamed variables
#define		PARAM_STRINGGLOBAL		58
#define		PS_STRINGGLOBAL  		4
#define		PARAM_STRINGGLOBAL_EXP	59
#define		PS_STRINGGLOBAL_EXP  	PS_EXP

// -------------------------------- External program II
#define		PARAM_PROGRAM2			60
typedef struct tagPRG2 {
	short	prgFlags;				// Default flags
	} prgParam2;
typedef		prgParam2 *			LPPRG2;	
#define		PS_PRG2					sizeof(prgParam2)

// -------------------------------- Alterable strings
#define		PARAM_ALTSTRING		   	61
#define		PS_ALTSTRING  			4
#define		PARAM_ALTSTRING_EXP		62
#define		PS_ALTSTRING_EXP	 	PS_EXP

// -------------------------------- A pointner to a filename, version 2
#define		PARAM_FILENAME2			63
#define		PS_FILEN				_MAX_PATH
// B nom de fichier...


// STRUCTURE FOR FAST LOOPS
///////////////////////////////////////////////////////////////////////
typedef struct tagFL
{
	LPSTR	next;
	char	name[64];
	WORD	flags;
	long	index;
} FastLoop;
typedef	FastLoop *	LPFL;

#define MAX_FASTLOOPNAME	64
#define FLFLAG_STOP			0x0001

///////////////////////////////////////////////////////////////
//
// DEBUGGER
//
///////////////////////////////////////////////////////////////
#define DBCOMMAND_RUNNING		0
#define DBCOMMAND_TORUN			1
#define DBCOMMAND_PAUSE			2
#define DBCOMMAND_TOPAUSE		3
#define DBCOMMAND_STEP			4

// TREE identification
enum
{
	DBTYPE_SYSTEM,
	DBTYPE_OBJECT,
	DBTYPE_HO,
	DBTYPE_MOVEMENT,
	DBTYPE_ANIMATION,
	DBTYPE_VALUES,
	DBTYPE_COUNTER,
	DBTYPE_SCORE,
	DBTYPE_LIVES,
	DBTYPE_TEXT,
	DBTYPE_EXTENSION
};

// Genric entries in the tree
enum
{
	DB_END=0xFFFF,
	DB_PARENT=0x8000
};
#define DB_EDITABLE		0x80

// Communication buffer size
#define DB_BUFFERSIZE			256

#define DB_MAXGLOBALVALUES		1000	// Maximum number of global values displayed in the debugger
#define DB_MAXGLOBALSTRINGS		1000

// System tree entries
enum
{
	DB_SYSTEM,
	DB_TIMER,
	DB_FPS,
	DB_FRAMENUMBER,
	DB_GLOBALVALUE,
	DB_GLOBALSTRING,
	DB_GVALUE,
	DB_GSTRING,
};
// Headerobject tree entries
enum
{
	DB_XY,
	DB_SIZE
};

// Movements tree entries
enum
{
	DB_MOVEMENTS,
	DB_MVTTYPE,
	DB_SPEED,
	DB_DIRECTION,
	DB_ACCELERATION,
	DB_DECELERATION
};

// Animations tree entries
enum
{
	DB_ANIMATIONS,
	DB_ANIMNAME,
	DB_FRAME
};

// Values tree entries
enum
{
	DB_VALUE,
	DB_VALUE0,
	DB_VALUE1,
	DB_VALUE2,
	DB_VALUE3,
	DB_VALUE4,
	DB_VALUE5,
	DB_VALUE6,
	DB_VALUE7,
	DB_VALUE8,
	DB_VALUE9,
	DB_VALUE10,
	DB_VALUE11,
	DB_VALUE12,
	DB_VALUE13,
	DB_VALUE14,
	DB_VALUE15,
	DB_VALUE16,
	DB_VALUE17,
	DB_VALUE18,
	DB_VALUE19,
	DB_VALUE20,
	DB_VALUE21,
	DB_VALUE22,
	DB_VALUE23,
	DB_VALUE24,
	DB_VALUE25,
	DB_ALTSTRING,
	DB_ALTSTRING0,
	DB_ALTSTRING1,
	DB_ALTSTRING2,
	DB_ALTSTRING3,
	DB_ALTSTRING4,
	DB_ALTSTRING5,
	DB_ALTSTRING6,
	DB_ALTSTRING7,
	DB_ALTSTRING8,
	DB_ALTSTRING9,
	DB_FLAGS,
	DB_FLAG0,
	DB_FLAG1,
	DB_FLAG2,
	DB_FLAG3,
	DB_FLAG4,
	DB_FLAG5,
	DB_FLAG6,
	DB_FLAG7,
	DB_FLAG8,
	DB_FLAG9,
	DB_FLAG10,
	DB_FLAG11,
	DB_FLAG12,
	DB_FLAG13,
	DB_FLAG14,
	DB_FLAG15,
	DB_FLAG16,
	DB_FLAG17,
	DB_FLAG18,
	DB_FLAG19,
	DB_FLAG20,
	DB_FLAG21,
	DB_FLAG22,
	DB_FLAG23,
	DB_FLAG24,
	DB_FLAG25,
	DB_FLAG26,
	DB_FLAG27,
	DB_FLAG28,
	DB_FLAG29,
	DB_FLAG30,
	DB_FLAG31
};
enum 
{
	DB_COUNTERVALUE,
	DB_COUNTERMIN,
	DB_COUNTERMAX
};	
enum 
{
	DB_SCOREVALUE
};	
enum 
{
	DB_LIVESVALUE
};	
enum 
{
	DB_TEXTVALUE
};	

#define GETDBPARAMTYPE(a)		((a&0xFF000000)>>24)
#define GETDBPARAMID(a)			((a&0x00FFFF00)>>8)
#define GETDBPARAMCOMMAND(a)	(a&0x000000FF)
#define GETDBPARAM(a, b, c)		((a&0xFF)<<24)|((b&0xFFFF)<<8)|(c&0xFF)

typedef struct
{
	LPSTR pTitle;
	int value;
	LPSTR pText;
	int lText;
} EditDebugInfo;

///////////////////////////////////////////////////////////////////////
//
// RUNTIME BUFFER
//
///////////////////////////////////////////////////////////////////////

//Modes de demo
enum
{
	DEMONOTHING,
	DEMORECORD,
	DEMOPLAY,
};

typedef void (*ACTIONENDROUTINE)();

typedef void (*OBLROUTINE)(headerObject*);
typedef struct tagOBL {
	headerObject*	oblOffset;
	OBLROUTINE		oblRoutine;
	} objectsList;
typedef	objectsList *			LPOBL;

#define		GAMEBUFFERS_SIZE	(12*1024)
#define		GAME_MAXOBJECTS		266
#define		OBJECT_MAX			128L
#define		OBJECT_SIZE			256L
#define		OBJECT_SHIFT		8
#define		MAX_INTERMEDIATERESULTS		256
#define		MAX_TEMPSTRINGS				256

typedef struct tagRH2 {
	DWORD		rh2OldPlayer;				// Previous player entries
	DWORD		rh2NewPlayer;				// Modified player entries
	DWORD		rh2InputMask;				// Inhibated players entries
	DWORD		rh2InputPlayers;			// Valid players entries (mask!)
	BYTE 		rh2MouseKeys;				// Mousekey entries
	BYTE		rh2ActionLoop;				// Actions flag
	BYTE 		rh2ActionOn;				// Flag are we in actions?
	BYTE		rh2EnablePick;  			// Are we in pick for actions?

	int		  	rh2EventCount;				// Number of the event 
	qualToOi*	rh2EventQualPos;			// ***Position in event objects
	headerObject* rh2EventPos;				// ***Position in event objects
	objInfoList* rh2EventPosOiList; 		// ***Position in oilist for TYPE exploration 
	objInfoList* rh2EventPrev;				// ***Previous object address

	pev*		rh2PushedEvents;			// ***
	LPBYTE		rh2PushedEventsTop;			// ***
	LPBYTE		rh2PushedEventsMax;			// ***
	int	  		rh2NewPushedEvents;			// 

	int		  	rh2ActionCount;				// Action counter
	int		  	rh2ActionLoopCount;	  		// Action loops counter
	ACTIONENDROUTINE rh2ActionEndRoutine;	// End of action routine
	WORD	  	rh2CreationCount;			// Number of objects created since beginning of frame
	short		rh2EventType;					
	POINT 		rh2Mouse;					// Mouse coordinate
	POINT 		rh2MouseClient;				// Mouse coordinates in the window
	short	  	rh2CurrentClick;			// For click events II
	short		rh2Free2;					
	headerObject** rh2ShuffleBuffer;		// ***
	headerObject** rh2ShufflePos;			// ***
	int		  	rh2ShuffleNumber;			

	POINT		rh2MouseSave;				// Mouse saving when pause
	int		  	rh2PauseCompteur;
	DWORD		rh2PauseTimer;
	UINT	  	rh2PauseVbl;
	FARPROC	   	rh2LoopTraceProc;       	// Debugging routine
	FARPROC	   	rh2EventTraceProc;

	} runHeader2;


// Flags pour rh3Scrolling
#define RH3SCROLLING_SCROLL					0x0001
#define RH3SCROLLING_REDRAWLAYERS			0x0002
#define RH3SCROLLING_REDRAWALL				0x0004
#define RH3SCROLLING_REDRAWTOTALCOLMASK		0x0008

#define	GAME_XBORDER		480
#define	GAME_YBORDER		300

typedef struct tagRH3 {

	WORD		rh3Graine;					// random generator seed
	WORD		rh3Free;					// Alignment...

	int		  	rh3DisplayX;				// To scroll
	int	  		rh3DisplayY;

	long		rh3CurrentMenu;				// For menu II events

	int 		rh3WindowSx;   				// Window size
	int 		rh3WindowSy;

	short	  	rh3CollisionCount;			// Collision counter 
	char		rh3DoStop;					// Force the test of stop actions
	char		rh3Scrolling;				// Flag: we need to scroll

	int		  	rh3Panic;

	int		  	rh3PanicBase;
	int		  	rh3PanicPile;

//	short	  	rh3XBorder_;				// Authorised border
//	short	  	rh3YBorder_;
	int	 	  	rh3XMinimum;   				// Object inactivation coordinates
	int	 	  	rh3YMinimum;
	int	 	  	rh3XMaximum;
	int	 	  	rh3YMaximum;
	int	 	  	rh3XMinimumKill;			// Object destruction coordinates
	int	 	  	rh3YMinimumKill;
	int	 	  	rh3XMaximumKill;
	int	 	  	rh3YMaximumKill;

	} runHeader3;

// Extensions 
// ~~~~~~~~~~~~~~~~~~~~
#define		KPX_MAXNUMBER					96
typedef struct tagKPXMSG {
	short 	( WINAPI * routine) (DWORD, HWND, UINT, WPARAM, LPARAM);
	} kpxMsg;
#define		KPX_MAXFUNCTIONS				32
typedef struct tagKPXLIB {
	long 	( WINAPI * routine) (headerObject*, WPARAM, LPARAM);
	} kpxLib;
#define IsRunTimeFunctionPresent(num)	(num < KPX_MAXFUNCTIONS && ((LPRH)rdPtr->rHo.hoAdRunHeader)->rh4.rh4KpxFunctions[num].routine != NULL)
#define	callRunTimeFunction(rdPtr,num,wParam,lParam)	( ((LPRH)rdPtr->rHo.hoAdRunHeader)->rh4.rh4KpxFunctions[num].routine((LPHO)rdPtr, wParam, lParam) )
#define	callRunTimeFunction2(hoPtr,num,wParam,lParam)	( ((LPRH)hoPtr->hoAdRunHeader)->rh4.rh4KpxFunctions[num].routine(hoPtr, wParam, lParam) )

#define	RFUNCTION_REHANDLE					0
#define	RFUNCTION_GENERATEEVENT				1
#define	RFUNCTION_PUSHEVENT					2
#define	RFUNCTION_GETSTRINGSPACE_EX			3
#define	RFUNCTION_GETPARAM1					4
#define	RFUNCTION_GETPARAM2					5
#define	RFUNCTION_PUSHEVENTSTOP				6
#define	RFUNCTION_PAUSE						7
#define	RFUNCTION_CONTINUE					8
#define	RFUNCTION_REDISPLAY					9
#define	RFUNCTION_GETFILEINFOS				10
#define	RFUNCTION_SUBCLASSWINDOW			11
#define	RFUNCTION_REDRAW					12
#define	RFUNCTION_DESTROY					13
#define	RFUNCTION_GETSTRINGSPACE			14
#define	RFUNCTION_EXECPROGRAM				15
#define RFUNCTION_GETOBJECTADDRESS			16
#define RFUNCTION_GETPARAM					17
#define RFUNCTION_GETEXPPARAM				18
#define RFUNCTION_GETPARAMFLOAT				19
#define RFUNCTION_EDITINT					20
#define RFUNCTION_EDITTEXT					21
#define RFUNCTION_CALLMOVEMENT				22
#define RFUNCTION_SETPOSITION				23

#define CNC_GetParameter(rdPtr)							callRunTimeFunction(rdPtr, RFUNCTION_GETPARAM, 0xFFFFFFFF, 0)
#define CNC_GetIntParameter(rdPtr)						callRunTimeFunction(rdPtr, RFUNCTION_GETPARAM, 0, 0)
#define CNC_GetStringParameter(rdPtr)					callRunTimeFunction(rdPtr, RFUNCTION_GETPARAM, 0xFFFFFFFF, 0)
#define CNC_GetFloatParameter(rdPtr)					callRunTimeFunction(rdPtr, RFUNCTION_GETPARAM, 2, 0)
#define CNC_GetFloatValue(rdPtr, par)					callRunTimeFunction(rdPtr, RFUNCTION_GETPARAMFLOAT, par, 0)
#define CNC_GetFirstExpressionParameter(rdPtr, lParam, wParam)	callRunTimeFunction(rdPtr, RFUNCTION_GETPARAM1, wParam, lParam)
#define CNC_GetNextExpressionParameter(rdPtr, lParam, wParam)	callRunTimeFunction(rdPtr, RFUNCTION_GETPARAM2, wParam, lParam)

#define MAX_FRAMERATE						10

typedef void (* CALLANIMATIONS) (headerObject*, int);
#define callAnimations(hoPtr, anim) ( (hoPtr->hoAdRunHeader)->rh4.rh4Animations(hoPtr, anim) )

typedef DWORD (* CALLDIRATSTART) (headerObject*, DWORD);
#define callDirAtStart(hoPtr, initDir) ( (hoPtr->hoAdRunHeader)->rh4.rh4DirAtStart(hoPtr, initDir) )

typedef BOOL (* CALLMOVEIT) (headerObject*);
#define callMoveIt(hoPtr) ( (hoPtr->hoAdRunHeader)->rh4.rh4MoveIt(hoPtr) )

#ifdef __cplusplus
typedef BOOL (* CALLAPPROACHOBJECT) (headerObject*, int destX, int destY, int maxX, int maxY, int htFoot, int planCol, int& x, int &y);
#define callApproachObject(x, y, hoPtr, destX, destY, maxX, maxY, htFoot, planCol) ( (hoPtr->hoAdRunHeader)->rh4.rh4ApproachObject(hoPtr, destX, destY, maxX, maxY, htFoot, planCol, x, y) )
#else
typedef BOOL (* CALLAPPROACHOBJECT) (headerObject*, int destX, int destY, int maxX, int maxY, int htFoot, int planCol, int* x, int *y);
#define callApproachObject(x, y, hoPtr, destX, destY, maxX, maxY, htFoot, planCol) ( (hoPtr->hoAdRunHeader)->rh4.rh4ApproachObject(hoPtr, destX, destY, maxX, maxY, htFoot, planCol, &x, &y) )
#endif

typedef void (* CALLCOLLISIONS) (headerObject*);
#define callCollisions(hoPtr) ( (hoPtr->hoAdRunHeader)->rh4.rh4Collisions(hoPtr) )

typedef BOOL (* CALLTESTPOSITION) (headerObject*, int x, int y, int htFoot, int planCol, BOOL flag);
#define callTestPosition(hoPtr, x, y, htFoot, planCol) ( (hoPtr->hoAdRunHeader)->rh4.rh4TestPosition(hoPtr, x, y, htFoot, planCol, 0) )

typedef BYTE (* CALLGETJOYSTICK) (headerObject*, int);
#define callGetJoystick(hoPtr, player) ( (hoPtr->hoAdRunHeader)->rh4.rh4Joystick(hoPtr, player) )

typedef BOOL (* CALLCOLMASKTESTRECT) (headerObject*, int x, int y , int sx, int sy, int nLayer, int plan);
#define callColMaskTestRect(hoPtr, x, y, sx, sy, nLayer, plan) ( (hoPtr->hoAdRunHeader)->rh4.rh4ColMaskTestRect(hoPtr, x, y, sx, sy, nLayer, plan) )

typedef BOOL (* CALLCOLMASKTESTPOINT) (headerObject*, int x, int y, int nLayer, int plan);
#define callColMaskTestPoint(hoPtr, x, y, nLayer, plan) ( (hoPtr->hoAdRunHeader)->rh4.rh4ColMaskTestPoint(hoPtr, x, y, nLayer, plan) )


typedef struct tagRH4 {

	kpj*		rh4KpxJumps;							// Jump table offset
	short		rh4KpxNumOfWindowProcs;					// Number of routines to call
	short		rh4Free;
	kpxMsg		rh4KpxWindowProc[KPX_MAXNUMBER];		// Message handle routines
	kpxLib		rh4KpxFunctions[KPX_MAXFUNCTIONS];		// Available internal routines
	CALLANIMATIONS	rh4Animations;						
	CALLDIRATSTART	rh4DirAtStart;						
	CALLMOVEIT		rh4MoveIt;
	CALLAPPROACHOBJECT rh4ApproachObject;
	CALLCOLLISIONS rh4Collisions;
	CALLTESTPOSITION rh4TestPosition;
	CALLGETJOYSTICK rh4GetJoystick;
	CALLCOLMASKTESTRECT rh4ColMaskTestRect;
	CALLCOLMASKTESTPOINT rh4ColMaskTestPoint;

	DWORD		rh4SaveVersion;
	event*		rh4ActionStart;							// Sauvergarde action courante
	int			rh4PauseKey;
	LPSTR		rh4CurrentFastLoop;
	int			rh4EndOfPause;
	int		  	rh4EventCountOR;						// Number of the event for OR conditions
	short		rh4ConditionsFalse;
	short		rh4MouseWheelDelta;
	int			rh4OnMouseWheel;
	LPSTR		rh4PSaveFilename;
	UINT		rh4MusicHandle;
	DWORD		rh4MusicFlags;
	DWORD		rh4MusicLoops;
	int			rh4LoadCount;
	short		rh4DemoMode;
	short		rh4Free4;
	CDemoRecord* rh4Demo;
	char		rh4QuitString[52];						// FREE!!!! GREAT!

	DWORD		rh4PickFlags0;							// 00-31
	DWORD		rh4PickFlags1;							// 31-63
	DWORD		rh4PickFlags2;							// 64-95
	DWORD		rh4PickFlags3;							// 96-127
	LPDWORD		rh4TimerEventsBase;						// Timer events base

	short		rh4DroppedFlag;
	short		rh4NDroppedFiles;
	LPSTR		rh4DroppedFiles;
	LPFL		rh4FastLoops;
	LPSTR		rh4CreationErrorMessages;
	CValue		rh4ExpValue1;							// New V2
	CValue		rh4ExpValue2;

	long		rh4KpxReturn;				// WindowProc return 
	LPOBL		rh4ObjectCurCreate;
	short		rh4ObjectAddCreate;
	WORD		rh4Free10;					// For step through : fake key pressed
	HINSTANCE	rh4Instance;				// Application instance
	HWND		rh4HStopWindow;				// STOP window handle
	char		rh4DoUpdate;				// Flag for screen update on first loop
	char		rh4MenuEaten;				// Menu handled in an event?
	short		rh4Free2;
	int			rh4OnCloseCount;			// For OnClose event
	short		rh4CursorCount;				// Mouse counter
	short		rh4ScrMode;					// Current screen mode
	HPALETTE	rh4HPalette;				// Handle current palette
	int 		rh4VBLDelta;				// Number of VBL
	DWORD		rh4LoopTheoric;				// Theorical VBL counter
	DWORD		rh4EventCount;
	drawRoutine* rh4FirstBackDrawRoutine;	// Backrgound draw routines list
	drawRoutine* rh4LastBackDrawRoutine;		// Last routine used

	DWORD		rh4ObjectList;				// Object list offset
	short		rh4LastQuickDisplay;		// Quick - display list
	BYTE		rh4CheckDoneInstart;		// Build92 to correct start of frame with fade in
	BYTE		rh4Free0;
	mv *		rh4Mv;						// Yves's data
	DWORD		rh4Free1;					// String buffer position
	headerObject*	rh4_2ndObject;	 		// Collision object address
	short 		rh4_2ndObjectNumber;        // Number for collisions
	short		rh4FirstQuickDisplay;		// Quick-display object list
	int			rh4WindowDeltaX;			// For scrolling
	int			rh4WindowDeltaY;               
	UINT		rh4TimeOut;					// For time-out!
	int			rh4MouseXCenter;			// To correct CROSOFT bugs!
	int			rh4MouseYCenter;			// To correct CROSOFT bugs!
	int			rh4TabCounter;				// Objects with tabulation
	
	DWORD		rh4AtomNum;					// For child window handling
	DWORD		rh4AtomRd;
	DWORD		rh4AtomProc;
	short		rh4SubProcCounter;			// To accelerate the windows
	short		rh4Free3;
	
	int			rh4PosPile;								// Expression evaluation pile position
	expression*	rh4ExpToken;							// Current position in expressions
	CValue*		rh4Results[MAX_INTERMEDIATERESULTS];	// Result pile
	long		rh4Operators[MAX_INTERMEDIATERESULTS];	// Operators pile
	LPSTR		rh4PTempStrings[MAX_TEMPSTRINGS];		// Temporary string calculation positiion
	int			rh4NCurTempString;						// Pointer on the current string
	DWORD		rh4FrameRateArray[MAX_FRAMERATE];		// Framerate calculation buffer
	int			rh4FrameRatePos;						// Position in buffer
	DWORD		rh4FrameRatePrevious;					// Previous time 

	} runHeader4;

#define		GAMEFLAGS_VBLINDEP				0x0002
#define		GAMEFLAGS_LIMITEDSCROLL			0x0004
#define		GAMEFLAGS_FIRSTLOOPFADEIN		0x0010
#define		GAMEFLAGS_LOADONCALL			0x0020
#define		GAMEFLAGS_REALGAME				0x0040
#define		GAMEFLAGS_PLAY					0x0080
//#define	GAMEFLAGS_FADEIN				0x0080
//#define	GAMEFLAGS_FADEOUT				0x0100
#define		GAMEFLAGS_INITIALISING			0x0200

typedef struct RunHeader {
	npWin		rhIdEditWin;
	npWin		rhIdMainWin;
	npAppli		rhIdAppli;

	HWND		rhHEditWin;
	HWND		rhHMainWin;
	HWND		rhHTopLevelWnd;

	CRunApp*	rhApp;						// Application info
	CRunFrame*	rhFrame;					// Frame info

	DWORD		rhJoystickPatch;			// To reroute the joystick

	BYTE		rhFree10;					// Current movement needs to be stopped
	BYTE		rhFree12; 					// Event evaluation flag
	BYTE		rhNPlayers;					// Number of players
	BYTE		rhMouseUsed;				// Players using the mouse

	WORD		rhGameFlags;				// Game flags
	WORD		rhFree;						// Alignment
	DWORD		rhPlayer;					// Current players entry

	short 		rhQuit;
	short		rhQuitBis; 					// Secondary quit (scrollings)
	DWORD		rhFree11;					// Value to return to the editor
	DWORD		rhQuitParam;

// Buffers
	int			rhNObjects;
	int			rhMaxObjects;
	
	DWORD		rhFree0;			    
	DWORD		rhFree1;				
	DWORD		rhFree2;					
	DWORD		rhFree3;

	int 		rhNumberOi;					// Number of OI in the list
	objInfoList*	rhOiList;					// ObjectInfo list

	LPDWORD		rhEvents[NUMBEROF_SYSTEMTYPES+1];	// Events pointers
	LPDWORD		rhEventLists;		 		// Pointers on pointers list
	LPDWORD		rhFree8;					// Timer pointers
	LPDWORD		rhEventAlways;				// Pointers on events to see at each loop
	LPEVG		rhPrograms;					// Program pointers
	OINUM*		rhLimitLists;				// Movement limitation list
	qualToOi*	rhQualToOiList;				// Conversion qualifier->oilist

	DWORD		rhSBuffers;					// Buffer size /1024	
	LPBYTE		rhBuffer;					// Position in current buffer
	LPBYTE		rhFBuffer;					// End of current buffer
	LPBYTE		rhBuffer1;					// First buffer
	LPBYTE		rhBuffer2;					// Second buffer

	int 		rhLevelSx;					// Window size
	int 		rhLevelSy;
	int 		rhWindowX;   				// Start of window in X/Y
	int 		rhWindowY;

	UINT		rhVBLDeltaOld;				// Number of VBL
	UINT		rhVBLObjet;					// For the objects
	UINT		rhVBLOld;					// For the counter

	int			free10;						
	WORD		rhMT_VBLStep;   			// Path movement variables
	WORD 		rhMT_VBLCount;
	DWORD		rhMT_MoveStep;

	int			rhLoopCount;				// Number of loops since start of level
	UINT		rhTimer;					// Timer in 1/50 since start of level
	UINT		rhTimerOld;					// For delta calculation
	UINT		rhTimerDelta;				// For delta calculation

	LPEVG		rhEventGroup;				// Current group
	long 		rhCurCode;					// Current event
	short		rhCurOi;
	short		rhFree4;					// Alignment
	long		rhCurParam[2];
	short 		rhCurObjectNumber;	 		// Object number
	short 		rh1stObjectNumber;          // Number, for collisions

	long		rhOiListPtr;				// OI list enumeration
	short 		rhObListNext;				// Branch label

	short		rhDestroyPos;
	long		rhFree5;				
	long		rhFree6;

	runHeader2	rh2;						// Sub-structure #1
	runHeader3	rh3;						// Sub-structure #2
	runHeader4	rh4;						// Sub-structure #3

	LPDWORD		rhDestroyList;				// Destroy list address

	int			rhDebuggerCommand;			// Current debugger command
	char		rhDebuggerBuffer[DB_BUFFERSIZE];		// Code transmission buffer
	char*		rhDbOldHO;
	WORD		rhDbOldId;
	WORD		rhFree7;					

	objectsList*	rhObjectList;			// Object list address

	} RunHeader;
typedef	RunHeader 	* fprh;
typedef	RunHeader 	* LPRH;
typedef	RunHeader 	* LPRUNHEADER;


///////////////////////////////////////////////////////////////
//
// RUNTIME OBJECT DEFINITION
//
///////////////////////////////////////////////////////////////

#define HOX_INT

typedef short (WINAPI *HOROUTINE)(headerObject*);

#ifdef __cplusplus
class headerObject {
public:
#else
#undef headerObject
typedef struct headerObject {
#endif
	short  	hoNumber;					// Number of the object
	short 	hoNextSelected;				// Selected object list!!! DO NOT CHANGE POSITION!!!

	int		hoSize;						// Structure size
    LPRH	hoAdRunHeader;				// Run-header address
#ifdef __cplusplus
	headerObject* hoAddress;			
#else
	LPVOID	hoAddress;
#endif
	HFII	hoHFII;						// Number of LevObj
	OINUM  	hoOi;						// Number of OI
	short  	hoNumPrev;					// Same OI previous object
	short  	hoNumNext;					// ... next
	short 	hoType;						// Type of the object
	WORD  	hoCreationId;				// Number of creation
	objInfoList* hoOiList;				// Pointer to OILIST information
	LPDWORD hoEvents;					// Pointer to specific events
	UINT	hoFree0;					// Free
	LPBYTE	hoPrevNoRepeat;	   			// One-shot event handling
	LPBYTE	hoBaseNoRepeat;

	int 	hoMark1;					// #of loop marker for the events
	int 	hoMark2;
	char*	hoMT_NodeName;				// Name fo the current node for path movements

	int		hoEventNumber;				// Number of the event called (for extensions)
	int		hoFree2;
	LPOC	hoCommon;					// Common structure address

#ifdef HOX_INT
	union
	{
	struct
		{
			int  	hoCalculX;					// Low weight value
			int  	hoX;          	      		// X coordinate
			int  	hoCalculY;					// Low weight value
			int  	hoY;						// Y coordinate
		};
	struct	
		{
			__int64 hoCalculXLong;
			__int64 hoCalculYLong;
		};
	};
#else
	union
	{
	struct
		{
			short 	hoCalculX;					// Low weight value
			short	hoX;          	      		// X coordinate
			short	hoCalculY;					// Low weight value
			short	hoY;						// Y coordinate
		};
	struct	
		{
			int hoCalculXLong;
			int hoCalculYLong;
		};
	};
#endif
	int		hoImgXSpot;					// Hot spot of the current image
	int		hoImgYSpot;
	int		hoImgWidth;					// Width of the current picture
	int		hoImgHeight;
	RECT	hoRect;						// Display rectangle
	
	DWORD	hoOEFlags;					// Objects flags
	short	hoFlags;					// Flags
	BYTE	hoSelectedInOR;				// Selection lors d'un evenement OR
	BYTE	hoFree;						// Alignement 
	int		hoOffsetValue;				// Values structure offset
	UINT	hoLayer;					// Layer

	HOROUTINE hoHandleRoutine;			// General handle routine
	HOROUTINE hoModifRoutine;  			// Modification routine when coordinates have been modified
	HOROUTINE hoDisplayRoutine;			// Display routine

	short	hoLimitFlags;				// Collision limitation flags
	short	hoNextQuickDisplay;			// Quickdraw list
	saveRect hoBackSave;				// Background
	
	LPEVP	hoCurrentParam;				// Address of the current parameter

	int		hoOffsetToWindows;			// Offset to windows
	DWORD	hoIdentifier;				// ASCII identifier of the object

#ifdef __cplusplus
};
#else
} headerObject;
#endif
typedef	headerObject*	LPHO;

#define	HOF_DESTROYED		0x0001
#define	HOF_TRUEEVENT		0x0002
#define	HOF_REALSPRITE		0x0004
#define	HOF_FADEIN			0x0008
#define	HOF_FADEOUT			0x0010
#define	HOF_OWNERDRAW		0x0020
#define	HOF_NOCOLLISION		0x2000
#define	HOF_FLOAT			0x4000
#define	HOF_STRING			0x8000


// --------------------------------------
// Object's movement structure
// --------------------------------------
typedef struct tagRM {

	int  	rmAcc;						// Current acceleration
	int  	rmDec;						// Current Decelaration 
	int		rmCollisionCount;			// Collision counter
	int		rmFree2;
	int  	rmStopSpeed;				// If stopped: speed to take again
	int  	rmFree0;					
	int  	rmFree1;					
	int  	rmAccValue;					// Acceleration calculation
	int  	rmDecValue;					// Deceleration calculation
	short	rmEventFlags;				// To accelerate events
	int  	rmFree3;	
	int  	rmFree4;
	BOOL	rmFree5;
	BOOL	rmFree6;

 	BOOL	rmFree7;
	BOOL	rmMoveFlag;					// Messages/movements

	BOOL	rmWrapping;					// For CHECK POSITION
	BOOL 	rmReverse;					// Ahaid or reverse?

	BOOL  	rmBouncing;					// Bouncing?
	int		rmMvtNum;					// Number of the current movement

	union	
	{
	struct
		{
		BYTE 	rmMvData[80];			// Space for data
		};
	struct	
		{
		int		MT_Speed;
		int	 	MT_Sinus;
		int	 	MT_Cosinus;
		DWORD 	MT_Longueur;
		int	 	MT_XOrigin;
		int	 	MT_YOrigin;
		int	 	MT_XDest;
		int	 	MT_YDest;
		int	 	MT_MoveNumber;
		LPMOVEDEF MT_MoveAddress;
		BOOL	MT_Direction;
		LPMOVETAPED	MT_Movement;
		int		MT_Calculs;
		int		MT_XStart;
		int		MT_YStart;
		int		MT_Pause;
		char*	MT_GotoNode;
		};
	struct
		{
		int	 	MM_DXMouse;
		int	 	MM_DYMouse;
		int	 	MM_FXMouse;
		int	 	MM_FYMouse;
		int	 	MM_Stopped;
		int	 	MM_OldSpeed;
		};	
	struct
		{
		int	 	MG_Bounce;
		long	MG_OkDirs;
		int	 	MG_BounceMu;
		int	 	MG_Speed;
		int 	MG_LastBounce;
		int	 	MG_DirMask;
		};
	struct
		{
		int	 	MR_Bounce;
		int	 	MR_BounceMu;
		int	 	MR_Speed;
		int	 	MR_RotSpeed;
		int	 	MR_RotCpt;
		int	 	MR_RotPos;
		int	 	MR_RotMask;
		int	 	MR_OkReverse;
		int	 	MR_OldJoy;
		int 	MR_LastBounce;
		};
	struct
		{
		long	MB_StartDir;
		int	 	MB_Angles;
		int	 	MB_Securite;
		int	 	MB_SecuCpt;
		int	 	MB_Bounce;
		int	 	MB_Speed;
		int	 	MB_MaskBounce;
		int	 	MB_LastBounce;
		int	 	MB_Blocked;
		};
	struct
		{
		int	 	MBul_Wait;
		LPHO	MBul_ShootObject;
		};
	struct
		{
		int		MP_Type;
		int	 	MP_Bounce;
		int	 	MP_BounceMu;
		int	 	MP_XSpeed;
		int	 	MP_Gravity;
		int	 	MP_Jump;
		int	 	MP_YSpeed;
		int	 	MP_XMB;
		int	 	MP_YMB;
		int	 	MP_HTFOOT;
		int	 	MP_JumpControl;
		int	 	MP_JumpStopped;
		int	 	MP_PreviousDir;
		LPHO	MP_ObjectUnder;
		int		MP_XObjectUnder;
		int		MP_YObjectUnder;
		BOOL	MP_NoJump;
		};
	};
	} rMvt;
typedef rMvt*	LPRMV;
enum
{
	MPTYPE_WALK,
	MPTYPE_CLIMB,
	MPTYPE_JUMP,
	MPTYPE_FALL,
	MPTYPE_CROUCH,
	MPTYPE_UNCROUCH
};

// ----------------------------------------
// Object's animation structure
// ----------------------------------------
typedef struct tagRA {
	int	 		raAnimForced;				// Flags if forced
	int	 		raAnimDirForced;
	int	 		raAnimSpeedForced;
	BOOL		raAnimStopped;
	int	 		raAnimOn;					// Current animation
	LPAN		raAnimOffset;
	int	 		raAnimDir;				// Direction of current animation
	int	 		raAnimPreviousDir;		// Previous OK direction
	LPAD		raAnimDirOffset;
	int	 	 	raAnimSpeed;
	int	 		raAnimMinSpeed;			// Minimum speed of movement
	int	 		raAnimMaxSpeed;			// Maximum speed of movement
	int	 		raAnimDeltaSpeed;
	int	 		raAnimCounter;			// Animation speed counter
	int	 		raAnimDelta;				// Speed counter
	int	 		raAnimRepeat;				// Number of repeats
	int	 		raAnimRepeatLoop;			// Looping picture
	int	 		raAnimFrame;				// Current frame
	int	 		raAnimNumberOfFrame;   	// Number of frames

	int	 		raAnimFrameForced;

	} rAni;
typedef rAni*	LPRA;

// ----------------------------------------
// Sprite display structure
// ----------------------------------------
typedef struct tagRSPR {

	int	 		rsFlash;				// Flash objets
	int	 		rsFlashCpt;
	int	   		rsLayer;				// Layer
	int			rsZOrder;				// Z-order value
	int	 		rsCreaFlags;			// Creation flags
	COLORREF	rsBackColor;			// background saving color
	DWORD		rsEffect;				// Sprite effects
	DWORD		rsEffectParam;
	WORD	 	rsFlags;				// Handling flags
	WORD		rsFadeCreaFlags;		// Saved during a fadein
	} rSpr;
typedef rSpr *	LPRSP;

#define			RSFLAG_HIDDEN			0x0001
#define			RSFLAG_INACTIVE			0x0002
#define			RSFLAG_SLEEPING			0x0004
#define			RSFLAG_SCALE_RESAMPLE	0x0008	
#define			RSFLAG_ROTATE_ANTIA		0x0010
#define			RSFLAG_VISIBLE			0x0020


// ----------------------------------------
// Objects's internal variables (build # >= 243)
// ----------------------------------------
typedef struct tagRV {
	CValue*	rvpValues;
	long	rvFree1[VALUES_NUMBEROF_ALTERABLE-1];
	long	rvValueFlags;
	BYTE	rvFree2[VALUES_NUMBEROF_ALTERABLE];
	LPSTR	rvStrings[STRINGS_NUMBEROF_ALTERABLE];
	} rVal;
typedef rVal *	LPRVAL;
	
/*
	Obsolete (build 242 or below) - Note: don't remove this (for the SDK)
  
typedef struct tagRV {
	long	rvValues[VALUES_NUMBEROF_ALTERABLE];
	long	rvValueFlags;
	BYTE	rvValuesType[VALUES_NUMBEROF_ALTERABLE];
	LPSTR	rvStrings[STRINGS_NUMBEROF_ALTERABLE];
	} rVal;
typedef rVal *	LPRVAL;
*/


// -----------------------------------------------
// Objects animation and movement structure
// -----------------------------------------------
typedef void (* RCROUTINE)(LPHO);
typedef struct tagRCOM {

	int		rcOffsetAnimation; 			// Offset to anims structures
	int		rcOffsetSprite;				// Offset to sprites structures
	RCROUTINE	rcRoutineMove;			// Offset to movement routine
	RCROUTINE	rcRoutineAnimation;		// Offset to animation routine

	int	   	rcPlayer;					// Player who controls

	int	   	rcNMovement;				// Number of the current movement
	CRunMvt*	rcRunMvt;				// Pointer to extension movement
	fpSpr  	rcSprite;					// Sprite ID if defined
	int	 	rcAnim;						// Wanted animation
	int	   	rcImage;					// Current frame
	float	rcScaleX;					
	float	rcScaleY;
	int		rcAngle;
	int	   	rcDir;						// Current direction
	int	   	rcSpeed;					// Current speed
	int	   	rcMinSpeed;					// Minimum speed
	int	   	rcMaxSpeed;					// Maximum speed
	BOOL	rcChanged;					// Flag: modified object
	BOOL	rcCheckCollides;			// For static objects

	int	 	rcOldX;            			// Previous coordinates
	int	 	rcOldY;
	int	 	rcOldImage;
	int		rcOldAngle;
	int	 	rcOldDir;
	int	 	rcOldX1;					// For zone detections
	int	 	rcOldY1;
	int	 	rcOldX2;
	int	 	rcOldY2;

	long	rcFadeIn;
	long	rcFadeOut;

	} rCom;
	
	
// ------------------------------------------------------------
// ACTIVE OBJECTS DATAZONE
// ------------------------------------------------------------
typedef struct tagRO {

	headerObject 	roHo;		  		// Common structure 

	rCom			roc;				// Anim/movement structure
	rMvt			rom;				// Movement structure 
	rAni			roa;				// Animation structure 
	rSpr			ros;				// Sprite handling structure
	rVal			rov;				// Values structure

	} RunObject;
typedef	RunObject *				LPRO;
typedef RunObject *				LPRUNOBJECT;

#define	EF_GOESINPLAYFIELD			0x0001
#define	EF_GOESOUTPLAYFIELD			0x0002
#define	EF_WRAP						0x0004



// ------------------------------------------------------
// EXTENSION EDITION HEADER STRUCTURE
// ------------------------------------------------------
typedef struct tagEditExtension_v1
{
	short			extSize;
	short			extMaxSize;
	short			extOldFlags;		// For conversion purpose
	short			extVersion;			// Version number
} extHeader_v1;
typedef extHeader_v1*	LPEXTV1;

typedef struct extHeader
{
	DWORD			extSize;
	DWORD			extMaxSize;
	DWORD			extVersion;			// Version number
	LPVOID			extID;				// object's identifier
	LPVOID			extPrivateData;		// private data
} extHeader;
typedef extHeader *		LPEXT;

// ------------------------------------------------------
// System objects (text, question, score, lives, counter)
// ------------------------------------------------------
typedef	struct	tagRs {

	headerObject	rsHo;				// For all the objects
	rCom			rsc;				// Anims / movements / sprites structures
	rMvt			rom;				// Mouvement structure
	rSpr			rss;				// Sprite handling
	
	short			rsPlayer;			// Number of the player if score or lives
	short			rsFlags;			// Type + flags

	union {
		LPBYTE		rsSubAppli;			// Application (CCA object)
	};
	union
	{
		long			rsMini;
		long			rsOldLevel;
	};
	union
	{
		long			rsMaxi;				// 
		long			rsLevel;
	};
	CValue			rsValue;
	LONG			rsBoxCx;			// Dimensions box (for lives, counters, texts)
	LONG			rsBoxCy;			
	double			rsMiniDouble;		
	double			rsMaxiDouble;
	short			rsOldFrame;			// Counter only 
	BYTE			rsHidden;
	BYTE			rsFree;
	LPSTR			rsTextBuffer;		// Text buffer
	int				rsLBuffer;			// Length of the buffer
	DWORD			rsFont;				// Temporary font for texts
	union {
		COLORREF	rsTextColor;		// Text color
		COLORREF	rsColor1;			// Bar color
	};
	COLORREF		rsColor2;			// Gradient bar color
} rs;
typedef	rs *	LPRS;


// OILIST Structure : Data concerning the objects in the game
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define MAX_QUALIFIERS					8	// Maximum number of qualifiers per object
typedef LPHO (*OILROUTINE)(objInfoList*, BOOL*);

#ifdef __cplusplus
class objInfoList {
public:
#else
#undef objInfoList
typedef struct objInfoList {
#endif
	OINUM		oilOi;  				 // THE oi
	short		oilListSelected;		 // First selection !!! DO NOT CHANGE POSITION !!!
	short		oilType;				 // Type of the object
	short		oilObject;				 // First objects in the game
	DWORD		oilEvents;				 // Events
	char		oilWrap;				 // WRAP flags
	char		oilNextFlag;
	WORD		oilFree;				 // Not used
	int			oilNObjects;			 // Current number
	int			oilActionCount;			 // Action loop counter
	int			oilActionLoopCount;	  	 // Action loop counter
	OILROUTINE	oilCurrentRoutine;		 // Current routine for the actions
	int			oilCurrentOi;			 // Current object
	int			oilNext;				 // Pointer on the next
	int			oilEventCount;			 // When the event list is done
	int			oilNumOfSelected;		 // Number of selected objects
	DWORD		oilOEFlags;				 // Object's flags
	short		oilLimitFlags;			 // Movement limitation flags
	short		oilLimitList;         	 // Pointer to limitation list
	short		oilOIFlags;				 // Objects preferences
	short		oilOCFlags2;			 // Objects preferences II
	long		oilInkEffect;			 // Ink effect
	long		oilEffectParam;			 // Ink effect param
	HFII		oilHFII;				 // First available frameitem
	COLORREF 	oilBackColor;			 // Background erasing color
	short		oilQualifiers[MAX_QUALIFIERS];		// Qualifiers for this object
	BYTE		oilName[24];	 		// Name	
	int			oilEventCountOR;		 // Selection in a list of events with OR
#ifdef __cplusplus
};
#else
} objInfoList;
#endif
typedef	objInfoList	*	LPOIL;

#define	OILIMITFLAGS_BORDERS		0x000F
#define	OILIMITFLAGS_BACKDROPS		0x0010
#define	OILIMITFLAGS_QUICKCOL		0x0100
#define	OILIMITFLAGS_QUICKBACK		0x0200
#define	OILIMITFLAGS_QUICKBORDER	0x0400
#define	OILIMITFLAGS_QUICKSPR		0x0800
#define	OILIMITFLAGS_QUICKEXT		0x1000
#define	OILIMITFLAGS_ALL			0xFFFF

// Object creation structure
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct tagCOB {
	LO*		cobLevObj;				// Leave first!
	WORD	cobLevObjSeg;
	WORD	cobFlags;
	int		cobX;
	int		cobY;
	int		cobDir;
	int		cobLayer;
	int		cobZOrder;
	} createObjectInfo;
typedef	createObjectInfo *	LPCOB;
typedef	createObjectInfo *	fpcob;

// Flags for Create Objects
// -------------------------
#define	COF_NOMOVEMENT		0x0001
#define	COF_HIDDEN			0x0002
#define	COF_FIRSTTEXT		0x0004

// Qualifier to oilist for machine langage
// ---------------------------------------
typedef LPHO (*QOIROUTINE)(qualToOi*, BOOL* pRepeatFlag);

#ifdef __cplusplus
class qualToOi {
public:
#else
#undef qualToOi
typedef struct qualToOi {
#endif
	short		qoiCurrentOi;
	short		qoiNext;
	int			qoiActionPos;
	QOIROUTINE	qoiCurrentRoutine;
	int			qoiActionCount;
	int			qoiActionLoopCount;
	char		qoiNextFlag;
	char		qoiSelectedFlag;
	OINUM		qoiOi;						// Array OINUM / OFFSET
	short		qoiOiList;
#ifdef __cplusplus
};
#else
} qualToOi;
#endif
typedef	qualToOi * LPQOI;


// ------------------------------------------------------------------
// ------------------------------------------------------------------
// EXTENSION HANDLING
// ------------------------------------------------------------------
// ------------------------------------------------------------------

#ifdef STAND_ALONE
typedef BOOL (WINAPI *EXTCONDROUTINE)(headerObject*, long, long);
typedef short (WINAPI *EXTACTROUTINE)(headerObject*, long, long);
typedef long (WINAPI *EXTEXPROUTINE)(headerObject*, long);
#else
#define EXTCONDROUTINE	VOID
#define EXTACTROUTINE	VOID
#define EXTEXPROUTINE	VOID
#endif

// Information structure about the extension
#ifdef __cplusplus
class kpxRunInfos {
public:
#else
typedef struct kpxRunInfos {
#endif
	EXTCONDROUTINE* conditions;			// 00 Offset to condition jump list
	EXTACTROUTINE* actions;				// 04 Offset to action jump list
	EXTEXPROUTINE* expressions;			// 08 Offset to expression jump list
	short	numOfConditions;			// 0C Number of conditions
	short	numOfActions;				// 0E Number of actions
	short	numOfExpressions;			// 10 Number of expressions
	WORD	editDataSize;				// 12 Size of the data zone when exploded
	DWORD	editFlags;					// 14 Object flags
	char	windowProcPriority;			// 16 Priority of the routine 0-255
	char	free;						
	short	editPrefs;					// 18 Preferences d'edition
	long	identifier;					// 1A Chaine d'identification
	short	version;					// 1E Version courante
										// 20
#ifdef __cplusplus
};
#else
} kpxRunInfos;
#endif
typedef kpxRunInfos *		LPKPXRUNINFOS;
typedef kpxRunInfos *		fpKpxRunInfos;

// Extension jump table
// --------------------
typedef short (WINAPI * CREATERUNOBJECT_PROC) (LPHO hoPtr, LPEXT extPtr, LPCOB);
typedef unsigned short (WINAPI * GETRUNDATASIZE_PROC)(LPRH, LPEXT);
typedef short (WINAPI * DESTROYRUNOBJECT_PROC)(LPHO, long fast);
typedef short (WINAPI * HANDLERUNOBJECT_PROC)(LPHO);
typedef short (WINAPI * DISPLAYRUNOBJECT_PROC)(LPHO);
typedef cSurface* (WINAPI * GETRUNOBJECTSURFACE_PROC)(LPHO);
typedef short (WINAPI * REINITRUNOBJECT_PROC)(LPHO);
typedef short (WINAPI * PAUSERUNOBJECT_PROC)(LPHO);
typedef short (WINAPI * CONTINUERUNOBJECT_PROC)(LPHO);
typedef short (WINAPI * PREPARETOSAVE_PROC)(LPHO);
typedef short (WINAPI * SAVEBACKGROUND_PROC)(LPHO);
typedef short (WINAPI * RESTOREBACKGROUND_PROC)(LPHO);
typedef short (WINAPI * CHANGERUNDATA_PROC)(LPHO, LPHO);
typedef short (WINAPI * KILLBACKGROUND_PROC) (LPHO);
typedef short (WINAPI * GETZONEINFO_PROC)(LPHO);
typedef LPWORD (WINAPI * GETDEBUGTREE_PROC)(LPHO);
typedef void (WINAPI * GETDEBUGITEM_PROC)(LPSTR, LPHO, int);
typedef void (WINAPI* EDITDEBUGITEM_PROC)(LPHO, int);
typedef void (WINAPI* GETRUNOBJECTFONT_PROC)(LPHO, LOGFONT*);
typedef void (WINAPI* SETRUNOBJECTFONT_PROC)(LPHO, LOGFONT*, RECT*);
typedef COLORREF (WINAPI* GETRUNOBJECTTEXTCOLOR_PROC)(LPHO);
typedef void (WINAPI* SETRUNOBJECTTEXTCOLOR_PROC)(LPHO, COLORREF);
typedef short (WINAPI * GETRUNOBJECTWINDOW_PROC)(LPHO);
typedef sMask* (WINAPI * GETRUNOBJECTCOLLISIONMASK_PROC)(LPHO, LPARAM);
typedef BOOL (WINAPI * SAVERUNOBJECT_PROC)(LPHO, HANDLE);
typedef BOOL (WINAPI * LOADRUNOBJECT_PROC)(LPHO, HANDLE);

#ifdef __cplusplus
class kpj {
public:
#else
#undef kpj
typedef struct kpj {
#endif
	CREATERUNOBJECT_PROC			CreateRunObject;	
	DESTROYRUNOBJECT_PROC			DestroyRunObject;   
	HANDLERUNOBJECT_PROC			HandleRunObject;    
	DISPLAYRUNOBJECT_PROC			DisplayRunObject;   
	GETRUNOBJECTSURFACE_PROC		GetRunObjectSurface;   
	REINITRUNOBJECT_PROC			ReInitRunObject;    
	PAUSERUNOBJECT_PROC				PauseRunObject;     
	CONTINUERUNOBJECT_PROC			ContinueRunObject;  
	PREPARETOSAVE_PROC				PrepareToSave;      
	PREPARETOSAVE_PROC				PrepareToSave2; 	
	GETRUNDATASIZE_PROC				GetRunObjectDataSize;
	SAVEBACKGROUND_PROC				SaveBackground;		
	RESTOREBACKGROUND_PROC			RestoreBackground;	
	CHANGERUNDATA_PROC				ChangeRunData;		
	KILLBACKGROUND_PROC				KillBackground;		
	GETZONEINFO_PROC				GetZoneInfo;		
	GETDEBUGTREE_PROC				GetDebugTree;		
	GETDEBUGITEM_PROC				GetDebugItem;		
	EDITDEBUGITEM_PROC				EditDebugItem;
	GETRUNOBJECTFONT_PROC			GetRunObjectFont;
	SETRUNOBJECTFONT_PROC			SetRunObjectFont;
	GETRUNOBJECTTEXTCOLOR_PROC		GetRunObjectTextColor;
	SETRUNOBJECTTEXTCOLOR_PROC		SetRunObjectTextColor;
	GETRUNOBJECTWINDOW_PROC			GetRunObjectWindow;
	GETRUNOBJECTCOLLISIONMASK_PROC	GetRunObjectCollisionMask;
	SAVERUNOBJECT_PROC				SaveRunObject;
	LOADRUNOBJECT_PROC				LoadRunObject;
	kpxRunInfos	infos;   
	
#ifdef __cplusplus
};
#else
} kpj;
#endif
typedef	kpj	 *      LPKPJ;

#define	KPJ_SHIFT			7
#define	KPJ_SIZE			0x80
#define KPJ_MAXNUMBER		128
#define	KPJ_STEP			8
#define	kpjJCreate				0x00
#define	kpjJDestroy				0x04
#define	kpjJRoutine				0x08
#define	kpjJDisplay				0x0C
#define	kpjJReInit				0x10
#define	kpjJPause				0x14
#define	kpjJEndPause			0x18
#define	kpjJPrepareToSave		0x1C
#define	kpjJRestart				0x20
#define	kpjJGetRunDataSize		0x24
#define	kpjJSaveBackground		0x28
#define	kpjJRestoreBackground	0x2C
#define	kpjJKillBackground		0x30

// Possible states of the application, see LApplication::SetRunningState()
#define	GAMEON_EMPTY					0
#define GAMEON_UNLOADED					1
#define	GAMEON_TIME0					2
#define	GAMEON_PAUSED					3
#define	GAMEON_RUNNING					4
#define	GAMEON_STEPPING					5


//////////////////////////////////////////////////////////////////////////////
// Condition/action jumps
typedef BOOL (*CALLCOND1_ROUTINE)(event* pe, LPHO pHo);
typedef BOOL (*CALLCOND2_ROUTINE)(event* pe);
typedef void (*CALLACTION_ROUTINE)(event* pe);
typedef void (*CALLEXP_ROUTINE)();
typedef BOOL (*CONDROUTINE)(event* pe, LPHO pHo);
typedef void (*ACTROUTINE)(event* pe);
typedef void (*EXPROUTINE)();
typedef void (*CALLOPERATOR_ROUTINE)();

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////
//
//	Run-time Definitions
//

#define	COMPARE_EQ				0
#define COMPARE_NE				1
#define	COMPARE_LE				2
#define	COMPARE_LT				3
#define	COMPARE_GE				4
#define	COMPARE_GT				5

#ifdef	RUN_TIME

	#define OINUM_ERROR				-1
	#define B2L(a,b,c,d)    ((DWORD)(((DWORD)((unsigned char)(d))<<24)|((DWORD)((unsigned char)(c))<<16)|((DWORD)((unsigned char)(b))<<8)|(DWORD)((unsigned char)(a))))
	
	// Pour la routine GetFileInfos
	#define	FILEINFO_DRIVE			1
	#define	FILEINFO_DIR			2
	#define	FILEINFO_PATH			3
	#define	FILEINFO_APPNAME		4
	#define	FILEINFO_TEMPPATH		5

	// MACRO: next event
	#define		EVTNEXT(p)		   		((LPEVT)((LPBYTE)p+p->evtSize))
	// MACRO: returns first param
	#define		EVTPARAMS(p) 			((LPEVP)(p->evtCode<0 ? (LPBYTE)p+CND_SIZE : (LPBYTE)p+ACT_SIZE ))
	// MACRO: returns the extension condition code
	#define		EXTCONDITIONNUM(i)		(-((short)(i>>16))-1)           
	#define		EXTACTIONNUM(i)			((short)(i>>16))

	#define			KPXNAME_SIZE				60
	#define			BADNAME_SIZE			    (OINAME_SIZE+KPXNAME_SIZE+8)

	// Extensions
	// ----------
	typedef struct TABKPT {
		kpj			kpx[2];
		} kpxData;
	typedef	kpxData	*	fpkpdt;

	// Main loop exit codes
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
	#define		LOOPEXIT_NEXTLEVEL		1
	#define		LOOPEXIT_PREVLEVEL		2
	#define		LOOPEXIT_GOTOLEVEL		3
	#define		LOOPEXIT_NEWGAME		4
	#define		LOOPEXIT_PAUSEGAME		5
	#define		LOOPEXIT_SAVEAPPLICATION 6
	#define		LOOPEXIT_LOADAPPLICATION 7
	#define		LOOPEXIT_SAVEFRAME		8
	#define		LOOPEXIT_LOADFRAME		9
	#define		LOOPEXIT_ENDGAME		-2

	#define		LOOPEXIT_QUIT			100
	#define		LOOPEXIT_RESTART		101
	#define		LOOPEXIT_TIMER			102
	#define		LOOPEXIT_STEP			103
	#define		LOOPEXIT_MONITOR		104
	#define		LOOPEXIT_EVENTEDITOR	105
	#define		LOOPEXIT_ESC			106
	#define		LOOPEXIT_MENU			107
	#define		LOOPEXIT_JUMPTO			108
	#define		LOOPEXIT_RESET			109
	#define		LOOPEXIT_PAUSE			110
	#define		LOOPEXIT_PLAY			111

	// Backdrop draw routines
	// ----------------------------------------
	typedef void (* DRAWROUTINE_PROC)(drawRoutine*);

	#ifdef __cplusplus
	class drawRoutine {
	public:
	#else
	#undef drawRoutine
	typedef struct drawRoutine {
	#endif
		#ifdef __cplusplus
			drawRoutine* next;
		#else
			LPVOID	next;
		#endif
		DRAWROUTINE_PROC routine;
		long	param1;
		long	param2;
		long	param3;
		long	param4;
		long	param5;
		long	param6;
	#ifdef __cplusplus
	};
	#else
	} drawRoutine;
	#endif

	// To push an event
	// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	#define		MAX_PUSHEDEVENTS	32
	typedef void (*PEV_ROUTINE)(LPHO, long);

	#ifdef __cplusplus
	class pev {
	public:
	#else
	#undef pev
	typedef struct pev {
	#endif
		long	pevCode;
		PEV_ROUTINE	pevRoutine;
		long	pevParam;
		LPHO	pevObject;
		short	pevOi;
	#ifdef __cplusplus
	};
	#else
	} pev;
	#endif
	#define	PIXEL_SPEED					5

	// QUALIFIERS
	typedef struct
	{
		OINUM		qOi;
		ITEMTYPE	qType;
		WORD		qList;
	} qualifierLoad;
	typedef qualifierLoad *	LPQLOAD;

#endif	// RUN_TIME

#ifndef __cplusplus
#undef headerObject
#undef kpj
#undef objInfoList
#undef drawRoutine
#undef LO
#undef pev
#undef qualToOi
//#undef CValue
#undef CRunApp
#undef CRunFrame
#endif;

// Restores structure alignment...
#ifndef      _H2INC 
#pragma pack( pop, _pack_cncf_ )
#endif

#endif //CNCF_H



