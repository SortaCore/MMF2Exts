/* Copyright (c) 1996-2014 Clickteam
*
* This source code is part of the iOS exporter for Clickteam Multimedia Fusion 2
* and Clickteam Fusion 2.5.
*
* Permission is hereby granted to any person obtaining a legal copy
* of Clickteam Multimedia Fusion 2 or Clickteam Fusion 2.5 to use or modify this source
* code for debugging, optimizing, or customizing applications created with
* Clickteam Multimedia Fusion 2 and/or Clickteam Fusion 2.5.
* Any other use of this source code is prohibited.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
* IN THE SOFTWARE.
*/
//----------------------------------------------------------------------------------
//
// CEVENTS : actions, conditions et expressions
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>
#import "CValue.h"
#import "CRVal.h"
#import "CObjInfo.h"
#import "vector"

#pragma pack(push, _pack_)
#pragma pack(2)

//----------------------------------------------------------------------------------
//
// CNCF.H event definitions
//
//----------------------------------------------------------------------------------
typedef unsigned char BYTE;
typedef BYTE* LPBYTE;
typedef unsigned short WORD;
typedef WORD* LPWORD;
typedef unsigned int DWORD;
typedef DWORD* LPDWORD;
typedef short* LPSHORT;
typedef short OINUM;
#define rhEventProgram rhPtr->rhEvtPrg
#define LPHO CObject*

// Eventgroup structure, before conditions and actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
typedef struct tagEVG
	{
		short	evgSize;				// 0 Size of the group (<=0)
		BYTE	evgNCond;				// 2 Number of conditions
		BYTE	evgNAct;				// 3 Number of actions
		WORD	evgFlags;				// 4 Flags
		WORD	evgFree;				// 6 Alignment
		DWORD	evgInhibit;				// 8 If the group is inhibited
		DWORD	evgInhibitCpt;			// 12 Counter
	} eventGroup;
typedef		eventGroup	*		PEVG;
typedef		eventGroup	*		LPEVG;
#define		EVG_SIZE				14

// MACRO: next group
#define		EVGNEXT(evgPtr)		   	((LPEVG)((char*)evgPtr-evgPtr->evgSize))
// MACRO: first event
#define		EVGFIRSTEVT(p)	   		((LPEVT)((char*)p+sizeof(eventGroup)))
// MACRO: number of events in the group
#define		EVGNEVENTS(p)	 		(p->evgNCond+p->evgNAct)

// Internal flags of eventgroups
#define		EVGFLAGS_ONCE			0x0001
#define		EVGFLAGS_NOTALWAYS		0x0002
#define		EVGFLAGS_REPEAT			0x0004
#define		EVGFLAGS_NOMORE			0x0008
#define		EVGFLAGS_SHUFFLE		0x0010
#define		EVGFLAGS_EDITORMARK		0x0020
#define		EVGFLAGS_HASCHILDREN	0x0040
#define		EVGFLAGS_BREAK	  	0x0080
#define		EVGFLAGS_BREAKPOINT		0x0100
#define		EVGFLAGS_ALWAYSCLEAN	0x0200
#define		EVGFLAGS_ORINGROUP		0x0400
//#define		EVGFLAGS_2MANYACTIONS	0x0400
#define		EVGFLAGS_STOPINGROUP	0x0800
//#define		EVGFLAGS_NOTASSEMBLED	0x1000
#define		EVGFLAGS_ORLOGICAL		0x1000
#define		EVGFLAGS_GROUPED		0x2000
#define		EVGFLAGS_INACTIVE		0x4000
#define		EVGFLAGS_HASPARENT	  0x8000
#define		EVGFLAGS_LIMITED		(EVGFLAGS_SHUFFLE+EVGFLAGS_NOTALWAYS+EVGFLAGS_REPEAT+EVGFLAGS_NOMORE)
#define		EVGFLAGS_DEFAULTMASK	(EVGFLAGS_BREAKPOINT+EVGFLAGS_GROUPED+EVGFLAGS_HASCHILDREN+EVGFLAGS_HASPARENT)
//#define		EVGFLAGS_FADE			(EVGFLAGS_FADEIN|EVGFLAGS_FADEOUT)

#define	 EVENTOPTION_BREAKCHILD  0x0001

typedef struct tagEVT
	{
		short	evtSize;				// 0 Size of the event
		union
		{
			struct
			{
				int		evtCode;		// 2 Code (hi:NUM lo:TYPE)
			} evtLCode;
			struct
			{
				short	evtType;		// 2 Type of object
				short	evtNum;			// 4 Number of action/condition
			} evtSCode;
		} evtCode;
		short	evtOi;					// 6 OI if normal object
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
//#define		ACTFLAGS_REPEAT			0x0001

// For flags II
// -------------
#define		EVFLAG2_NOT			0x0001
#define		EVFLAG2_NOTABLE		0x0002
#define		EVFLAGS_NOTABLE		(EVFLAG2_NOTABLE*256)
#define		EVFLAGS_MONITORABLE	0x0004
#define		EVFLAGS_TODELETE	0x0008
#define		EVFLAGS_NEWSOUND	0x0010
#define		EVFLAG2_MASK		(EVFLAG2_NOT|EVFLAG2_NOTABLE|EVFLAGS_MONITORABLE)
#define 	EVFLAG2_NOOBJECTINTERDEPENDENCE	0x20

// MACRO: Returns the code for an extension
#define		EXTCONDITIONNUM(i)		(-((short)(i>>16))-1)
#define		EXTACTIONNUM(i)			((short)(i>>16))
#define		GetEventCode(p)			(p&0xFFFF0000)
#define		EVGDELTA(a, b)			((DWORD)((char*)a-(char*)b))
#define		EVTDELTA(a, b)			((DWORD)((char*)a-(char*)b))
#define		EVGOFFSET(a, b)			((LPEVG)((char*)a+b))
#define		EVTOFFSET(a, b)			((LPEVT)((char*)a+b))
#define		EVTTYPE(a)				((short)a)
#define		EVTNUM(i)				((short)(i>>16))

// PARAM Structure
// ~~~~~~~~~~~~~~~
typedef struct eventParam
	{
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
				int		evpL0;
				int		evpL1;
				int		evpL2;
				int		evpL3;
			} evpL;
		} evp;
	} eventParam;
typedef	eventParam	*			PEVP;
typedef	eventParam	*			LPEVP;
typedef	eventParam	*			fpevp;

// MACRO: next parameter
#define		EVPNEXT(p)		   		((LPEVP)((char*)p+p->evpSize))
#define		EVTNEXT(p)		   		((LPEVT)((char*)p+p->evtSize))
#define		EVTPARAMS(p) 			((LPEVP)(p->evtCode.evtLCode.evtCode<0 ? (char*)p+CND_SIZE : (char*)p+ACT_SIZE ))
#define		EXTCONDITIONNUM(i)		(-((short)(i>>16))-1)
#define		EXTACTIONNUM(i)			((short)(i>>16))

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
typedef struct tagEXP
	{
		union
		{
			struct
			{
				int	expCode;		// 2 Code (hi:NUM lo:TYPE)
			} expLCode;
			struct
			{
				short	expType;		// 2 Type of object
				short	expNum;			// 3 Expression number
			} expSCode;
		} expCode;
		short	expSize;
		union
		{
			struct {
				short	expOi;
				short	expOiList;
			} expo;
			struct {
				int		expLParam;
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
				int		expExtCode;
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
#define		EXPNEXT(expPtr)			((LPEXP)((char*)expPtr+expPtr->expSize))
#define		EXPFLAG_STRING			0x0001
#define		EXPFLAG_DOUBLE			0x0002
#define		nextToken()				rhPtr->rh4ExpToken=(LPEXP)((LPBYTE)rhPtr->rh4ExpToken+rhPtr->rh4ExpToken->expSize)
#define		getCurrentResult()		rhPtr->rh4Results[rhPtr->rh4PosPile]
#define		getPreviousResult()		rhPtr->rh4Results[rhPtr->rh4PosPile-1]
#define		getNextResult()			rhPtr->rh4Results[rhPtr->rh4PosPile+1]
#define	COMPARE_EQ				0
#define COMPARE_NE				1
#define	COMPARE_LE				2
#define	COMPARE_LT				3
#define	COMPARE_GE				4
#define	COMPARE_GT				5

// FAST LOOP ACCELERATION
///////////////////////////////////////////////////////////////
class CPosStartLoop
{
public:
	CPosStartLoop(LPEVP pEvp, NSString* pName)
	{
		m_pEvp = pEvp;
		m_name = pName;
	}
	~CPosStartLoop()
	{
	}
	LPEVP m_pEvp;
	NSString* m_name;
};
class CPosOnLoop
{
public:
	enum
	{
		POL_STEP = 4
	};
	LPDWORD m_deltas;
	int m_length;
	int m_position;
	NSString* m_name;
	BOOL m_bOR;
	int m_fastLoopIndex;

	CPosOnLoop(NSString* pName, int fastLoopIndex)
	{
		m_name = pName;
		m_length = 1;
		m_deltas = (LPDWORD)malloc( (m_length * 2 + 1 )* sizeof(DWORD));
		m_position = 0;
		m_bOR = FALSE;
		m_fastLoopIndex = fastLoopIndex;
	}
	~CPosOnLoop()
	{
		free(m_deltas);
	}
	void AddOnLoop(DWORD delta1, BOOL delta2)
	{
		if (m_position == m_length)
		{
			m_length += POL_STEP;
			m_deltas = (LPDWORD)realloc(m_deltas, (m_length *2 + 1) * sizeof(DWORD));
		}
		m_deltas[m_position * 2] = delta1;
		m_deltas[m_position * 2 + 1] = delta2;
		m_position++;
		m_deltas[m_position * 2] = 0xFFFFFFFF;
	}
};

typedef struct saveSelection {
	CObjInfo* poil;
	std::vector<short> selectedObjects;
} saveSelection;

typedef struct eventOffsets {
	DWORD evgOffset;
	DWORD evtOffset;
} eventOffsets;

///////////////////////////////////////////////////////////////////////
//
// DEFINITION OF EVENTS AND ACTIONS CODES
//
///////////////////////////////////////////////////////////////////////

// SYSTEM Conditions / Actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//#define		OBJ_SYSTEM				-1
//-1&255=255
#define	CND_STARTCHILDEVENT		((-43*256)|255)
#define CNDL_STARTCHILDEVENT	((-43*65536)|65535)
#define	CND_ENDCHILDEVENT		((-42*256)|255)
#define CNDL_ENDCHILDEVENT		((-42*65536)|65535)
#define CND_ISPROFILING			((-41*256)|255)
#define CNDL_ISPROFILING		((-41*65536)|65535)
#define CND_RUNNINGAS			((-40*256)|255)
#define CNDL_RUNNINGAS			((-40*65536)|65535)

#define CND_COMPAREGDBL_GT	-39
#define CND_COMPAREGDBL_GE	-38
#define CND_COMPAREGDBL_LT	-37
#define CND_COMPAREGDBL_LE	-36
#define CND_COMPAREGDBL_NE	-35
#define CND_COMPAREGDBL_EQ	-34
#define CND_COMPAREGINT_GT	-33
#define CND_COMPAREGINT_GE	-32
#define CND_COMPAREGINT_LT	-31
#define CND_COMPAREGINT_LE	-30
#define CND_COMPAREGINT_NE	-29
#define CND_COMPAREGINT_EQ	-28

#define CND_CHANCE			((-26*256)|255)
#define CND_ORLOGICAL		((-25*256)|255)
#define CNDL_ORLOGICAL		((-25*65536)|65535)
#define CND_OR				((-24*256)|255)
#define CNDL_OR				((-24*65536)|65535)
#define CND_GROUPSTART		((-23*256)|255)
#define CNDL_GROUPSTART		((-23*65536)|65535)
#define CND_CLIPBOARD		((-22*256)|255)
#define CND_ONCLOSE			((-21*256)|255)
#define CNDL_ONCLOSE		((-21*65536)|65535)
#define	CND_COMPAREGSTRING	((-20*256)|255)
#define	CNDL_COMPAREGSTRING	((-20*65536)|65535)
#define CND_MENUVISIBLE		((-19*256)|255)
#define CND_MENUENABLED		((-18*256)|255)
#define CND_MENUCHECKED		((-17*256)|255)
#define	CND_ONLOOP			((-16*256)|255)
#define	CNDL_ONLOOP			((-16*65536)|65535)
#define CND_DROPFILES		((-15*256)|255)
#define CNDL_DROPFILES		((-15*65536)|65535)
#define	CND_MENUSELECTED	((-14*256)|255)
#define CNDL_MENUSELECTED	((-14*65536)|65535)
#define	CND_RECORDKEY		((-13*256)|255)
#define CNDL_RECORDKEY		((-13*65536)|65535)
#define	CND_GROUPACTIVATED	((-12*256)|255)
#define	CNDL_GROUPACTIVATED	((-12*65536)|65535)
#define	CND_ENDGROUP		((-11*256)|255)
#define CNDL_ENDGROUP		((-11*65536)|65535)
#define	CND_GROUP			((-10*256)|255)
#define CNDL_GROUP			((-10*65536)|65535)
#define	CND_REMARK			((-9*256)|255)
#define CNDL_REMARK			((-9*65536)|65535)
#define	CND_COMPAREG		((-8*256)|255)
#define	CNDL_COMPAREG		((-8*65536)|65535)
#define	CND_NOTALWAYS		((-7*256)|255)
#define	CNDL_NOTALWAYS		((-7*65536)|65535)
#define	CND_ONCE			((-6*256)|255)
#define CNDL_ONCE			((-6*65536)|65535)
#define	CND_REPEAT			((-5*256)|255)
#define CNDL_REPEAT			((-5*65536)|65535)
#define	CND_NOMORE			((-4*256)|255)
#define CNDL_NOMORE			((-4*65536)|65535)
#define	CND_COMPARE			((-3*256)|255)
#define	CND_NEVER			((-2*256)|255)
#define CNDL_NEVER			((-2*65536)|65535)
#define	CND_ALWAYS			((-1*256)|255)
#define CNDL_ALWAYS			((-1*65536)|65535)

#define	ACT_SKIP			((0*256)|255)
#define	ACTL_SKIP			((0*65536)|65535)
#define	ACT_SKIPMONITOR		((1*256)|255)
#define	ACTL_SKIPMONITOR	((1*65536)|65535)
#define	ACT_EXECPROG		((2*256)|255)
#define	ACT_SETVARG			((3*256)|255)
#define	ACTL_SETVARG		((3*65536)|65535)
#define	ACT_SUBVARG			((4*256)|255)
#define	ACTL_SUBVARG		((4*65536)|65535)
#define	ACT_ADDVARG			((5*256)|255)
#define	ACTL_ADDVARG		((5*65536)|65535)
#define	ACT_GRPACTIVATE		((6*256)|255)
#define	ACTL_GRPACTIVATE	((6*65536)|65535)
#define	ACT_GRPDEACTIVATE	((7*256)|255)
#define	ACTL_GRPDEACTIVATE	((7*65536)|65535)
#define	ACT_MENUACTIVATE	((8*256)|255)
#define	ACT_MENUDEACTIVATE	((9*256)|255)
#define	ACT_MENUCHECK		((10*256)|255)
#define	ACT_MENUNCHECK		((11*256)|255)
#define	ACT_MENUSHOW		((12*256)|255)
#define	ACTL_MENUSHOW		((12*65536)|65535)
#define	ACT_MENUHIDE		((13*256)|255)
#define	ACTL_MENUHIDE		((13*65536)|65535)
#define	ACT_STARTLOOP		((14*256)|255)
#define	ACTL_STARTLOOP		((14*65536)|65535)
#define	ACT_STOPLOOP		((15*256)|255)
#define	ACTL_STOPLOOP		((15*65536)|65535)
#define	ACT_SETLOOPINDEX	((16*256)|255)
#define	ACTL_SETLOOPINDEX	((16*65536)|65535)
#define	ACT_RANDOMIZE		((17*256)|255)
#define ACT_SENDMENUCMD		((18*256)|255)
#define ACT_SETGLOBALSTRING	((19*256)|255)
#define	ACTL_SETGLOBALSTRING ((19*65536)|65535)
#define ACT_SENDCLIPBOARD	((20*256)|255)
#define ACT_CLEARCLIPBOARD	((21*256)|255)
#define ACT_EXECPROG2		((22*256)|255)
#define ACT_OPENDEBUGGER	((23*256)|255)
#define ACT_PAUSEDEBUGGER	((24*256)|255)
#define ACT_EXTRACTBINFILE	((25*256)|255)
#define ACT_RELEASEBINFILE	((26*256)|255)

// Optimized functions (hidden)
enum {
	ACT_SETVARGINT = 27,
	ACT_SETVARGINTNUMEXP,
	ACT_SETVARGDBL,
	ACT_SETVARGDBLNUMEXP,
	ACT_ADDVARGINT,
	ACT_ADDVARGINTNUMEXP,
	ACT_ADDVARGDBL,
	ACT_ADDVARGDBLNUMEXP,
	ACT_SUBVARGINT,
	ACT_SUBVARGINTNUMEXP,
	ACT_SUBVARGDBL,
	ACT_SUBVARGDBLNUMEXP,
};

#define ACT_STARTPROFILING	((39*256)|255)
#define ACTL_STARTPROFILING	((39*65536)|65535)
#define ACT_STOPPROFILING	((40*256)|255)
#define ACTL_STOPPROFILING	((40*65536)|65535)
#define ACT_CLEARDEBUGOUTPUT	((41*256)|255)
#define ACTL_CLEARDEBUGOUTPUT	((41*65536)|65535)
#define ACT_DEBUGOUTPUT		((42*256)|255)
#define ACTL_DEBUGOUTPUT	((42*65536)|65535)
#define ACT_EXECUTECHILDEVENTS	((43*256)|255)
#define ACTL_EXECUTECHILDEVENTS	((43*65536)|65535)

#define	EXP_LONG			((0*256)|255)
#define	EXPL_LONG			((0*65536)|65535)
#define	EXP_RANDOM			((1*256)|255)
#define	EXPL_RANDOM			((1*65536)|65535)
#define	EXP_VARGLO			((2*256)|255)
#define	EXPL_VARGLO			((2*65536)|65535)
#define	EXP_STRING			((3*256)|255)
#define	EXPL_STRING			((3*65536)|65535)
#define	EXP_STR				((4*256)|255)
#define	EXPL_STR			((4*65536)|65535)
#define	EXP_VAL				((5*256)|255)
#define	EXPL_VAL			((5*65536)|65535)
#define	EXP_DRIVE			((6*256)|255)
#define	EXPL_DRIVE			((6*65536)|65535)
#define	EXP_DIRECTORY		((7*256)|255)
#define	EXPL_DIRECTORY		((7*65536)|65535)
#define	EXP_PATH			((8*256)|255)
#define	EXPL_PATH			((8*65536)|65535)
#define	EXP_APPNAME			((9*256)|255)
#define	EXPL_APPNAME		((9*65536)|65535)
#define EXP_SIN				((10*256)|255)
#define EXPL_SIN			((10*65536)|65535)
#define EXP_COS				((11*256)|255)
#define EXPL_COS			((11*65536)|65535)
#define EXP_TAN				((12*256)|255)
#define EXPL_TAN			((12*65536)|65535)
#define EXP_SQR				((13*256)|255)
#define EXPL_SQR			((13*65536)|65535)
#define EXP_LOG				((14*256)|255)
#define EXPL_LOG			((14*65536)|65535)
#define EXP_LN				((15*256)|255)
#define EXPL_LN				((15*65536)|65535)
#define EXP_HEX				((16*256)|255)
#define EXPL_HEX			((16*65536)|65535)
#define EXP_BIN				((17*256)|255)
#define EXPL_BIN			((17*65536)|65535)
#define EXP_EXP				((18*256)|255)
#define EXPL_EXP			((18*65536)|65535)
#define EXP_LEFT			((19*256)|255)
#define EXPL_LEFT			((19*65536)|65535)
#define EXP_RIGHT			((20*256)|255)
#define EXPL_RIGHT			((20*65536)|65535)
#define EXP_MID				((21*256)|255)
#define EXPL_MID			((21*65536)|65535)
#define EXP_LEN				((22*256)|255)
#define EXPL_LEN			((22*65536)|65535)
#define	EXP_DOUBLE			((23*256)|255)
#define	EXPL_DOUBLE			((23*65536)|65535)
#define	EXP_VARGLONAMED		((24*256)|255)
#define	EXPL_VARGLONAMED	((24*65536)|65535)
#define EXP_ENTERSTRINGHERE	((25*256)|255)
#define EXPL_ENTERSTRINGHERE ((25*65536)|65535)
#define EXP_ENTERVALUEHERE	((26*256)|255)
#define EXPL_ENTERVALUEHERE	((26*65536)|65535)
#define	EXP_FLOAT			((27*256)|255)
#define	EXPL_FLOAT			((27*65536)|65535)
#define	EXP_INT				((28*256)|255)
#define	EXPL_INT			((28*65536)|65535)
#define EXP_ABS				((29*256)|255)
#define EXPL_ABS			((29*65536)|65535)
#define EXP_CEIL			((30*256)|255)
#define EXPL_CEIL			((30*65536)|65535)
#define	EXP_FLOOR			((31*256)|255)
#define	EXPL_FLOOR			((31*65536)|65535)
#define	EXP_ACOS			((32*256)|255)
#define	EXPL_ACOS			((32*65536)|65535)
#define	EXP_ASIN			((33*256)|255)
#define	EXPL_ASIN			((33*65536)|65535)
#define	EXP_ATAN			((34*256)|255)
#define	EXPL_ATAN			((34*65536)|65535)
#define	EXP_NOT				((35*256)|255)
#define	EXPL_NOT			((35*65536)|65535)
#define	EXP_NDROPFILES		((36*256)|255)
#define	EXPL_NDROPFILES		((36*65536)|65535)
#define	EXP_DROPFILE		((37*256)|255)
#define	EXPL_DROPFILE		((37*65536)|65535)
#define	EXP_GETCOMMANDLINE	((38*256)|255)
#define	EXPL_GETCOMMANDLINE	((38*65536)|65535)
#define	EXP_GETCOMMANDITEM	((39*256)|255)
#define	EXPL_GETCOMMANDITEM	((39*65536)|65535)
#define	EXP_MIN				((40*256)|255)
#define	EXPL_MIN			((40*65536)|65535)
#define	EXP_MAX				((41*256)|255)
#define	EXPL_MAX			((41*65536)|65535)
#define	EXP_GETRGB			((42*256)|255)
#define	EXPL_GETRGB			((42*65536)|65535)
#define	EXP_GETRED			((43*256)|255)
#define	EXPL_GETRED			((43*65536)|65535)
#define	EXP_GETGREEN		((44*256)|255)
#define	EXPL_GETGREEN		((44*65536)|65535)
#define	EXP_GETBLUE			((45*256)|255)
#define	EXPL_GETBLUE		((45*65536)|65535)
#define	EXP_LOOPINDEX		((46*256)|255)
#define	EXPL_LOOPINDEX		((46*65536)|65535)
#define EXP_NEWLINE			((47*256)|255)
#define EXPL_NEWLINE		((47*65536)|65535)
#define EXP_ROUND			((48*256)|255)
#define EXPL_ROUND			((48*65536)|65535)
#define	EXP_STRINGGLO		((49*256)|255)
#define	EXPL_STRINGGLO		((49*65536)|65535)
#define	EXP_STRINGGLONAMED	((50*256)|255)
#define	EXPL_STRINGGLONAMED	((50*65536)|65535)
#define EXP_LOWER			((51*256)|255)
#define EXPL_LOWER			((51*65536)|65535)
#define EXP_UPPER			((52*256)|255)
#define EXPL_UPPER			((52*65536)|65535)
#define EXP_FIND			((53*256)|255)
#define EXPL_FIND			((53*65536)|65535)
#define EXP_REVERSEFIND		((54*256)|255)
#define EXPL_REVERSEFIND	((54*65536)|65535)
#define EXP_GETCLIPBOARD	((55*256)|255)
#define	EXP_TEMPPATH		((56*256)|255)
#define EXP_BINFILETEMPNAME	((57*256)|255)
#define	EXP_FLOATSTR		((58*256)|255)
#define	EXPL_FLOATSTR		((58*65536)|65535)
#define	EXP_ATAN2			((59*256)|255)
#define	EXPL_ATAN2			((59*65536)|65535)
#define	EXP_ZERO			((60*256)|255)
#define	EXPL_ZERO			((60*65536)|65535)
#define	EXP_EMPTY			((61*256)|255)
#define	EXPL_EMPTY			((61*65536)|65535)
//#define	EXP_DISTANCE		((62*256)|255)
//#define	EXP_ANGLE			((63*256)|255)
#define	EXP_RANGE			((64*256)|255)
#define EXP_RANDOMRANGE		((65*256)|255)

// Hidden
#define EXP_LOOPINDEXBYINDEX	((66*256)|255)
#define EXPL_LOOPINDEXBYINDEX   ((66*65536)|65535)

#define	EXP_PARENTH1		((-1*256)|255)
#define	EXPL_PARENTH1		((-1*65536)|65535)
#define	EXP_PARENTH2		((-2*256)|255)
#define	EXPL_PARENTH2		((-2*65536)|65535)
#define	EXP_VIRGULE			((-3*256)|255)
#define	EXPL_VIRGULE		((-3*65536)|65535)

// SPEAKER Conditions / Actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// #define		TYPE_SPEAKER	  		-2
//(TYPE_SPEAKER&255)=254
#define CND_SPCHANNELPAUSED ((-9*256)|254)
#define CNDL_SPCHANNELPAUSED ((-9*65536)|65534)
#define CND_NOSPCHANNELPLAYING ((-8*256)|254)
#define CNDL_NOSPCHANNELPLAYING ((-8*65536)|65534)
#define	CND_MUSPAUSED		((-7*256)|254)
#define	CND_SPSAMPAUSED		((-6*256)|254)
#define	CNDL_SPSAMPAUSED	((-6*65536)|65534)
#define	CND_MUSICENDS		((-5*256)|254)
#define	CNDL_MUSICENDS		((-5*65536)|65534)
#define	CND_NOMUSPLAYING	((-4*256)|254)
#define	CNDL_NOMUSPLAYING	((-4*65536)|65534)
#define	CND_NOSAMPLAYING	((-3*256)|254)
#define	CNDL_NOSAMPLAYING	((-3*65536)|65534)
#define	CND_NOSPMUSPLAYING	((-2*256)|254)
#define	CND_NOSPSAMPLAYING	((-1*256)|254)
#define	CNDL_NOSPSAMPLAYING	((-1*65536)|65534)
#define	ACT_PLAYSAMPLE		((0*256)|254)
#define	ACTL_PLAYSAMPLE		((0*65536)|65534)
#define	ACT_STOPSAMPLE		((1*256)|254)
#define	ACTL_STOPSAMPLE		((1*65536)|65534)
#define	ACT_PLAYMUSIC		((2*256)|254)
#define	ACTL_PLAYMUSIC		((2*65536)|65534)
#define	ACT_STOPMUSIC		((3*256)|254)
#define	ACTL_STOPMUSIC		((3*65536)|65534)
#define	ACT_PLAYLOOPSAMPLE 	((4*256)|254)
#define	ACTL_PLAYLOOPSAMPLE ((4*65536)|65534)
#define	ACT_PLAYLOOPMUSIC 	((5*256)|254)
#define	ACT_STOPSPESAMPLE	((6*256)|254)
#define	ACTL_STOPSPESAMPLE	((6*65536)|65534)
#define	ACT_PAUSESAMPLE		((7*256)|254)
#define	ACTL_PAUSESAMPLE	((7*65536)|65534)
#define	ACT_RESUMESAMPLE	((8*256)|254)
#define	ACTL_RESUMESAMPLE	((8*65536)|65534)
#define	ACT_PAUSEMUSIC		((9*256)|254)
#define	ACT_RESUMEMUSIC		((10*256)|254)
#define	ACT_PLAYCHANNEL		((11*256)|254)
#define	ACTL_PLAYCHANNEL	((11*65536)|65534)
#define	ACT_PLAYLOOPCHANNEL	((12*256)|254)
#define	ACTL_PLAYLOOPCHANNEL	((12*65536)|65534)
#define	ACT_PAUSECHANNEL	((13*256)|254)
#define	ACTL_PAUSECHANNEL	((13*65536)|65534)
#define	ACT_RESUMECHANNEL	((14*256)|254)
#define	ACTL_RESUMECHANNEL	((14*65536)|65534)
#define	ACT_STOPCHANNEL		((15*256)|254)
#define	ACTL_STOPCHANNEL		((15*65536)|65534)
#define	ACT_SETCHANNELPOS	((16*256)|254)
#define	ACTL_SETCHANNELPOS	((16*65536)|65534)
#define	ACT_SETCHANNELVOL	((17*256)|254)
#define	ACTL_SETCHANNELVOL	((17*65536)|65534)
#define	ACT_SETCHANNELPAN	((18*256)|254)
#define	ACTL_SETCHANNELPAN	((18*65536)|65534)
#define	ACT_SETSAMPLEPOS	((19*256)|254)
#define	ACTL_SETSAMPLEPOS	((19*65536)|65534)
#define	ACT_SETSAMPLEMAINVOL	((20*256)|254)
#define	ACTL_SETSAMPLEMAINVOL	((20*65536)|65534)
#define	ACT_SETSAMPLEVOL		((21*256)|254)
#define	ACTL_SETSAMPLEVOL		((21*65536)|65534)
#define	ACT_SETSAMPLEMALNPAN	((22*256)|254)
#define	ACTL_SETSAMPLEMALNPAN	((22*65536)|65534)
#define	ACT_SETSAMPLEPAN		((23*256)|254)
#define	ACTL_SETSAMPLEPAN		((23*65536)|65534)
#define ACT_PAUSEALLCHANNELS	((24*256)|254)
#define ACTL_PAUSEALLCHANNELS	((24*65536)|65534)
#define ACT_RESUMEALLCHANNELS	((25*256)|254)
#define ACTL_RESUMEALLCHANNELS	((25*65536)|65534)
#define ACT_PLAYMUSICFILE		((26*256)|254)
#define ACT_PLAYLOOPMUSICFILE	((27*256)|254)
#define ACT_PLAYFILECHANNEL		((28*256)|254)
#define	ACTL_PLAYFILECHANNEL	((28*65536)|65534)
#define ACT_PLAYLOOPFILECHANNEL		((29*256)|254)
#define ACTL_PLAYLOOPFILECHANNEL	((29*65536)|65534)
#define ACT_LOCKCHANNEL			((30*256)|254)
#define ACTL_LOCKCHANNEL			((30*65536)|65534)
#define ACT_UNLOCKCHANNEL		((31*256)|254)
#define ACTL_UNLOCKCHANNEL		((31*65536)|65534)
#define ACT_SETCHANNELFREQ		((32*256)|254)
#define ACT_SETSAMPLEFREQ		((33*256)|254)

#define	EXP_GETSAMPLEMAINVOL	((0*256)|254)
#define	EXP_GETSAMPLEVOL		((1*256)|254)
#define	EXP_GETCHANNELVOL		((2*256)|254)
#define	EXPL_GETCHANNELVOL		((2*65536)|65534)
#define	EXP_GETSAMPLEMAINPAN	((3*256)|254)
#define	EXP_GETSAMPLEPAN		((4*256)|254)
#define	EXP_GETCHANNELPAN		((5*256)|254)
#define	EXPL_GETCHANNELPAN		((5*65536)|65534)
#define	EXP_GETSAMPLEPOS		((6*256)|254)
#define	EXP_GETCHANNELPOS		((7*256)|254)
#define	EXPL_GETCHANNELPOS		((7*65536)|65534)
#define	EXP_GETSAMPLEDUR		((8*256)|254)
#define	EXP_GETCHANNELDUR		((9*256)|254)
#define	EXP_GETSAMPLEFREQ		((10*256)|254)
#define	EXP_GETCHANNELFREQ		((11*256)|254)


// GAME Conditions / Actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//#define		TYPE_GAME		  		-3
//(TYPE_GAME&255)=253

#define CND_FRAMESAVED		((-10*256)|253)
#define CND_FRAMELOADED		((-9*256)|253)
#define	CNDL_ENDOFPAUSE		((-8*65536)|65533)
#define CND_ENDOFPAUSE		((-8*256)|253)
#define CND_ISVSYNCON		((-7*256)|253)
#define	CND_ISLADDER		((-6*256)|253)
#define	CND_ISOBSTACLE		((-5*256)|253)
#define	CND_QUITAPPLICATION	((-4*256)|253)
#define	CNDL_QUITAPPLICATION	((-4*65536)|65533)
#define	CND_LEVEL			((-3*256)|253)
#define	CND_END				((-2*256)|253)
#define	CNDL_END			((-2*65536)|65533)
#define	CND_START			((-1*256)|253)
#define	CNDL_START			((-1*65536)|65533)

#define	ACT_NEXTLEVEL	   ((0*256)|253)
#define	ACTL_NEXTLEVEL	   ((0*65536)|65533)
#define	ACT_PREVLEVEL	   ((1*256)|253)
#define	ACTL_PREVLEVEL	   ((1*65536)|65533)
#define	ACT_GOLEVEL	 	((2*256)|253)
#define	ACTL_GOLEVEL	 	((2*65536)|65533)
#define	ACT_PAUSE		   ((3*256)|253)
#define	ACT_ENDGAME		 ((4*256)|253)
#define	ACTL_ENDGAME		 ((4*65536)|65533)
#define	ACT_RESTARTGAME	 ((5*256)|253)
#define	ACTL_RESTARTGAME	 ((5*65536)|65533)
#define	ACT_RESTARTLEVEL	((6*256)|253)
#define	ACT_CDISPLAY		((7*256)|253)
#define	ACT_CDISPLAYX		((8*256)|253)
#define	ACT_CDISPLAYY		((9*256)|253)
#define	ACT_LOADGAME		((10*256)|253)
#define	ACT_SAVEGAME		((11*256)|253)
#define ACT_CLS				((12*256)|253)
#define	ACT_CLEARZONE		((13*256)|253)
#define ACT_FULLSCREENMODE	((14*256)|253)
#define ACT_WINDOWEDMODE	((15*256)|253)
#define ACT_SETFRAMERATE	((16*256)|253)
#define ACT_PAUSEKEY		((17*256)|253)
#define ACT_PAUSEANYKEY		((18*256)|253)
#define	ACT_SETVSYNCON		((19*256)|253)
#define	ACT_SETVSYNCOFF		((20*256)|253)
#define	ACT_SETVIRTUALWIDTH	((21*256)|253)
#define	ACT_SETVIRTUALHEIGHT ((22*256)|253)
#define ACT_SETFRAMEBDKCOLOR ((23*256)|253)
#define ACT_DELCREATEDBKDAT ((24*256)|253)
#define ACT_DELALLCREATEDBKD ((25*256)|253)
#define ACT_SETFRAMEWIDTH	((26*256)|253)
#define ACT_SETFRAMEHEIGHT	((27*256)|253)
#define ACT_SAVEFRAME		((28*256)|253)
#define ACT_LOADFRAME		((29*256)|253)
#define ACT_LOADAPPLICATION	((30*256)|253)
#define ACT_PLAYDEMO		((31*256)|253)
#define ACT_SETFRAMEEFFECT	((32*256)|253)
#define ACT_SETFRAMEEFFECTPARAM	((33*256)|253)
#define ACT_SETFRAMEEFFECTPARAMTEXTURE	((34*256)|253)
#define ACT_SETFRAMEALPHACOEF	((35*256)|253)
#define ACT_SETFRAMERGBCOEF	((36*256)|253)

#define	EXP_GAMLEVEL		((0*256)|253)
#define	EXP_GAMNPLAYER		((1*256)|253)
#define	EXP_PLAYXLEFT		((2*256)|253)
#define	EXP_PLAYXRIGHT		((3*256)|253)
#define	EXP_PLAYYTOP		((4*256)|253)
#define	EXP_PLAYYBOTTOM		((5*256)|253)
#define	EXP_PLAYWIDTH		((6*256)|253)
#define	EXP_PLAYHEIGHT		((7*256)|253)
#define	EXP_GAMLEVELNEW		((8*256)|253)
#define	EXP_GETCOLLISIONMASK ((9*256)|253)
#define EXP_FRAMERATE		((10*256)|253)
#define EXP_GETVIRTUALWIDTH	((11*256)|253)
#define EXP_GETVIRTUALHEIGHT ((12*256)|253)
#define EXP_GETFRAMEBKDCOLOR ((13*256)|253)
#define	EXP_GRAPHICMODE		((14*256)|253)
#define	EXP_PIXELSHADERVERSION ((15*256)|253)
#define EXP_FRAMEALPHACOEF	((16*256)|253)
#define EXP_FRAMERGBCOEF	((17*256)|253)
#define EXP_FRAMEEFFECTPARAM	((18*256)|253)


// TIMER Conditions / Actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define	CND_EVERY2	   	((-8*256)|(OBJ_TIMER&255))
#define CND_TIMEREQUALS		((-7*256)|(OBJ_TIMER&255))
#define CND_ONEVENT			((-6*256)|(OBJ_TIMER&255))
#define	CND_TIMEOUT	   	((-5*256)|(OBJ_TIMER&255))
#define	CND_EVERY	   	((-4*256)|(OBJ_TIMER&255))
#define	CNDL_EVERY	   	((-4*65536)|(OBJ_TIMER&0xFFFF))
#define	CND_TIMER	   	((-3*256)|(OBJ_TIMER&255))
#define	CNDL_TIMER	   	((-3*65536)|(OBJ_TIMER&0xFFFF))
#define	CND_TIMERINF	   	((-2*256)|(OBJ_TIMER&255))
#define	CNDL_TIMERINF	  	((-2*65536)|(OBJ_TIMER&0xFFFF))
#define	CND_TIMERSUP	   	((-1*256)|(OBJ_TIMER&255))
#define	CNDL_TIMERSUP	  	((-1*65536)|(OBJ_TIMER&0xFFFF))
#define	ACT_SETTIMER		((0*256)|(OBJ_TIMER&255))
#define	ACT_EVENTAFTER	  ((1*256)|(OBJ_TIMER&255))
#define	ACT_NEVENTSAFTER	((2*256)|(OBJ_TIMER&255))
#define	EXP_TIMVALUE		((0*256)|(OBJ_TIMER&255))
#define	EXP_TIMCENT			((1*256)|(OBJ_TIMER&255))
#define	EXP_TIMSECONDS		((2*256)|(OBJ_TIMER&255))
#define	EXP_TIMHOURS		((3*256)|(OBJ_TIMER&255))
#define	EXP_TIMMINITS		((4*256)|(OBJ_TIMER&255))
#define	EXP_EVENTAFTER		((5*256)|(OBJ_TIMER&255))
#define NUM_ONEVENT			-6
#define	NUM_EVERY	   	-4
#define	NUM_TIMER	   	-3
#define	NUM_TIMERINF	   	-2
#define	NUM_TIMERSUP	   	-1

// KEYBOARD Conditions / Actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define	CNDL_ONMOUSEWHEELDOWN  	((-12*65536)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_ONMOUSEWHEELDOWN   	((-12*256)|(OBJ_KEYBOARD&255))
#define	CNDL_ONMOUSEWHEELUP  	((-11*65536)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_ONMOUSEWHEELUP   	((-11*256)|(OBJ_KEYBOARD&255))
#define	CND_MOUSEON		   	((-10*256)|(OBJ_KEYBOARD&255))
#define	CND_ANYKEY			((-9*256)|(OBJ_KEYBOARD&255))
#define	CNDL_ANYKEY			((-9*65536)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_MKEYDEPRESSED	((-8*256)|(OBJ_KEYBOARD&255))
#define	CND_MCLICKONOBJECT	((-7*256)|(OBJ_KEYBOARD&255))
#define	CNDL_MCLICKONOBJECT	((-7*65536)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_MCLICKINZONE 	((-6*256)|(OBJ_KEYBOARD&255))
#define	CNDL_MCLICKINZONE 	((-6*65536)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_MCLICK	 		((-5*256)|(OBJ_KEYBOARD&255))
#define	CNDL_MCLICK	 		((-5*65536)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_MONOBJECT		((-4*256)|(OBJ_KEYBOARD&255))
#define	CNDL_MONOBJECT		((-4*65536)|(OBJ_KEYBOARD&0xFFFF))
#define	CND_MINZONE			((-3*256)|(OBJ_KEYBOARD&255))
#define	CND_KBKEYDEPRESSED 	((-2*256)|(OBJ_KEYBOARD&255))
#define	CND_KBPRESSKEY   	((-1*256)|(OBJ_KEYBOARD&255))
#define CNDL_KBPRESSKEY		((-1*65536)|(OBJ_KEYBOARD&0xFFFF))
#define	ACT_HIDECURSOR		((0*256)|(OBJ_KEYBOARD&255))
#define	ACT_SHOWCURSOR		((1*256)|(OBJ_KEYBOARD&255))
#define	EXP_XMOUSE			((0*256)|(OBJ_KEYBOARD&255))
#define	EXP_YMOUSE			((1*256)|(OBJ_KEYBOARD&255))
#define	EXP_MOUSEWHEELDELTA	((2*256)|(OBJ_KEYBOARD&255))


// PLAYERS Conditions / Actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define	CND_JOYPUSHED		((-6*256)|(OBJ_PLAYER&255))
#define	CND_NOMORELIVE		((-5*256)|(OBJ_PLAYER&255))
#define	CNDL_NOMORELIVE		((-5*65536)|(OBJ_PLAYER&0xFFFF))
#define	CND_JOYPRESSED		((-4*256)|(OBJ_PLAYER&255))
#define	CNDL_JOYPRESSED		((-4*65536)|(OBJ_PLAYER&0xFFFF))
#define	CND_LIVE			((-3*256)|(OBJ_PLAYER&255))
#define	CND_SCORE			((-2*256)|(OBJ_PLAYER&255))
#define	CND_PLAYERPLAYING   ((-1*256)|(OBJ_PLAYER&255))

#define	ACT_SETSCORE	  	((0*256)|(OBJ_PLAYER&255))
#define	ACT_SETLIVES	  	((1*256)|(OBJ_PLAYER&255))
#define	ACT_NOINPUT	  	((2*256)|(OBJ_PLAYER&255))
#define	ACT_RESTINPUT	  	((3*256)|(OBJ_PLAYER&255))
#define	ACT_ADDSCORE	  	((4*256)|(OBJ_PLAYER&255))
#define	ACT_ADDLIVES	  	((5*256)|(OBJ_PLAYER&255))
#define	ACT_SUBSCORE	  	((6*256)|(OBJ_PLAYER&255))
#define	ACT_SUBLIVES	  	((7*256)|(OBJ_PLAYER&255))
#define	ACT_SETINPUT	  	((8*256)|(OBJ_PLAYER&255))
#define	ACT_SETINPUTKEY		((9*256)|(OBJ_PLAYER&255))
#define	ACT_SETPLAYERNAME	((10*256)|(OBJ_PLAYER&255))

#define	EXP_PLASCORE		((0*256)|(OBJ_PLAYER&255))
#define	EXP_PLALIVES		((1*256)|(OBJ_PLAYER&255))
#define	EXP_GETINPUT		((2*256)|(OBJ_PLAYER&255))
#define	EXP_GETINPUTKEY		((3*256)|(OBJ_PLAYER&255))
#define	EXP_GETPLAYERNAME	((4*256)|(OBJ_PLAYER&255))
#define	NUM_JOYPRESSED		-4

// CREATE Conditions / Actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define	CND_CHOOSEALLINLINE	((-23*256)|(OBJ_CREATE&255))
#define	CND_CHOOSEFLAGRESET	((-22*256)|(OBJ_CREATE&255))
#define	CND_CHOOSEFLAGSET 	((-21*256)|(OBJ_CREATE&255))
#define	CND_CHOOSEVALUE 	((-20*256)|(OBJ_CREATE&255))
#define	CND_PICKFROMID		((-19*256)|(OBJ_CREATE&255))
#define	CND_CHOOSEALLINZONE ((-18*256)|(OBJ_CREATE&255))
#define	CND_CHOOSEALL	   ((-17*256)|(OBJ_CREATE&255))
#define	CND_CHOOSEZONE	  ((-16*256)|(OBJ_CREATE&255))
#define	CND_NUMOFALLOBJECT  ((-15*256)|(OBJ_CREATE&255))
#define	CND_NUMOFALLZONE	((-14*256)|(OBJ_CREATE&255))
#define	CND_NOMOREALLZONE   ((-13*256)|(OBJ_CREATE&255))
#define	CND_CHOOSEFLAGRESET_OLD	((-12*256)|(OBJ_CREATE&255))
#define	CND_CHOOSEFLAGSET_OLD 	((-11*256)|(OBJ_CREATE&255))
//...
#define	CND_CHOOSEVALUE_OLD 	((-8*256)|(OBJ_CREATE&255))
#define	CND_PICKFROMID_OLD		((-7*256)|(OBJ_CREATE&255))
#define	CND_CHOOSEALLINZONE_OLD ((-6*256)|(OBJ_CREATE&255))
#define	CND_CHOOSEALL_OLD	   ((-5*256)|(OBJ_CREATE&255))
#define	CND_CHOOSEZONE_OLD	  ((-4*256)|(OBJ_CREATE&255))
#define	CND_NUMOFALLOBJECT_OLD  ((-3*256)|(OBJ_CREATE&255))
#define	CND_NUMOFALLZONE_OLD	((-2*256)|(OBJ_CREATE&255))
#define	CND_NOMOREALLZONE_OLD   ((-1*256)|(OBJ_CREATE&255))
#define	ACT_CREATEBYNAME		((1*256)|(OBJ_CREATE&255))
#define	ACTL_CREATEBYNAME		((1*65536)|(OBJ_CREATE&0xFFFF))
#define	ACT_CREATE				((0*256)|(OBJ_CREATE&255))
#define	EXP_CRENUMBERALL		((0*256)|(OBJ_CREATE&255))
#define	EXP_LASTFIXEDVALUE		((1*256)|(OBJ_CREATE&255))
#define	NUM_END				-2
#define	NUM_START			-1

// COMMON CONDITIONS FOR NORMAL OBJECTS
//////////////////////////////////////////
#define	EVENTS_EXTBASE				80

#define	CNDIDX_EXTONLOOP_INTERNAL_FIRST	(52)					// 1-based index of first available common condition
#define	CNDIDX_EXTONLOOP_INTERNAL_LAST	(EVENTS_EXTBASE)		// 1-based index of last common condition
#define CNDIDX_EXTONLOOP_INTERNAL_MAXNUMBER (CNDIDX_EXTONLOOP_INTERNAL_LAST + 1 - CNDIDX_EXTONLOOP_INTERNAL_FIRST)

// For Active objects their internal oneach conditions continue after the normal conditions of the object
#define NUMBER_OF_ACTIVEOBJECT_CONDITIONS		4
#define	CNDIDX_EXTONLOOP_INTERNAL_LAST_ONLYACTIVEOBJECTS	199 + EVENTS_EXTBASE	// conditions below CNDIDX_EXTONLOOP_INTERNAL_LAST_ONLYACTIVEOBJECTS are reserved for internal oneach conditions for active objects
#define CNDIDX_EXTONLOOP_INTERNAL_MAXNUMBER_ONLYACTIVEOBJECTS (CNDIDX_EXTONLOOP_INTERNAL_LAST_ONLYACTIVEOBJECTS + 1 - NUMBER_OF_ACTIVEOBJECT_CONDITIONS - CNDIDX_EXTONLOOP_INTERNAL_FIRST)

#define	CND_EXTISCOLLIDINGAT		(-50*256)
#define	CNDL_EXTISCOLLIDINGAT	   (-50*65536)
#define CND_EXTCMPINSTANCEDATA		(-49*256)
#define CNDL_EXTCMPINSTANCEDATA		(-49*65536)
#define CND_EXTPICKMAXVALUE			(-48*256)
#define CNDL_EXTPICKMAXVALUE		(-48*65536)
#define CND_EXTPICKMINVALUE			(-47*256)
#define CNDL_EXTPICKMINVALUE		(-47*65536)
#define	CND_EXTCMPLAYER	   			(-46*256)
#define	CNDL_EXTCMPLAYER			(-46*65536)
#define	CND_EXTCOMPARE				(-45*256)
#define	CNDL_EXTCOMPARE				(-45*65536)
#define	CND_EXTPICKCLOSEST			(-44*256)
#define	CNDL_EXTPICKCLOSEST			(-44*65536)
#define	CNDIDX_EXTCMPVARDBL			(-43)
#define	CND_EXTCMPVARDBL			(CNDIDX_EXTCMPVARDBL*256)
#define	CNDL_EXTCMPVARDBL			(CNDIDX_EXTCMPVARDBL*65536)
#define	CNDIDX_EXTCMPVARINT			(-42)
#define	CND_EXTCMPVARINT			(CNDIDX_EXTCMPVARINT*256)
#define	CNDL_EXTCMPVARINT			(CNDIDX_EXTCMPVARINT*65536)
#define	CND_EXTONLOOP		 		(-41*256)
#define	CNDL_EXTONLOOP		 		(-41*65536)
#define	CND_EXTISSTRIKEOUT			(-40*256)
#define CND_EXTISUNDERLINE			(-39*256)
#define CND_EXTISITALIC				(-38*256)
#define CND_EXTISBOLD				(-37*256)
#define	CND_EXTCMPVARSTRING			(-36*256)
#define	CNDL_EXTCMPVARSTRING		(-36*65536)
#define	CND_EXTPATHNODENAME			(-35*256)
#define	CNDL_EXTPATHNODENAME		(-35*65536)
#define	CND_EXTCHOOSE				(-34*256)
#define	CND_EXTNOMOREOBJECT			(-33*256)
#define	CNDL_EXTNOMOREOBJECT		(-33*65536)
#define	CND_EXTNUMOFOBJECT			(-32*256)
#define	CND_EXTNOMOREZONE			(-31*256)
#define	CND_EXTNUMBERZONE			(-30*256)
#define	CND_EXTSHOWN				(-29*256)
#define	CND_EXTHIDDEN				(-28*256)
#define	CND_EXTCMPVAR				(-27*256)
#define	CNDL_EXTCMPVAR				(-27*65536)
#define	CND_EXTCMPVARFIXED			(-26*256)
#define	CND_EXTFLAGSET				(-25*256)
#define	CND_EXTFLAGRESET			(-24*256)
#define	CND_EXTISCOLBACK			(-23*256)
#define	CND_EXTNEARBORDERS			(-22*256)
#define	CND_EXTENDPATH	  			(-21*256)
#define	CNDL_EXTENDPATH	  			(-21*65536)
#define	CND_EXTPATHNODE				(-20*256)
#define	CNDL_EXTPATHNODE			(-20*65536)
#define	CND_EXTCMPACC				(-19*256)
#define	CND_EXTCMPDEC	 			(-18*256)
#define	CND_EXTCMPX	 	  			(-17*256)
#define	CND_EXTCMPY   				(-16*256)
#define	CND_EXTCMPSPEED			 (-15*256)
#define	CND_EXTCOLLISION   			(-14*256)
#define	CNDL_EXTCOLLISION   		(-14*65536)
#define	CND_EXTCOLBACK			  (-13*256)
#define	CNDL_EXTCOLBACK			 (-13*65536)
#define	CND_EXTOUTPLAYFIELD		 (-12*256)
#define	CNDL_EXTOUTPLAYFIELD		(-12*65536)
#define	CND_EXTINPLAYFIELD		  (-11*256)
#define	CNDL_EXTINPLAYFIELD		 (-11*65536)
#define	CND_EXTISOUT				(-10*256)
#define	CNDL_EXTISOUT				(-10*65536)
#define	CND_EXTISIN				 (-9 *256)
#define	CNDL_EXTISIN				(-9 *65536)
#define	CND_EXTFACING			   (-8 *256)
#define	CND_EXTSTOPPED			  (-7 *256)
#define	CND_EXTBOUNCING				(-6 *256)
#define	CND_EXTREVERSED			 (-5 *256)
#define	CND_EXTISCOLLIDING		  (-4 *256)
#define	CNDL_EXTISCOLLIDING		 (-4 *65536)
#define	CND_EXTANIMPLAYING		  (-3 *256)
#define	CND_EXTANIMENDOF			(-2 *256)
#define	CNDL_EXTANIMENDOF			(-2 *65536)
#define	CND_EXTCMPFRAME	 		(-1 *256)

#define	ACT_EXTEXTRA				(0 *256)
enum {
	SUBACT_SETFLAG=1,
	SUBACT_MAX
};
#define	ACT_EXTSETPOS				(1 *256)
#define	ACT_EXTSETX					(2 *256)
#define	ACT_EXTSETY					(3 *256)
#define	ACT_EXTSTOP					(4 *256)
#define	ACTL_EXTSTOP				(4 *65536)
#define	ACT_EXTSTART				(5 *256)
#define	ACT_EXTSPEED				(6 *256)
#define	ACT_EXTMAXSPEED				(7 *256)
#define	ACT_EXTWRAP					(8 *256)
#define	ACTL_EXTWRAP				(8 *65536)
#define	ACT_EXTBOUNCE				(9 *256)
#define	ACTL_EXTBOUNCE				(9 *65536)
#define	ACT_EXTREVERSE				(10*256)
#define	ACT_EXTNEXTMOVE				(11*256)
#define	ACT_EXTPREVMOVE				(12*256)
#define	ACT_EXTSELMOVE				(13*256)
#define	ACT_EXTLOOKAT				(14*256)
#define	ACT_EXTSTOPANIM				(15*256)
#define	ACT_EXTSTARTANIM			(16*256)
#define	ACT_EXTFORCEANIM			(17*256)
#define	ACT_EXTFORCEDIR				(18*256)
#define	ACT_EXTFORCESPEED			(19*256)
#define	ACTL_EXTFORCESPEED			(19*65536)
#define	ACT_EXTRESTANIM				(20*256)
#define	ACT_EXTRESTDIR				(21*256)
#define	ACT_EXTRESTSPEED			(22*256)
#define	ACT_EXTSETDIR				(23*256)
#define	ACT_EXTDESTROY				(24*256)
#define	ACT_EXTSHUFFLE				(25*256)
#define	ACTL_EXTSHUFFLE				(25*65536)
#define	ACT_EXTHIDE					(26*256)
#define	ACT_EXTSHOW					(27*256)
#define	ACT_EXTDISPLAYDURING		(28*256)
#define	ACT_EXTSHOOT				(29*256)
#define	ACTL_EXTSHOOT				(29*65536)
#define	ACT_EXTSHOOTTOWARD			(30*256)
#define	ACTL_EXTSHOOTTOWARD			(30*65536)
#define	ACT_EXTSETVAR				(31*256)
#define	ACTL_EXTSETVAR				(31*65536)
#define	ACT_EXTADDVAR				(32*256)
#define	ACTL_EXTADDVAR				(32*65536)
#define	ACT_EXTSUBVAR				(33*256)
#define	ACTL_EXTSUBVAR				(33*65536)
#define	ACT_EXTDISPATCHVAR			(34*256)
#define	ACTL_EXTDISPATCHVAR			(34*65536)
#define	ACT_EXTSETFLAG				(35*256)
#define	ACTL_EXTSETFLAG				(35*65536)
#define	ACT_EXTCLRFLAG				(36*256)
#define	ACTL_EXTCLRFLAG				(36*65536)
#define	ACT_EXTCHGFLAG				(37*256)
#define	ACTL_EXTCHGFLAG				(37*65536)
#define	ACT_EXTINKEFFECT			(38*256)
#define ACT_EXTSETSEMITRANSPARENCY	(39*256)
#define ACT_EXTFORCEFRAME			(40*256)
#define ACT_EXTRESTFRAME			(41*256)
#define ACT_EXTSETACCELERATION		(42*256)
#define ACT_EXTSETDECELERATION		(43*256)
#define ACT_EXTSETROTATINGSPEED		(44*256)
#define ACT_EXTSETDIRECTIONS		(45*256)
#define ACT_EXTBRANCHNODE			(46*256)
#define ACT_EXTSETGRAVITY			(47*256)
#define ACT_EXTGOTONODE				(48*256)
#define	ACT_EXTSETVARSTRING			(49*256)
#define	ACTL_EXTSETVARSTRING		(49*65536)
#define ACT_EXTSETFONTNAME			(50*256)
#define ACT_EXTSETFONTSIZE			(51*256)
#define ACT_EXTSETBOLD				(52*256)
#define ACT_EXTSETITALIC			(53*256)
#define ACT_EXTSETUNDERLINE			(54*256)
#define	ACT_EXTSETSRIKEOUT			(55*256)
#define	ACT_EXTSETTEXTCOLOR			(56*256)
#define ACT_EXTSPRFRONT				(57*256)
#define ACT_EXTSPRBACK				(58*256)
#define	ACT_EXTMOVEBEFORE			(59*256)
#define	ACT_EXTMOVEAFTER			(60*256)
#define ACT_EXTMOVETOLAYER			(61*256)
#define ACT_EXTADDTODEBUGGER		(62*256)
#define ACT_EXTSETEFFECT			(63*256)
#define ACT_EXTSETEFFECTPARAM		(64*256)
#define ACT_EXTSETALPHACOEF			(65*256)
#define ACT_EXTSETRGBCOEF			(66*256)
#define ACT_EXTSETEFFECTPARAMTEXTURE (67*256)
#define ACT_EXTSETFRICTION			(68*256)
#define ACT_EXTSETELASTICITY		(69*256)
#define ACT_EXTAPPLYIMPULSE			(70*256)
#define ACT_EXTAPPLYANGULARIMPULSE	(71*256)
#define ACT_EXTAPPLYFORCE			(72*256)
#define ACT_EXTAPPLYTORQUE			(73*256)
#define ACT_EXTSETLINEARVELOCITY	(74*256)
#define ACT_EXTSETANGULARVELOCITY	(75*256)
#define ACT_EXTFOREACH				(76*256)
#define ACT_EXTFOREACH2				(77*256)
#define ACTL_EXTFOREACH2			(77*65536)
#define ACT_EXTSTOPFORCE			(78*256)
#define ACT_EXTSTOPTORQUE			(79*256)
#define ACT_EXTSETDENSITY			(80*256)			// NOT USED IN PHYSCS ACTI0N MENU
#define ACT_EXTSETGRAVITYSCALE		(81*256)			// NOT USED IN PHYSCS ACTI0N MENU
#define	ACT_EXTEXT					(80*256)
#define	ACTL_EXTEXT					(80*65536)

#define	EXP_EXTYSPR					( 1*256)
#define	EXP_EXTISPR					( 2*256)
#define	EXP_EXTSPEED	   			( 3*256)
#define	EXP_EXTACC		 			( 4*256)
#define	EXP_EXTDEC		 			( 5*256)
#define	EXP_EXTDIR					( 6*256)
#define	EXP_EXTXLEFT				( 7*256)
#define	EXP_EXTXRIGHT				( 8*256)
#define	EXP_EXTYTOP					( 9*256)
#define	EXP_EXTYBOTTOM				(10*256)
#define	EXP_EXTXSPR					(11*256)
#define	EXP_EXTIDENTIFIER			(12*256)
#define	EXP_EXTFLAG					(13*256)
#define EXP_EXTNANI					(14*256)
#define	EXP_EXTNOBJECTS				(15*256)
#define	EXP_EXTVAR					(16*256)
#define	EXPL_EXTVAR					(16*65536)
#define EXP_EXTGETSEMITRANSPARENCY	(17*256)
#define EXP_EXTNMOVE				(18*256)
#define	EXP_EXTVARSTRING			(19*256)
#define	EXPL_EXTVARSTRING			(19*65536)
#define EXP_EXTGETFONTNAME			(20*256)
#define EXP_EXTGETFONTSIZE			(21*256)
#define EXP_EXTGETFONTCOLOR			(22*256)
#define EXP_EXTGETLAYER				(23*256)
#define EXP_EXTGETGRAVITY			(24*256)
#define EXP_EXTXAP					(25*256)
#define EXP_EXTYAP					(26*256)
#define EXP_EXTALPHACOEF			(27*256)
#define EXP_EXTRGBCOEF				(28*256)
#define EXP_EXTEFFECTPARAM			(29*256)
#define EXP_EXTVARBYINDEX			(30*256)
#define EXP_EXTVARSTRINGBYINDEX		(31*256)
#define EXP_EXTDISTANCE			 (32*256)
#define EXP_EXTANGLE				(33*256)
#define EXP_EXTLOOPINDEX			(34*256)
#define EXP_EXTGETFRICTION		  (35*256)
#define EXP_EXTGETRESTITUTION	   (36*256)
#define EXP_EXTGETDENSITY		   (37*256)
#define EXP_EXTGETVELOCITY		  (38*256)
#define EXP_EXTGETANGLE			 (39*256)
#define EXP_EXTWIDTH				(40*256)
#define EXP_EXTHEIGHT			   (41*256)
#define EXP_EXTGETMASS			  (42*256)
#define EXP_EXTGETANGULARVELOCITY	(43*256)
#define EXP_EXTGETNAME				(44*256)
#define EXP_NUMBEROFSELECTED		(45*256)
#define EXP_EXTINSTANCEDATA			(46*256)

// TEXT Conditions / Actions
////////////////////////////////////////////
#define	ACT_STRDESTROY	  	 (( (EVENTS_EXTBASE+0)*256)|3)
#define	ACT_STRDISPLAY		 (( (EVENTS_EXTBASE+1)*256)|3)
#define	ACT_STRDISPLAYDURING (( (EVENTS_EXTBASE+2)*256)|3)
#define	ACT_STRSETCOLOUR	 (( (EVENTS_EXTBASE+3)*256)|3)
#define	ACT_STRSET	   		 (( (EVENTS_EXTBASE+4)*256)|3)
#define	ACT_STRPREV	   		 (( (EVENTS_EXTBASE+5)*256)|3)
#define	ACT_STRNEXT	   		 (( (EVENTS_EXTBASE+6)*256)|3)
#define ACT_STRDISPLAYSTRING (( (EVENTS_EXTBASE+7)*256)|3)
#define	ACT_STRSETSTRING	 (( (EVENTS_EXTBASE+8)*256)|3)
#define	EXP_STRNUMBER		 (( (EVENTS_EXTBASE+0)*256)|3)
#define	EXP_STRGETCURRENT	 (( (EVENTS_EXTBASE+1)*256)|3)
#define	EXP_STRGETNUMBER	 (( (EVENTS_EXTBASE+2)*256)|3)
#define	EXP_STRGETNUMERIC	 (( (EVENTS_EXTBASE+3)*256)|3)
#define EXP_STRGETNPARA		 (( (EVENTS_EXTBASE+4)*256)|3)

// SPRITES Conditions / Actions
/////////////////////////////////////////////
#define	CND_CMPSCALEY	   	(((-EVENTS_EXTBASE-4)*256)|2)
#define	CND_CMPSCALEX	   	(((-EVENTS_EXTBASE-3)*256)|2)
#define	CND_CMPANGLE	   	(((-EVENTS_EXTBASE-2)*256)|2)
#define	CND_SPRCLICK	   	(((-EVENTS_EXTBASE-1)*256)|2)
#define	ACT_SPRPASTE		(((EVENTS_EXTBASE+0)*256)|2)
#define	ACT_SPRFRONT		(((EVENTS_EXTBASE+1)*256)|2)
#define	ACT_SPRBACK			(((EVENTS_EXTBASE+2)*256)|2)
#define	ACT_SPRADDBKD		(((EVENTS_EXTBASE+3)*256)|2)
#define ACT_SPRREPLACECOLOR	(((EVENTS_EXTBASE+4)*256)|2)
#define ACT_SPRSETSCALE		(((EVENTS_EXTBASE+5)*256)|2)
#define ACT_SPRSETSCALEX	(((EVENTS_EXTBASE+6)*256)|2)
#define ACT_SPRSETSCALEY	(((EVENTS_EXTBASE+7)*256)|2)
#define ACT_SPRSETANGLE		(((EVENTS_EXTBASE+8)*256)|2)
#define ACT_SPRLOADFRAME	(((EVENTS_EXTBASE+9)*256)|2)
#define ACTL_SPRLOADFRAME	(((EVENTS_EXTBASE+9)*65536)|2)
#define EXP_GETRGBAT		(((EVENTS_EXTBASE+0)*256)|2)
#define EXP_GETSCALEX		(((EVENTS_EXTBASE+1)*256)|2)
#define EXP_GETSCALEY		(((EVENTS_EXTBASE+2)*256)|2)
#define EXP_GETANGLE		(((EVENTS_EXTBASE+3)*256)|2)

// COUNTERS Conditions / Actions
//////////////////////////////////////////////
#define	CND_CCOUNTER	   	(((-EVENTS_EXTBASE-1)*256)|7)
#define	ACT_CSETVALUE		(( (EVENTS_EXTBASE+0)*256)|7)
#define	ACT_CADDVALUE		(( (EVENTS_EXTBASE+1)*256)|7)
#define	ACT_CSUBVALUE		(( (EVENTS_EXTBASE+2)*256)|7)
#define	ACT_CSETMIN			(( (EVENTS_EXTBASE+3)*256)|7)
#define	ACT_CSETMAX			(( (EVENTS_EXTBASE+4)*256)|7)
#define	ACT_CSETCOLOR1		(( (EVENTS_EXTBASE+5)*256)|7)
#define	ACT_CSETCOLOR2		(( (EVENTS_EXTBASE+6)*256)|7)
#define	EXP_CVALUE			(( (EVENTS_EXTBASE+0)*256)|7)
#define	EXP_CGETMIN			(( (EVENTS_EXTBASE+1)*256)|7)
#define	EXP_CGETMAX			(( (EVENTS_EXTBASE+2)*256)|7)
#define	EXP_CGETCOLOR1		(( (EVENTS_EXTBASE+3)*256)|7)
#define	EXP_CGETCOLOR2		(( (EVENTS_EXTBASE+4)*256)|7)

// QUESTIONS Conditions / Actions
///////////////////////////////////////////////
#define	CND_QEQUAL			(((-EVENTS_EXTBASE-3)*256)|4)
#define	CNDL_QEQUAL			(((-EVENTS_EXTBASE-3)*65536)|4)
#define	CND_QFALSE			(((-EVENTS_EXTBASE-2)*256)|4)
#define	CNDL_QFALSE			(((-EVENTS_EXTBASE-2)*65536)|4)
#define	CND_QEXACT			(((-EVENTS_EXTBASE-1)*256)|4)
#define	CNDL_QEXACT			(((-EVENTS_EXTBASE-1)*65536)|4)
#define	ACT_QASK			(( (EVENTS_EXTBASE+0)*256)|4)


// Formatted text Conditions / actions / expressions
//////////////////////////////////////////////////////////////////
#define	ACT_RTFSETXPOS					(((EVENTS_EXTBASE+0)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSETYPOS					(((EVENTS_EXTBASE+1)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSETZOOM					(((EVENTS_EXTBASE+2)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_CLEAR				(((EVENTS_EXTBASE+3)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_WORDSTRONCE		(((EVENTS_EXTBASE+4)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_WORDSTRNEXT		(((EVENTS_EXTBASE+5)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_WORDSTRALL		(((EVENTS_EXTBASE+6)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_WORD				(((EVENTS_EXTBASE+7)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_LINE				(((EVENTS_EXTBASE+8)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_PARAGRAPH			(((EVENTS_EXTBASE+9)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_PAGE				(((EVENTS_EXTBASE+10)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_ALL				(((EVENTS_EXTBASE+11)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_RANGE				(((EVENTS_EXTBASE+12)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFSELECT_BOOKMARK			(((EVENTS_EXTBASE+13)*256)|(OBJ_RTF&0x00FF))
#define ACT_RTFSETFOCUSWORD				(((EVENTS_EXTBASE+14)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHT_OFF				(((EVENTS_EXTBASE+15)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTTEXT_COLOR  		(((EVENTS_EXTBASE+16)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTTEXT_BOLD	  		(((EVENTS_EXTBASE+17)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTTEXT_ITALIC 		(((EVENTS_EXTBASE+18)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTTEXT_UNDERL 		(((EVENTS_EXTBASE+19)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTTEXT_OUTL	  		(((EVENTS_EXTBASE+20)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTBACK_COLOR  		(((EVENTS_EXTBASE+21)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTBACK_RECT	  		(((EVENTS_EXTBASE+22)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTBACK_MARKER 		(((EVENTS_EXTBASE+23)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTBACK_HATCH  		(((EVENTS_EXTBASE+24)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFHLIGHTBACK_INVERSE		(((EVENTS_EXTBASE+25)*256)|(OBJ_RTF&0x00FF))
#define	ACT_RTFDISPLAY					(((EVENTS_EXTBASE+26)*256)|(OBJ_RTF&0x00FF))
#define ACT_RTFSETFOCUSPREV				(((EVENTS_EXTBASE+27)*256)|(OBJ_RTF&0x00FF))
#define ACT_RTFSETFOCUSNEXT				(((EVENTS_EXTBASE+28)*256)|(OBJ_RTF&0x00FF))
#define ACT_RTFREMOVEFOCUS				(((EVENTS_EXTBASE+29)*256)|(OBJ_RTF&0x00FF))
#define ACT_RTFAUTOON					(((EVENTS_EXTBASE+30)*256)|(OBJ_RTF&0x00FF))
#define ACT_RTFAUTOOFF					(((EVENTS_EXTBASE+31)*256)|(OBJ_RTF&0x00FF))
#define ACT_RTFINSERTSTRING				(((EVENTS_EXTBASE+32)*256)|(OBJ_RTF&0x00FF))
#define ACT_RTFLOADTEXT					(((EVENTS_EXTBASE+33)*256)|(OBJ_RTF&0x00FF))
#define ACT_RTFINSERTTEXT				(((EVENTS_EXTBASE+34)*256)|(OBJ_RTF&0x00FF))


#define	EXP_RTFXPOS						(((EVENTS_EXTBASE+0)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYPOS						(((EVENTS_EXTBASE+1)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSXPAGE					(((EVENTS_EXTBASE+2)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSYPAGE					(((EVENTS_EXTBASE+3)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFZOOM						(((EVENTS_EXTBASE+4)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFWORDMOUSE				(((EVENTS_EXTBASE+5)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFWORDXY					(((EVENTS_EXTBASE+6)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFWORD						(((EVENTS_EXTBASE+7)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXWORD					(((EVENTS_EXTBASE+8)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYWORD					(((EVENTS_EXTBASE+9)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSXWORD					(((EVENTS_EXTBASE+10)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSYWORD					(((EVENTS_EXTBASE+11)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFLINEMOUSE				(((EVENTS_EXTBASE+12)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFLINEXY					(((EVENTS_EXTBASE+13)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXLINE					(((EVENTS_EXTBASE+14)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYLINE					(((EVENTS_EXTBASE+15)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSXLINE					(((EVENTS_EXTBASE+16)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSYLINE					(((EVENTS_EXTBASE+17)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFPARAMOUSE				(((EVENTS_EXTBASE+18)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFPARAXY					(((EVENTS_EXTBASE+19)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXPARA					(((EVENTS_EXTBASE+20)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYPARA					(((EVENTS_EXTBASE+21)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSXPARA					(((EVENTS_EXTBASE+22)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFSYPARA					(((EVENTS_EXTBASE+23)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXWORDTEXT				(((EVENTS_EXTBASE+24)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYWORDTEXT				(((EVENTS_EXTBASE+25)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXLINETEXT				(((EVENTS_EXTBASE+26)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYLINETEXT				(((EVENTS_EXTBASE+27)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFXPARATEXT				(((EVENTS_EXTBASE+28)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFYPARATEXT				(((EVENTS_EXTBASE+29)*256)|(OBJ_RTF&0x00FF))
#define	EXP_RTFMEMSIZE					(((EVENTS_EXTBASE+30)*256)|(OBJ_RTF&0x00FF))
#define EXP_RTFGETFOCUSWORD				(((EVENTS_EXTBASE+31)*256)|(OBJ_RTF&0x00FF))
#define EXP_RTFGETHYPERLINK				(((EVENTS_EXTBASE+32)*256)|(OBJ_RTF&0x00FF))

#define CND_CCAISPAUSED					(((-EVENTS_EXTBASE-4)*256)|(OBJ_CCA&0x00FF))
#define CND_CCAISVISIBLE				(((-EVENTS_EXTBASE-3)*256)|(OBJ_CCA&0x00FF))
#define CND_CCAAPPFINISHED				(((-EVENTS_EXTBASE-2)*256)|(OBJ_CCA&0x00FF))
#define CNDL_CCAAPPFINISHED				(((-EVENTS_EXTBASE-2)*65536)|(OBJ_CCA&0xFFFF))
#define CND_CCAFRAMECHANGED				(((-EVENTS_EXTBASE-1)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCARESTARTAPP				(((EVENTS_EXTBASE+0)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCARESTARTFRAME				(((EVENTS_EXTBASE+1)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCANEXTFRAME				(((EVENTS_EXTBASE+2)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCAPREVIOUSFRAME			(((EVENTS_EXTBASE+3)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCAENDAPP					(((EVENTS_EXTBASE+4)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCANEWAPP					(((EVENTS_EXTBASE+5)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCAJUMPFRAME				(((EVENTS_EXTBASE+6)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCASETGLOBALVALUE			(((EVENTS_EXTBASE+7)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCASHOW						(((EVENTS_EXTBASE+8)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCAHIDE						(((EVENTS_EXTBASE+9)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCASETGLOBALSTRING			(((EVENTS_EXTBASE+10)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCAPAUSEAPP					(((EVENTS_EXTBASE+11)*256)|(OBJ_CCA&0x00FF))
#define ACT_CCARESUMEAPP				(((EVENTS_EXTBASE+12)*256)|(OBJ_CCA&0x00FF))
#define EXP_CCAGETFRAMENUMBER			(((EVENTS_EXTBASE+0)*256)|(OBJ_CCA&0x00FF))
#define EXP_CCAGETGLOBALVALUE			(((EVENTS_EXTBASE+1)*256)|(OBJ_CCA&0x00FF))
#define EXP_CCAGETGLOBALSTRING			(((EVENTS_EXTBASE+2)*256)|(OBJ_CCA&0x00FF))

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
	int			posDir;					//12
	short		posTypeParent;			//16
	short		posOiList;				//18
	short		posLayer;				//20
} PositionParam;
typedef	PositionParam *			 	LPPOS;

// CREATE PARAM Structure
// ~~~~~~~~~~~~~~~~~~~~~~
typedef struct tagCDP {
	PositionParam   cdpPos;			// Position structure
	short			cdpHFII;		// FrameItemInstance number
	short			cdpOi;			// OI of the object to create
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
	LPEVG		glEvg;
	short		glId;
} groupList;
typedef		groupList *		LPGL;

typedef struct tagGroupFind {
	short id;
	LPEVG evg;
} groupFind;

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

// -------------------------------- A pointer to a filename, version 2
#define		PARAM_FILENAME2			63
#define		PS_FILEN				_MAX_PATH
// B nom de fichier...

// -------------------------------- The name of an effect
#define		PARAM_EFFECT			64
#define		PS_EFFECT				2
// B name of the effect

// -------------------------------- Runtime number
// W0-
#define		PARAM_RUNTIME			67
#define		PS_RUNTIME				2

#define RUNTIME_WINDOWS		0
#define RUNTIME_MAC			1
#define RUNTIME_SWF			2
#define	RUNTIME_ANDROID		3
#define RUNTIME_IOS			4
#define	RUNTIME_HTML5		5
#define	RUNTIME_XNA			6
#define	RUNTIME_MAX			7

// Parameter for On Each loops
#define		PARAM_MULTIPLEVAR	68

#define		MULTIVARF_VALUE1			0x0001
#define		MULTIVARF_VALUE1ISGLOBAL	0x0002
#define		MULTIVARF_VALUE1ISDOUBLE	0x0004
#define		MULTIVARF_VALUE2			0x0010
#define		MULTIVARF_VALUE2ISGLOBAL	0x0020
#define		MULTIVARF_VALUE2ISDOUBLE	0x0040
#define		MULTIVARF_VALUE3			0x0100
#define		MULTIVARF_VALUE3ISGLOBAL	0x0200
#define		MULTIVARF_VALUE3ISDOUBLE	0x0400
#define		MULTIVARF_VALUE4			0x1000
#define		MULTIVARF_VALUE4ISGLOBAL	0x2000
#define		MULTIVARF_VALUE4ISDOUBLE	0x4000

// Used only in editor
#define		MULTIVARF_COMPAREMULTIPLEVALUE		0x80000000

#define		MULTIVAR_MAXVALUE			4

typedef struct evoValue {

	int valIndex;
	DWORD valCompareOp;
	union {
		DWORD valCompareWithLong;
		double valCompareWithDouble;
	};
} evoValue;

typedef struct multipleFlagAndVariableParam {
	DWORD	flags;				// what the structure contains: MULTIVARF_VALUE1 | etc.

	DWORD	flagMasks;			// object flags
	DWORD	flagValues;			// flag values

	evoValue	values[MULTIVAR_MAXVALUE];

} multipleFlagAndVariableParam;

// Nested event
#define PARAM_CHILDEVENT	69

#define PARAM_ZONE_EXPRESSION	72

typedef struct childEventParam {
	DWORD	evgOffsetList;		// offset of eventOffsets list in eventPointers
	short	objList[2];			// object list (oi & oil), ends with -1? ou alors on met le nombre d'abord...
} childEventParam;

//////////////////////////////////////////////////////////////////////////////
// Condition/action jumps
@class CRun;
@class CObject;

typedef BOOL (*CALLCOND1_ROUTINE)(LPEVT pe, CRun* rhPtr, CObject* pHo);
typedef BOOL (*CALLCOND2_ROUTINE)(LPEVT pe, CRun* rhPtr);
typedef void (*CALLACTION_ROUTINE)(LPEVT pe, CRun* rhPtr);
typedef void (*CALLEXP_ROUTINE)(CRun* rhPtr);
typedef BOOL (*CONDROUTINE)(LPEVT pe, CRun* rhPtr, CObject* pHo);
typedef void (*ACTROUTINE)(LPEVT pe, CRun* rhPtr);
typedef void (*EXPROUTINE)(CRun* rhPtr);
typedef void (*CALLOPERATOR_ROUTINE)(CRun* rhPtr);

// EVENEMENTS POUSSES
typedef struct tagPushedEvent {
	int routine;
	int code;
	int param;
	short oi;
	CObject* object;
} PushedEvent;

typedef struct tagTimerEvent
{
	void* next;
	int type;
	NSString* name;
	int timer;
	int timerNext;
	int timerPosition;
	int loops;
	int index;
	BOOL bDelete;
}TimerEvent;
typedef	TimerEvent* LPTIMEREVENT;
#define TIMEREVENTTYPE_ONESHOT 0
#define TIMEREVENTTYPE_REPEAT 1

#define STEPFOREACH 10
typedef struct tagForEach
{
	void* next;
	int length;
	OINUM oi;
	int index;
	NSString* name;
	int number;
	BOOL stop;
	BOOL toDelete;
	LPHO objects[STEPFOREACH];
}ForEach;
typedef	ForEach*	LPFOREACH;

#pragma pack(pop, _pack_)

@interface CEvents : NSObject {

}

@end


typedef BOOL (*ECVROUTINE)(LPHO pHo, int value);
typedef BOOL (*EVAOROUTINE)(event* pe, CRun* rhPtr, LPHO pHo);
typedef BOOL (*EEOROUTINE)(event* pe, CRun* rhPtr, LPHO pHo, int value);
typedef BOOL (*EEOROUTINEDOUBLE)(event* pe, CRun* rhPtr, LPHO pHo, double value);

BOOL negaFALSE(event* pe);
BOOL negaTRUE(event* pe);
BOOL compareTo(CValue* pValue1, CValue* pValue2, short comp);
BOOL compareToInt(CValue* pValue1, int value2, short comp);
BOOL compareToDouble(CValue* pValue1, double value2, short comp);
BOOL compareTer(int value1, int value2, int comparaison);
BOOL compareTerDouble(double value1, double value2, int comparaison);
BOOL compareCondition(event* pe, CRun* rhPtr, int value1);
BOOL cndNoSpSamPlaying(event* pe, CRun* rhPtr, LPHO pHo);
BOOL cndNoSpChannelPlaying(event* pe, CRun* rhPtr, LPHO pHo);
BOOL cndNoSamPlaying(event* pe, CRun* rhPtr, LPHO pHo);
BOOL cndSpSamPaused(event* pe, CRun* rhPtr, LPHO pHo);
BOOL cndSpChannelPaused(event* pe, CRun* rhPtr, LPHO pHo);
void actPlaySample(event* pe, CRun* rhPtr);
void actPlaySample2(event* pe, CRun* rhPtr);
void actStopSpeSample(event* pe, CRun* rhPtr);
void actPlayLoopSample(event* pe, CRun* rhPtr);
void actStopAllSamples(event* pe, CRun* rhPtr);
void actPauseSample(event* pe, CRun* rhPtr);
void actPauseAllChannels(event* pe, CRun* rhPtr);
void actResumeSample(event* pe, CRun* rhPtr);
void actResumeAllChannels(event* pe, CRun* rhPtr);
void actPlayChannel(event* pe, CRun* rhPtr);
void actPlayLoopChannel(event* pe, CRun* rhPtr);
void actPauseChannel(event* pe, CRun* rhPtr);
void actResumeChannel(event* pe, CRun* rhPtr);
void actStopChannel(event* pe, CRun* rhPtr);
void actSetPosChannel(event* pe, CRun* rhPtr);
void actSetPosSample(event* pe, CRun* rhPtr);
void actSetVolumeChannel(event* pe, CRun* rhPtr);
void actSetPanChannel(event* pe, CRun* rhPtr);
void actSetFreqChannel(event* pe, CRun* rhPtr);
void actSetSampleMainVolume(event* pe, CRun* rhPtr);
void actSetSampleVolume(event* pe, CRun* rhPtr);
void actSetSampleMainPan(event* pe, CRun* rhPtr);
void actSetSamplePan(event* pe, CRun* rhPtr);
void actSetFreqSample(event* pe, CRun* rhPtr);
void actLockChannel(event* pe, CRun* rhPtr);
void actUnlockChannel(event* pe, CRun* rhPtr);
void expSampleMainVolume(CRun* rhPtr);
void expSampleVolume(CRun* rhPtr);
void expChannelVolume(CRun* rhPtr);
void expSampleMainPan(CRun* rhPtr);
void expSamplePan(CRun* rhPtr);
void expChannelPan(CRun* rhPtr);
void expSampleFrequency(CRun* rhPtr);
void expChannelFrequency(CRun* rhPtr);
void expSamplePosition(CRun* rhPtr);
void expChannelPosition(CRun* rhPtr);
void expSampleDuration(CRun* rhPtr);
void expChannelDuration(CRun* rhPtr);
void expChannelSampleName(CRun* rhPtr);
void expXMouse(CRun* rhPtr);
void expYMouse(CRun* rhPtr);
BOOL eva1MClick(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva2MClick(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva1MClickOnObject(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva2MClickOnObject(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaMOnObject(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaOnMousePressed(event* pe, CRun* rhPtr, LPHO pHo);
BOOL mouseInZone(int* pZone, CRun* rhPtr);
BOOL eva1MClickInZone(event* pe, CRun* rhPtr, LPHO pHO);
BOOL eva2MClickInZone(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaMInZone(event* pe, CRun* rhPtr, LPHO pHo);
void expTim_Value(CRun* rhPtr);
void expTim_Cent(CRun* rhPtr);
void expTim_Sec(CRun* rhPtr);
void expTim_Min(CRun* rhPtr);
void expTim_Hour(CRun* rhPtr);
BOOL evaTimerEqu(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaTimerInf(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaTimerSup(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaTimeOut(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaEvery(event* pe, CRun* rhPtr, LPHO pHo);
void actSetTimer(event* pe, CRun* rhPtr);
BOOL evaLevel(event* pe, CRun* rhPtr, LPHO pHo);
void expGam_NLevelOld(CRun* rhPtr);
void expGam_NLevel(CRun* rhPtr);
BOOL evaStart(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaEnd(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaIsObstacle(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaIsLadder(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaEndOfPause2(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaFrameLoaded(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaFrameSaved(event* pe, CRun* rhPtr, LPHO pHo);
void expGam_GetCollisionMask(CRun* rhPtr);
void expGam_FrameRate(CRun* rhPtr);
void expGam_GetVirtualWidth(CRun* rhPtr);
void expGam_GetVirtualHeight(CRun* rhPtr);
void expGam_GetFrameBkdColor(CRun* rhPtr);
void expGam_GraphicMode(CRun* rhPtr);
void expGam_PixelShaderV(CRun* rhPtr);
void expGam_FrameAlphaCoef(CRun* rhPtr);
void expGam_FrameRGBCoef(CRun* rhPtr);
void expGam_FrameEffectParam(CRun* rhPtr);
void actRestartLevel(event* pe, CRun* rhPtr);
void actNextLevel(event* pe, CRun* rhPtr);
void actPrevLevel(event* pe, CRun* rhPtr);
void actGotoLevel(event* pe, CRun* rhPtr);
void actEndGame(event* pe, CRun* rhPtr);
void actRestartGame(event* pe, CRun* rhPtr);
void actPauseGame(event* pe, CRun* rhPtr);
void setDisplay(CRun* rhPtr, int x, int y, int nLayer, DWORD flags);
BOOL read_Position(CRun* rhPtr, LPPOS pPos, DWORD getDir, int* pX, int* pY, int* pDir, BOOL* pBRepeat, int* pLayer);
void actCDisplay(event* pe, CRun* rhPtr);
void actCDisplayX(event* pe, CRun* rhPtr);
void actCDisplayY(event* pe, CRun* rhPtr);
void actSetVirtualWidth(event* pe, CRun* rhPtr);
void actSetVirtualHeight(event* pe, CRun* rhPtr);
void actSetFrameBkdColor(event* pe, CRun* rhPtr);
void actDelCreatedBkdAt(event* pe, CRun* rhPtr);
void actSetFrameWidth(event* pe, CRun* rhPtr);
void actSetFrameHeight(event* pe, CRun* rhPtr);
void actSetStretchResampling(event* pe, CRun* rhPtr);
void actDelAllCreatedBkd(event* pe, CRun* rhPtr);
void actCls(event* pe, CRun* rhPtr);
void actClearZone(event* pe, CRun* rhPtr);
void actSetFrameRate(event* pe, CRun* rhPtr);
void actPauseAnyKey(event* pe, CRun* rhPtr);
void actSaveFrame(event* pe, CRun* rhPtr);
void actLoadFrame(event* pe, CRun* rhPtr);
void actLoadApplication(event* pe, CRun* rhPtr);
void actSetFrameEffect(event*pe, CRun* rhPtr);
void actSetFrameEffectParam(event*pe, CRun* rhPtr);
void actSetFrameEffectParamTexture(event*pe, CRun* rhPtr);
void actSetFrameAlphaCoef(event*pe, CRun* rhPtr);
void actSetFrameRGBCoef(event*pe, CRun* rhPtr);
void actPlayDemo(event*pe, CRun* rhPtr);
void expGam_NPlayer(CRun* rhPtr);
void expGam_PlayWidth(CRun* rhPtr);
void expGam_PlayHeight(CRun* rhPtr);
void expGam_PlayXLeft(CRun* rhPtr);
void expGam_PlayXRight(CRun* rhPtr);
void expGam_PlayYTop(CRun* rhPtr);
void expGam_PlayYBottom(CRun* rhPtr);
BOOL evaChooseValue1(event* pe, CRun* rhPtr, ECVROUTINE pRoutine);
BOOL evaChooseValue2(event* pe, CRun* rhPtr, ECVROUTINE pRoutine);
BOOL pickFlagSet(LPHO pHo, int value);
BOOL evaChooseFlagSet_old(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaChooseFlagSet(event* pe, CRun* rhPtr, LPHO pHo);
BOOL pickFlagReset(LPHO pHo, int value);
BOOL evaChooseFlagReset_old(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaChooseFlagReset(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaChooseValue_old(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaChooseValue(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaPickFromId(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaNumOfAllObjects(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNumOfAllObjects_old(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaChooseAll(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaChooseAll_old(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaChooseZone(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaChooseZone_old(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaChooseAllInZone(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaChooseAllInZone_old(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaChooseAllInLine(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNoMoreAllZone(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNoMoreAllZone_old(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNumOfAllZone(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNumOfAllZone_old(event* pe, CRun* rhPtr, LPHO pHo);
void actCreateObject(event* pe, CRun* rhPtr);
void actCreateObjectExp(event* pe, CRun* rhPtr);
void expCre_NumberAll(CRun* rhPtr);
void expCre_LastFixedValue(CRun* rhPtr);
void expPla_GetScore(CRun* rhPtr);
void expPla_GetLives(CRun* rhPtr);
void actPla_SetScore(event* pe, CRun* rhPtr);
void actPla_AddScore(event* pe, CRun* rhPtr);
void actPla_SubScore(event* pe, CRun* rhPtr);
void actPla_FinishLives(CRun* rhPtr, int joueur, int live);
void actPla_SetLives(event* pe, CRun* rhPtr);
void actPla_AddLives(event* pe, CRun* rhPtr);
void actPla_SubLives(event* pe, CRun* rhPtr);
void actPla_SetPlayerName(event* pe, CRun* rhPtr);
void expPla_GetPlayerName(CRun* rhPtr);
BOOL evaScores(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaLives(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNoMoreLive(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva1JoyPressed(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva2JoyPressed(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaJoyPushed(event* pe, CRun* rhPtr, LPHO pHo);
void actNoInput(event* pe, CRun* rhPtr);
void actRestoreInput(event* pe, CRun* rhPtr);
void expSys_Rien(CRun* rhPtr);
BOOL evaOnLoop(event* pe, CRun* rhPtr, LPHO pHo);
void actStartLoop(event* pe, CRun* rhPtr);
void actSetLoopIndex(event* pe, CRun* rhPtr);
void actStopLoop(event* pe, CRun* rhPtr);
void ActRandomize(event* pe, CRun* rhPtr);
void expSys_LoopIndex(CRun* rhPtr);
void expSys_LoopIndexByIndex(CRun* rhPtr);
BOOL evaOnce(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNotAlways(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaRepeat(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNoMore(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompare(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalIntEQ(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalDblEQ(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalIntNE(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalDblNE(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalIntLE(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalDblLE(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalIntLT(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalDblLT(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalIntGE(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalDblGE(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalIntGT(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalDblGT(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobal(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCompareGlobalString(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaRunningAs(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaStartChildEvent(event* pe, CRun* rhPtr, LPHO pHo);
CValue* getGlobal(event* pe, CRun* rhPtr, int* num);
void actSetGlobal(event* pe, CRun* rhPtr);
void actSetGlobalInt(event* pe, CRun* rhPtr);
void actSetGlobalDbl(event* pe, CRun* rhPtr);
void actSetGlobalIntNumExp(event* pe, CRun* rhPtr);
void actSetGlobalDblNumExp(event* pe, CRun* rhPtr);
void actSetGlobalString(event* pe, CRun* rhPtr);
void actAddGlobal(event* pe, CRun* rhPtr);
void actAddGlobalInt(event* pe, CRun* rhPtr);
void actAddGlobalDbl(event* pe, CRun* rhPtr);
void actAddGlobalIntNumExp(event* pe, CRun* rhPtr);
void actAddGlobalDblNumExp(event* pe, CRun* rhPtr);
void actSubGlobal(event* pe, CRun* rhPtr);
void actSubGlobalInt(event* pe, CRun* rhPtr);
void actSubGlobalDbl(event* pe, CRun* rhPtr);
void actSubGlobalIntNumExp(event* pe, CRun* rhPtr);
void actSubGlobalDblNumExp(event* pe, CRun* rhPtr);
void expSys_GlobalValue(CRun* rhPtr);
void expSys_GlobalValueNamed(CRun* rhPtr);
void expSys_GlobalString(CRun* rhPtr);
void expSys_GlobalStringNamed(CRun* rhPtr);
void actExecuteChildEvents(event* pe, CRun* rhPtr);
LPEVG InactGroup(LPEVG evgPtr);
LPEVG ActGroup(LPEVG evgPtr);
LPEVG GrpActivate(LPEVG evgPtr);
void actGrpActivate(event* pe, CRun* rhPtr);
LPEVG GrpDeactivate(LPEVG evgPtr);
void actGrpDesactivate(event* pe, CRun* rhPtr);
BOOL evaGrpActivated(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaOnGroupStart(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaChance(event* pe, CRun* rhPtr, LPHO pHo);
void actExtractBinFile(event* pe, CRun* rhPtr);
void actReleaseBinFile(event* pe, CRun* rhPtr);
void expSys_String(CRun* rhPtr);
//void expSys_StringRegex(CRun* rhPtr);
void expSys_StringReplace(CRun* rhPtr);
void expSys_Long(CRun* rhPtr);
void expSys_Random(CRun* rhPtr);
void expSys_Zero(CRun* rhPtr);
void expSys_Empty(CRun* rhPtr);
void funcVal(NSString* pString, CValue* pValue);
void expSys_Val(CRun* rhPtr);
void expSys_NewLine(CRun* rhPtr);
void expSys_Str(CRun* rhPtr);
void expSys_SIN(CRun* rhPtr);
void expSys_COS(CRun* rhPtr);
void expSys_TAN(CRun* rhPtr);
void expSys_CEIL(CRun* rhPtr);
void expSys_ABS(CRun* rhPtr);
void expSys_FLOOR(CRun* rhPtr);
void expSys_ASIN(CRun* rhPtr);
void expSys_ACOS(CRun* rhPtr);
void expSys_ATAN(CRun* rhPtr);
void expSys_ATAN2(CRun* rhPtr);
void expSys_NOT(CRun* rhPtr);
void expSys_SQR(CRun* rhPtr);
void expSys_LOG(CRun* rhPtr);
void expSys_LN(CRun* rhPtr);
void expSys_EXP(CRun* rhPtr);
void expSys_INT(CRun* rhPtr);
void expSys_Round(CRun* rhPtr);
void expSys_Min(CRun* rhPtr);
void expSys_Max(CRun* rhPtr);
void expSys_GetRGB(CRun* rhPtr);
void expSys_GetRed(CRun* rhPtr);
void expSys_GetGreen(CRun* rhPtr);
void expSys_GetBlue(CRun* rhPtr);
void expSys_LEN(CRun* rhPtr);
void expSys_HEX(CRun* rhPtr);
void expSys_BIN(CRun* rhPtr);
void expSys_LEFT(CRun* rhPtr);
void expSys_RIGHT(CRun* rhPtr);
void expSys_MID(CRun* rhPtr);
void expSys_DOUBLE(CRun* rhPtr);
void expSys_Lower(CRun* rhPtr);
void expSys_Upper(CRun* rhPtr);
void expSys_FloatStr(CRun* rhPtr);
void expSys_Find(CRun* rhPtr);
void expSys_FindReverse(CRun* rhPtr);
void expSys_RuntimeName(CRun* rhPtr);
short txtDisplay(event* pe, CRun* rhPtr, short oi, int txtNumber);
short txtDoDisplay(event* pe, CRun* rhPtr, int txtNumber);
void actTxtDisplay(event* pe, CRun* rhPtr);
void actTxtDisplayDuring(event* pe, CRun* rhPtr);
void actTxtPrevious(event* pe, CRun* rhPtr);
void actTxtNext(event* pe, CRun* rhPtr);
void actTxtSet(event* pe, CRun* rhPtr);
void actTxtDisplayString(event* pe, CRun* rhPtr);
void actTxtSetString(event* pe, CRun* rhPtr);
void actTxtSetColour(event* pe, CRun* rhPtr);
void actTxtDestroy(event* pe, CRun* rhPtr);
void expTxtNumber(CRun* rhPtr);
void expTxtGetCurrent(CRun* rhPtr);
void expTxtGetNumeric(CRun* rhPtr);
void expTxtGetNumber(CRun* rhPtr);
void expTxtGetNPara(CRun* rhPtr);
int qstCreate(event* pe, CRun* rhPtr, short oi);
void actQstAsk(event* pe, CRun* rhPtr);
BOOL eva1QstEqual(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva2QstEqual(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva2QstExact(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva2QstFalse(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva1QstExact(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva1QstFalse(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCounter(event* pe, CRun* rhPtr, LPHO pHoDummy);
void expCpt_GetValue(CRun* rhPtr);
void actCpt_SetValue(event* pe, CRun* rhPtr);
void actCpt_AddValue(event* pe, CRun* rhPtr);
void actCpt_SubValue(event* pe, CRun* rhPtr);
void expCpt_GetMin(CRun* rhPtr);
void expCpt_GetMax(CRun* rhPtr);
void actCpt_SetMin(event* pe, CRun* rhPtr);
void actCpt_SetMax(event* pe, CRun* rhPtr);
void expCpt_GetColor1(CRun* rhPtr);
void expCpt_GetColor2(CRun* rhPtr);
void actCpt_SetColor1(event* pe, CRun* rhPtr);
void actCpt_SetColor2(event* pe, CRun* rhPtr);
BOOL evaObject(event* pe, CRun* rhPtr, EVAOROUTINE pRoutine);
BOOL NearBord(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNearBorders(event* pe, CRun* rhPtr, LPHO pHo);
BOOL checkMarkEvt(int mark, CRun* rhPtr);
BOOL NPath(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNodePath(event* pe, CRun* rhPtr, LPHO pHo);
BOOL NNPath(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaPathNodeName2(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaPathNodeName1(event* pe, CRun* rhPtr, LPHO pHo);
BOOL EPath(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaEndPath(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaExpObject(event* pe, CRun* rhPtr, EEOROUTINE pRoutine);
BOOL evaExpObjectDouble(event* pe, CRun* rhPtr, EEOROUTINEDOUBLE pRoutine);
BOOL CheckMultipleFlagAndVariableParam(multipleFlagAndVariableParam* pMultipleParam, LPHO pHo);
BOOL CheckMultipleFlagAndVariableParamNoGlobalValue(multipleFlagAndVariableParam* pMultipleParam, LPHO pHo);
BOOL FSet(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL evaFlagSet(event* pe, CRun* rhPtr, LPHO pHo);
BOOL FReset(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL evaFlagReset(event* pe, CRun* rhPtr, LPHO pHo);
BOOL compFixed(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL evaVarCompareFixed(event* pe, CRun* rhPtr, LPHO pHo);
BOOL XComp(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL evaXCompare(event* pe, CRun* rhPtr, LPHO pHo);
BOOL YComp(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL evaYCompare(event* pe, CRun* rhPtr, LPHO pHo);
BOOL SComp(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL evaSpeedCompare(event* pe, CRun* rhPtr, LPHO pHo);
BOOL AccComp(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL evaCmpAcc(event* pe, CRun* rhPtr, LPHO pHo);
BOOL DecComp(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL evaCmpDec(event* pe, CRun* rhPtr, LPHO pHo);
BOOL FrameComp(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL evaCmpFrame(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCmpVar(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaCmpVarConst(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaCmpVarString(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaIsColliding(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL colGetList(CRun* rhPtr, short oiList, short lookFor);
BOOL compute_NoRepeatCol(int identifier, LPHO pHo);
BOOL eva1Collision(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaChoose(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaPickClosest(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaPickMinimumExpressionValue(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaPickMaximumExpressionValue(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL evaExtCompare(event* pe, CRun* rhPtr, LPHO pHoIn);
BOOL LayerComp(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL evaExtCmpLayer(event* pe, CRun* rhPtr, LPHO pHo);
BOOL InstanceValueComp(event* pe, CRun* rhPtr, LPHO pHo, int _value);
BOOL evaExtCmpInstanceValue(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva1GoesInPlayfield(event* pe, CRun* rhPtr, LPHO pHo);
BOOL GOut(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva2GoesOutPlayfield(event* pe, CRun* rhPtr, LPHO pHo);
BOOL GIn(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva2GoesInPlayfield(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsOut(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaIsOutPlayfield(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsIn(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaIsInPlayfield(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva1ColBack(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsColBack(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva2ColBack(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNoMoreObject(event* pe, CRun* rhPtr, int sub);
BOOL eva1NoMore(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva2NoMore(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNoMoreZone(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNumberZone(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaNumOfObject(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsShown(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaShown(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsHidden(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaHidden(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsStopped(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaStopped(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsBouncing(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaBouncing(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsReversed(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaReversed(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsFacing2(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL IsFacing1(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaFacing(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva1AnOver(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsOver2(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL IsOver1(event* pe, CRun* rhPtr, LPHO pHo);
BOOL eva2AnOver(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsPlaying2(event* pe, CRun* rhPtr, LPHO pHo, int value);
BOOL IsPlaying1(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaAnPlaying(event* pe, CRun* rhPtr, LPHO pHo);
void shtCreate(event* pe, CRun* rhPtr, LPHO pHoSource, int x, int y, int dir);
void actShootToward(event* pe, CRun* rhPtr);
void actShoot(event* pe, CRun* rhPtr);
void actShuffle(event* pe, CRun* rhPtr);
void actAnStop(event* pe, CRun* rhPtr);
void actAnStart(event* pe, CRun* rhPtr);
void actAnForce(event* pe, CRun* rhPtr);
void actAnRestore(event* pe, CRun* rhPtr);
void actAnDirForce(event* pe, CRun* rhPtr);
void actAnDirRestore(event* pe, CRun* rhPtr);
void actAnSpeedForce(event* pe, CRun* rhPtr);
void actAnFrameForce(event* pe, CRun* rhPtr);
void actAnFrameRestore(event* pe, CRun* rhPtr);
void actAnSpeedRestore(event* pe, CRun* rhPtr);
void actAnRestart(event* pe, CRun* rhPtr);
BOOL AngleComp(event* pe, CRun* rhPtr, LPHO pHo, double value);
BOOL evaSpr_CmpAngle(event* pe, CRun* rhPtr, LPHO pHo);
BOOL XScaleComp(event* pe, CRun* rhPtr, LPHO pHo, double value);
BOOL evaSpr_CmpScaleX(event* pe, CRun* rhPtr, LPHO pHo);
BOOL YScaleComp(event* pe, CRun* rhPtr, LPHO pHo, double value);
BOOL evaSpr_CmpScaleY(event* pe, CRun* rhPtr, LPHO pHo);
void actPasteSprite(event* pe, CRun* rhPtr);
void actSpriteAddBkd(event* pe, CRun* rhPtr);
void actReplaceColor(event* pe, CRun* rhPtr);
void actSpriteReplaceColor(CObject* pHo, CRun* rhPtr, int color1, int color2);
void SetScale(LPHO pHo, CRun* rhPtr, float fScaleX, float fScaleY, BOOL bResample);
void actSetScale(event* pe, CRun* rhPtr);
void actSetScaleX(event* pe, CRun* rhPtr);
void actSetScaleY(event* pe, CRun* rhPtr);
void actSetAngle(event* pe, CRun* rhPtr);
void actSetDirection(event* pe, CRun* rhPtr);
void actLookAt(event* pe, CRun* rhPtr);
void actSetPosition(event* pe, CRun* rhPtr);
void actSetXPosition(event* pe, CRun* rhPtr);
void actSetYPosition(event* pe, CRun* rhPtr);
void actWrap(event* pe, CRun* rhPtr);
void actBounce(event* pe, CRun* rhPtr);
void actReverse(event* pe, CRun* rhPtr);
void actStop(event* pe, CRun* rhPtr);
void actSetSpeed(event* pe, CRun* rhPtr);
void actMvSetAcc(event* pe, CRun* rhPtr);
void actMvSetDec(event* pe, CRun* rhPtr);
void actMvSetRotSpeed(event* pe, CRun* rhPtr);
void actMvSet8Dirs(event* pe, CRun* rhPtr);
void actSetMaxSpeed(event* pe, CRun* rhPtr);
void actSetGravity(event* pe, CRun* rhPtr);
void actStart(event* pe, CRun* rhPtr);
void actNextMovement(event* pe, CRun* rhPtr);
void actPreviousMovement(event* pe, CRun* rhPtr);
void actSelectMovement(event* pe, CRun* rhPtr);
void actBranchNode(event* pe, CRun* rhPtr);
void actGotoNode(event* pe, CRun* rhPtr);
void exp_GetNMovement(CRun* rhPtr);
void actDisappear(event* pe, CRun* rhPtr);
void actShow(event* pe, CRun* rhPtr);
void actSpriteBack(event* pe, CRun* rhPtr);
void actSpriteFront(event* pe, CRun* rhPtr);
void expSpr_GetRGBAt(CRun* rhPtr);
void expSpr_GetScaleX(CRun* rhPtr);
void expSpr_GetScaleY(CRun* rhPtr);
void expSpr_GetAngle(CRun* rhPtr);
void actHide(event* pe, CRun* rhPtr);
void actFlash(event* pe, CRun* rhPtr);
void actSetVar(event* pe, CRun* rhPtr);
void actSetVarString(event* pe, CRun* rhPtr);
void actAddVar(event* pe, CRun* rhPtr);
void actSubVar(event* pe, CRun* rhPtr);
void actDispatchVar(event* pe, CRun* rhPtr);
void actSetFlag(event* pe, CRun* rhPtr);
void actChgFlag(event* pe, CRun* rhPtr);
void actClrFlag(event* pe, CRun* rhPtr);
void actSetInkEffect(event* pe, CRun* rhPtr);
void actSetEffect(event* pe, CRun* rhPtr);
void actSetEffectParam(event* pe, CRun* rhPtr);
void actSetEffectParamTexture(event* pe, CRun* rhPtr);
void actSetSemiTransparency(event* pe, CRun* rhPtr);
void actSetAlphaCoef(event* pe, CRun* rhPtr);
void actSetRGBCoef(event* pe, CRun* rhPtr);
void exp_GetSemiTransparency(CRun* rhPtr);
void exp_AlphaCoef(CRun* rhPtr);
void exp_RGBCoef(CRun* rhPtr);
void exp_Flag(CRun* rhPtr);
void exp_Id(CRun* rhPtr);
void exp_Var(CRun* rhPtr);
void exp_VarByIndex(CRun* rhPtr);
void exp_VarString(CRun* rhPtr);
void exp_VarStringByIndex(CRun* rhPtr);
void exp_X(CRun* rhPtr);
void exp_Y(CRun* rhPtr);
void exp_XLeft(CRun* rhPtr);
void exp_XRight(CRun* rhPtr);
void exp_YTop(CRun* rhPtr);
void exp_YBottom(CRun* rhPtr);
void exp_XAP(CRun* rhPtr);
void exp_YAP(CRun* rhPtr);
void exp_EffectParam(CRun* rhPtr);
void exp_Dir(CRun* rhPtr);
void exp_Image(CRun* rhPtr);
void exp_NAni(CRun* rhPtr);
void exp_Speed(CRun* rhPtr);
void exp_Acc(CRun* rhPtr);
void exp_Dec(CRun* rhPtr);
void exp_Gravity(CRun* rhPtr);
void exp_Number(CRun* rhPtr);
void exp_NumberOfSelected(CRun* rhPtr);
void exp_InstanceValue(CRun* rhPtr);
BOOL IsBold(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaIsBold(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsItalic(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaIsItalic(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsUnderline(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaIsUnderline(event* pe, CRun* rhPtr, LPHO pHo);
BOOL IsStrikeOut(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaIsStrikeOut(event* pe, CRun* rhPtr, LPHO pHo);
void actSetFontName(event* pe, CRun* rhPtr);
void actSetFontSize(event* pe, CRun* rhPtr);
void actSetBold(event* pe, CRun* rhPtr);
void actSetItalic(event* pe, CRun* rhPtr);
void actSetUnderline(event* pe, CRun* rhPtr);
void actSetStrikeOut(event* pe, CRun* rhPtr);
void actSetTextColor(event* pe, CRun* rhPtr);
void exp_GetFontName(CRun* rhPtr);
void exp_GetFontSize(CRun* rhPtr);
void exp_GetFontColor(CRun* rhPtr);
void actExtSprFront(event* pe, CRun* rhPtr);
void actExtSprBack(event* pe, CRun* rhPtr);
void actMoveBefore(event* pe, CRun* rhPtr);
void actMoveAfter(event* pe, CRun* rhPtr);
void actMoveToLayer(event* pe, CRun* rhPtr);
void exp_GetLayer(CRun* rhPtr);
BOOL CcaFrameChanged(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCCAFRAMECHANGED(event* pe, CRun* rhPtr, LPHO pHo);
BOOL CcaAppFinished(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCCAAPPFINISHED(event* pe, CRun* rhPtr, LPHO pHo);
BOOL CcaIsVisible(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCCAISVISIBLE(event* pe, CRun* rhPtr, LPHO pHo);
BOOL CcaAppPaused(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaCCAAPPPAUSED(event* pe, CRun* rhPtr, LPHO pHo);
void actCCARESTARTAPP(event* pe, CRun* rhPtr);
void actCCARESTARTFRAME(event* pe, CRun* rhPtr);
void actCCANEXTFRAME(event* pe, CRun* rhPtr);
void actCCAPREVIOUSFRAME(event* pe, CRun* rhPtr);
void actCCAENDAPP(event* pe, CRun* rhPtr);
void actCCAJUMPFRAME(event* pe, CRun* rhPtr);
void actCCASETGLOBALVALUE(event* pe, CRun* rhPtr);
void actCCASHOW(event* pe, CRun* rhPtr);
void actCCAHIDE(event* pe, CRun* rhPtr);
void actCCASETGLOBALSTRING(event* pe, CRun* rhPtr);
void actCCAPAUSEAPP(event* pe, CRun* rhPtr);
void actCCARESUMEAPP(event* pe, CRun* rhPtr);
void expCCAGETFRAMENUMBER(CRun* rhPtr);
void expCCAGETGLOBALVALUE(CRun* rhPtr);
void expCCAGETGLOBALSTRING(CRun* rhPtr);
void OInvertSign(CRun* rhPtr);
void OParenthOpen(CRun* rhPtr);
void opePlus(CRun* rhPtr);
void opeMoins(CRun* rhPtr);
void opeMult(CRun* rhPtr);
void opeDiv(CRun* rhPtr);
void opeMod(CRun* rhPtr);
void opePow(CRun* rhPtr);
void opeOr(CRun* rhPtr);
void opeAnd(CRun* rhPtr);
void opeXor(CRun* rhPtr);
BOOL evaTRUE(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaFALSE(event* pe, CRun* rhPtr, LPHO pHo);
void actRien(event* pe, CRun* rhPtr);
void expRien(CRun* rhPtr);
void callAction_Player(event* pe, CRun* rhPtr);
void callAction_Keyboard(event* pe, CRun* rhPtr);
void callAction_Create(event* pe, CRun* rhPtr);
void callAction_Timer(event* pe, CRun* rhPtr);
void callAction_Game(event* pe, CRun* rhPtr);
void callAction_Speaker(event* pe, CRun* rhPtr);
void callAction_Systeme(event* pe, CRun* rhPtr);
void callAction_Sprite(event* pe, CRun* rhPtr);
void callAction_Text(event* pe, CRun* rhPtr);
void callAction_Quest(event* pe, CRun* rhPtr);
void callAction_Score(event* pe, CRun* rhPtr);
void callAction_Lives(event* pe, CRun* rhPtr);
void callAction_Counter(event* pe, CRun* rhPtr);
void callAction_Rtf(event* pe, CRun* rhPtr);
void callAction_Cca(event* pe, CRun* rhPtr);
void callAction_Ext(event* pe, CRun* rhPtr);
BOOL callCond1_Player(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Keyboard(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Create(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Timer(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Game(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Speaker(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Systeme(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Sprite(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Text(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Quest(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Score(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Lives(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Counter(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond1_Cca(event* pe, CRun* rhPtr, LPHO pHo);
void eva1Routine(event* pe, CRun* rhPtr, CObject* pHo);
BOOL callCond1_Ext(event* pe, CRun* rhPtr, LPHO pHo);
BOOL callCond2_Player(event* pe, CRun* rhPtr);
BOOL callCond2_Keyboard(event* pe, CRun* rhPtr);
BOOL callCond2_Create(event* pe, CRun* rhPtr);
BOOL callCond2_Timer(event* pe, CRun* rhPtr);
BOOL callCond2_Game(event* pe, CRun* rhPtr);
BOOL callCond2_Speaker(event* pe, CRun* rhPtr);
BOOL callCond2_Systeme(event* pe, CRun* rhPtr);
BOOL callCond2_Sprite(event* pe, CRun* rhPtr);
BOOL callCond2_Text(event* pe, CRun* rhPtr);
BOOL callCond2_Quest(event* pe, CRun* rhPtr);
BOOL callCond2_Score(event* pe, CRun* rhPtr);
BOOL callCond2_Lives(event* pe, CRun* rhPtr);
BOOL callCond2_Counter(event* pe, CRun* rhPtr);
BOOL callCond2_Cca(event* pe, CRun* rhPtr);
BOOL callCond2_Ext(event* pe, CRun* rhPtr);
void callExp_Player(CRun* rhPtr);
void callExp_Keyboard(CRun* rhPtr);
void callExp_Create(CRun* rhPtr);
void callExp_Timer(CRun* rhPtr);
void callExp_Game(CRun* rhPtr);
void callExp_Speaker(CRun* rhPtr);
void callExp_Systeme(CRun* rhPtr);
void callExp_Operators(CRun* rhPtr);
void callExp_Sprite(CRun* rhPtr);
void callExp_Text(CRun* rhPtr);
void callExp_Quest(CRun* rhPtr);
void callExp_Score(CRun* rhPtr);
void callExp_Lives(CRun* rhPtr);
void callExp_Counter(CRun* rhPtr);
void callExp_Cca(CRun* rhPtr);
void callExp_Ext(CRun* rhPtr);
void RACT_CCASETWIDTH(event* pe, CRun* rhPtr);
void RACT_CCASETHEIGHT(event* pe, CRun* rhPtr);

BOOL compute_GlobalNoRepeat(CRun* rhPtr);
BOOL GrowAlterableValues(CRVal* pRVal, int newNumber);
BOOL GrowAlterableStrings(CRVal* pRVal, int newNumber);

BOOL evaPressKey(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaKeyDepressed(event* pe, CRun* rhPtr, LPHO pHo);
BOOL evaTimerEquals(event* pe, CRun* rhPtr, CObject* pHo);
BOOL evaEvery2(event* pe, CRun* rhPtr, CObject* pHo);
void RACT_EVENTAFTER(event* pe, CRun* rhPtr);
void RACT_NEVENTSAFTER(event* pe, CRun* rhPtr);
BOOL RCND_ONEVENT(event* pe, CRun* rhPtr, CObject* pHo);
void REXP_EVENTAFTER(CRun* rhPtr);
void actCreateByName(event* pe, CRun* rhPtr);
void actCreateByNameExp(event* pe, CRun* rhPtr);
void REXP_RANDOMRANGE(CRun* rhPtr);
void REXP_RANGE(CRun* rhPtr);
void REXP_EXTANGLE(CRun* rhPtr);
void REXP_EXTDISTANCE(CRun* rhPtr);
void REXP_ANGLE(CRun* rhPtr);
void REXP_DISTANCE(CRun* rhPtr);
void REXP_EXTGETANGLE(CRun* rhPtr);
void REXP_EXTGETVELOCITY(CRun* rhPtr);
void REXP_EXTGETDENSITY(CRun* rhPtr);
void REXP_EXTGETRESTITUTION(CRun* rhPtr);
void REXP_EXTGETFRICTION(CRun* rhPtr);
void RACT_EXTSETANGULARVELOCITY(event* pe, CRun* rhPtr);
void RACT_EXTSETLINEARVELOCITY(event* pe, CRun* rhPtr);
void RACT_EXTAPPLYTORQUE(event* pe, CRun* rhPtr);
void RACT_EXTSTOPTORQUE(event* pe, CRun* rhPtr);
void RACT_EXTSTOPFORCE(event* pe, CRun* rhPtr);
void RACT_EXTAPPLYFORCE(event* pe, CRun* rhPtr);
void RACT_EXTAPPLYANGULARIMPULSE(event* pe, CRun* rhPtr);
void RACT_EXTAPPLYIMPULSE(event* pe, CRun* rhPtr);
void RACT_EXTSETELASTICITY(event* pe, CRun* rhPtr);
void RACT_EXTSETFRICTION(event* pe, CRun* rhPtr);
void RACT_EXTFOREACH2(event* pe, CRun* rhPtr);
void RACT_EXTFOREACH(event* pe, CRun* rhPtr);
void addForEach(NSString* pName, LPHO pHo, OINUM oil, CRun* rhPtr);
BOOL RCND_EXTONLOOP2(event* pe, CRun* rhPtr);
BOOL RCND_EXTONLOOP(event* pe, CRun* rhPtr, CObject* pHo);
BOOL RCND_EXTONLOOP_INTERNAL(event* pe, CRun* rhPtr, CObject* pHo);
void REXP_EXTLOOPINDEX(CRun* rhPtr);
void endForEach(CRun* rhPtr);
BOOL RCND_EXTONLOOP2(event* pe, CRun* rhPtr, CObject* pHo);
void REXP_WIDTH(CRun* rhPtr);
void REXP_HEIGHT(CRun* rhPtr);
void REXP_EXTGETMASS(CRun* rhPtr);
void REXP_EXTGETANGULARVELOCITY(CRun* rhPtr);
void REXP_EXTGETNAME(CRun* rhPtr);
void actExtraAction(event* pe, CRun* rhPtr);

void AddPhysicsAttractor(CRun* rhPtr, CObject* pObject);

////// Clipboard //////
BOOL evaClipboard(event* pe, CRun* rhPtr, LPHO pHo);
void actSendClipboard(event* pe, CRun* rhPtr);
void actClearClipboard(event* pe, CRun* rhPtr);
void exp_Clipboard(CRun* rhPtr);

