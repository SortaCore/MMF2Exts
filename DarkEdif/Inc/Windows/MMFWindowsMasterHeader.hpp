///

#pragma once
#ifndef _WIN32
#error Included the wrong header for this OS.
#endif

#include "..\Shared\AllPlatformDefines.hpp"
#include "Windows\WindowsDefines.hpp"

#if EditorBuild
// For ext icon
#include "Surface.hpp"
#if IS_DARKEDIF_TEMPLATE
#include "SafeSurface.hpp"
#endif
// For ext properties
#include "Props.hpp"
#else
class cSurfaceImplementation;
class cSurface;
struct sMask;
typedef sMask CollisionMask;
#endif

#include "CFile.hpp"

// Marquage des ObjectInfo qualifiers
#define	OIFLAG_QUALIFIER			0x8000
#define	NDQUALIFIERS				100
#define MAX_EVENTPROGRAMS			256

// Possible states of the application, see LApplication::SetRunningState()
enum class GAMEON {
	EMPTY,
	UNLOADED,
	TIME0,
	PAUSED,
	RUNNING,
	STEPPING,
};

// Changes the structure alignment
// See http://stupefydeveloper.blogspot.co.uk/2009/01/c-alignment-of-structure.html
#ifndef	_H2INC
	#pragma pack(push, mmf_master_header)
	#pragma pack(2)
#endif

namespace DarkEdif {
	class ObjectSelection;
}
namespace Edif {
	class Runtime;
}
struct ConditionOrActionManager_Windows;
struct ExpressionManager_Windows;

// This declaration list cannot be cut down
class CImageFilterMgr;
struct CSoundFilterMgr;
struct CSoundManager;
struct CRunApp;
struct CRunFrame;
struct CEditApp;
struct CEditFrame;
struct EDITDATA;
struct ObjectInfo;
struct dllTrans;
struct kpj;
struct drawRoutine;
struct LevelObject;
struct objInfoList;
struct HeaderObject;
struct qualToOi;
struct pev;
struct mv;
struct CRunMvt;
struct CMvt;
struct CDemoRecord;
struct CIPhoneJoystick;
struct CIPhoneAd;
struct RunObject;

// Runtime, instance-specific data for object - it is a std::shared_ptr on Android, beware!
typedef RunObject* RunObjectMultiPlatPtr;
// Fusion application data
typedef CRunApp CRunAppMultiPlat;
// Fusion frame data
typedef CRunFrame CRunFrameMultiPlat;

// Callback function identifiers for CallFunction
enum class CallFunctionIDs {
	// Editor only
	INSERTPROPS = 1,		// Insert properties into Property window
	REMOVEPROP,				// Remove property
	REMOVEPROPS,			// Remove properties
	REFRESHPROP,			// Refresh property
	REALLOCEDITDATA,		// Reallocate edPtr
	GETPROPVALUE,			// Get object's property value
	GETAPPPROPVALUE,		// Get application's property value
	GETFRAMEPROPVALUE,		// Get frame's property value
	SETPROPVALUE,			// Set object's property value
	SETAPPPROPVALUE,		// Set application's property value
	SETFRAMEPROPVALUE,		// Set frame's property value
	GETPROPCHECK,			// Get object's property check state
	GETAPPPROPCHECK,		// Get application's property check state
	GETFRAMEPROPCHECK,		// Get frame's property check state
	SETPROPCHECK,			// Set object's property check state
	SETAPPPROPCHECK,		// Set application's property check state
	SETFRAMEPROPCHECK,		// Set frame's property check state
	INVALIDATEOBJECT,		// Refresh object in frame editor
	RECALCLAYOUT,			// Recalc runtime layout (docking)
	GETNITEMS,				// Get number of items - not yet implemented
	GETNEXTITEM,			// Get next item - not yet implemented
	GETNINSTANCES,			// Get number of item instances - not yet implemented
	GETNEXTINSTANCE,		// Get next item instance - not yet implemented

	// Editor & runtime
	MALLOC = 100,			// Allocate memory
	CALLOC,					// Allocate memory & set it to 0
	REALLOC,				// Re-allocate memory
	FREE,					// Free memory
	GETSOUNDMGR,			// Get sound manager
	CLOSESOUNDMGR,			// Close sound manager
	ENTERMODALLOOP,			// Reserved
	EXITMODALLOOP,			// Reserved
	CREATEEFFECT,			// Create effect (runtime only)
	DELETEEFFECT,			// Delete effect (runtime only)
	CREATEIMAGEFROMFILEA,	// Create image from file (runtime only)
	NEEDBACKGROUNDACCESS,	// HWA : tell the frame the frame surface can be read (runtime only)
	ISHWA,					// Returns TRUE if HWA version (editor and runtime)
	ISUNICODE,				// Returns TRUE if the editor or runtime is in Unicode mode
	ISUNICODEAPP,			// Returns TRUE if the application being loaded is a Unicode application
	GETAPPCODEPAGE,			// Returns the code page of the application
	CREATEIMAGEFROMFILEW,	// Create image from file (runtime only)
};

struct Obj
{
	NO_DEFAULT_CTORS_OR_DTORS(Obj);
	unsigned int	size,		// Taille de l'objet: 0 = fin objets
					PrevSize,	// Taille objet precedent (0 = 1er objet)
					Type,		// Type d'objet (1=vide,2=appli,3=fenetre,...)
					Prev,		// Adresse objet precedent de meme type (0 = first)
					Next;		// Adresse objet suivant de meme type (0 = last)
};
//typedef Obj *npObj;
//typedef Obj *fpObj;

struct Spr
{
	NO_DEFAULT_CTORS_OR_DTORS(Spr);
	#ifndef SPRLIST
		Obj			Obj;
	#endif
	unsigned int	Flags;	// Flags
	unsigned short	Layer;	// Sprite plane (layer)
	short			Angle;	// Angle
	int				ZOrder,	// Z-order value
					X,		// X Position
					Y,		// Y Position
					X1,		// Bounding box, top left corner X pos
					Y1,		// Bounding box, top left corner Y pos
					X2,		// Bounding box, bottom right corner X pos
					Y2,		// Bounding box, bottom right corner Y pos
					Xnew,	// New X Position
					Ynew,	// New Y Position
					X1new,	// New bounding box, top left corner X pos
					Y1new,	// New bounding box, top left corner Y pos
					X2new,	// New bounding box, bottom right corner X pos
					Y2new,	// New bounding box, bottom right corner Y pos
					X1z,	// Background bounding box, top left corner X pos
					Y1z,	// Background bounding box, top left corner Y pos
					X2z,	// Background bounding box, bottom right corner X pos
					Y2z;	// Background bounding box, bottom right corner Y pos
	float			XScale,
					YScale;
	// Temporary values for collisions
	unsigned short	TempImg;	// TODO: use unsigned intlater?
	short			TempAngle;
	float			TempScaleX,
					TempScaleY;
	// Image or owner-draw routine
	union {
		struct {
			unsigned int Img;			 // Numero d'image
			unsigned int ImgNew;		  // Nouvelle image
		};
		LPARAM Rout;			// Ownerdraw callback routine
	};

	// Ink effect
	unsigned int	Effect;			// 0 = normal, 1 = semi-transparent, > 16 = routine
	LPARAM			EffectParam;	// parametre effet (coef transparence, etc...)

	// Fill color (wipe with color mode)
	COLORREF	BackColor;

	// Surfaces
	cSurfaceImplementation *	BackSurf;	// Background surface, if no general background surface

	cSurfaceImplementation *	Sf;			// Surface (if stretched or rotated)
	CollisionMask *				ColMask;	// Collision mask (if stretched or rotated)

	cSurfaceImplementation*	TempSf;			// Temp surface (if stretched or rotated)
	CollisionMask *			TempColMask;	// Temp collision mask (if stretched or rotated)

	// User data
	LPARAM		ExtraInfo;

	// Colliding ites
	int					CollisList[2];	// liste de ites entrant en collisions
};

// Maximum number of parameters
#define EVI_MAXPARAMS				16

// Structure de definition des conditions / actions POUR LES EXTENSIONS V1
struct infosEvents {
	NO_DEFAULT_CTORS_OR_DTORS(infosEvents);
	short 	code;					// Le numero de type + code event
	short	flags;					// Les flags a mettre dans l'event
	short	param[2];				// Le type des parametres
	short	paramTitle[2];			// Le titre de chacun des params
};
//typedef	infosEvents *			LPINFOEVENTS;
//typedef	infosEvents *				PINFOEVENTS;

// Event information structure
// ---------------------------
struct eventInformations {
	NO_DEFAULT_CTORS_OR_DTORS(eventInformations);
	short		menu;	// Menu identifier
	short		string;	// String identifier
	infosEvents	infos;	// Sub structure
};
//typedef	eventInformations *		LPEVENTINFOS;
//typedef	eventInformations *				NPEVENTINFOS;

//typedef	eventInformations2 *	eventInformations2 *;

//#define EVINFO2_NEXT(p)			((eventInformations2 *)((unsigned char *)p + sizeof(eventInformations2) + p->infos.nParams * 2 * sizeof(short)))
//#define	EVINFO2_PARAM(p,n)		(*(unsigned short *)((unsigned char *)p + sizeof(eventInformations2) + n * sizeof(short)))
//#define	EVINFO2_PARAMTITLE(p,n)	(*(unsigned short *)((unsigned char *)p + sizeof(eventInformations2) + p->infos.nParams * sizeof(short) + n * sizeof(short)))


// CNCF.H


// Definitions for extensions
#define TYPE_LONG	0x0000
#define TYPE_INT	TYPE_LONG
#define TYPE_STRING	0x1
#define TYPE_FLOAT	0x2				// Pour les extensions
#define TYPE_DOUBLE 0x2

struct CValueMultiPlat {
	NO_DEFAULT_CTORS_OR_DTORS(CValueMultiPlat);
	unsigned int m_type,
				 m_padding;
	union
	{
		std::int32_t m_long;
		double		 m_double;
		TCHAR *		 m_pString;
	};
};

typedef short *				LPSHORT;

//class CValue;
// Structure for SaveRect
struct saveRect {
	NO_DEFAULT_CTORS_OR_DTORS(saveRect);
	unsigned char * pData;
	RECT			rc;
};
//typedef saveRect* fpSaveRect;

// Number of values
#define ALTERABLE_VALUES_COUNT 26
#define ALTERABLE_STRINGS_COUNT 10

typedef short ITEMTYPE;
typedef short OINUM;
typedef short HFII;
// LOGFONT 16 structure for compatibility with old extensions
/////////////////////////////////////////////////////////////
struct LOGFONTV1 {
	short			Height,
					Width,
					Escapement,
					Orientation,
					Weight;
	unsigned char	Italic,
					Underline,
					StrikeOut,
					CharSet,
					OutPrecision,
					ClipPrecision,
					Quality,
					PitchAndFamily;
	TCHAR			FaceName[32];
};

// Flags modifiable by the program
enum class OEPREFS : short;
enum class OEFLAGS : unsigned int;

//////////////////////////////////////////////////////////////////////////////

//
// ObjectsCommon - Dynamic items
//
class Objects_Common {
	NO_DEFAULT_CTORS_OR_DTORS(Objects_Common);
public:

	unsigned long	size;			// Total size of the structures

	unsigned short	Movements,		// Offset of the movements (addr of Object_Common + Movements = addr of rMvt struct)
					Animations, 	// Offset of the animations (addr of Object_Common + Animations = addr of rAni struct)
					Version,		// For version versions > MOULI
					Counter,		// Pointer to COUNTER structure (addr of Object_Common + Counter = addr of counter struct)
					data,			// Pointer to DATA structure (addr of Object_Common + Data = addr of rData struct)
					Free;			// IGNORE: Padding the shorts to 4 bytes
	OEFLAGS			OEFlags;		// New flags?

	unsigned short	Qualifiers[8],	// Qualifier list (Runtime is hard-capped to 8 qualifiers per object, OC_MAX_QUALIFIERS)
					Extension,		// Extension structure
					Values,			// Values structure
					Strings,		// String structure

					Flags2;			// New news flags, before it was ocEvents
	OEPREFS			OEPrefs;		// Automatically modifiable flags
	unsigned long	Identifier;		// Identifier d'objet (char[4], made with 'ABCD')

	COLORREF		BackColor;		// Background color
	unsigned long	FadeIn,			// Offset fade in
					FadeOut,		// Offset fade out
					ValueNames,		// For the debugger
					StringNames;
};
//typedef OC * LPOC;
//typedef OC * fpoc;

struct OCValues
{
	NO_DEFAULT_CTORS_OR_DTORS(OCValues);
	unsigned short	number;
	long			values[1];
};
// typedef	OCValues *			LPOCVALUES;

struct OCStrings
{
	NO_DEFAULT_CTORS_OR_DTORS(OCStrings);
	unsigned short	number;
	TCHAR			str[2];
};
// typedef	OCStringsA*			LPOCSTRINGS;
// typedef	OCStringsW*			LPOCSTRINGS;

struct OCValueNames
{
	NO_DEFAULT_CTORS_OR_DTORS(OCValueNames);
	unsigned short	number;
	char	str[2];
};
//typedef	OCValueNames*		LPOCVALUENAMES;

#define	OCFLAGS2_DONTSAVEBKD		0x1
#define	OCFLAGS2_SOLIDBKD			0x2
#define	OCFLAGS2_COLBOX				0x4
#define	OCFLAGS2_VISIBLEATSTART		0x8
#define	OCFLAGS2_OBSTACLESHIFT		0x4
#define	OCFLAGS2_OBSTACLEMASK		0x0030
#define	OCFLAGS2_OBSTACLE_SOLID		0x10
#define	OCFLAGS2_OBSTACLE_PLATFORM	0x20
#define	OCFLAGS2_OBSTACLE_LADDER	0x0030
#define	OCFLAGS2_AUTOMATICROTATION	0x40

//////////////////////////////////////////////////////////////////////////////
//
// Counter
//
struct counter {
	NO_DEFAULT_CTORS_OR_DTORS(counter);
	unsigned short	ctSize;
	long			ctInit,				// Initial value
					ctMini,				// Minimal value
					ctMaxi;				// Maximal value
};
//typedef counter tagCT;
//typedef counter	* fpct;
//typedef counter	* fpCounter;

//////////////////////////////////////////////////////////////////////////////
//
// Animations
//

// Anim header
struct AnimHeader {
	NO_DEFAULT_CTORS_OR_DTORS(AnimHeader);
	unsigned short	size,				// Size of AnimHeader, its Animation structs, and their AnimDirs structs.
					AnimMax;			// New V2, number of entries in offset table
	short			OffsetToAnim[16];	// Minimum 16 animations, can be larger!
	// negative value indicates no animation in OffsetToAnim. Offset based on AnimHead addr.
};
//typedef AnimHeader * fpAnimHeader;
//typedef AnimHeader * fpah;
//typedef AnimHeader * LPAH;

// Animation structure - collection of AnimDirections
struct Animation {
	NO_DEFAULT_CTORS_OR_DTORS(Animation);
	short	OffsetToDir[32];	// Offset to animations with a direction (32 dirs max)
	// negative indicates no animation direction in OffsetToDir. Offset based on Animation addr.
};
//typedef Animation *	fpAnim;
//typedef Animation *	fpan;
//typedef Animation *	LPAN;

// AnimDirection - Info about a specific direction in a specific animation
struct AnimDirection {
	NO_DEFAULT_CTORS_OR_DTORS(AnimDirection);
	unsigned char	MinSpeed;		// Minimum speed
	unsigned char	MaxSpeed;		// Maximum speed
	unsigned short	Repeat,			// Number of loops
					RepeatFrame,	// Where to loop
					NumberOfFrame,	// Number of frames
					Frame[];		// Frames.
	// The Frame's contents are image indexes in app's image bank. See mvCreateImageFromFile().
};
//typedef AnimDirection *	fpAnimDir;
//typedef AnimDirection *	fpad;
//typedef AnimDirection *	LPAD;

// Chunks de sauvegarde
#define EVTFILECHUNK_HEAD				B2L('E','R','>','>')
#define EVTFILECHUNK_EVTHEAD			B2L('E','R','e','s')
#define EVTFILECHUNK_EVENTS				B2L('E','R','e','v')
#define EVTFILECHUNK_NEWITEMS			B2L('>','N','I','<')
#define EVTFILECHUNK_END				B2L('<','<','E','R')
// New to CF2.5+ build 292.6
#define EVTFILECHUNK_EVENTBLOCKTYPES	B2L('E','R','b','t')

// Marquage des OI qualifiers
#define	OIFLAG_QUALIFIER			0x8000
#define	NDQUALIFIERS				100
#define MAX_EVENTPROGRAMS			256

// COMMUNICATION STRUCTURE BETWEEN RUNTIME AND EDITOR
struct ComStructure
{
	NO_DEFAULT_CTORS_OR_DTORS(ComStructure);
	unsigned int		command;

	unsigned short		runMode;
	unsigned int		timer;

	unsigned short		code;
	OINUM				oi;
	unsigned int		param[2];
	char				path[MAX_PATH]; // TODO: ANSI only, or Unicode TCHAR?
};
const enum
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
enum class RUNMODE
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
enum class ANIMID
{
	STOP,
	WALK,
	RUN,
	APPEAR,
	DISAPPEAR,
	BOUNCE,
	SHOOT,
	JUMP,
	FALL,
	CLIMB,
	CROUCH,
	UNCROUCH,
	USER_DEFINED,
};
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

// Used with EVTFILECHUNK_EVENTBLOCKTYPES
struct EventBlockType {
	enum class EventBlockID : unsigned int
	{
		FrameEvents = 0,
		GlobalEvents = 1,
		// 2+ = Behaviour number 1 is 2, 2 is 3, etc.
		Behaviour = 2,
		// Indicates end of EventBlockType
		EndOfBlock = UINT32_MAX
	} type;
	DWORD oi; // object handle (if behavior)

	//  offset of the first event line relative to rhPtr->rhApp->rhFrame->m_eventPrograms
	DWORD blockOffset;
};

// Was EVGFLAGS_**
enum class EventGroupFlags : unsigned short
{
	Once = 0x1,
	NotAlways = 0x2,
	Repeat = 0x4,
	NoMore = 0x8,
	Shuffle = 0x10,
	EditorMark = 0x20,
	UndoMark = 0x40,
	ComplexGroup = 0x80,
	Breakpoint = 0x100,
	AlwaysClean = 0x200,
	OrInGroup = 0x400,
	StopInGroup = 0x800,
	OrLogical = 0x1000,
	Grouped = 0x2000,
	Inactive = 0x4000,
	NoGood = 0x8000,
	Limited = Shuffle | NotAlways | Repeat | NoMore,
	DefaultMask = Breakpoint | Grouped
};
enum_class_is_a_bitmask(EventGroupFlags);

//#define		EVGFLAGS_2MANYACTIONS	0x400
//#define		EVGFLAGS_NOTASSEMBLED	0x1000

// Eventgroup structure, before conditions and actions
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Differs from eventV1 by union variable types
struct event2 {
	NO_DEFAULT_CTORS_OR_DTORS(event2);
	short get_evtNum();
	OINUM get_evtOi();
	//short get_evtSize();
	std::int8_t get_evtFlags();
	void set_evtFlags(std::int8_t);

	event2* Next();
	int GetIndex();

DarkEdifInternalAccessProtected:
	short	evtSize;				// 0 Size of the event
	union
	{
		long		evtCode;		// 2 Code (hi:NUM lo:TYPE)
		struct
		{
			short	evtType,		// 2 Type of object
					evtNum;			// 4 Number of action/condition (SUBTRACT 80 FOR A/C ID, and negate for C ID)
		};
	};
	OINUM	evtOi;					// 6 Object Identifier (if normal object)
	short	evtOiList;				// 8 Pointer
	char	evtFlags,				// 10 Flags
			evtFlags2,				// 11 Flags II
			evtNParams,				// 12 Number of parameters
			evtDefType;				// 13 If default, type
	// Conditions
	short	evtIdentifier;	  		// 14 Event identifier
									// 16
};
//typedef	event	*	PEVT;
//typedef	event	*	LPEVT;

#define	CND_SIZE					sizeof(event2)
#define	ACT_SIZE					(sizeof(event2)-2) // Ignore Identifier

// Moved to AllPlatforms header
//#define		ACTFLAGS_REPEAT			0x1


// For flags II
// -------------
#define		EVFLAG2_NOT			0x1
#define		EVFLAG2_NOTABLE		0x2
#define		EVFLAGS_NOTABLE		(0x2 << 8)
#define		EVFLAGS_MONITORABLE	0x4
#define		EVFLAGS_TODELETE	0x8
#define		EVFLAGS_NEWSOUND	0x10
#define		EVFLAG2_MASK		(EVFLAG2_NOT|EVFLAG2_NOTABLE|EVFLAGS_MONITORABLE)

// MACRO: Returns the code for an extension
#define		EXTCONDITIONNUM(i)		(-((short)(i>>16))-1)
#define		EXTACTIONNUM(i)			((short)(i>>16))
struct RunHeader;
struct eventGroup {
	NO_DEFAULT_CTORS_OR_DTORS(eventGroup);
	std::uint8_t get_evgNCond();
	std::uint8_t get_evgNAct();
	std::uint16_t get_evgIdentifier();
	std::uint16_t get_evgInhibit();
	event2 * GetCAByIndex(size_t index);

DarkEdifInternalAccessProtected:
	friend RunHeader;

	short			evgSize;		// 0 Size of the group (<=0)
	unsigned char	evgNCond;		// 2 Number of conditions
	unsigned char	evgNAct;		// 3 Number of actions
	EventGroupFlags	evgFlags;		// 4 Flags
	short			evgInhibit;		// 6 If the group is inhibited
	unsigned short	evgInhibitCpt;	// 8 Counter
	unsigned short	evgIdentifier;	// 10 Unique identifier
	short 			evgUndo;		// 12 Identifier for UNDO
};

typedef		eventGroup	*		PEVG;
typedef		eventGroup	*		LPEVG;
#define		EVG_SIZE				14

// MACRO: next group
#define		EVGNEXT(evgPtr)		   	((LPEVG)((char *)evgPtr-evgPtr->evgSize))
// MACRO: first event
#define		EVGFIRSTEVT(p)	   		((LPEVT)((char *)p+sizeof(eventGroup)))
// MACRO: number of events in the group
#define		EVGNEVENTS(p)	 		(p->evgNCond+p->evgNAct)

// Condition and action structures
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct eventV1 {
	NO_DEFAULT_CTORS_OR_DTORS(eventV1);
	short	size;				// 0 Size of the event
	union
	{
		short		Code;		// 2 Code (hi:NUM lo:TYPE)
		struct
		{
			char	Type,		// 2 Type of object
					Num;		// 3 Number of action/condition
		};
	};
	OINUM	Oi;					// 4 Object Identifier (if normal object)
	short	OiList;				// 6 Pointer
	char	Flags,				// 8 Flags
			Flags2,				// 9 Flags II
			NParams,			// 10 Number of parameters
			DefType;			// 11 If default, type
	// Conditions only
	short	Identifier;	  		// 12 Event identifier
								// 14 = total
};
// typedef	eventV1	*	LPEVTV1;

#define	CND_SIZEV1					sizeof(eventV1)
#define	ACT_SIZEV1					(sizeof(event)-2) // Ignore Identifier
#define	EVGFIRSTEVTV1(p)	   		((LPEVTV1)((char *)p+sizeof(eventGroup)))
#define	EVTPARAMSV1(p) 				((LPEVP)( p->evtCode<0 ? (unsigned char * )p+CND_SIZEV1 : (unsigned char * )p+ACT_SIZEV1) )
#define	EVTNEXTV1(p)		 		((LPEVTV1)((unsigned char * )p+p->evtSize))



// PARAM Structure
// ~~~~~~~~~~~~~~~
class EventParam {
	NO_DEFAULT_CTORS_OR_DTORS(EventParam);
public:
	short			size,
					Code;
	union {
		struct {
			short	W[8];
		};
		struct {
			long	L[4];
		};
	} evp;
};
//typedef	eventParam	*			PEVP;
//typedef	eventParam	*			LPEVP;
//typedef	eventParam	*			fpevp;

// MACRO: next parameter
#define		EVPNEXT(p)		   		((EventParam *)((char *)p+p->evpSize))


// Operators / expressions parameters
// --------------------------------------------------
#define		MIN_LONG				(-10000000L)
#define		MAX_LONG				(10000000L)

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
struct expression {
	NO_DEFAULT_CTORS_OR_DTORS(expression);
	union {
		struct {
			long	Code;		// 2 Code (hi:NUM lo:TYPE)
		};
		struct {
			short	Type;		// 2 Type of object
			short	Num;			// 3 Expression number
		};
	};
	short	size;
	union
	{
		struct {
			short	Oi;
			short	OiList;
		} o;
		struct {
			long	LParam;
		} l;
		struct {
			double	Double;
			float	Float;
		} d;
		struct {
			short	WParam0;
			short	WParam1;
		} w;
		struct {
			short	Oi;
			short	OiList;
			short	Num;
		} v;
		struct {
			long	ExtCode;
			short	ExtNumber;
		} c;
	} u;
};
//typedef	expression *	LPEXP;
//typedef	expression *	PEXP;

#define		CMPOPE_EQU				(0)
#define		CMPOPE_DIF				(0x1)
#define		CMPOPE_LOWEQU			(0x2)
#define		CMPOPE_LOW				(CMPOPE_LOWEQU+CMPOPE_DIF)
#define		CMPOPE_GREEQU			(0x4)
#define		CMPOPE_GRE				(CMPOPE_GREEQU+CMPOPE_DIF)
#define		MAX_CMPOPE				6
#define		EXPNEXT(expPtr)			((expression *)((char *)expPtr+expPtr->expSize))

struct expressionV1 {
	NO_DEFAULT_CTORS_OR_DTORS(expressionV1);
	union
	{
		struct {
			short	Code;
		};
		struct {
			char	Type,
					Num;
		};
	};
	short	size;
	union {
		struct {
			short	Oi;
			short	OiList;
		} o;
		struct {
			long	LParam;
		} l;
		struct {
			double	Double;
			float	Float;
		} d;
		struct {
			short	WParam0;
			short	WParam1;
		} w;
		struct {
			short	Oi;
			short	OiList;
			short	Num;
		} v;
		struct {
			long	ExtCode;
			short	ExtNumber;
		} c;
	} u;
};
//typedef	expressionV1 *			LPEXPV1;

#define		EXPNEXTV1(expPtr)		((expressionV1 *)((char *)expPtr + expPtr->expSize))

/*
#define		Q_SPR	0x100
#define		Q_TXT	0x200
#define		Q_QST	0x400
#define		Q_ARE	0x800
#define		Q_CNT	0x1000
#define		Q_PLA	0x2000
#define		Q_GAM	0x4000
#define		Q_TIM	0x8000
#define		Q_COL	0x1
#define		Q_ZNE	0x2
#define		Q_MVT	0x4
#define		Q_ANI	0x8
#define		Q_OBJ	0x10
#define		Q_KEY	0x20
#define		Q_SYS	0x40
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
#define		NUMBER_OF_SYSTEM_TYPES	7
enum class OBJ {
	// Built-in objects, included in all programs
	PLAYER = -7,
	KEYBOARD,
	CREATE,
	TIMER,
	GAME,
	STORYBOARD = GAME, // I think?
	SPEAKER,
	SYSTEM,
	// Built-in objects, have to be added to frame
	QUICK_BACKGROUND,
	BACKGROUND,
	ACTIVE,
	TEXT,
	QUESTION_AND_ANSWER,
	SCORE,
	LIVES,
	COUNTER,
	// Built-in, separated for some reason
	FIRST_C_OBJECT,			// First custom object (mfx)
	RTF = FIRST_C_OBJECT,	// NOT Rich Edit object, the other one.
	SUB_APPLICATION,
	SYS_OBJ,				// ?
	LAST = SYS_OBJ,
};

// ------------------------------------------------------------
// EXTENSION OBJECT DATA ZONE
// ------------------------------------------------------------

enum class CPF {
	DIRECTION = 0x1,
	ACTION = 0x2,
	INITIALDIR = 0x4,
	DEFAULTDIR = 0x8
};

///////////////////////////////////////////////////////////////////////
//
// DEFINITION OF THE DIFFERENT PARAMETERS
//
///////////////////////////////////////////////////////////////////////

struct ParamObject {
	NO_DEFAULT_CTORS_OR_DTORS(ParamObject);
	unsigned short	OffsetListOI,	//
					Number,			//
					Type;			// Version > FVERSION_NEWOBJECTS
};
struct ParamTime {
	NO_DEFAULT_CTORS_OR_DTORS(ParamTime);
	long	Timer,		// Timer
			EqivLoops;	// Equivalent loops
};
struct ParamBorder {
	NO_DEFAULT_CTORS_OR_DTORS(ParamBorder);
	short BorderType;
	enum BorderFlags {
		LEFT	= 0x1,
		RIGHT	= 0x2,
		TOP		= 0x4,
		BOTTOM	= 0x8,
	};
};
struct ParamDir {
	NO_DEFAULT_CTORS_OR_DTORS(ParamDir);
	short Direction;	// Direction, 0-31 presumably.
};
struct ParamInt  {
	NO_DEFAULT_CTORS_OR_DTORS(ParamInt);
	short	Base,		// Short (or base)
			Maximum;	// Nothing (or maximum)
};
struct ParamSound {
	NO_DEFAULT_CTORS_OR_DTORS(ParamSound);
	short	Handle,
			Flags;
	TCHAR	name[64]; // Max sound name
	enum SoundFlags {
		UNINTERRUPTABLE = 0x1,
		BAD = 0x2,
		// CF2.5 and above only
		IPHONE_AUDIOPLAYER = 0x4,
		// CF2.5 and above only
		IPHONE_OPENAL = 0x8
	};
};
struct ParamPosition {
	NO_DEFAULT_CTORS_OR_DTORS(ParamPosition);
	short	Parent_ObjInfoNum,
			Flags,
			X, Y,
			Slope,
			Angle;				// Angle (forced short, 0-360?)
	long	Dir;				// Direction
	short	TypeParent,			//
			ObjInfoList,		//
			Layer;				//
};
struct ParamCreate {
	NO_DEFAULT_CTORS_OR_DTORS(ParamCreate);
	ParamPosition	Pos;	// Position structure
	HFII			FII;	// FrameItemInstance number
	OINUM			Oi;		// OI of the object to create
//	unsigned int	FII;	// No longer used
	unsigned int	Free;	// Ignore - Given due to no longer used FII
};
struct ParamShoot {
	NO_DEFAULT_CTORS_OR_DTORS(ParamShoot);
	ParamPosition	Pos;	// Position structure
	HFII			FII;	// FrameItemInstance number
	OINUM			Oi;		// OI of the object to shoot
	short			Speed;	// Speed
};
struct ParamAnimation {
	NO_DEFAULT_CTORS_OR_DTORS(ParamAnimation);
	short	Number;
	TCHAR *	name;
};
struct ParamNoP {
	NO_DEFAULT_CTORS_OR_DTORS(ParamNoP);
	short Unused;
};
struct ParamPlayer {
	NO_DEFAULT_CTORS_OR_DTORS(ParamPlayer);
	short Number;
};
struct ParamEvery {
	NO_DEFAULT_CTORS_OR_DTORS(ParamEvery);
	long	Delay,
			Counter;
};
struct ParamKey {
	NO_DEFAULT_CTORS_OR_DTORS(ParamKey);
	unsigned short VK_Code;	// Virtual Key Code
};
struct ParamSpeed {
	NO_DEFAULT_CTORS_OR_DTORS(ParamSpeed);
	int Speed;
};
struct ParamNewPosition {
	NO_DEFAULT_CTORS_OR_DTORS(ParamNewPosition);
	unsigned short Direction; // The rest of the position variables are specified elsewhere
};
struct ParamZone {
	NO_DEFAULT_CTORS_OR_DTORS(ParamZone);
	short	X1, Y1,	// Top-left corner
			X2, Y2;	// Bottom-right corner
};
struct ParamExpression {
	NO_DEFAULT_CTORS_OR_DTORS(ParamExpression);
	short	ID,
			Unknown[6];
};
struct ParamColour {
	NO_DEFAULT_CTORS_OR_DTORS(ParamColour);
	COLORREF	RGB;
	long		ID;
};
struct ParamBuffer {
	NO_DEFAULT_CTORS_OR_DTORS(ParamBuffer);
	void * Buffer; // Or it could be size, I dunno.
};
struct ParamFrame {
	NO_DEFAULT_CTORS_OR_DTORS(ParamFrame);
	unsigned short Number;
};
struct ParamSoundLoop {
	NO_DEFAULT_CTORS_OR_DTORS(ParamSoundLoop);
	short NumOfLoops;
};
struct ParamNewDir {
	NO_DEFAULT_CTORS_OR_DTORS(ParamNewDir);
	unsigned short	OldDir,	// No idea what's inside this struct, all we know is it's 4 bytes.
					NewDir;
};
struct ParamTextNum {
	NO_DEFAULT_CTORS_OR_DTORS(ParamTextNum);
	int Number;
};
struct ParamClick {
	NO_DEFAULT_CTORS_OR_DTORS(ParamClick);
	int Value;
	enum Masks {
		ClickMask = 0x00FF,
		DoubleClick = 0x0100
	};
};
struct ParamProgram {
	NO_DEFAULT_CTORS_OR_DTORS(ParamProgram);
	short	Flags;				// Default flags
	TCHAR	Path[MAX_PATH],		// name of the program
			Command[108];		// Command line

	enum Masks {
		Wait = 0x1,
		Hide = 0x2,
	};
};
struct ParamCondSound {
	NO_DEFAULT_CTORS_OR_DTORS(ParamCondSound);
	unsigned short	Number,
					Flags,
					Loops;
	TCHAR *			name;
};
struct ParamEditorComment {
	NO_DEFAULT_CTORS_OR_DTORS(ParamEditorComment);
	LOGFONTV1		LogFont;			// Font
	COLORREF		ColourFont,			// text color
					ColourBack;			// Background color
	short			Align;				// Alignement flags
	unsigned short	TextId;				// text number in the buffer
	TCHAR			Style[40];			// Style
};
struct ParamGroup {
	NO_DEFAULT_CTORS_OR_DTORS(ParamGroup);
	short			Flags,			// Active / Inactive?
					ID;				// Group identifier
	// May be editor-only, missing in runtime:
	TCHAR			Title[80],		// Title (max 80? chars)
					Password[16];	// Protection (max 16? chars)
	unsigned long	Checksum;		// Checksum
	enum Masks {
		INACTIVE		= 0x1,
		CLOSED			= 0x2,
		PARENT_INACTIVE	= 0x4,
		GROUP_INACTIVE	= 0x8,
		GLOBAL			= 0x10,
		/* Old flags
		FADE_IN			= 0x4,
		FADE_OUT		= 0x8,
		UNICODE_		= 0x10,*/
	};
};
#define GETEVPGRP(evpPtr) (paramGroup *)&evpPtr->evp.evp0
struct ParamGroupPointer {
	NO_DEFAULT_CTORS_OR_DTORS(ParamGroupPointer);
	void *	PointTo;
	short	ID;
};
struct ParamFilename {
	NO_DEFAULT_CTORS_OR_DTORS(ParamFilename);
	TCHAR * FileName;
};
struct ParamString {
	NO_DEFAULT_CTORS_OR_DTORS(ParamString);
	TCHAR * String;
};
struct ParamCmpTime {
	NO_DEFAULT_CTORS_OR_DTORS(ParamCmpTime);
	long	Timer,
			Loops;
	short Comparison;
};
struct ParamPasteSprite {
	NO_DEFAULT_CTORS_OR_DTORS(ParamPasteSprite);
	short	Flags,
			Security; // == padding?
};
struct ParamVKCode {
	NO_DEFAULT_CTORS_OR_DTORS(ParamVKCode);
	short Code;
};
struct ParamStringExp {
	NO_DEFAULT_CTORS_OR_DTORS(ParamStringExp);
	short	ID,
			Unknown[6];	// Assuming 6 from ParamExpression
};
struct ParamInkEffect {
	NO_DEFAULT_CTORS_OR_DTORS(ParamInkEffect);
	short	ID,					// ID of effect
			ParameterEffect;	// Effect parameter
	long	Free;				// Ignore - free
};
struct ParamMenu {
	NO_DEFAULT_CTORS_OR_DTORS(ParamMenu);
	long	ID,
			Security; // == Padding?
};
struct ParamVariable {
	NO_DEFAULT_CTORS_OR_DTORS(ParamVariable);
	long Value; // Global variable, alterable value, flag
};
struct ParamExtension
{
	NO_DEFAULT_CTORS_OR_DTORS(ParamExtension);
	short	size,
			Type,
			Code;
	char	data[2];	// MaxSize = 512, Size = 12, not sure if those are related
};
#define		PARAM_EXTBASE			1000
struct Param8Dirs {
	NO_DEFAULT_CTORS_OR_DTORS(Param8Dirs);
	long Flags;
	enum Mask {
		// Todo! Use bitXX.
	};
};
struct ParamMvt {
	NO_DEFAULT_CTORS_OR_DTORS(ParamMvt);
	short	mvtNumber;
	TCHAR	mvtName[32];	// Max movement name = 32 bytes
};
struct ParamProgram2 {
	NO_DEFAULT_CTORS_OR_DTORS(ParamProgram2);
	short	Flags; // Default flags
	enum Masks {
		Wait = 0x1,
		Hide = 0x2,
	};
};
struct ParamEffect {
	NO_DEFAULT_CTORS_OR_DTORS(ParamEffect);
	TCHAR *	name;
};

namespace Edif {
	Params ReadActionOrConditionParameterType(const char*, bool&);
	ExpParams ReadExpressionParameterType(const char*, bool&);
	ExpReturnType ReadExpressionReturnType(const char* text);
}

///////////////////////////////////////////////////////////////
// STRUCTURE FOR FAST LOOPS
///////////////////////////////////////////////////////////////////////
struct FastLoop
{
	NO_DEFAULT_CTORS_OR_DTORS(FastLoop);
	TCHAR *			Next;
	TCHAR			name[64];	// Max fast loop name (64 bytes)
	unsigned short	Flags;
	long			Index;
	enum Masks {
		Stop = 0x1,
	};
};
//typedef	FastLoop *	LPFL;

///////////////////////////////////////////////////////////////
// DEBUGGER
///////////////////////////////////////////////////////////////
enum class DEBUGGER {
	DEBUGGER_RUNNING,
	DEBUGGER_TO_RUN,
	DEBUGGER_PAUSE,
	DEBUGGER_TO_PAUSE,
	DEBUGGER_STEP,
};

// TREE identification
enum class DBTYPE
{
	SYSTEM,
	OBJECT,
	HO,
	MOVEMENT,
	ANIMATION,
	VALUES,
	COUNTER,
	SCORE,
	LIVES,
	TEXT,
	EXTENSION
};

// Generic entries in the tree
enum
{
	DB_END = 0xFFFF,
	DB_PARENT = 0x8000
};
#define DB_EDITABLE		0x80

// Communication buffer size
#define DB_BUFFERSIZE			256

#define DB_MAXGLOBALVALUES		1000	// Maximum number of global values displayed in the debugger
#define DB_MAXGLOBALSTRINGS		1000

// System tree entries
enum class SYSTEM_DB_TREE
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
#define GETDBPARAMTYPE(a)		((a&0xFF000000)>>24)
#define GETDBPARAMID(a)			((a&0x00FFFF00)>>8)
#define GETDBPARAMCOMMAND(a)	 (a&0x000000FF)
#define GETDBPARAM(a,b,c)		((a&0x000000FF)<<24)|((b&0xFFFF)<<8)|(c&0xFF)

struct EditDebugInfo
{
	TCHAR * Title;
	int		value;
	TCHAR * text;
	int		lText;
};

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

//typedef void (*ACTIONENDROUTINE)();
//typedef void (*OBLROUTINE)(HeaderObject *);

// CObject
struct objectsList {
	RunObject* oblOffset;
	void (* oblRoutine)(HeaderObject *);

	RunObjectMultiPlatPtr GetOblOffsetByIndex(std::size_t);
};
// typedef	objectsList *			LPOBL;

#define		GAMEBUFFERS_SIZE	(12*1024)
#define		GAME_MAXOBJECTS		266
#define		OBJECT_MAX			128L
#define		OBJECT_SIZE			256L
#define		OBJECT_SHIFT		8
#define		MAX_INTERMEDIATERESULTS		256
#define		STEP_TEMPSTRINGS	64

struct RunHeader2 {
	NO_DEFAULT_CTORS_OR_DTORS(RunHeader2);
	unsigned long	OldPlayer,		// Previous player entries
					NewPlayer,		// Modified player entries
					InputMask,		// Inhibated players entries
					InputPlayers;	// Valid players entries (mask!)
	unsigned char 	MouseKeys,		// Mousekey entries
					ActionLoop,		// Actions flag
					ActionOn,		// Flag: are we in actions?
					EnablePick;  	// Flag: Are we in pick for actions?

	int		  		EventCount;			// Number of the event
	qualToOi *		EventQualPos;		// Position in event objects
	HeaderObject  *	EventPos;			// Position in event objects
	objInfoList *	EventPosOiList, 	// Position in oilist for TYPE exploration
				*	EventPrev;			// Previous object address

	pev *			PushedEvents;		//
	unsigned char *	PushedEventsTop,	//
				  *	PushedEventsMax;	//
	int	  			NewPushedEvents;	//

	int		  		ActionCount;		// Action counter
	int		  		ActionLoopCount;	// Action loops counter
	void	(*ActionEndRoutine)();		// End of action routine

	// Number of objects created since frame start. Increments per object instance create of any object; wraps at 16-bit.
	// @remarks While CreationCount starts as 0, 0 isn't used since Fusion b243 and it is incremented again to 1
	unsigned short	CreationCount;		// Number of objects created since beginning of frame
	short			EventType;
	POINT 			Mouse;				// Mouse coordinate
	POINT 			MouseClient;		// Mouse coordinates in the window
	short	  		CurrentClick,		// For click events II
					Free2;				// Ignore - Padding
	HeaderObject **	ShuffleBuffer,
				 ** ShufflePos;
	int		  		ShuffleNumber;

	POINT			MouseSave;			// Mouse saving when pause
	int		  		PauseCompteur;
	unsigned long	PauseTimer;
	unsigned int	PauseVbl;
	FARPROC	   		LoopTraceProc,		// Debugging routine
					EventTraceProc;

};


// Flags pour rh3Scrolling
#define RH3SCROLLING_SCROLL					0x1
#define RH3SCROLLING_REDRAWLAYERS			0x2
#define RH3SCROLLING_REDRAWALL				0x4
#define RH3SCROLLING_REDRAWTOTALCOLMASK		0x8

#define	GAME_XBORDER		480
#define	GAME_YBORDER		300

struct RunHeader3 {
	NO_DEFAULT_CTORS_OR_DTORS(RunHeader3);
	unsigned short	Graine,			// Random generator seed
					Free;			// Ignore - padding

	int		  		DisplayX,		// To scroll
					DisplayY;

	long			CurrentMenu;	// For menu II events

	int 			WindowSx,		// Window size
					WindowSy;

	short			CollisionCount;	// Collision counter
	char			DoStop,			// Force the test of stop actions
					Scrolling;		// Flag: we need to scroll

	int		  		Panic,
					PanicBase,
					PanicPile,

//	short		  	XBorder_;		// Authorised border
//	short		  	YBorder_;
					XMinimum,   	// Object inactivation coordinates
					YMinimum,
					XMaximum,
					YMaximum,
					XMinimumKill,	// Object destruction coordinates
					YMinimumKill,
					XMaximumKill,
					YMaximumKill;
};

// Extensions
// ~~~~~~~~~~~~~~~~~~~~
#define		KPX_MAXNUMBER					96
typedef struct tagKPXMSG {
	short 	( WINAPI * routine) (unsigned int, HWND, unsigned int, WPARAM, LPARAM);
	} kpxMsg;
#define		KPX_MAXFUNCTIONS				32
typedef struct tagKPXLIB {
	long 	( WINAPI * routine) (HeaderObject *, WPARAM, LPARAM);
	} kpxLib;
#define IsRunTimeFunctionPresent(num)	(num < KPX_MAXFUNCTIONS && ((RunHeader *)rdPtr->rHo.AdRunHeader)->rh4.rh4KpxFunctions[(int)num].routine != NULL)
#define	CallRunTimeFunction(rdPtr,num,wParam,lParam)	(rdPtr->rHo.AdRunHeader->rh4.rh4KpxFunctions[(int)num].routine((HeaderObject *)rdPtr, wParam, lParam) )
#define	CallRunTimeFunction2(hoPtr,num,wParam,lParam)	(hoPtr->AdRunHeader->rh4.rh4KpxFunctions[(int)num].routine(hoPtr, wParam, lParam) )
#define	CallRunTimeFunction3(rh4_,num,wParam,lParam)	(rh4_.rh4KpxFunctions[num].routine(hoPtr, wParam, lParam) )

enum class RFUNCTION {
	//
	REHANDLE,				// Re-enable the call to Handle() every frame
	GENERATE_EVENT,			// Immediately create a triggered condition. Do not call in other threads.
	PUSH_EVENT,				// Generates the event after the next event loop, only should be used if possibly cancelling this event later. Do not call in other threads.
	GET_STRING_SPACE_EX,	// Allocates memory from MMF for a char * or wchar_t *, letting you store a string.
	GET_PARAM_1,			// Retrieves the value of the first parameter of an a/c/e.
	GET_PARAM_2,			// Retrieves the value of 2nd+ parameter (first call with this is 2nd, next 3rd, etc).
	PUSH_EVENT_STOP,		// Removes event created by Push Event. Cannot do this with Generate Event as it happens immediately.
	PAUSE,
	CONTINUE,
	REDISPLAY,				// Causes the entire frame to redraw
	GET_FILE_INFOS,			// Retrieve information about the current app (see FILEINFOS namespace and Edif.Runtime.cpp)
	SUB_CLASS_WINDOW,
	REDRAW,					// Causes the object to redraw this extension
	DESTROY,
	GET_STRING_SPACE,		// Deprecated GET_STRING_SPACE_EX; affords less memory
	EXECUTE_PROGRAM,
	GET_OBJECT_ADDRESS,
	GET_PARAM,
	GET_EXP_PARAM,
	GET_PARAM_FLOAT,
	EDIT_INT,				// Edittime only: Opens a dialog box to edit an integer
	EDIT_TEXT,				// Edittime only: Opens a dialog box to edit text.
	CALL_MOVEMENT,
	SET_POSITION,
	GET_CALL_TABLES
};

#define CNC_GetParameter(rdPtr)							CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM, 0xFFFFFFFF, 0)
#define CNC_GetIntParameter(rdPtr)						CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM, 0, 0)
#define CNC_GetStringParameter(rdPtr)					CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM, 0xFFFFFFFF, 0)
#define CNC_GetFloatParameter(rdPtr)					CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM, 2, 0)
#define CNC_GetFloatValue(rdPtr, par)					CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_FLOAT, par, 0)
#define CNC_GetFirstExpressionParameter(rdPtr, lParam, wParam)	CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_1, wParam, lParam)
#define CNC_GetNextExpressionParameter(rdPtr, lParam, wParam)	CallRunTimeFunction(rdPtr, RFUNCTION::GET_PARAM_2, wParam, lParam)

// ??
#define MAX_FRAMERATE						10

typedef void (* CALLANIMATIONS) (HeaderObject *, int);
#define callAnimations(hoPtr, anim) ( (hoPtr->hoAdRunHeader)->rh4.rh4Animations(hoPtr, anim) )

typedef unsigned int (* CALLDIRATSTART) (HeaderObject *, unsigned int);
#define callDirAtStart(hoPtr, initDir) ( (hoPtr->hoAdRunHeader)->rh4.rh4DirAtStart(hoPtr, initDir) )

typedef BOOL (* CALLMOVEIT) (HeaderObject *);
#define callMoveIt(hoPtr) ( (hoPtr->hoAdRunHeader)->rh4.rh4MoveIt(hoPtr) )

typedef BOOL (* CALLAPPROACHOBJECT) (HeaderObject *, int destX, int destY, int maxX, int maxY, int htFoot, int planCol, int& x, int &y);
#define callApproachObject(x, y, hoPtr, destX, destY, maxX, maxY, htFoot, planCol) ( (hoPtr->hoAdRunHeader)->rh4.rh4ApproachObject(hoPtr, destX, destY, maxX, maxY, htFoot, planCol, x, y) )

typedef void (* CALLCOLLISIONS) (HeaderObject *);
#define callCollisions(hoPtr) ( (hoPtr->hoAdRunHeader)->rh4.rh4Collisions(hoPtr) )

typedef BOOL (* CALLTESTPOSITION) (HeaderObject *, int x, int y, int htFoot, int planCol, BOOL flag);
#define callTestPosition(hoPtr, x, y, htFoot, planCol) ( (hoPtr->hoAdRunHeader)->rh4.rh4TestPosition(hoPtr, x, y, htFoot, planCol, 0) )

typedef std::uint8_t (* CALLGETJOYSTICK) (HeaderObject *, int);
#define callGetJoystick(hoPtr, player) ( (hoPtr->hoAdRunHeader)->rh4.rh4Joystick(hoPtr, player) )

typedef BOOL (* CALLCOLMASKTESTRECT) (HeaderObject *, int x, int y , int sx, int sy, int nLayer, int plan);
#define callColMaskTestRect(hoPtr, x, y, sx, sy, nLayer, plan) ( (hoPtr->hoAdRunHeader)->rh4.rh4ColMaskTestRect(hoPtr, x, y, sx, sy, nLayer, plan) )

typedef BOOL (* CALLCOLMASKTESTPOINT) (HeaderObject *, int x, int y, int nLayer, int plan);
#define callColMaskTestPoint(hoPtr, x, y, nLayer, plan) ( (hoPtr->hoAdRunHeader)->rh4.rh4ColMaskTestPoint(hoPtr, x, y, nLayer, plan) )

struct RunHeader4 {
	NO_DEFAULT_CTORS_OR_DTORS(RunHeader4);
	kpj *			rh4KpxJumps;				// Jump table offset
	short			rh4KpxNumOfWindowProcs,		// Number of routines to call
					rh4Free;					// Padding - ignore
	kpxMsg			rh4KpxWindowProc[96];		// Message handle routines (max 96)
	kpxLib			rh4KpxFunctions[32];		// Available internal routines (max 32)

	// ?
	void (* Animations)(HeaderObject  *, int);
	// Get direction at start
	unsigned int (* DirAtStart)(HeaderObject  *, unsigned int);
	// Move object
	BOOL (* MoveIt)(HeaderObject  *);
	// Approach object
	BOOL (* ApproachObject)(HeaderObject *, int destX, int destY, int maxX, int maxY, int htFoot, int planCol, int& x, int &y);
	// ?
	void (* Collisions)(HeaderObject  *);
	// ?
	void (* TestPosition)(HeaderObject  *);
	// ?
	std::uint8_t (* GetJoystick)(HeaderObject  *, int);
	// ?
	BOOL (* ColMaskTestRect)(HeaderObject *, int x, int y , int sx, int sy, int nLayer, int plan);
	// ?
	BOOL (* ColMaskTestPoint)(HeaderObject *, int x, int y, int nLayer, int plan);

	std::uint32_t		SaveVersion;
	event2 *			ActionStart;			// Save the current action
	int					PauseKey;
	TCHAR *				CurrentFastLoop;
	std::int32_t		EndOfPause;
	std::int32_t		EventCountOR;		// Number of the event for OR conditions
	std::int16_t		ConditionsFalse,
						MouseWheelDelta;
	std::int32_t		OnMouseWheel;
	TCHAR *				PSaveFilename;
	std::uint32_t		musicHandle;
	unsigned int 		musicFlags,
						musicLoops;
	int					loadCount;
	short				demoMode,
						saveFrame;
	CDemoRecord *		demo;
	int					saveFrameCount;
	double				mvtTimerCoef;

	CIPhoneJoystick *	iPhoneJoystick;
	CIPhoneAd *			iPhoneAd;
	void *				box2DBase;
	short				box2DSearched;
	void *				forEachs;
	void *				currentForEach;
	void *				currentForEach2;
	void *				timerEvents;
	void *				posOnLoop;
	short				complexOnLoop;
	char				quitString[4];		// Free, unknown usage

	std::uint32_t		pickFlags0,			// 00-31
						pickFlags1,			// 31-63
						pickFlags2,			// 64-95
						pickFlags3;			// 96-127
	std::uint32_t *		timerEventsBase;	// Timer events base

	std::int16_t		droppedFlag,
						nDroppedFiles;
	TCHAR *				droppedFiles;
	FastLoop *			fastLoops;
	TCHAR *				creationErrorMessages;
	CValueMultiPlat		expValue1,				// New V2
						expValue2;

	std::int32_t		kpxReturn;				// WindowProc return
	objectsList *		objectCurCreate;
	short				objectAddCreate;
	unsigned short		free10;					// For step through : fake key pressed
	HINSTANCE			instance;				// Application instance
	HWND				hStopWindow;			// STOP window handle
	char				doUpdate,				// Flag for screen update on first loop
						menuEaten;				// Menu handled in an event?
	short				free2;
	int					onCloseCount;			// For OnClose event
	short				cursorCount,			// Mouse counter
						scrMode;				// Current screen mode
	HPALETTE			hPalette;				// Handle current palette
	int 				vblDelta;				// Number of VBL
	std::uint32_t		loopTheoric,			// Theorical VBL counter
						eventCount;
	drawRoutine *		FirstBackDrawRoutine,	// Backrgound draw routines list
				*		LastBackDrawRoutine;	// Last routine used

	unsigned long		ObjectList;				// Object list offset
	short				LastQuickDisplay;		// Quick - display list
	unsigned char		CheckDoneInstart,		// Correct start of frame with fade in
						Free0;					// Ignore - padding
	mv *				rh4Mv;					// Yves' data
	HCURSOR				OldCursor;				// Old cursor for Show / HideMouse in Vitalize! mode
	HeaderObject  *		_2ndObject;	 			// Collision object address
	short 				_2ndObjectNumber,		// Number for collisions
						FirstQuickDisplay;		// Quick-display object list
	int					WindowDeltaX,			// For scrolling
						WindowDeltaY;
	unsigned int		TimeOut;				// For time-out
	int					MouseXCenter,			// To correct CROSOFT bugs
						MouseYCenter,			// To correct CROSOFT bugs
						TabCounter;				// Objects with tabulation

	unsigned long		AtomNum,				// For child window handling
						AtomRd,
						AtomProc;
	short				SubProcCounter,			// To accelerate the windows
						Free3;

	int					PosPile;								// Expression evaluation pile position
	expression *		ExpToken;							// Current position in expressions
	CValueMultiPlat *	Results[MAX_INTERMEDIATERESULTS];	// Result pile
	long				Operators[MAX_INTERMEDIATERESULTS];	// Operators pile

	TCHAR **			PTempStrings;		// Debut zone 256 long
	int					MaxTempStrings;
	long				Free4[256-2];		// Free buffer

	int					NCurTempString;					// Pointer on the current string
	unsigned long		FrameRateArray[MAX_FRAMERATE];	// Framerate calculation buffer
	int					FrameRatePos;					// Position in buffer
	unsigned long		FrameRatePrevious;				// Previous time
};

enum class GAMEFLAGS {
	VBLINDEP = 0x2,
	LIMITED_SCROLL = 0x4,
	FIRST_LOOP_FADE_IN = 0x10,
	LOAD_ON_CALL = 0x20,
	REAL_GAME = 0x40,
	PLAY = 0x80,
	//FADE_IN = 0x80,
	//FADE_OUT = 0x100,
	INITIALISING = 0x200,
};

// A stand-in for the CRun class on other platforms, which may have items from CEventProgram
struct RunHeader {
	NO_DEFAULT_CTORS_OR_DTORS(RunHeader);

	// Reads the EventCount variable from RunHeader2, used in object selection. DarkEdif-added function for cross-platform.
	int GetRH2EventCount();
	// Gets the EventCountOR, used in object selection in OR-related events. DarkEdif-added function for cross-platform.
	int GetRH4EventCountOR();
	event2* GetRH4ActionStart();

	// Reads the rh2.rh2ActionOn variable, used to indicate actions are being run (as opposed to conditions, or Handle, etc).
	bool GetRH2ActionOn();

	// Reads the rh2.rh2ActionCount variable, used in a fastloop to loop the actions.
	int GetRH2ActionCount();
	// Reads the rh2.rh2ActionLoopCount variable, used when looping object instances to run action on each selected instance one by one
	int GetRH2ActionLoopCount();

	// Sets the rh2.rh2ActionCount variable, used in an action with multiple instances selected, to repeat one action.
	void SetRH2ActionCount(int newActionCount);
	// Sets the rh2.rh2ActionLoopCount variable, used in actions when looping object instances to run an action on each instance
	void SetRH2ActionLoopCount(int newActLoopCount);

	eventGroup* get_EventGroup();
	// Gets number of OIList currently in frame, see GetOIListByIndex()
	// @remarks In non-Windows, this is normally a rhMaxOI variable, in Windows it's NumberOi, and includes an extra, invalid Oi
	std::size_t GetNumberOi();
	// Returns a pointer to a list of objects, length MaxObjects, number of non-null entries NObjects
	objectsList* get_ObjectList();
	// Gets max capacity of simultaneous object instances currently in frame (up to 30k?)
	std::size_t get_MaxObjects();
	// Gets number of valid object instances currently in frame
	std::size_t get_NObjects();
	CRunApp* get_App();

	objInfoList* GetOIListByIndex(std::size_t index);
	qualToOi* GetQualToOiListByOffset(std::size_t byteOffset);
	RunObjectMultiPlatPtr GetObjectListOblOffsetByIndex(std::size_t index);
	EventGroupFlags GetEVGFlags();

DarkEdifInternalAccessProtected:
	friend DarkEdif::ObjectSelection;
	friend ConditionOrActionManager_Windows;
	friend ExpressionManager_Windows;
	friend Edif::Runtime;
	void *				IdEditWin,			// npWin or Win *, but evaluates to void *
		 *				IdMainWin;
	void *				IdAppli;			// npAppli or Appli *, but evaluates to void *

	HWND				HEditWin,			// Call GetClientRect(HEditWin, RECT *) to get the frame width/height (non-virtual) area
						HMainWin,			// Call GetClientRect(HMainWin, RECT *) to get the real client area
						HTopLevelWnd;

	CRunApp *			App;				// Application info
	CRunFrameMultiPlat*	Frame;				// Frame info

	unsigned int		JoystickPatch;		// To reroute the joystick

	unsigned char		Free10,				// Current movement needs to be stopped
						Free12, 			// Event evaluation flag
						NPlayers,			// Number of players
						MouseUsed;			// Players using the mouse

	unsigned short		GameFlags,			// Game flags
						Free;				// Alignment
	unsigned int		Player;				// Current players entry

	short 				Quit,
						QuitBis; 			// Secondary quit (scrollings)
	unsigned int		Free11,				// Value to return to the editor
						QuitParam;

	// Buffers
	int					NObjects,
						MaxObjects;

	unsigned int		Free2[4];			// !No description in original SDK

	int 				NumberOi;			// Number of OI in the list
	objInfoList *		OiList;				// ObjectInfo list

	unsigned int *		Events[7+1],		// Events pointers (NUMBER_OF_SYSTEM_TYPES+1)
				 *		EventLists,		 	// Pointers on pointers list
				 *		Free8,				// Timer pointers
				 *		EventAlways;		// Pointers on events to see at each loop
	eventGroup *		Programs;			// Program pointers
	short *				LimitLists;			// Movement limitation list
	qualToOi *			QualToOiList;		// Conversion qualifier->oilist

	unsigned int		SBuffers;			// Buffer size /1024
	unsigned char * 	Buffer,				// Position in current buffer
				  * 	FBuffer,			// End of current buffer
				  * 	Buffer1,			// First buffer
				  * 	Buffer2;			// Second buffer

	int 				LevelSx,			// Window size
						LevelSy,
						WindowX,   			// Start of window in X/Y
						WindowY;

	unsigned int		VBLDeltaOld,		// Number of VBL
						VBLObjet,			// For the objects
						VBLOld;				// For the counter

	int					EventsSize;
	unsigned short		MT_VBLStep,   		// Path movement variables
						MT_VBLCount;
	unsigned int		MT_MoveStep;

	int					LoopCount;			// Number of loops (FPS) since start of level (including Before Frame Transition?)
	unsigned int		Timer,				// Timer in 1/1000 since start of level
						TimerOld,			// For delta calculation
						TimerDelta;			// For delta calculation again

	eventGroup *		EventGroup;			// Current group
	long 				CurCode;			// Current event
	short				CurOi,
						Free4;				// Alignment
	long				CurParam[2];
	short 				CurObjectNumber,	// Object number
						FirstObjectNumber;	// Number, for collisions

	long				OiListPtr;			// OI list enumeration
	short 				ObListNext,			// Branch label

						DestroyPos;
	long				Free5,
						Free6;

	RunHeader2			rh2;				// Sub-structure #1
	RunHeader3			rh3;				// Sub-structure #2
	RunHeader4			rh4;				// Sub-structure #3

	unsigned long *		DestroyList;		// Destroy list address

	int					DebuggerCommand;	// Current debugger command
	char				Free13[DB_BUFFERSIZE];		// Buffer libre!
	void *				DbOldHO;
	unsigned short		DbOldId,
						Free7;

	objectsList *		ObjectList;			// Object list address
};
//typedef	RunHeader 	* fprh;
//typedef	RunHeader 	* RunHeader *;
//typedef	RunHeader 	* LPRUNHEADER;


///////////////////////////////////////////////////////////////
//
// RUNTIME OBJECT DEFINITION
//
///////////////////////////////////////////////////////////////

#define HOX_INT

struct ForbiddenInternals;
struct HeaderObject {
	NO_DEFAULT_CTORS_OR_DTORS(HeaderObject);
	friend ForbiddenInternals;
	friend ConditionOrActionManager_Windows;
	friend ExpressionManager_Windows;
	friend Edif::Runtime;
	friend RunObject;
DarkEdifInternalAccessProtected:
	// 0+ unique instance number, used in identifying it out of the runtime duplicates
	// @remarks Is non-contiguous and may be out of order, but the linked lists should be in consistent reverse-creation order;
	//			last created is in ObjInfoList::Object, traverse with HeaderObject::NumNext until negative to get to first created.
	short Number;
	// A Number of the next selected instance, or 0x8000 set for selection to end here.
	short NextSelected;
	// The RUNDATA/RunObject size, including the HeaderObject
	int size;
	// Run-header address; a CRun class in non-Windows
	RunHeader* AdRunHeader;
private:
	// Pointer to this - might be an anachronism from 16 -> 32-bit Windows memory addresses
	HeaderObject* Address;
DarkEdifInternalAccessProtected:
	// Number of LevObj or HeaderFrameInstance
	short HFII;
	// The 0+ index of this object info in rhPtr->OIList?
	short Oi;
	// Same OI, previous object instance, a Number; 0x8000 bit is set if invalid
	short NumPrev;
	// Same OI, next object instance, a Number; 0x8000 bit is set if invalid
	short NumNext;
	// Type of the object - OBJ_XXX or OBJ::XXX enum
	// @remarks including the 7 built-in OBJ in event editor from -7 to -1, then other built-ins from 0 until 9.
	short Type;

	// rh2CreationCount, incrementing per object instance create of any object, wraps at 16-bit; but shouldn't be 0
	// @remarks While rh2CreationCount starts as 0, 0 isn't used since Fusion b243, and it is incremented to 1 instead
	unsigned short CreationId;

	// Pointer to this object's static Object Info List entry, which is used for all instances of this object
	objInfoList * OiList;

	// Pointer to which call table array index to use when handling events, set to objectInfoList::Events
	// @remarks This is old and deep Fusion code, probably from 16-bit KnP days. Only the built-in extensions use different tables.
	unsigned int * Events;

	// Ignore - padding
private:
	unsigned int Free0;
DarkEdifInternalAccessProtected:

	// List of extension identifiers for one-shot collision event and playzone enter/exit handling
	// @remarks PrevNoRepeat is set at start of event handling tick; BaseNoRepeat is live list.
	//			Not entirely sure what this prevents, but I wouldn't recommend messing with them.
	unsigned char * PrevNoRepeat, * BaseNoRepeat;
	// Used with path movement, works with rhLoopCount
	int Mark1, Mark2;
	// Name of the current node for path movements
	TCHAR * MT_NodeName;

	// 0+ ID of the current A/C/E call, set before their ext function is run
	int EventNumber;

	// Ignore - padding
private:
	int Free2;
DarkEdifInternalAccessProtected:

	// Common structure address (OC struct)
	Objects_Common *	Common;

	// TODO: These are used by built-in movements, but what for?
	union {
		struct {
			int CalculX,	// Low weight value
				X,			// X coordinate
				CalculY,	// Low weight value
				Y;			// Y coordinate
		};
		struct {
			std::int64_t CalculXLong,
						 CalculYLong;
		};
	};

	// Hot spot of the current image
	int	ImgXSpot, ImgYSpot;
	// Width/Height of the current image
	int ImgWidth, ImgHeight;

	// Display rectangle
	// TODO: Relative to frame, window, client area, scroll area?
	RECT Rect;

	// Objects flags (originally OEFLAG_XX enum)
	OEFLAGS OEFlags;

	// HeaderObjectFlags (originally HOF_XX enum)
	HeaderObjectFlags Flags;

	// 0 or 1; indicates this object was filtered/selected by conditions in the event.
	// If 0, all the object instances should be considered selected; if 1, only the selected ones.
	// Invalid if not an OR event! (check (EventGroup->Flags & EventGroupFlags::OrInGroup) != 0)
	// This is useless for exts, as the runtime only uses it to temporarily mark objects
	// before restoring their selection after running conditions, and before running the actions;
	// so both during condition evaluation, and during actions, there's no reason to preserve it.
	bool SelectedInOR;

	// Ignore - padding
private:
	char Free;
DarkEdifInternalAccessProtected:

	// Offset from HeaderObject -> AltVals struct. Only valid if OEFlags includes OEFLAGS::VALUES.
	int OffsetValue;

	// Fusion frame layer, 0+?
	// TODO: Confirm the bottom layer number 0 or 1 based
	unsigned int Layer;

	// Pointer to HandleRunObject routine
	// @remarks Calls Extension::Handle in Edif-based exts
	short (* hoHandleRoutine)(HeaderObject *);

	// Modification routine when coordinates have been modified; updates coordinate and redraws display
	// @remarks Is run if HandleRunObject() returns with roc.rocChanged true
	short (* hoModifRoutine)(HeaderObject *);

	// Pointer to DisplayRunObject; redraws this object
	short (* hoDisplayRoutine)(HeaderObject *);

	// Collision limitation flags (OILIMITFLAG_XXX)
	short LimitFlags;

	// Quickdraw list - a mainly Windows-only optimization for software (non-Direct3D) display mode
	// @remarks I only see this as -1 or -2 in non-Windows runtime usage, but it's a Windows optimization
	short NextQuickDisplay;

	// Background
	saveRect BackSave;

	// Address of the current A/C/E parameter, set before A/C/E func is run
	EventParam * CurrentParam;

	// Offset to the window handle structure, which is an int handle_count, followed by a HANDLE[handle_count] array.
	// Due to 32-bit variable use, you can specify any RAM address by the offset.
	std::ptrdiff_t OffsetToWindows;

	// ASCII identifier of the object, in 'ABCD' format.
	// @remarks Due to small endianness, this is reversed in one way of reading it, and will read as 'D','C','B','A'.
	//			You might be able to get away with unprintable ASCII characters, but not worth risking.
	//			This is part of object differentiation, along with resource file magic number, and causes strange
	//			Fusion editor behaviour if you have multiple with same identifier: for example, Popup Message Object 2,
	//			and Input Object, both cause out-of-context popups questioning what their unsaid a/c/e intended object is.
	unsigned int Identifier;

public:
	// Unique 0+ instance in rhPtr->ObjectList; use with rhPtr->GetObjectListOblOffsetByIndex()
	// Part of fixed value, with CreationId.
	// @remarks First instance is 0, and they're created in a generally incrementing way, but if objects are
	//			destroyed, their Numbers are used by new creations, so don't rely on it for creation order;
	//			instead, use the NumNext chain.
	short get_Number();

	// 1+ ID, based on rhPtr->rh2CreationCount. Incremented and wraps at MAX_UINT16, but this never is 0.
	// Part of fixed value, with Number.
	// @remarks For example, creating/destroying objects will hit 65,535 objects, even though the simultaneous
	//			number of objects cannot exceed 30k due to frame properties.
	//			So you cannot rely on this for ordering by creation order. Instead, use the NumNext chain.
	unsigned short get_CreationId();

	// Next Number for this object, including unselected instances. If negative/0x8000 bit is set, is not a real Number.
	short get_NumNext();

	// The 0+ index of this object info in rhPtr->OIList; use with rhPtr->GetOIListByIndex(), or just read get_OiList()
	// @remarks OI aka object information in OIList is static information shared among all instances of an object
	short get_Oi();

	// Reads the OIList entry for this object; Obj Info is static information shared among all instances of an object
	objInfoList* get_OiList();

	// Reads the header object flags, which is a mix of flags for multiple purposes
	HeaderObjectFlags get_Flags();

	// Reads the current frame's RunHeader/CRun data, which is live information for the frame.
	RunHeader * get_AdRunHeader();

	// Reads next selected Number for this object, or negative/0x8000 is set, is not a Number and selection chain ends here
	// @remarks While Number is probably more sense as unsigned with the 0x8000 limitation, every runtime uses it as signed.
	short get_NextSelected();

	// Changes the next selected Number in this object's selection chain; -1 or 0x8000 bit is set to end the selection chain here
	void set_NextSelected(short);

	// Reads whether this instance is selected in the current OR event.
	// @remarks At the moment, I don't think these should be get/set by extensions.
	//			It's useless as OR handling and switching between OR-separated conditions is done by runtime,
	//			and OR selection is set into normal selection vars (ListSelected chain), before the actions/conditions are called.
	//			So reading it is pointless, and writing it even more pointless, even in a condition in an OR event.
	bool get_SelectedInOR();

	// Sets whether this instance is selected in the current OR event.
	// TODO: confirm if expression-generated events inside OR events require this to be preserved
	void set_SelectedInOR(bool);

	// Gets the fixed value by combining Number and CreationId; the result should never be 0 or -1, although can be negative
	// @remarks This should be an unsigned int, but I'm leaving it as signed for consistency with runtime and other exts
	int GetFixedValue();
};
// typedef	LPHO HeaderObject*;

// --------------------------------------
// Object's movement structure
// --------------------------------------
struct rMvt {
	NO_DEFAULT_CTORS_OR_DTORS(rMvt);
DarkEdifInternalAccessProtected:
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
	BOOL 	rmReverse;					// Ahead or reverse?

	BOOL  	rmBouncing;					// Bouncing?
	int		rmMvtNum;					// Number of the current movement

	union
	{
		struct
		{
			unsigned char 	rmMvData[80];			// Space for data
		};
		struct
		{
			int		MT_Speed;
			int	 	MT_Sinus;
			int	 	MT_Cosinus;
			unsigned int 	MT_Longueur;
			int	 	MT_XOrigin;
			int	 	MT_YOrigin;
			int	 	MT_XDest;
			int	 	MT_YDest;
			int	 	MT_MoveNumber;
			void * MT_MoveAddress;
			BOOL	MT_Direction;
			void *	MT_Movement;
			int		MT_Calculs;
			int		MT_XStart;
			int		MT_YStart;
			int		MT_Pause;
			TCHAR *	MT_GotoNode;
		};
		struct
		{
			int	 	MM_DXMouse;
			int	 	MM_DYMouse;
			int	 	MM_FXMouse;
			int	 	MM_FYMouse;
			int	 	MM_Stopped;
			int	 	MM_OldSpeed;
			unsigned char	MM_MvOpt;
		};
		struct
		{
			int	 	MG_Bounce;
			long	MG_OkDirs;
			int	 	MG_BounceMu;
			int	 	MG_Speed;
			int 	MG_LastBounce;
			int	 	MG_DirMask;
			unsigned char	MG_MvOpt;
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
			unsigned char	MR_MvOpt;
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
			HeaderObject *	MBul_ShootObject;
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
			HeaderObject *	MP_ObjectUnder;
			int		MP_XObjectUnder;
			int		MP_YObjectUnder;
			BOOL	MP_NoJump;
		};
	};
};
// typedef rMvt*	LPRMV;
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
struct rAni {
	NO_DEFAULT_CTORS_OR_DTORS(rAni);
DarkEdifInternalAccessProtected:
	int	 			AnimForced,		// Flags if forced
					DirForced,
					SpeedForced;
	BOOL			Stopped;
	int	 			On;				// Current ation
	Animation *		Offset;
	int	 			Dir,			// Direction of current ation
					PreviousDir;	// Previous OK direction
	AnimDirection *	DirOffset;
	int	 			Speed,
					MinSpeed,		// Minimum speed of movement
					MaxSpeed,		// Maximum speed of movement
					DeltaSpeed,
					Counter,		// Animation speed counter
					Delta,			// Speed counter
					Repeat,			// Number of repeats
					RepeatLoop,		// Looping picture
					Frame,			// Current frame
					NumberOfFrame,	// Number of frames
					FrameForced;
};
//typedef rAni*	LPRA;

// ----------------------------------------
// Sprite display structure
// ----------------------------------------
enum class RSFLAG : std::uint16_t {
	HIDDEN = 0x1,
	INACTIVE = 0x2,
	SLEEPING = 0x4,
	SCALE_RESAMPLE = 0x8,
	ROTATE_ANTIA = 0x10,
	VISIBLE = 0x20,
	CREATED_EFFECT = 0x40
};
enum_class_is_a_bitmask(RSFLAG);
struct Sprite {
	NO_DEFAULT_CTORS_OR_DTORS(Sprite);
DarkEdifInternalAccessProtected:
	int	 			Flash,				// When flashing objects
					FlashCount,
					Layer,				// Layer
					ZOrder,				// Z-order value
					CreationFlags;		// Creation flags
	COLORREF		BackColor;			// background saving color
	unsigned int	Effect;				// Sprite effects
	LPARAM			EffectParam;
	RSFLAG			Flags;				// Handling flags
	unsigned short	FadeCreationFlags;	// Saved during a fadein
};
//typedef Sprite *	LPRSP;
//typedef rSpr	Sprite;



// ----------------------------------------
// Objects's internal variables (build # >= 243)
// ----------------------------------------
struct AltVals {
	NO_DEFAULT_CTORS_OR_DTORS(AltVals);
DarkEdifInternalAccessProtected:
	friend RunObject;
	union {
		struct CF25 {
			CValueMultiPlat * Values;
			int				  NumAltValues;
			int				  Free1[25 - 1];	// 26 = number of alterable values
			std::uint32_t	  InternalFlags;
			std::uint8_t	  Free2[26];		// 26 = number of alterable values
			const TCHAR * *   Strings;			// Alterable strings (will be null if never used, including if blank in obj properties); change with mvMalloc/mvFree
			int				  NumAltStrings;
		} CF25;
		struct MMF2 {
			CValueMultiPlat * rvpValues;
			int				  rvFree1[26 - 1];
			std::uint32_t	  rvValueFlags;
			std::uint8_t	  rvFree2[26];
			const TCHAR *	  rvStrings[10];	// Alterable strings (will be null if never used, including if blank in obj properties); change with mvMalloc/mvFree
		} MMF2;
	};

public:
	std::size_t GetAltValueCount() const;
	std::size_t GetAltStringCount() const;
	const TCHAR* GetAltStringAtIndex(const std::size_t) const;
	const CValueMultiPlat * GetAltValueAtIndex(const std::size_t) const;
	void SetAltStringAtIndex(const std::size_t, const std::tstring_view&);
	void SetAltValueAtIndex(const std::size_t, const double);
	void SetAltValueAtIndex(const std::size_t, const int);
	// Bitmask of all internal flags; flag N is accessed via ((InternalFlags & (1 << N)) != 0).
	std::uint32_t GetInternalFlags() const;
	void SetInternalFlags(const std::uint32_t);
};
//typedef AltVals *	LPRVAL;

/*
	Obsolete (build 242 or below) - Note: don't remove this (for the SDK)

typedef struct tagRV {
	long	rvValues[VALUES_NUMBEROF_ALTERABLE];
	long	rvValueFlags;
	unsigned char	rvValuesType[VALUES_NUMBEROF_ALTERABLE];
	char *	rvStrings[STRINGS_NUMBEROF_ALTERABLE];
	} AltVals;
typedef AltVals *	LPRVAL;
*/


// -----------------------------------------------
// Objects animation and movement structure
// -----------------------------------------------
#ifndef ANGLETYPE
	#ifdef HWABETA
		typedef float AngleVar;
	#else
		typedef int AngleVar;
	#endif
#endif
typedef void (* RCROUTINE)(HeaderObject *);
struct rCom {
	NO_DEFAULT_CTORS_OR_DTORS(rCom);
DarkEdifInternalAccessProtected:
	friend RunObject;
	int			rcOffsetAnimation; 		// Offset to anims structures
	int			rcOffsetSprite;			// Offset to sprites structures
	RCROUTINE	rcRoutineMove;			// Offset to movement routine
	RCROUTINE	rcRoutineAnimation;		// Offset to animation routine

	int	   		rcPlayer;				// Player who controls

	int	   		rcNMovement;			// Number of the current movement
	CRunMvt *	rcRunMvt;				// Pointer to extension movement
	Spr *  		rcSprite;				// Sprite ID if defined
	int	 		rcAnim;					// Wanted animation
	int	   		rcImage;				// Current frame
	float		rcScaleX;
	float		rcScaleY;
	AngleVar	rcAngle;
	int	   		rcDir;					// Current direction
	int	   		rcSpeed;				// Current speed
	int	   		rcMinSpeed;				// Minimum speed
	int	   		rcMaxSpeed;				// Maximum speed
	BOOL		rcChanged;				// Flag: modified object
	BOOL		rcCheckCollides;		// For static objects

	int	 		rcOldX;					// Previous coordinates
	int	 		rcOldY;
	int	 		rcOldImage;
	AngleVar	rcOldAngle;
	int	 		rcOldDir;
	int	 		rcOldX1;					// For zone detections
	int	 		rcOldY1;
	int	 		rcOldX2;
	int	 		rcOldY2;

	long		rcFadeIn;
	long		rcFadeOut;
};


// ------------------------------------------------------------
// ACTIVE OBJECTS DATAZONE
// ------------------------------------------------------------

class Extension;
struct ForbiddenInternals2;

// RUNDATA, but with all OEFlags and all parts, like what an Active object has
struct RunObject {
	NO_DEFAULT_CTORS_OR_DTORS(RunObject);
	// Reads the header information all objects have available. Cross-platform safe. Never null.
	HeaderObject* get_rHo();
	// Reads the common data used by objects with movements or animations; null if not available
	rCom* get_roc();
	// Reads the movement data; null if not available
	rMvt* get_rom();
	// Reads animation data; null if not available
	rAni* get_roa();
	Sprite* get_ros();
	// Reads alt values, strings, internal flags; null if not available
	AltVals* get_rov();

	// Windows only: For Edif/DarkEdif exts, returns Extension pointer by calculating RUNDATA size.
	// @remarks While it's permitted to put extra variables in RUNDATA, this function assumes pExtension is immediately following
	//			the common structs like AltVals, and that the various structs are local SDK's sizes.
	//			Variables between will break this function; variables after pExtension in RUNDATA won't.
	Extension* GetExtension();

	DarkEdifInternalAccessProtected:
	friend ForbiddenInternals;
	friend ForbiddenInternals2;
	friend ConditionOrActionManager_Windows;
	friend ExpressionManager_Windows;
	HeaderObject  	rHo;		  		// Common structure

	rCom			roc;				// Anim/movement structure
	rMvt			rom;				// Movement structure
	rAni			roa;				// Animation structure
	Sprite			ros;				// Sprite handling structure
	AltVals			rov;				// Values structure
	// Internal usage only: sets the extension pointer, used during Create/DestroyRunObject
	void SetExtension(Extension* const ext);
};
//typedef RunObject *	FPRUNOBJECT;
//typedef RunObject *	LPRUNOBJECT;

#define	GOESINPLAYFIELD		0x1
#define	GOESOUTPLAYFIELD	0x2
#define	WRAP				0x4



// ------------------------------------------------------
// EXTENSION EDITION HEADER STRUCTURE
// ------------------------------------------------------
struct extHeader_v1
{
	NO_DEFAULT_CTORS_OR_DTORS(extHeader_v1);
DarkEdifInternalAccessProtected:
	short extSize,
		  extMaxSize,
		  extOldFlags,		// For conversion purpose
		  extVersion;		// Version number
};
//typedef extHeader_v1*	extHeader *V1;

// ------------------------------------------------------
// System objects (text, question, score, lives, counter)
// ------------------------------------------------------
struct rs {
	NO_DEFAULT_CTORS_OR_DTORS(rs);
DarkEdifInternalAccessProtected:
	HeaderObject 	HeaderObject;	// For all the objects
	rCom			Common;			// Anims / movements / sprites structures
	rMvt			Movement;		// Mouvement structure
	Sprite			Sprite;			// Sprite handling

	short			Player;			// Number of the player if score or lives
	short			Flags;			// Type + flags

	union {
		unsigned char * SubAppli;	// Application (CCA object)
	};
	union
	{
		int			Mini;
		int			OldLevel;
	};
	union
	{
		int			Maxi;				//
		int			Level;
	};
	CValueMultiPlat Value;
	LONG			BoxCx;			// Dimensions box (for lives, counters, texts)
	LONG			BoxCy;
	double			MiniDouble;
	double			MaxiDouble;
	short			OldFrame;		// Counter only
	unsigned char	Hidden;
	unsigned char	Free;
	TCHAR *			TextBuffer;		// text buffer
	int				LBuffer;		// Length of the buffer in BYTES
	unsigned int	Font;			// Temporary font for texts
	union {
		COLORREF	TextColor;		// text color
		COLORREF	Color1;			// Bar color
	};
	COLORREF		Color2;			// Gradient bar color
};
//typedef	rs *	LPRS;


//////////////////////////////////////////////////////////////////////////////
//
// OI (frame object)
//

// Object Info Load Flags
enum class OILFlags : std::uint16_t
{
	OC_LOADED = 0x1,			//
	ELT_LOADED = 0x2,			//
	TO_LOAD = 0x4,				//
	TO_DELETE = 0x8,			//
	CUR_FRAME = 0x10,			//
	TO_RELOAD = 0x20,			// Reload images when frame change
	IGNORE_LOAD_ON_CALL = 0x40,	// Ignore load on call option
};
enum_class_is_a_bitmask(OILFlags);

// OILIST Structure : Data concerning the objects in the game
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// CObjInfo in most platforms
struct objInfoList {
	NO_DEFAULT_CTORS_OR_DTORS(objInfoList);
DarkEdifInternalAccessProtected:
	// Unique ObjectInfo number, in rhPtr->OIList
	short Oi;

	// First selected object instance's HeaderObject::Number, or -1 for no selection
	// @remarks If EventCount does not match rh2EventCount, this value is ignored
	short ListSelected;

	// Type of the object - OBJ:: enum?
	short Type;

	// First object instance's HeaderObject::Number; 0x8000 bit is set if invalid
	short Object;

	// listPointers offset for this object's conditions
	// @remarks Deep Fusion internals, an offset in a list of calltables for conditions,
	//			used by all exts but mostly for built-in Fusion objects
	unsigned int Events;

	// WRAP flags, used with instances' movement struct rmWrapping
	std::int8_t Wrap;

	// bool; used to indicate whether to move to next object instance during action instance loop
	// @remarks Ties with EventProgram RepeatFlag, which doesn't feature in Windows SDK
	std::int8_t NextFlag;

private:
	unsigned short	oilFree; // Padding

DarkEdifInternalAccessProtected:
	// Number of all object instances of this type
	int				NObjects;

	// Used to reset selection between actions; if matching rhPtr->rh2ActionCount, the ListSelected-NextSelected chain is used,
	// otherwise implicitly all instances are still selected (i.e. the Object-NumNext chain).
	// Then the first instance is set to CurrentOi and the loop starts.
	int ActionCount;

	// I'm not sure how this plays with ActionCount.
	// Used to repeat an action selection between actions; if not matching rhPtr->rh2ActionCount, inspects CurrentRoutine
	// to determine which object instances should be looped on.
	// Then the first instance is set to CurrentOi and the loop starts.
	int ActionLoopCount;

	// Current routine for the actions
	HeaderObject * (*CurrentRoutine)(objInfoList*, BOOL*);

	// Current object HeaderObject::Number, used during looping instances in applying actions to them
	int CurrentOi;

	// Next selected object HeaderObject::Number, used during looping instances in applying actions to them
	// @remarks Despite the 32-bit int type, this is actually a short
	int Next;

	// Used to invalidate selection between events; if matching rhPtr->rh4EventCount, the ListSelected-NextSelected chain is used,
	// otherwise implicitly all instances are still selected (i.e. the Object-NumNext chain).
	int EventCount;

	// Number of selected objects - invalid in OR events during second set of OR conditions (in MMF2 thru CF2.5 Fusion build 295.10)
	int NumOfSelected;

	// Object's OEFLAGS, the default copied during new instance create
	OEFLAGS OEFlags;

	short			LimitFlags,		// Movement limitation flags
					LimitList;		// Pointer to limitation list
	OILFlags		OIFlags;		// Objects preferences
	short			OCFlags2;		// Objects preferences II
	int				InkEffect,		// Ink effect
					EffectParam;	// Ink effect param
	short			HFII;			// First available frameitem
	COLORREF 		BackColor;		// Background erasing color
	short			Qualifiers[MAX_QUALIFIERS];		// Qualifiers for this object
	TCHAR			name[OINAME_SIZE];	 	// user-specified object name, cropped to 24 chars, guaranteed end with NULL
	int				EventCountOR;	// Selection in a list of events with OR
	#ifdef HWABETA
		short *		lColList;		// Liste de collisions sprites
	#endif

public:
	// If this matches rh2EventCount, the selection of ListSelected-NextSelected is applied, otherwise it is
	// ignored and all instances are implicitly selected.
	// @remarks This allows a fast way to invalidate the whole selection linked list when events switch.
	//			EventCount is incremented per event evaluation, but can be increased any amount,
	//			not specifically one.
	//			Decreasing may accidentally validate older events' eventcount selection.
	int get_EventCount();

	// The first object HeaderObject::Number in current selected object list, or -1.
	// @remarks -1 when no selection. Selection does not apply if EventCount differs to rh2EventCount.
	short get_ListSelected();

	// The number of selected objects of this type. Is 0 when no selection. Irrelevant if EventCount does not match rh2EventCount. 
	// @remarks The first selected object Number is ListSelected, then HeaderObject::NextSelected,
	//			and keep selecting NextSelected until HO::NS is negative.
	//			Selection does not apply if EventCount differs to rh2EventCount.
	//			This is a helper value, and won't prevent ListSelected-NextSelected chain going beyond this count.
	int get_NumOfSelected();

	// The unique number of this object; the OiList index. Same among instances.
	// @remarks Used to indicate difference between e.g. Active 1 and Active 2. Does not follow any pattern.
	//			OIs can be negative when indicating a qualifer ID (has 0x8000 flag), but this
	//			should not apply for OIL::Oi here, as OIL is one object only.
	short get_Oi();

	// The count of all object instances of this type, selected or not.
	// @remarks This is a helper value, and won't prevent Object-NumNext iteration going beyond it.
	int get_NObjects();

	// The first object index in RunHeader::ObjectList, regardless of selection.
	// @remarks Set to LAST created object so the newly created objects are created faster.
	//			Thus loops of objects start from the last created and go backwards to first created.
	short get_Object();

	// The object's name, as user wrote it; null-terminated, cropped to 23 characters max, 24 with null
	// @remarks The name is the top field in the About tab of object properties.
	//			This cropping is done on Fusion runtime side, and stored in built apps pre-cropped,
	//			so it is cropped on all platforms.
	const TCHAR * get_name();

	// Reads the qualifiers array of this object at passed zero-based index. Pass 0 to 7 only. Returns -1 if invalid.
	// @remarks Objects cannot have more than MAX_QUALIFIERS (8) qualifiers.
	//			The returned value can be used with GetQualToOiListByOffset().
	//			The array is contiguous, so qualifiers removed in editor are shuffled down to fill the gap.
	short get_QualifierByIndex(std::size_t);

	// When an ActionLoop is active, this is the next object number in the loop (-1, Object, NextSelected).
	int get_oilNext();

	// When an ActionLoop is active, this is whether to iterate further or not.
	bool get_oilNextFlag();

	// When an Action repeat or ActionLoop is active, this specifies which object to loop on.
	// @remarks 0 indicates no 2nd object (gao2ndNone)
	//			1 indicates to use one 2nd object (gao2ndOneOnly)
	//			2 indicates to use the ListSelected->NextSelected current selected chain (gao2ndCurrent)
	//			3 indicates to use the Object->NumNext chain (gao2ndAll)
	decltype(CurrentRoutine) get_oilCurrentRoutine();

	// When an Action is active, this specifies which object is currently being iterated. -1 if invalid.
	int get_oilCurrentOi();

	// When an Action is active, this applies oilCurrentRountine.
	int get_oilActionCount();

	// When an ActionLoop is active (Action repeating in a fastloop), this applies oilCurrentRountine.
	int get_oilActionLoopCount();

	// Sets the number of selected, presumably after object selection linked list changes.
	// @remarks This is a helper value, and won't prevent ListSelected-NextSelected loops going beyond it.
	void set_NumOfSelected(int);

	// Sets the first selected instance to a HeaderObject::Number, or -1 for no selection.
	// @remarks The Number is a ObjectList index, usable with rhPtr->GetObjectListOblOffsetByIndex().
	void set_ListSelected(short);

	// Sets the value of a event count, which sets the validation of ListSelected-NextSelected chain.
	// @remarks If this matches rh2EventCount, the chain is used, otherwise it is ignored and
	//			implicitly all object instances are selected.
	//			To have none selected explicitly instead, you set EventCount to match rh2EventCount,
	//			set ListSelected to -1, and NumSelected to 0.
	void set_EventCount(int);

	// Sets all the variables for making selection explicitly none for this object.
	// This will mean actions for that object will not run for any of them! To invalidate selection
	void SelectNone(RunHeader * rhPtr);

	// Sets all the variables for making selection explicitly all objects instances for this object.
	// @explicitAll: If true, slowly and explicitly sets every instance selected in the selection chain.
	//				 Otherwise, uses a faster implicit-all selection by invalidating the chain.
	// @remarks Ignores objects in Object-NumNext chain marked as destroyed.
	void SelectAll(RunHeader * rhPtr, bool explicitAll = false);
private:
	int get_EventCountOR();
	void set_EventCountOR(int);
};
//typedef	objInfoList	*	objInfoList *;

#define	OILIMITFLAGS_BORDERS		0x000F
#define	OILIMITFLAGS_BACKDROPS		0x10
#define	OILIMITFLAGS_ONCOLLIDE		0x80	// used by HWA
#define	OILIMITFLAGS_QUICKCOL		0x100
#define	OILIMITFLAGS_QUICKBACK		0x200
#define	OILIMITFLAGS_QUICKBORDER	0x400
#define	OILIMITFLAGS_QUICKSPR		0x800
#define	OILIMITFLAGS_QUICKEXT		0x1000
#define	OILIMITFLAGS_ALL			0xFFFF

// Object creation structure
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
struct CreateObjectInfo {
	NO_DEFAULT_CTORS_OR_DTORS(CreateObjectInfo);
DarkEdifInternalAccessProtected:
	LevelObject *	cobLevObj;		// Leave first!
	unsigned short	cobLevObjSeg,
					cobFlags;
	int				cobX,
					cobY,
					cobDir,
					cobLayer,
					cobZOrder;
};
//typedef	CreateObjectInfo *	LPCOB;
//typedef	CreateObjectInfo *	fpcob;

// Flags for Create Objects
// -------------------------
#define	COF_NOMOVEMENT		0x1
#define	COF_HIDDEN			0x2
#define	COF_FIRSTTEXT		0x4

// Qualifier to oilist for machine langage
// ---------------------------------------

struct qualToOi {
	NO_DEFAULT_CTORS_OR_DTORS(qualToOi);
DarkEdifInternalAccessProtected:
	short		CurrentOi,
				Next;
	int			ActionPos;
	HeaderObject * (*CurrentRountine)(qualToOi*, BOOL* pRepeatFlag);
	int			ActionCount,
				ActionLoopCount;
	char		NextFlag,
				SelectedFlag;

	short		OiAndOiList[];

public:
	short get_Oi(std::size_t idx);
	short get_OiList(std::size_t idx);
	// Returns all OiList from internal array, used for looping through a qualifier's object IDs
	std::vector<short> GetAllOi();
	// Returns all OiList from internal array, used for looping through a qualifier's objInfoList
	std::vector<short> GetAllOiList();
private:
	std::vector<short> HalfVector(std::size_t first);
};

// ------------------------------------------------------------------
// ------------------------------------------------------------------
// EXTENSION HANDLING
// ------------------------------------------------------------------
// ------------------------------------------------------------------

// Information structure about the extension
struct ForbiddenInternals2;
struct kpxRunInfos {
	NO_DEFAULT_CTORS_OR_DTORS(kpxRunInfos);
DarkEdifInternalAccessProtected:
	friend ForbiddenInternals2;
	void *			Conditions;			// 00 Ignore - requires STAND_ALONE - Offset to condition jump list
	void *			Actions;			// 04 Ignore - requires STAND_ALONE - Offset to action jump list
	void *			Expressions;		// 08 Ignore - requires STAND_ALONE - Offset to expression jump list
	short			NumOfConditions;	// 0C Number of conditions
	short			NumOfActions;		// 0E Number of actions
	short			NumOfExpressions;	// 10 Number of expressions
	unsigned short	EDITDATASize;		// 12 Size of the data zone when exploded
	OEFLAGS			EditFlags;			// 14 Object flags
	char			WindowProcPriority;	// 16 Priority of the routine 0-255
	char			Free;
	OEPREFS			EditPrefs;			// 18 Editing Preferences
	DWORD			Identifier;			// 1A Identification string
	short			Version;			// 1E current version
										// 20
};

// Extension jump table
// --------------------

struct kpj {
	NO_DEFAULT_CTORS_OR_DTORS(kpj);
DarkEdifInternalAccessProtected:
	short				(FusionAPI * CreateRunObject)			(HeaderObject *, extHeader *, CreateObjectInfo *);
	short				(FusionAPI * DestroyRunObject)			(HeaderObject *, long);
	short				(FusionAPI * HandleRunObject)			(HeaderObject *);
	short				(FusionAPI * DisplayRunObject)			(HeaderObject *);
	cSurface *			(FusionAPI * GetRunObjectSurface)		(HeaderObject *);
	short				(FusionAPI * ReInitRunObject)			(HeaderObject *);
	short				(FusionAPI * PauseRunObject)			(HeaderObject *);
	short				(FusionAPI * ContinueRunObject)			(HeaderObject *);
	short				(FusionAPI * PrepareToSave)				(HeaderObject *);
	short				(FusionAPI * PrepareToSave2)			(HeaderObject *);
	short				(FusionAPI * SaveBackground)			(HeaderObject *);
	short				(FusionAPI * RestoreBackground)			(HeaderObject *);
	short				(FusionAPI * ChangeRunData)				(HeaderObject *, HeaderObject *);
	short				(FusionAPI * KillBackground)			(HeaderObject *);
	short				(FusionAPI * GetZoneInfo)				(HeaderObject *);
	unsigned short *	(FusionAPI * GetDebugTree)				(HeaderObject *);
	void 				(FusionAPI * GetDebugItem)				(TCHAR *, HeaderObject *, int);
	void 				(FusionAPI * EditDebugItem)				(HeaderObject *, int);
	void 				(FusionAPI * GetRunObjectFont)			(HeaderObject *, LOGFONT *);
	void 				(FusionAPI * SetRunObjectFont)			(HeaderObject *, LOGFONT *, RECT *);
	COLORREF 			(FusionAPI * GetRunObjectTextColor)		(HeaderObject *);
	void				(FusionAPI * SetRunObjectTextColor)		(HeaderObject *, COLORREF);
	short				(FusionAPI * GetRunObjectWindow)		(HeaderObject *);
	CollisionMask *		(FusionAPI * GetRunObjectCollisionMask)	(HeaderObject *, LPARAM);
	BOOL				(FusionAPI * SaveRunObject)				(HeaderObject *, HANDLE);
	BOOL				(FusionAPI * LoadRunObject)				(HeaderObject *, HANDLE);
	void				(FusionAPI * GetRunObjectMemoryUsage)	(HeaderObject *, int *, int *, int *);

	kpxRunInfos	infos;
};

#define	KPJ_SHIFT				7
#define	KPJ_SIZE				0x80
#define KPJ_MAXNUMBER			128
#define	KPJ_STEP				8
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

// Condition/action jumps
struct CallTables
{
	NO_DEFAULT_CTORS_OR_DTORS(CallTables);
DarkEdifInternalAccessProtected:
	BOOL (** pConditions1)(event2* pe, HeaderObject * pHo);
	BOOL (** pConditions2)(event2* pe);
	void (** pActions)(event2* pe);
	void (** pExpressions)();
};

#define	COMPARE_EQ				0
#define COMPARE_NE				1
#define	COMPARE_LE				2
#define	COMPARE_LT				3
#define	COMPARE_GE				4
#define	COMPARE_GT				5

#define OINUM_ERROR				(-1)

// Macro, converts 4 unsigned chars to one unsigned int
#define B2L(a,b,c,d)	((unsigned int)(((unsigned int)((unsigned char)(d))<<24)|((unsigned int)((unsigned char)(c))<<16)|((unsigned int)((unsigned char)(b))<<8)|(unsigned int)((unsigned char)(a))))

// For GetFileInfos
enum class FILEINFO {
	DRIVE = 1,
	DIR,
	PATH,
	APP_NAME,
	TEMP_PATH
};

#define		KPXNAME_SIZE	60
#define		BADNAME_SIZE	(OINAME_SIZE+KPXNAME_SIZE+8)

// Extensions
// ----------


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

struct drawRoutine {
	NO_DEFAULT_CTORS_OR_DTORS(drawRoutine);
DarkEdifInternalAccessProtected:
	drawRoutine* next;
	DRAWROUTINE_PROC routine;
	long	param1;
	long	param2;
	long	param3;
	long	param4;
	long	param5;
	long	param6;
};

// To push an event
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define		MAX_PUSHEDEVENTS	32

struct pev {
	NO_DEFAULT_CTORS_OR_DTORS(pev);
DarkEdifInternalAccessProtected:
	long	pevCode;
	void (* pevRoutine)(HeaderObject *, long);
	long	pevParam;
	HeaderObject *	pevObject;
	short	pevOi;
};
#define	PIXEL_SPEED					5

// QUALIFIERS
struct qualifierLoad
{
	NO_DEFAULT_CTORS_OR_DTORS(qualifierLoad);
DarkEdifInternalAccessProtected:
	OINUM		qOi;
	ITEMTYPE	qType;
	unsigned short		qList;
};
//typedef qualifierLoad *	LPQLOAD;

// start of alignment
// last realignment

#ifndef	  _H2INC

// Object instance parent types
enum {
	PARENT_NONE,
	PARENT_FRAME,
	PARENT_FRAMEITEM,
	PARENT_QUALIFIER
};

// Proc definition
typedef void (FusionAPI * UPDATEFILENAMEPROCA)(const char *, char *);
typedef void (FusionAPI * UPDATEFILENAMEPROCW)(const wchar_t *, wchar_t *);
typedef void (FusionAPI * UPDATEFILENAMEPROC)(const TCHAR *, TCHAR *);

// Obsolete
enum class MODIF {
	SIZE,
	PLANE,
	FLAGS,
	PICTURE,
	HFRAN,
	BOX,
	TEXT,
	PREFS
};
#define	KPX_MODIF_BASE	1024

#endif	// _H2INC

// Standard objects
#define	KPX_BASE	32		// Extensions

// Menu header (v2)
struct MenuHdr {
	NO_DEFAULT_CTORS_OR_DTORS(MenuHdr);
DarkEdifInternalAccessProtected:
	unsigned int	HdrSize,		// == sizeof(MenuHdr)
					MenuOffset,		// From start of MenuHdr
					MenuSize,
					AccelOffset,	// From start of MenuHdr
					AccelSize;		// Total data size = mhHdrSize + mhMenuSize + mhAccelSize
};

//////////////////////////////////////////////////////////////////////////////
// Application mini Header
//
struct AppMiniHeader {
	NO_DEFAULT_CTORS_OR_DTORS(AppMiniHeader);
DarkEdifInternalAccessProtected:

	union {
		char			Type[4];	// "PAME"
		unsigned long	FullType;
	};
	unsigned short		Version;	// Version number
	unsigned short		SubVersion;	// Sub-version number
	unsigned int		PrdVersion;	// MMF version
	unsigned int		PrdBuild;	// MMF build number

};

#ifdef _UNICODE
	#define	RUNTIME_DWTYPE				'UMAP'
	#define RUNTIME_DWTYPE_VTZ			'UZTV'
#else
	#define	RUNTIME_DWTYPE				'EMAP'
	#define RUNTIME_DWTYPE_VTZ			' ZTV'
#endif
#define	RUNTIME_VERSION_MMF1		0x0300
#define	RUNTIME_VERSION_MMF15		0x0301
#define	RUNTIME_VERSION_MMF2		0x0302
#define	RUNTIME_VERSION				RUNTIME_VERSION_MMF2

//////////////////////////////////////////////////////////////////////////////
// Application full Header
//

enum {
	CTRLTYPE_MOUSE,		// not used
	CTRLTYPE_JOY1,
	CTRLTYPE_JOY2,
	CTRLTYPE_JOY3,
	CTRLTYPE_JOY4,
	CTRLTYPE_KEYBOARD,
	MAX_CTRLTYPE
};

#define PLAYERNAME_SIZE		100
struct PlayerCtrls {
	NO_DEFAULT_CTORS_OR_DTORS(PlayerCtrls);
DarkEdifInternalAccessProtected:
	unsigned short	PlayerCtrls_Type,		// Control type per player (0 = keyboard, 1-4 = joy1-4)
					PlayerCtrls_Keys[8];	// Control keys per player
};
//typedef PlayerCtrls* fpPlayerCtrls;

struct AppHeader {
	NO_DEFAULT_CTORS_OR_DTORS(AppHeader);
DarkEdifInternalAccessProtected:
	friend CRunAppMultiPlat;
	friend Edif::Runtime;
	unsigned int	size;					// Structure size
	unsigned short	Flags,					// Flags (GA_XXX defines)
					NewFlags,				// New flags (GANF_XXX defines)
					Mode,					// Graphic mode
					OtherFlags;				// Other Flags (GAOF_XXX defines?)
	short			XWinSize,				// Window x-size
					YWinSize;				// Window y-size
	unsigned int	ScoreInit,				// Initial score
					LivesInit;				// Initial number of lives
	PlayerCtrls		PlayerCtrls[4];			// Player controls
	unsigned int	BorderColour,			// Border colour
					NbFrames,				// Number of frames (taking into account pauses?)
					FrameRate;				// Application FPS
	unsigned char	MDIWindowMenu,			// Index of Window menu for MDI applications
					Free[3];				// Padding to a multiple of 4 bytes
};

// gaFlags
#define	GA_BORDERMAX				0x1
#define	GA_NOHEADING				0x2
#define	GA_PANIC					0x4
#define	GA_SPEEDINDEPENDANT			0x8
#define	GA_STRETCH					0x10
#define	GA_LOADALLIMAGESATSTART		0x20
#define	GA_LOADALLSOUNDSATSTART		0x40
#define	GA_MENUHIDDEN				0x80
#define	GA_MENUBAR					0x100
#define	GA_MAXIMISE					0x200
#define	GA_MIX						0x400
#define	GA_FULLSCREENATSTART		0x800
#define	GA_FULLSCREENSWITCH			0x1000
#define	GA_PROTECTED				0x2000
#define	GA_COPYRIGHT				0x4000
#define	GA_ONEFILE					0x8000
#define	GANF_SAMPLESOVERFRAMES		0x1
#define	GANF_RELOCFILES				0x2
#define	GANF_RUNFRAME				0x4
#define	GANF_SAMPLESEVENIFNOTFOCUS	0x8
#define	GANF_NOMINIMIZEBOX			0x10
#define	GANF_NOMAXIMIZEBOX			0x20
#define	GANF_NOTHICKFRAME			0x40
#define	GANF_DONOTCENTERFRAME		0x80
#define	GANF_SCREENSAVER_NOAUTOSTOP	0x100
#define	GANF_DISABLE_CLOSE			0x200
#define	GANF_HIDDENATSTART			0x400
#define	GANF_XPVISUALTHEMESUPPORT	0x800
#define	GANF_VSYNC					0x1000
#define	GANF_RUNWHENMINIMIZED		0x2000
#define	GANF_MDI					0x4000
#define	GANF_RUNWHILERESIZING		0x8000
#define	GAOF_DEBUGGERSHORTCUTS		0x1
#define	GAOF_DDRAW					0x2
#define	GAOF_DDRAWVRAM				0x4
#define	GAOF_OBSOLETE				0x8
#define	GAOF_AUTOIMGFLT				0x10
#define	GAOF_AUTOSNDFLT				0x20
#define	GAOF_ALLINONE				0x40
#define	GAOF_SHOWDEBUGGER			0x80
#define	GAOF_RESERVED_1				0x100
#define	GAOF_RESERVED_2				0x200
#define	GAOF_RESERVED_3				0x400
#define	GAOF_RESERVED_4				0x800
#define	GAOF_JAVASWING				0x1000
#define	GAOF_JAVAAPPLET				0x2000
#define	GAOF_D3D9					0x4000
#define	GAOF_D3D8					0x8000

// Optional header
struct AppHeader2 {
	NO_DEFAULT_CTORS_OR_DTORS(AppHeader2);
DarkEdifInternalAccessProtected:
	unsigned int	Options,
					BuildType,
					BuildFlags;
	unsigned short	ScreenRatioTolerance,
					ScreenAngle;			// 0 (no rotation/portrait), 1 (90 clockwise/landscape left), 2 (90 anticlockwise/landscape right), 3 (automatic portrait), 4 (automatic landscape), 5 (fully automatic)
	unsigned int	Unused2;
};

enum class AH2OPT {
	KEEPSCREENRATIO = 0x1,
	FRAMETRANSITION = 0x2,		// (HWA only) a frame has a transition
	RESAMPLESTRETCH = 0x4,		// (HWA only) "resample when resizing" (works with "resize to fill window" option)
	GLOBALREFRESH = 0x8,		// (Mobile) force global refresh
	MULTITASK = 0x10,			// (iPhone) Multitask
	RTL = 0x20,					// (Unicode) Right-to-left reading
	STATUSLINE = 0x40,			// (iPhone/Android) Display status line
	RTLLAYOUT = 0x80,			// (Unicode) Right-to-left layout
	ENABLEIAD = 0x100,			// (iPhone) Enable iAd
	IADBOTTOM = 0x200,			// (iPhone) Display ad at bottom
	AUTOEND = 0x400,			// (Android)
	DISABLEBACKBUTTON = 0x800,	// (Android) Disable Back button behavior
	ANTIALIASED = 0x1000,		// (iPhone) Smooth resizing on bigger screens
	CRASHREPORTING = 0x2000,		// (Android) Enable online crash reporting
};

enum class SCREENORIENTATION {
	PORTRAIT,
	LANDSCAPE_LEFT,
	LANDSCAPE_RIGHT,
	AUTO,
	LANDSCAPE_AUTO,
	PORTRAIT_AUTO,
};

#ifndef	  _H2INC

// Build type values
enum class BUILDTYPE {
	STANDALONE,
	SCREENSAVER,
	INTERNETAPP,
	JAVA,
	JAVASTANDALONE,
	JAVAAPPLET,
	JAVAWEBSTART,
	JAVAMOBILE,
	JAVABLURAY,
	JAVAMAC,
	FLASH,
	JAVABLACKBERRY,
	ANDROID,
	IPHONE,
	IPHONEDEVEL,
	IPHONEFINAL,
	XNA_WINDOWS_APP,
	MAC,
	XNA_WINDOWS,
	XNA_XBOX,
	XNA_PHONE,
	XNA_XBOX_APP,
	XNA_PHONE_APP,
	STDMAX,			// end of standard build types
};

// Build flag values
#define	BUILDFLAG_MAXCOMP			0x1
#define BUILDFLAG_COMPSND			0x2
#define BUILDFLAG_INCLUDEEXTFILES	0x4
#define BUILDFLAG_MANUALIMGFILTERS	0x8
#define BUILDFLAG_MANUALSNDFILTERS	0x10
#define	BUILDFLAG_NOAUTOEXTRACT		0x20
#define	BUILDFLAG_NOAPPLETCHECK		0x40
#define	BUILDFLAG_TEST				0x80
#define	BUILDFLAG_NOWARNINGS		0x100

#endif // _H2INC

//////////////////////////////////////////////////////////////////////////////
// Element of chunk Extensions
//
#ifndef	  _H2INC
struct ExtDesc {
	NO_DEFAULT_CTORS_OR_DTORS(ExtDesc);
DarkEdifInternalAccessProtected:
	unsigned short	extSize,
					extIndex;
	unsigned int	extMagicNumber;
};
// typedef ExtDesc *extHeader *DESC;

struct ExtDesc2 {
	NO_DEFAULT_CTORS_OR_DTORS(ExtDesc2);
DarkEdifInternalAccessProtected:
	unsigned short	extSize,
					extIndex;
	unsigned int	extMagicNumber,
					extVersionLS,
					extVersionMS;
};
//typedef ExtDesc2 *extHeader *DESC2;
#endif // _H2INC

//////////////////////////////////////////////////////////////////////////////
// Movement Extensions Chunk
//
struct MvtExtDesc {
	NO_DEFAULT_CTORS_OR_DTORS(MvtExtDesc);
DarkEdifInternalAccessProtected:
	unsigned short	extTotalSize,
					extHdrSize;
	unsigned int	extBuild;
};
//typedef MvtExtDesc *LPMVTEXTDESC;

//////////////////////////////////////////////////////////////////////////////
// Frame Header
//
struct FrameHeader {
	NO_DEFAULT_CTORS_OR_DTORS(FrameHeader);
DarkEdifInternalAccessProtected:
	// Frame width/height
	int				Width,			// Frame width in pixels
					Height;			// Frame height in pixels
	COLORREF		Background;

	// Options
	unsigned int	Flags;
};

// leFlags
#define	LDISPLAYNAME		0x1
#define	LGRABDESKTOP		0x2
#define	LKEEPDISPLAY		0x4
// #define LFADEIN			0x8
// #define LFADEOUT			0x10
#define	LTOTALCOLMASK		0x20
#define	LPASSWORD			0x40
#define	LRESIZEATSTART		0x100
#define	LDONOTCENTER		0x200
#define	LFORCE_LOADONCALL	0x400
#define	LNOSURFACE			0x800
#define	LRESERVED_1			0x1000
#define	LRESERVED_2			0x2000
#define	LRECORDDEMO			0x4000
#define	LTIMEDMVTS			0x8000

//////////////////////////////////////////////////////////////////////////////
// Layers
//

#define FLOPT_XCOEF				0x1
#define FLOPT_YCOEF				0x2
#define FLOPT_NOSAVEBKD			0x4
#define FLOPT_WRAP_OBSOLETE		0x8
#define FLOPT_VISIBLE			0x10
#define FLOPT_WRAP_HORZ			0x20
#define FLOPT_WRAP_VERT			0x40
#define FLOPT_PREVIOUSEFFECT	0x80
#define FLOPT_REDRAW			0x010000
#define FLOPT_TOHIDE			0x020000
#define FLOPT_TOSHOW			0x040000

struct EditFrameLayer
{
	NO_DEFAULT_CTORS_OR_DTORS(EditFrameLayer);
DarkEdifInternalAccessProtected:
	unsigned int	Options;		// Options
	float			xCoef, yCoef;	// X/Y Coefficents
	unsigned int	nBkdLOs,
					nFirstLOIndex;
};

struct EditFrameLayerEffect {
	NO_DEFAULT_CTORS_OR_DTORS(EditFrameLayerEffect);
DarkEdifInternalAccessProtected:
	unsigned int	InkFx,
					RGBA,
					ExtInkFxIdx,
					NumOfParams;
	LPARAM			paramData;		// offset
};

// Effects
#ifdef HWABETA

struct EffectHdr {
	NO_DEFAULT_CTORS_OR_DTORS(EffectHdr);
DarkEdifInternalAccessProtected:
	unsigned int	EffectNameOffset,
					EffectDataOffset,
					EffectParamsOffset,
					Options;
};

#define EFFECTOPT_BKDTEXTUREMASK	0x000F

struct EffectParamsHdr {
	NO_DEFAULT_CTORS_OR_DTORS(EffectParamsHdr);
DarkEdifInternalAccessProtected:
	unsigned int	NumOfParams,
					ParamTypesOffset,
					ParamNamesOffset;
};

struct EffectRunData {
	NO_DEFAULT_CTORS_OR_DTORS(EffectRunData);
DarkEdifInternalAccessProtected:
	unsigned int	EffectIndex,
					NumOfParams;
};

struct FrameEffect {
	NO_DEFAULT_CTORS_OR_DTORS(FrameEffect);
DarkEdifInternalAccessProtected:
	unsigned int	InkEffect,
					InkEffectParam;
};

#endif // HWABETA

//////////////////////////////////////////////////////////////////////////////
// ObjInfo/FrameItem Header
//
struct ObjInfoHeader
{
	NO_DEFAULT_CTORS_OR_DTORS(ObjInfoHeader);
DarkEdifInternalAccessProtected:
	unsigned short	Handle,
					Type,
					Flags,			// Memory flags
					Reserved;		// Not used
	unsigned int	InkEffect,		// Ink effect
					InkEffectParam;	// Ink effect param
};

// oiFlags (all OIF_* enum)
enum class OIFlags : short
{
	LOAD_ON_CALL = 0x1,
	DISCARDABLE = 0x2,
	GLOBAL = 0x4,
	RESERVED_1 = 0x8,
	GLOBAL_EDITOR_NO_SYNC = 0x10,
	GLOBAL_EDITOR_FORCE_SYNC = 0x20,
};


//////////////////////////////////////////////////////////////////////////////
// LevObj/FrameItemInstance
//

#ifndef	  _H2INC
struct diskLO {
	NO_DEFAULT_CTORS_OR_DTORS(diskLO);
DarkEdifInternalAccessProtected:
	unsigned short	LO_Handle;			// HLO
	unsigned short	OI_Handle;			// HOI
	int				X, Y;				// Coords
	unsigned short	ParentType;			// Parent type
	unsigned short	OI_ParentHandle;	// HOI Parent
	unsigned short	Layer;				// Layer
	unsigned short	Free;				// Not used
};
// typedef diskLO * fpDiskLO;
#endif  // _H2INC

//////////////////////////////////////////////////////////////////////////////
//
// ObjInfo/FrameItem ObjectsCommon
//

// Obstacle types
enum class OBSTACLE {
	NONE,
	SOLID,
	PLATFORM,
	LADDER,
	TRANSPARENT_,		// for Add Backdrop
};

////////////////////////////////
// Static object - ObjectsCommon
//
#ifndef	  _H2INC
struct Static_OC {
	NO_DEFAULT_CTORS_OR_DTORS(Static_OC);
DarkEdifInternalAccessProtected:
	// Size
	unsigned int	size;				// OC size?

	// Obstacle type & collision mode
	unsigned short	ObstacleType;		// Obstacle type
	unsigned short	ColMode;			// Collision mode (0 = fine, 1 = box)

	int				X, Y;				// Size
};
//typedef Static_OC * LPStatic_OC;
#endif  // _H2INC

///////////////////////////////////////////////////////////////
// Fill Type & shapes - Definitions
//

#ifndef	  _H2INC
// Gradient
typedef struct GradientData {
	NO_DEFAULT_CTORS_OR_DTORS(GradientData);
DarkEdifInternalAccessProtected:
	COLORREF		color1,
					color2;
	unsigned int	GradientFlags; // prev. vertical
} GradientData;

// Shapes
enum class SHAPE {
	NONE,			// error'd
	LINE,
	RECTANGLE,
	ELLIPSE
};

// Fill types
enum class FILLTYPE {
	NONE,
	SOLID,
	GRADIENT,
	MOTIF
};

// Line flags
#define	LININVX	0x1
#define	LININVY	0x2

#endif // _H2INC

///////////////////////////////////////////////////////////////
// Fill Type - Part of FilledShape
//
#ifndef	  _H2INC

class FillType_Data {
	NO_DEFAULT_CTORS_OR_DTORS(FillType_Data);
public:
	unsigned short			FillType;

	// Fill type
	union {
		// Lines in non-filled mode
		struct {
			unsigned int LineFlags;		// Only for lines in non filled mode
		};

		// Solid colour
		struct {
			COLORREF Color;			// Solid
		};

		// Gradient
		struct {
			COLORREF	 Color1,
						 Color2;
			unsigned int GradientFlags;
		};

		// Image
		struct {
			unsigned short Image;			// Image
		};
	};
};

#endif // _H2INC

///////////////////////////////////////////////////////////////
// Filled Shape - Part of QuickBackdrop / Counter ObjectsCommon
//
#ifndef	  _H2INC

class FilledShape_Data {
	NO_DEFAULT_CTORS_OR_DTORS(FilledShape_Data);
	unsigned short	BorderSize;			// Border
	COLORREF		BorderColor;
	unsigned short	Shape;				// Shape
	FillType_Data	FillData;			// Fill Type infos
};
// typedef FilledShape_Data * LPFilledShape_Data;

#endif // _H2INC

/////////////////////////////////
// Quick backdrop - ObjectsCommon
//
#ifndef	  _H2INC

struct QuickBackdrop_OC {
	NO_DEFAULT_CTORS_OR_DTORS(QuickBackdrop_OC);
DarkEdifInternalAccessProtected:
	unsigned int		size;

	unsigned short		ObstacleType;	// Obstacle type (0
	unsigned short		ColMode;		// Collision mode (0 = fine, 1 = box)

	int					X, Y;			// Size

	FilledShape_Data	FilledShape;	// Filled shape infos
};
typedef QuickBackdrop_OC * LPQuickBackdrop_OC;

#endif // _H2INC

/////////////////////////////////
// Backdrop - ObjectsCommon
//
#ifndef	  _H2INC

struct Backdrop_OC {
	NO_DEFAULT_CTORS_OR_DTORS(Backdrop_OC);
	DarkEdifInternalAccessProtected:

	unsigned int	size;

	unsigned short	ObstacleType;	// Obstacle type (0
	unsigned short	ColMode;		// Collision mode (0 = fine, 1 = box)

	int				X, Y;			// Size

	unsigned short	Image;			// Image
};
typedef Backdrop_OC * LPBackdrop_OC;

#endif // _H2INC

//////////////////////////////////////////////////////////////////////////////
//
// ObjInfo/FrameItem ObjectsCommon Data
//

//////////////////////////////////////////////////////////////////////////////
// ImageSet
//

class ImageSet_Data {
	NO_DEFAULT_CTORS_OR_DTORS(ImageSet_Data);
DarkEdifInternalAccessProtected:
	unsigned short		nbFrames;		// Number of frames
										// Followed by list of image handles (unsigned short[])
};
typedef ImageSet_Data * LPImageSet_Data;

////////////////////////////////////////
// text - ocData
//
struct otText {
	NO_DEFAULT_CTORS_OR_DTORS(otText);
DarkEdifInternalAccessProtected:
	unsigned int	otDWSize;
	int				otCx;
	int				otCy;
	unsigned int	otNumberOfText;			// Paragraph number (> 1 if question object)
	unsigned int	otStringOffset[];		// String offsets
};
typedef otText	*	fpot;
//#define sizeof_ot	(sizeof(otText)-sizeof(unsigned int))

typedef	struct txString {
	NO_DEFAULT_CTORS_OR_DTORS(txString);
DarkEdifInternalAccessProtected:
	unsigned short		tsFont;				// Font
	unsigned short		tsFlags;			// Flags
	COLORREF			tsColor;			// Color
	TCHAR				tsChar[];
} txString;
typedef	txString	*	fpts;
//#define	sizeof_ts	8					// (sizeof(txString)-1)

#define	TSF_LEFT		0x0000			// DT_LEFT
#define	TSF_HCENTER		0x1			// DT_CENTER
#define	TSF_RIGHT		0x2			// DT_RIGHT
#define	TSF_VCENTER		0x4			// DT_VCENTER
#define	TSF_HALIGN		0x000F			// DT_LEFT | DT_RIGHT | DT_CENTER | DT_VCENTER | DT_BOTTOM

#define	TSF_CORRECT		0x100
#define	TSF_RELIEF		0x200
#define TSF_RTL			0x400

////////////////////////////////////////
// Scores, lives, counters
//
struct CtAnim_Data {
	NO_DEFAULT_CTORS_OR_DTORS(CtAnim_Data);
DarkEdifInternalAccessProtected:

	unsigned int		odDWSize;
	int					odCx;					// Size: only lives & counters
	int					odCy;
	unsigned short		odPlayer;				// Player: only score & lives
	unsigned short		odDisplayType;			// CTA_xxx
	unsigned short		odDisplayFlags;			// BARFLAG_INVERSE
	unsigned short		odFont;					// Font

	// Followed by display data:
//	union {
//		ImageSet_Data		odImages;
//		FilledShape_Data	odShape;
//	};

};
//typedef CtAnim_Data * LPCtAnim_Data;

// Display types
enum class CTA {
	HIDDEN,
	DIGITS,
	VBAR,
	HBAR,
	ANIM,
	TEXT,
};

// Display flags
#define	CPTDISPFLAG_INTNDIGITS				0x000F		// 0 = normal display, other value = pad with zeros or truncate
#define	CPTDISPFLAG_FLOATNDIGITS			0x00F0		// add 1 to get the number of significant digits to display
#define	CPTDISPFLAG_FLOATNDIGITS_SHIFT		4
#define	CPTDISPFLAG_FLOATNDECIMALS			0xF000		// number of digits to display after the decimal point
#define	CPTDISPFLAG_FLOATNDECIMALS_SHIFT	12
#define	BARFLAG_INVERSE						0x100
#define	CPTDISPFLAG_FLOAT_FORMAT			0x200		// 1 to use the specified numbers of digits, 0 to use standard display (%g)
#define	CPTDISPFLAG_FLOAT_USENDECIMALS		0x400		// 1 to use the specified numbers of digits after the decimal point
#define	CPTDISPFLAG_FLOAT_PADD				0x800		// 1 to left padd with zeros

// Counters images (0-9 for regular numbers)
enum class COUNTER_IMAGE {
	SIGN_NEG = 10,
	SIGN_PLUS,
	POINT,
	EXP,
	MAX
};
#define	V1_COUNTER_IMAGE_MAX	(COUNTER_IMAGE_SIGN_NEG+1)

////////////////////////////////////////
//
// Objet RTF - ocData
//
#ifndef	  _H2INC

struct ocRTF {
	NO_DEFAULT_CTORS_OR_DTORS(ocRTF);
DarkEdifInternalAccessProtected:
	unsigned int	size;
	unsigned int	Version;	// 0
	unsigned int	Options;	// Options
	COLORREF		BackColor;	// Background color
	LONG			XSize;		// Size
	LONG			YSize;
};
//typedef ocRTF * LPOCRTF;

//#define	RTFOPT_TRANSPARENT	0x1		// Transparent
//#define	RTFOPT_VSLIDER		0x2		// Display vertical slider if necessary
//#define	RTFOPT_HSLIDER		0x4		// Display horizontal slider if necessary

#endif // _H2INC

////////////////////////////////////////
//
// Objet CCA - ocData
//
#ifndef	  _H2INC

struct ocCCA {
	NO_DEFAULT_CTORS_OR_DTORS(ocCCA);
DarkEdifInternalAccessProtected:
	unsigned int	size;
	int				XSize,			// Size (ignored)
					YSize;
	unsigned short	Version,		// 0
					NumStartFrame;
	unsigned int	Options,		// Options
					IconOffset,		// Icon offset
					Free;			// Ignore - reserved
};
//typedef ocCCA * LPOCCCA;

// Options
#define	CCAF_SHARE_GLOBALVALUES		0x1
#define	CCAF_SHARE_LIVES			0x2
#define	CCAF_SHARE_SCORES			0x4
#define	CCAF_SHARE_WINATTRIB		0x8
#define	CCAF_STRETCH				0x10
#define	CCAF_POPUP					0x20
#define CCAF_CAPTION				0x40
#define CCAF_TOOLCAPTION			0x80
#define CCAF_BORDER					0x100
#define CCAF_WINRESIZE				0x200
#define CCAF_SYSMENU				0x400
#define CCAF_DISABLECLOSE			0x800
#define CCAF_MODAL					0x1000
#define CCAF_DIALOGFRAME			0x2000
#define	CCAF_INTERNAL				0x4000
#define	CCAF_HIDEONCLOSE			0x8000
#define	CCAF_CUSTOMSIZE				0x00010000
#define	CCAF_INTERNALABOUTBOX		0x00020000
#define	CCAF_CLIPSIBLINGS			0x00040000
#define	CCAF_SHARE_PLAYERCTRLS		0x00080000
#define CCAF_MDICHILD				0x00100000
#define CCAF_DOCKED					0x00200000
#define	CCAF_DOCKING_AREA			0x00C00000
#define	CCAF_DOCKED_LEFT			0x00000000
#define	CCAF_DOCKED_TOP				0x00400000
#define	CCAF_DOCKED_RIGHT			0x00800000
#define	CCAF_DOCKED_BOTTOM			0x00C00000
#define	CCAF_REOPEN					0x01000000
#define CCAF_MDIRUNEVENIFNOTACTIVE	0x02000000
#define CCAF_DISPLAYASSPRITE		0x04000000

#endif // _H2INC

////////////////////////////////////////
//
// Transition
//

#ifndef	  _H2INC

// Transition header
struct TransitionHdr {
	NO_DEFAULT_CTORS_OR_DTORS(TransitionHdr);
DarkEdifInternalAccessProtected:
	unsigned int		trDllID;				// DLL id
	unsigned int		trID;					// Transition ID
	unsigned int		trDuration;				// Duration
	unsigned int		trFlags;				// From / to color / background
	unsigned int		trColor;				// Color
};

// Transition run-time data
class Transition_Data {
	NO_DEFAULT_CTORS_OR_DTORS(Transition_Data);
DarkEdifInternalAccessProtected:
	TransitionHdr	trHdr;
	unsigned int	trDllNameOffset;
	unsigned int	trParamsOffset;
	unsigned int	trParamsSize;
									// Followed by DLL name & transition parameters
};
typedef Transition_Data * LPTRANSITIONDATA;

#define	TRFLAG_COLOR	0x1
#define TRFLAG_UNICODE	0x2

#endif // _H2INC

// text alignment flags
#define	TEXT_ALIGN_LEFT		0x1
#define	TEXT_ALIGN_HCENTER	0x2
#define	TEXT_ALIGN_RIGHT	0x4
#define	TEXT_ALIGN_TOP		0x8
#define	TEXT_ALIGN_VCENTER	0x10
#define	TEXT_ALIGN_BOTTOM	0x20

// Right-to-left ordering
#define	TEXT_RTL			0x100

// text caps
#define	TEXT_FONT			0x00010000
#define	TEXT_COLOR			0x00020000
#define	TEXT_COLOR16		0x00040000

//////////////////////////////////////////////////////////////////////////////
//
//	Run-time Definitions
//

#ifdef	RUN_TIME

// Versions
#define	KNPV1_VERSION			0x126		// First K&P version
#define	CNCV1_VERSION			0x207		// Click & Create / The Games Factory
#define	KNP_VERSION				0x300

// Internet Versions
#define	FIRST_NETVERSION		0x1		// First vitalize version
#define	CCN_NETVERSION			0x2		// CCN applications
#define	MORECOMP_NETVERSION		0x003		// Short OIs, compression of levObjs, adpcm sounds,
#define	CNC_NETVERSION			0x003		// Current Internet Version

// Collision mask margins
#define		COLMASK_XMARGIN			64
#define		COLMASK_YMARGIN			16

// Messages
#define		UM_KNP_GETAPPID		(WM_APP+7)	// No longer used
#define		UM_HVSCROLL			(WM_APP+8)	// No longer used

// App IDs
#define		APPID_KNC		0x3952			// No longer used
#define		APPID_KNCSA		0x9172			// No longer used

// Control IDs
#define		ID_EDIT_WIN		300				// ID of edit window

#endif	// RUN_TIME

// HLO
//#define	HLO			unsigned short
#define	HLO_ERROR	(-1)
//typedef	HLO *		LPHLO;

// Frame password size
#define	PASSWORD_SIZE			9

// Cross-ref tables
enum {
	IMG_TAB,
	FONT_TAB,
	SOUND_TAB,
	MUSIC_TAB,
	MAX_TABREF
};

#ifdef RUN_TIME

// Preferences
#define	PRMUSICON		0x4000
#define	PRSOUNDON		0x8000

// Frame handles
#define	HCELL			unsigned short
#define	HCELL_ERROR		((unsigned short)-1)

// Jump flags
#define	MPJC_NOJUMP		0
#define	MPJC_DIAGO		1
#define	MPJC_BUTTON1	2
#define	MPJC_BUTTON2	3

// How to play (no longer used)
enum	{
	HTP_NO=0,
	HTP_TXT,
	HTP_WRI,
	HTP_HLP
};
#define	NB_HTPTYPE	4

#endif	// RUN_TIME

// Versions
#define MMFVERSION_MASK		0xFFFF0000
#define MMFBUILD_MASK		0x00000FFF		// MMF build
#define MMFVERFLAG_MASK		0x0000F000
#define MMFVERFLAG_HOME		0x00008000		// TGF
#define MMFVERFLAG_PRO		0x00004000		// MMF Pro
#define MMFVERFLAG_DEMO		0x00002000		// Demo
#define MMFVERFLAG_PLUGIN	0x00001000		// Plugin
#define MMFVERSION_15		0x01050000		// MMF 1.5
#define MMFVERSION_20		0x02000000		// MMF 2.0
#define CFVERSION_25		0x02050000		// CF 2.5
#define	MMF_CURRENTVERSION	MMFVERSION_20

// Build numbers
enum class MMF_BUILD {
	NONAME = 203,
	MMF_BUILD_MENUIMAGES,
	MMF_BUILD_SUBAPPICON,
	MMF_BUILD_WINMENUIDX,
	MMF_BUILD_NOTUSED,
	MMF_BUILD_ABOUTTEXT,
	FRAMEDPOS,
	TRSPCOLOR,
	BLD211,
	DEMO,
	PREBETA_2,
	PREBETA_3,
	PREBETA_4,	// First update of Jason's book
	ALTVALUES,
	EDTCHUNKS,
	COUNTERTEXT,
	BLD_SDK,		// Pre-SDK version,
	BLD_SDK2,		// Second SDK version,
	MVTSDK2,		// Second Mvt SDK version
	FIXCOLLISANDBEH,
	PROPFILTERS,
	MFXEXTS,
	LASTFRAMEOFFSET,
	EDUDEMOFEB06,
	GLBOBJBUG,
	IDTC,
	BLD229,
	BLD230,
	BLD231,
	FIXQUALIF,
	BLD233,
	BLD234_TGFPR,
	BLD235,
	BLD236,
	BLD237,
	BLD238,
	BLD239_CD,
	BLD240_DEMO,
	BLD241_SP1,
	BLD242_SP2,
	BLD243_SP3,
	BLD244_SP4,
	BLD245_SP5,
	BLD246_SP6,
	BLD247_SP7,
	BLD248_JAVA,
	BLD249_MOBILE_FLASH,
	BLD250,
	BLD251,
	BLD252,
	BLD253,
	BLD254,
};
#define MMF_CURRENTBUILD 255

// MFA file format versions
#define MFA_BUILD_ALTSTR			1						// Alterable strings
#define MFA_BUILD_COUNTERTEXT		2						// text mode in counters
#define MFA_BUILD_LASTFRAMEOFFSET	3						// Additional frame offset
#define MFA_BUILD_FIXQUALIF			4						// Fix in qualifiers + prd version
#ifdef _UNICODE
	#define MFA_BUILD_LANGID		5						// Language ID
	#define MFA_CURRENTBUILD		MFA_BUILD_LANGID
#else
	#define MFA_CURRENTBUILD		MFA_BUILD_FIXQUALIF
#endif

struct EditSurfaceParams;
struct EditImageParams;
struct EditAnimationParams;

// Global variables structure
struct mv {
	NO_DEFAULT_CTORS_OR_DTORS(mv);
	// Common to editor and runtime
	HINSTANCE			HInst;				// Application HINSTANCE
	void *				IdAppli;			// Application object in DLL
	void *				IdMainWin;			// Main window object in DLL
	void *				IdEditWin;			// Child window object in DLL
	HWND				HMainWin,			// Main window handle
						HEditWin;			// Child window handle
	HPALETTE			HPal256;			// 256 color palette
	unsigned short		AppMode,			// Screen mode with flags
						ScrMode;			// Screen mode (SM_8=256, SM_15=32768, SM_16=65536, SM_32=16 million colors)
	unsigned int		EditDXDocToClient,	// Edit time only: top-left coordinates
						EditDYDocToClient;
	CImageFilterMgr *	ImgFilterMgr;		// Image filter manager
	CSoundFilterMgr *	SndFilterMgr;		// Sound filter manager
	CSoundManager *		SndMgr;				// Sound manager

	union {
		CEditApp *		EditApp;			// Current application, edit time (not used)
		CRunApp *		RunApp;				// Current application, runtime
	};
	union {
		CEditFrame *	EditFrame;
		CRunFrame *		RunFrame;
	};

	// Runtime
	RunHeader *			RunHdr;				// Current RunHeader
	unsigned int		PextsHoldingGlobals;				// Preferences (sound on/off)
	TCHAR *				subType;
	BOOL				FullScreen;			// Full screen mode
	TCHAR *				MainAppFileName;	// App filename
	int					AppListCount;
	int					AppListSize;
	CRunApp**			AppList;
	int					ExtListCount;
	int					ExtListSize;
	TCHAR **			ExtList;
	int					NbDllTrans;
	dllTrans*			DllTransList;
	unsigned int		JoyCaps[32];
	HHOOK				HMsgHook;
	int					ModalLoop;
	int					ModalSubAppCount;
	void *				Free[5];

	// Functions
	////////////

	// Editor: Open Help file
	void (CALLBACK * HelpA) (const char * pHelpFile, unsigned int nID, LPARAM lParam);

	// Editor: Get default font for object creation
	BOOL (CALLBACK * GetDefaultFontA) (LOGFONTA * plf, char * pStyle, int cbSize);

	// Editor: Edit images and animations
	BOOL (CALLBACK * EditSurfaceA) (void * edPtr, EditImageParams * pParams, HWND hParent);
	BOOL (CALLBACK * EditImageA) (void * edPtr, EditImageParams * pParams, HWND hParent);
	BOOL (CALLBACK * EditAnimationA) (void * edPtr, EditAnimationParams * pParams, HWND hParent);

	// Runtime: Extension User data
	void * (CALLBACK * GetExtUserData) (CRunApp* pApp, HINSTANCE hInst);
	void * (CALLBACK * SetExtUserData) (CRunApp* pApp, HINSTANCE hInst, void * pData);

	// Runtime: Register dialog box
	void (CALLBACK * RegisterDialogBox) (HWND hDlg);
	void (CALLBACK * UnregisterDialogBox) (HWND hDlg);

	// Runtime: Add surface as backdrop object
	void (CALLBACK * AddBackdrop) (cSurface * pSf, int x, int y, unsigned int dwInkEffect, unsigned int dwInkEffectParam, int nObstacleType, int nLayer);

	// Runtime: Binary files
	BOOL (CALLBACK * GetFileA)(const char * pPath, char * pFilePath, unsigned int dwFlags);
	void (CALLBACK * ReleaseFileA)(const char * pPath);
	HANDLE (CALLBACK * OpenHFileA)(const char * pPath, unsigned int * pDwSize, unsigned int dwFlags);
	void (CALLBACK * CloseHFile)(HANDLE hf);

	// Plugin: download file
	int (CALLBACK * LoadNetFileA) (char * pFilename);

	// Plugin: send command to Vitalize
	int (CALLBACK * NetCommandA) (int, void *, unsigned int, void *, unsigned int);

	// Editor & Runtime: Returns the version of MMF or of the runtime
	unsigned int (CALLBACK * GetVersion) ();

	// Editor & Runtime: callback function for properties or other functions
	LRESULT	(CALLBACK * CallFunction) (EDITDATA * edPtr, CallFunctionIDs nFnc, LPARAM lParam1, LPARAM lParam2, LPARAM lParam3);

	// Editor: Open Help file (UNICODE)
	void (CALLBACK * HelpW) (const wchar_t * pHelpFile, unsigned int nID, LPARAM lParam);

	// Editor: Get default font for object creation (UNICODE)
	BOOL (CALLBACK * GetDefaultFontW) (LOGFONTW * plf, wchar_t * pStyle, int cbSize);

	// Editor: Edit images and animations (UNICODE)
	BOOL (CALLBACK * EditSurfaceW) (EDITDATA * edPtr, EditImageParams * Params, HWND Parent);
	BOOL (CALLBACK * EditImageW) (EDITDATA * edPtr, EditImageParams * Params, HWND Parent);
	BOOL (CALLBACK * EditAnimationW) (EDITDATA * edPtr, EditAnimationParams * Params, HWND Parent);

	// Runtime: Binary files (UNICODE
	BOOL (CALLBACK * GetFileW)(const wchar_t * pPath, wchar_t * pFilePath, unsigned int dwFlags);
	void (CALLBACK * ReleaseFileW)(const wchar_t * pPath);
	HANDLE (CALLBACK * OpenHFileW)(const wchar_t * pPath, unsigned int * pDwSize, unsigned int dwFlags);

	// Plugin: download file
	int	(CALLBACK * LoadNetFileW) (wchar_t * pFilename);

	// Plugin: send command to Vitalize
	int	(CALLBACK * NetCommandW) (int, void *, unsigned int, void *, unsigned int);

	// Place-holder for next versions
	void * AdditionalFncs[6];

};
//typedef mv *LPMV;

#ifdef _UNICODE
	#define mvHelp	mvHelpW
	#define mvGetDefaultFont	mvGetDefaultFontW
	#define mvEditSurface	mvEditSurfaceW
	#define mvEditImage	mvEditImageW
	#define mvEditAnimation	mvEditAnimationW
	#define mvGetFile	mvGetFileW
	#define mvReleaseFile	mvReleaseFileW
	#define mvLoadNetFile	mvLoadNetFileW
	#define mvNetCommand	mvNetCommandW
	#define	mvGetFile		mvGetFileW
	#define	mvReleaseFile	mvReleaseFileW
	#define mvOpenHFile		mvOpenHFileW
#else
	#define mvHelp	mvHelpA
	#define mvGetDefaultFont	mvGetDefaultFontA
	#define mvEditSurface	mvEditSurfaceA
	#define mvEditImage	mvEditImageA
	#define mvEditAnimation	mvEditAnimationA
	#define mvGetFile	mvGetFileA
	#define mvReleaseFile	mvReleaseFileA
	#define mvLoadNetFile	mvLoadNetFileA
	#define mvNetCommand	mvNetCommandA
	#define	mvGetFile		mvGetFileA
	#define	mvReleaseFile	mvReleaseFileA
	#define mvOpenHFile		mvOpenHFileA
#endif

// 3rd parameter of CREATEIMAGEFROMFILE
struct CreateImageFromFileInfo {
	NO_DEFAULT_CTORS_OR_DTORS(CreateImageFromFileInfo);
	int			nSize,
				xHS,
				yHS,
				xAP,
				yAP;
	COLORREF	trspColor;
};

#if EditorBuild
// Callback function macros for mvCallFunction
inline void mvInsertProps(mv * mV, EDITDATA * edPtr, PropData* pProperties, unsigned int nInsertPropID, BOOL bAfter) \
	{ if (!pProperties) return; mV->CallFunction(edPtr, CallFunctionIDs::INSERTPROPS, (LPARAM)pProperties, (LPARAM)nInsertPropID, (LPARAM)bAfter); }

inline void mvRemoveProp(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::REMOVEPROP, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

inline void mvRemoveProps(mv * mV, EDITDATA * edPtr, PropData* pProperties) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::REMOVEPROPS, (LPARAM)pProperties, (LPARAM)0, (LPARAM)0); }

inline void mvRefreshProp(mv * mV, EDITDATA * edPtr, unsigned int nPropID, BOOL bReInit) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::REFRESHPROP, (LPARAM)nPropID, (LPARAM)bReInit, (LPARAM)0); }

inline void * mvReAllocEditData(mv * mV, EDITDATA * edPtr, unsigned int dwNewSize) \
	{ return (void *)mV->CallFunction(edPtr, CallFunctionIDs::REALLOCEDITDATA, (LPARAM)edPtr, dwNewSize, 0); }


inline Prop * mvGetPropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (Prop *)mV->CallFunction(edPtr, CallFunctionIDs::GETPROPVALUE, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

inline Prop * mvGetAppPropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (Prop *)mV->CallFunction(edPtr, CallFunctionIDs::GETAPPPROPVALUE, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

inline Prop * mvGetFramePropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (Prop *)mV->CallFunction(edPtr, CallFunctionIDs::GETFRAMEPROPVALUE, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

inline void mvSetPropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID, Prop * pValue) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETPROPVALUE, (LPARAM)nPropID, (LPARAM)pValue, (LPARAM)0); }

inline void mvSetAppPropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID, Prop * pValue) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETAPPPROPVALUE, (LPARAM)nPropID, (LPARAM)pValue, (LPARAM)0); }

inline void mvSetFramePropValue(mv * mV, EDITDATA * edPtr, unsigned int nPropID, Prop * pValue) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETFRAMEPROPVALUE, (LPARAM)nPropID, (LPARAM)pValue, (LPARAM)0); }

inline unsigned int mvGetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (unsigned int)mV->CallFunction(edPtr, CallFunctionIDs::GETPROPCHECK, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

inline unsigned int mvGetAppPropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (unsigned int)mV->CallFunction(edPtr, CallFunctionIDs::GETAPPPROPCHECK, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

inline unsigned int mvGetFramePropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID) \
	{ return (unsigned int)mV->CallFunction(edPtr, CallFunctionIDs::GETFRAMEPROPCHECK, (LPARAM)nPropID, (LPARAM)0, (LPARAM)0); }

inline void mvSetPropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID, unsigned int nCheck) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETPROPCHECK, (LPARAM)nPropID, (LPARAM)nCheck, (LPARAM)0); }

inline void mvSetAppPropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID, unsigned int nCheck) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETAPPPROPCHECK, (LPARAM)nPropID, (LPARAM)nCheck, (LPARAM)0); }

inline void mvSetFramePropCheck(mv * mV, EDITDATA * edPtr, unsigned int nPropID, unsigned int nCheck) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::SETFRAMEPROPCHECK, (LPARAM)nPropID, (LPARAM)nCheck, (LPARAM)0); }

// Forces a redraw of the object in the frame editor and updates its icon
inline void mvInvalidateObject(mv * mV, EDITDATA * edPtr) \
	{ mV->CallFunction(edPtr, CallFunctionIDs::INVALIDATEOBJECT, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

#endif // EditorBuild

inline void * mvMalloc(mv * mV, int nSize) \
	{ return (void *)mV->CallFunction(NULL, CallFunctionIDs::MALLOC, (LPARAM)nSize, (LPARAM)0, (LPARAM)0); }

inline void * mvCalloc(mv * mV, int nSize) \
	{ return (void *)mV->CallFunction(NULL, CallFunctionIDs::CALLOC, (LPARAM)nSize, (LPARAM)0, (LPARAM)0); }

inline void * mvReAlloc(mv * mV, void * ptr, int nNewSize) \
	{ return (void *)mV->CallFunction(NULL, CallFunctionIDs::REALLOC, (LPARAM)ptr, (LPARAM)nNewSize, (LPARAM)0); }

inline void mvFree(mv * mV, void * ptr) \
	{ mV->CallFunction(NULL, CallFunctionIDs::FREE, (LPARAM)ptr, (LPARAM)0, (LPARAM)0); }

inline void mvRecalcLayout(mv * mV) \
	{ mV->CallFunction(NULL, CallFunctionIDs::RECALCLAYOUT, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

inline CSoundManager* mvGetSoundMgr(mv * mV) \
	{ mV->CallFunction(NULL, CallFunctionIDs::GETSOUNDMGR, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

inline void mvCloseSoundMgr(mv * mV) \
	{ mV->CallFunction(NULL, CallFunctionIDs::CLOSESOUNDMGR, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

inline int mvGetNItems(mv * mV, EDITDATA * edPtr, const char * extName) \
	{ return mV->CallFunction(edPtr, CallFunctionIDs::GETNITEMS, (LPARAM)extName, (LPARAM)0, (LPARAM)0); }

inline void * mvGetFirstItem(mv * mV, EDITDATA * edPtr, const char * extName) \
	{ return (void *)mV->CallFunction(edPtr, CallFunctionIDs::GETNEXTITEM, (LPARAM)extName, (LPARAM)0, (LPARAM)0); }

inline void * mvGetNextItem(mv * mV, EDITDATA * edPtr, void * edPtr1, const char * extName) \
	{ return (void *)mV->CallFunction(edPtr, CallFunctionIDs::GETNEXTITEM, (LPARAM)edPtr1, (LPARAM)extName, (LPARAM)0); }

#ifdef HWABETA

inline BOOL mvCreateEffect(mv * mV, const char * pEffectName, LPINT pEffect, LPARAM* pEffectParam) \
	{ return (BOOL)mV->CallFunction(NULL, CallFunctionIDs::CREATEEFFECT, (LPARAM)pEffectName, (LPARAM)pEffect, (LPARAM)pEffectParam); }

inline void mvDeleteEffect(mv * mV, int nEffect, LPARAM lEffectParam) \
	{ mV->CallFunction(NULL, CallFunctionIDs::DELETEEFFECT, (LPARAM)nEffect, (LPARAM)lEffectParam, (LPARAM)0); }

#endif // HWABETA

inline BOOL mvCreateImageFromFileA(mv * mV, LPWORD pwImg, const char * pFilename, CreateImageFromFileInfo* pInfo) \
	{ return (BOOL)mV->CallFunction(NULL, CallFunctionIDs::CREATEIMAGEFROMFILEA, (LPARAM)pwImg, (LPARAM)pFilename, (LPARAM)pInfo); }

inline BOOL mvCreateImageFromFileW(mv * mV, LPWORD pwImg, const wchar_t * pFilename, CreateImageFromFileInfo* pInfo) \
	{ return (BOOL)mV->CallFunction(NULL, CallFunctionIDs::CREATEIMAGEFROMFILEW, (LPARAM)pwImg, (LPARAM)pFilename, (LPARAM)pInfo); }

inline void * mvNeedBackgroundAccess(mv * mV, CRunFrame* pFrame, BOOL bNeedAccess) \
	{ return (void *)mV->CallFunction(NULL, CallFunctionIDs::NEEDBACKGROUNDACCESS, (LPARAM)pFrame, (LPARAM)bNeedAccess, (LPARAM)0); }

inline BOOL mvIsHWAVersion(mv * mV) \
	{ return mV->CallFunction(NULL, CallFunctionIDs::ISHWA, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

// Returns true if the runtime is Unicode. Will return true for ANSI extensions used by the Unicode runtime.
inline BOOL mvIsUnicodeVersion(mv * mV) \
	{ return mV->CallFunction(NULL, CallFunctionIDs::ISUNICODE, (LPARAM)0, (LPARAM)0, (LPARAM)0); }

// Expects parameter of mV->mVEditApp
// see https://github.com/clickteam-plugin/Surface/blob/master/General.cpp#L204
inline BOOL mvIsUnicodeApp(mv * mV, void * pApp) \
	{ return mV->CallFunction(NULL, CallFunctionIDs::ISUNICODEAPP, (LPARAM)pApp, (LPARAM)0, (LPARAM)0); }

// Expects parameter of mV->mvEditApp
// see https://github.com/clickteam-plugin/Surface/blob/master/General.cpp#L204
inline int mvGetAppCodePage(mv * mV, void * pApp) \
	{ return mV->CallFunction(NULL, CallFunctionIDs::GETAPPCODEPAGE, (LPARAM)pApp, (LPARAM)0, (LPARAM)0); }

#ifdef _UNICODE
	#define mvCreateImageFromFile	mvCreateImageFromFileW
#else
	#define mvCreateImageFromFile	mvCreateImageFromFileA
#endif

// Options for OpenHFile
#define	OHFF_LOCALFILE		0x1		// Vitalize mode only: don't try to download file from server

// EnumEltProc definition
typedef	int (CALLBACK* ENUMELTPROC)(unsigned short *, int, LPARAM, LPARAM);

// kpxGetInfos
enum class KGI : int {
	VERSION,			// Version (required)
	NOTUSED,			// Not used
	PLUGIN,				// Version for plug-in (required)
	MULTIPLESUBTYPE,	// Allow sub-types
	NOTUSED2,			// Reserved
	ATX_CONTROL,		// Not used
	PRODUCT,			// Minimum product the extension is compatible with
	BUILD,				// Minimum build the extension is compatible with
	UNICODE_,			// Returns TRUE if the extension is in Unicode
};

// Extension function table
struct kpxFunc {
	NO_DEFAULT_CTORS_OR_DTORS(kpxFunc);
	HINSTANCE	kpxHInst;
	TCHAR *		kpxName,
		  *		kpxSubType;
	void *		kpxUserData;

	// See KGI:: above for int
	unsigned int (FusionAPI * kpxGetInfos) (int);
	int  (FusionAPI * kpxLoadObject) (mv *, ObjectInfo *, unsigned char * , int);
	void (FusionAPI * kpxUnloadObject) (mv *, unsigned char * , int);
	void (FusionAPI * kpxUpdateFileNames) (mv *, TCHAR *, unsigned char * , void (FusionAPI *)(LPCTSTR, TCHAR *));
	short(FusionAPI * kpxGetRunObjectInfos) (mv *, unsigned char * );
	short(FusionAPI * kpxWindowProc) (unsigned char * , HWND, unsigned int, WPARAM, LPARAM);
	int	 (FusionAPI * kpxEnumElts) (mv *, unsigned char * , ENUMELTPROC, ENUMELTPROC, LPARAM, LPARAM);
	int  (FusionAPI * kpxLoadData) (mv *, ObjectInfo *, unsigned char * , HFILE, unsigned int);		// Not used
	int  (FusionAPI * kpxUnloadData) (mv *, ObjectInfo *, unsigned char *);					// Not used
	void (FusionAPI * kpxStartApp) (mv *, CRunApp *);
	void (FusionAPI * kpxEndApp) (mv *, CRunApp *);
	void (FusionAPI * kpxStartFrame) (mv *, CRunApp *, int);
	void (FusionAPI * kpxEndFrame) (mv *, CRunApp *, int);
	HGLOBAL (FusionAPI * kpxUpdateEditStructure) (mv *, void *);

	// Vitalise function
	#ifdef VITALIZE
		BOOL				bValidated;
	#endif
	BOOL				bUnicode;
};
//typedef kpxFunc * fpKpxFunc;

// Movement Extension
struct MvxFnc {
	NO_DEFAULT_CTORS_OR_DTORS(MvxFnc);
	HINSTANCE	mvxHInst;
	TCHAR *		mvxFileTitle;

	CMvt * (CALLBACK * mvxCreateMvt) (unsigned int);

	#ifdef VITALIZE
		BOOL	bValidated;
	#endif
};

// Restore structure alignment
// See http://stupefydeveloper.blogspot.co.uk/2009/01/c-alignment-of-structure.html
#pragma pack (pop, mmf_master_header)


//////////////////////////////////////////////////////////////////////////////
//
// LO (frame object instance)
//
//
// Note: mainly used at runtime

struct LevelObject {
	NO_DEFAULT_CTORS_OR_DTORS(LevelObject);
	unsigned short	Handle,			// HLO
					OiHandle;		// HOI
	int				X, Y;			// Coords
	unsigned short	ParentType,		// Parent type
					OiParentHandle,	// HOI Parent
					Layer,			// Layer
					Type;
	Spr *			Spr[4];			// Sprite handles for backdrop objects from layers > 1
};
// typedef LO *LPLO;
// typedef	LO *fpLevObj;

#include "ObjectInfo.hpp"


///////////////////////////////////////////////
//
// Added backdrop objects
//
struct bkd2 {
	NO_DEFAULT_CTORS_OR_DTORS(bkd2);
	unsigned short	loHnd,			// 0
					oiHnd;			// 0
	int				x,
					y;
	unsigned short	img,
					colMode,
					nLayer,
					obstacleType;
	Spr *			pSpr[4];
	unsigned int	inkEffect,
					inkEffectParam;
};
// typedef bkd2 *LPBKD2;

#define MAX_BKD2	(curFrame.m_maxObjects)

///////////////////////////////////////////////
//
// Layer at runtime
//
struct RunFrameLayer
{
	NO_DEFAULT_CTORS_OR_DTORS(RunFrameLayer);
	// name
	TCHAR *		pName;			// name

	// Offset
	int			x, y,			// Current offset
				dx, dy,			// Offset to apply to the next refresh

	// Added backdrops
				m_nBkd2Count;
	bkd2 *		m_pBkd2;

	// Ladders
	int			m_nLadderMax,
				m_nLadders;
	RECT *		m_pLadders;

	// Z-order max index for dynamic objects
	int			m_nZOrderMax;

	// Permanent data (EditFrameLayer)
	unsigned int	dwOptions;		// Options
	float			xCoef;
	float			yCoef;
	unsigned int	nBkdLOs,		// Number of backdrop objects
					nFirstLOIndex;	// Index of first backdrop object in LO table

	// EditFrameLayerEffect
	#ifdef HWABETA
		unsigned int	dwEffect;
		LPARAM			dwEffectParam;	// CEffectEx si extended
	#endif // HWABETA

	// Backup for restart
	unsigned int	backUp_dwOptions;
	float			backUp_xCoef,
					backUp_yCoef;
	unsigned int	backUp_nBkdLOs,
					backUp_nFirstLOIndex;

	#ifdef HWABETA
	//	unsigned int		backUp_dwEffect;		// A voir
	//	LPARAM		backUp_dwEffectParam;
	#endif // HWABETA

};

// Object transition data
struct objTransInfo {
	NO_DEFAULT_CTORS_OR_DTORS(objTransInfo);
	struct CTransition *	m_pTrans;				// Transition object
	cSurface *		m_SfSave,				// Background surface
			 *		m_SfDisplay,			// Working surface
			 *		m_Sf1,					// Source surface
			 *		m_Sf2;					// Destination surface
	BOOL			m_fadeOut,				// fade type
					m_fill1withbackground,	// source = background (opaque mode)
					m_fill2withbackground,	// dest = background (opaque mode)
					m_saved;				// m_SfSave has been updated
	int				m_xsave,
					m_ysave,
					m_cxsave,
					m_cysave;
	// HWA only!
	BOOL			m_bStepDrawBlit;		// Use StepDrawBlit instead of StepDraw
};
// typedef objTransInfo * LPOBJTRANSINFO;

//////////////////////////////////////////////////////////////////////////////
//
// Frame at runtime
//

#define	MAX_TEMPSTRING	16
#define IPHONEOPT_JOYSTICK_FIRE1 0x1
#define IPHONEOPT_JOYSTICK_FIRE2 0x2
#define IPHONEOPT_JOYSTICK_LEFTHAND 0x4
#define	IPHONEFOPT_MULTITOUCH			0x8
#define	IPHONEFOPT_SCREENLOCKING		0x10
#define	IPHONEFOPT_IPHONEFRAMEIAD		0x20
#define JOYSTICK_NONE 0x0000
#define JOYSTICK_TOUCH 0x1
#define JOYSTICK_ACCELEROMETER 0x2
#define JOYSTICK_EXT 0x0003


struct CRunFrame {
	NO_DEFAULT_CTORS_OR_DTORS(CRunFrame);
	// Header
	FrameHeader			hdr;

	// name
	TCHAR *				name;

	// Palette
	LPLOGPALETTE		palette;

	// Background surface
	cSurface *			pSurface;

	// Coordinates of top-level pixel in edit window
	int					leX,
						leY,
						leLastScrlX,
						leLastScrlY,

	// Layers
						NumberOfLayers;
	RunFrameLayer *		LayerAccess;

	// Virtual rectangle
	RECT				VirtualRect;			// Contains the frame's virtual width/height in right/bottom, respectively

	// Instances of frame objects
	int					FrameObjects_MaxIndex,
						FrameObjects_MaxHandle;
	unsigned short *	lo_handle_to_index;
	LevelObject *		los;

	// Variables for enumeration functions
	int					loFranIndex,
						oiFranIndex;

	// Remap table or palette handle for sub-application
	unsigned char * 	remapTable,

	// Transitions
				  * 	fadeIn,
				  *		fadeOut;
	cSurface *			pFadeInSurface1,
			 *			pFadeInSurface2;
	CTransition *		pTrans;

	// Exit code
	unsigned int		levelQuit;

	// Events
	int					rhOK;				// TRUE when the events are initialized
	RunHeader *			rhPtr;
	eventGroup *		eventPrograms;
	unsigned int		free[256-1];		// 256 = max event programs
	objInfoList *		oiList;
	void *				free0;
	unsigned short * 	qualToOiList,
				   * 	qualOilPtr,		// Do not liberate!
				   * 	qualOilPtr2;	// Do not liberate!
	short *				limitBuffer;
	unsigned int  * 	listPointers,
				  *		eventPointers;
	qualifierLoad * 	qualifiers;
	short				nQualifiers;

	short				nConditions[7 + (int)OBJ::LAST]; // Number of system types + OBJ_LAST
	unsigned int		free2[256];			// 256 = max event programs
	unsigned short		wJoystick,
						wIPhoneOptions;
	unsigned char * 	swapBuffers;
	unsigned int		objectList;
	unsigned char * 	destroyList;
	int					free3,						// Ignore - reserved
						NumberOfPlayers,
						NumberOfPlayersReal,
						Level_loop_state,
						StartLeX,
						StartLeY;
	short				MaxObjects,
						MaxOI;
	objectsList *		ObjectsList_Access;
	int					ObjectsList_Count;
	BOOL				EventsBranched,
						Fade;
	unsigned int		FadeTimerDelta,
						FadeVblDelta,
						PasteMask;

	int					CurTempString;
	TCHAR *				TempString[MAX_TEMPSTRING];	// not used

	// Other
	cSurface *			SaveSurface;
	int					EditWinWidth,	// Frame (not virtual) width
						EditWinHeight;	// Frame (not virtual) height
	unsigned int		ColMaskBits;
	TCHAR *				demoFilePath;
	unsigned short		RandomSeed,
						Free;							// Ignore - padding
	unsigned int		MvtTimerBase;

	#ifdef HWABETA
		unsigned char * LayerEffects;

		// Frame effect
		FrameEffect *		Effect;						// Frame effect (chunk data, contains effect index & param used in blit)
		struct CEffectEx *	EffectEx;					// Current effect
		bool				FrameEffectChanged;			// Frame effect has been modified
		bool				AlwaysUseSecondarySurface;	// This frame always use a secondary surface

		// Secondary surface (render target used if background or frame effects)
		cSurface *			SecondarySurface;

		// List of sub-app surfaces to refresh at the end in D3D full screen mode
		struct CPList *		SurfacedSubApps;
	#endif
};
// typedef CRunFrame *fpRunFrame;

////////////////////////
//
// Binary files
//
struct CBinaryFile {
	TCHAR			Path[MAX_PATH],	// path stored in ccn file
		  *			TempPath;		// path in temporary folder, if any
	unsigned int	FileSize,		// file size
					FileOffset;		// file offset in EXE/CCN file
	int				TempCount;		// usage count

	CBinaryFile() :
		TempPath(NULL), FileSize(0), FileOffset(0), TempCount(0)
	{
		memset(Path, 0, MAX_PATH);
	}
	~CBinaryFile()
	{
		if (TempPath)
		{
			_tremove(TempPath);
			free(TempPath);
			TempPath = NULL;
			TempCount = 0;
		}
	}
};

//////////////////////////////////////////////////////////////////////////////
//
// Application
//

#define	ARF_MENUINIT				0x1
#define	ARF_MENUIMAGESLOADED		0x2		// menu images have been loaded into memory
#define	ARF_INGAMELOOP				0x4
#define ARF_PAUSEDBEFOREMODALLOOP	0x8

struct CRunApp {
	NO_DEFAULT_CTORS_OR_DTORS(CRunApp);
	CRunApp* get_ParentApp();
	std::size_t GetNumFusionFrames();

DarkEdifInternalAccessProtected:
	friend Edif::Runtime;
	// Application info
	AppMiniHeader	miniHdr;			// Version
	AppHeader		hdr;				// General info
	TCHAR *			name,				// name of the application
		  *			appFileName,		// filename (temporary file in editor mode)
		  *			editorFileName,		// filename of original .mfa file
		  *			copyright,			// copyright
		  *			aboutText;			// text to display in the About box

	// File infos
	TCHAR *			targetFileName;		// filename of original CCN/EXE file
	TCHAR *			tempPath;			// Temporary directory for external files
	HFILE			file;				// File handle
	unsigned int	startOffset;

	// Help file
	TCHAR *			doc;				// Help file pathname

	// Icon
	unsigned char *	icon16x16x8;		// = LPBITMAPINFOHEADER
	HICON			hIcon16x16x8;		// Small icon for the main window

	// Menu
	HMENU			hRunMenu;			// Menu
	unsigned char * accels;				// Accelerators
	TCHAR *			pMenuTexts;			// Menu texts (for ownerdraw menu)
	unsigned char * pMenuImages;		// Images index used in the menu
	MenuHdr *		pMenu;

	// Frame offsets
	int					frameMaxIndex;				// Max. number of frames
	int					frameMaxHandle;				// Max. frame handle
	unsigned short *	frame_handle_to_index;		// Handle -> index table
	unsigned int *		frameOffset;				// Frame offsets in the file

	// Frame passwords
	TCHAR **			framePasswords;				// Table of frame passwords (TCHAR * [])

	// Extensions
	int				nbKpx;						// Number of extensions
	kpxFunc *		kpxTab;						// Function table 1
	kpj (*kpxDataTable)[2];						// Function table 2 - pointer to kpj[2]

	// Movement Extensions
	int				nbMvx;						// Number of movement extensions
	MvxFnc *		mvxTable;					// DLL info

	// Elements
	TCHAR *			eltFileName[MAX_TABREF];	// Element banks
	HFILE			hfElt[MAX_TABREF];

	unsigned int	eltBaseOff;
	unsigned short	nbEltOff[MAX_TABREF];		// Sizes of file offset tables
	unsigned int * adTabEltOff[MAX_TABREF];	// File offsets of bank elements

	unsigned short	 nbEltMemToDisk[MAX_TABREF];	// Size of elt cross-ref tables
	unsigned short * EltMemToDisk[MAX_TABREF],		// Element memory index -> element disk index
				   * EltDiskToMem[MAX_TABREF],		// Element disk index -> memory index

					tabNbCpt[MAX_TABREF];		// Sizes of usage count tables
	int *			tabAdCpt[MAX_TABREF];		// Usage count tables of bank elements

	// Binary files
	unsigned int	binaryFiles[8];

	// Temporary images
	unsigned int	NumberOfImagesTemp;			// List of temporary images (used by Replace Color action)
	unsigned char * ImagesTemp;					// Pointer to temporary images

	// Frame objects
	int					oiMaxIndex;
	int					oiMaxHandle;
	unsigned short *	oi_handle_to_index;
	ObjectInfo **		ois;
	int					oiFranIndex;			// for enumerating
	int					oiExtFranIndex;			// for enumerating

	// Sub-application
	CRunApp *		ParentApp;					// Parent application
	void *			SubAppObject;				// LPRS
	unsigned int	SubAppOptions;				// Sub-app options
	BOOL			SubAppIsVisible;			// Sub-app visibility
	HICON			SubAppIcon;					// Sub-app icon
	int				cx;							// Subapp: valid if stretched
	int				cy;

	// DLL infos
	void *			idAppli;					// App object in DLL
	int				nDepth;						// Screen depth
	cSurface*		protoLogScreen;				// Surface prototype

	// Edit window
	HWND			hEditWin;					// Edit Window handle
	void *			idEditWin;					// Edit Window identifier

	// Current frame
	CRunFrame*		Frame;						// Pointer to current frame

	// Run-time status
	int				bResizeTimer,
					refTime,
					appRunningState,
					startFrame,
					nextFrame,
					nCurrentFrame;
	BOOL			bWakeUp;
	unsigned short	oldFlags,
					appRunFlags;
	BOOL			bPlayFromMsgProc;

	// Debugger
	struct CDebugger *		pDebugger;

	// Full Screen
	int				depthFullScreen;
	WINDOWPLACEMENT sWndPlacement;				// Window position backup
	int				oldCxMax,					// Window size backup
					oldCyMax;
	cSurface *		pFullScreenSurface;

	// Global data
	BOOL			bSharePlayerCtrls,			// Sub-app: TRUE if shares player controls
					bShareLives,				// Sub-app: TRUE if shares lives
					bShareScores,				// Sub-app: TRUE if shares scores
					bShareGlobalValues,			// Sub-app: TRUE if shares global values
					bShareGlobalStrings;		// Sub-app: TRUE if shares global strings

	// Players
	PlayerCtrls	*	pPlayerCtrls;
	int *			pLives,
		*			pScores;

	// Global values (warning: not valid if sub-app and global values are shared)
	unsigned char * pGlobalValuesInit;
	int				nGlobalValues;				// Number of global values
	CValueMultiPlat * pGlobalValues;			// Global values
	unsigned char *	pFree,						// No longer used
				  *	pGlobalValueNames,

	// Global strings (warning: not valid if sub-app and global values are shared)
				  *	pGlobalStringInit;			// Default global string values
	int				nGlobalStrings;				// Number of global strings
	TCHAR **		pGlobalString;				// Pointers to global strings
	unsigned char * pGlobalStringNames;

	// Global objects
	unsigned char * AdGO;						// Global objects data

	// FRANCOIS
	short			NConditions[7+32-1];		// NUMBER_OF_SYSTEM_OBJECTS + KPX_BASE - 1->0 base change

	// External sound files
	TCHAR *			pExtMusicFile,
		  *			pExtSampleFile[32];			// External sample file per channel

	int				nInModalLoopCount;
	TCHAR *			pPlayerNames;
	unsigned int	dwColorCache;

	unsigned char * pVtz4Opt;					// not used
	unsigned int	dwFree;						// not used

	// Application load
	TCHAR *			pLoadFilename;
	unsigned int	saveVersion;
	BOOL			bLoading;

	// Bluray
	void *			pBROpt;

	// Build info
	AppHeader2 *	pHdr2;

	// Code page
	struct CRunApp_Unicode {
		unsigned int	dwCodePage;
		bool			bUnicodeAppFile;
	};

	#ifdef _UNICODE
		CRunApp_Unicode codePage;
	#endif

	// Effects
	#ifdef HWABETA
		unsigned char * HWA_Effects;					// Effects used in the application
		cSurface *		HWA_OldSecondarySurface;		// Secondary surface of the last frame, used in transitions
		bool			HWA_AlwaysUseSecondarySurface,	// At least one frame has a transition => always use a secondary surface in all the frames
						HWA_ShowWindowedMenu;			// To show menu after switch from full screen to windowed mode
		int				HWA_SubAppShowCount;			// To show the child window, otherwise it's not displayed...
	#endif // HWABETA
};
// typedef CRunApp*	fpRunApp;
//#endif // mmf_master_header

#define	KPX_MAGICNUMBER		200			// Magic number, used in Ext.rc

#if 0
// Dibs
// ----
typedef Appli void;
unsigned long FusionAPI InitDibHeader(Appli *, int, int, int, BITMAPINFO *);
void	FusionAPI FillDib(BITMAPINFO *, COLORREF);
unsigned long	FusionAPI ImageToDib(Appli *, DWORD, DWORD, LPBYTE);
unsigned long	FusionAPI DibToImage(Appli *, Image *, BITMAPINFOHEADER *);
unsigned long	FusionAPI DibToImageEx(Appli *, Image *, BITMAPINFOHEADER *, COLORREF, DWORD);
void	FusionAPI RemapDib(BITMAPINFO *, Appli *, LPBYTE);
#endif

#include "StockSDKDefines.hpp"
