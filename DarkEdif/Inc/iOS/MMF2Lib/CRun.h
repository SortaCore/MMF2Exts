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
// CRUN : BOucle principale
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>
#import "CEvents.h"
#import "CRunApp.h"
//#import "CRect.h"
//#import "CBox2D.h"
#import "vector"

// Flags
#define GAMEFLAGS_VBLINDEP 0x0002
#define GAMEFLAGS_LIMITEDSCROLL 0x0004
#define GAMEFLAGS_FIRSTLOOPFADEIN 0x0010
#define GAMEFLAGS_LOADONCALL 0x0020
#define GAMEFLAGS_REALGAME 0x0040
#define GAMEFLAGS_PLAY 0x0080
#define GAMEFLAGS_INITIALISING 0x0200

// Flags pour DrawLevel
#define DLF_DONTUPDATE 0x0002
#define DLF_DRAWOBJECTS 0x0004
#define DLF_RESTARTLEVEL 0x0008
#define DLF_DONTUPDATECOLMASK 0x0010
#define DLF_COLMASKCLIPPED 0x0020
#define DLF_SKIPLAYER0 0x0040
#define DLF_REDRAWLAYER 0x0080
#define DLF_STARTLEVEL 0x0100
#define GAME_XBORDER 480
#define GAME_YBORDER 300
#define COLMASK_XMARGIN 64
#define COLMASK_YMARGIN 16
#define WRAP_X 1
#define WRAP_Y 2
#define WRAP_XY 4

// Flags pour rh3Scrolling
#define RH3SCROLLING_SCROLL 0x0001
#define RH3SCROLLING_REDRAWLAYERS 0x0002
#define RH3SCROLLING_REDRAWALL 0x0004
#define RH3SCROLLING_REDRAWTOTALCOLMASK 0x0008

// Types d'obstacles
#define OBSTACLE_NONE 0
#define OBSTACLE_SOLID 1
#define OBSTACLE_PLATFORM 2
#define OBSTACLE_LADDER 3
#define OBSTACLE_TRANSPARENT 4		// for Add Backdrop

//Flags pour createobject
#define COF_NOMOVEMENT 0x0001
#define COF_HIDDEN 0x0002
#define COF_FIRSTTEXT 0x0004
#define COF_CREATEDATSTART 0x0008
#define MAX_FRAMERATE 10

// Main loop exit codes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define LOOPEXIT_NEXTLEVEL 1
#define LOOPEXIT_PREVLEVEL 2
#define LOOPEXIT_GOTOLEVEL 3
#define LOOPEXIT_NEWGAME 4
#define LOOPEXIT_PAUSEGAME 5
#define LOOPEXIT_ENDGAME -2
#define LOOPEXIT_QUIT 100
#define LOOPEXIT_RESTART 101
#define LOOPEXIT_APPLETPAUSE 102
#define BORDER_LEFT 1
#define BORDER_RIGHT 2
#define BORDER_TOP 4
#define BORDER_BOTTOM 8
#define BORDER_ALL 15
#define MAX_INTERMEDIATERESULTS 160
#define STEP_TEMPVALUES 200
#define MAX_SUBAPPS 5

// Collision detection acceleration
#define OBJZONE_WIDTH	512
#define OBJZONE_HEIGHT	512

//@class CRunApp;
@class CSpriteGen;
@class CRunFrame;
@class CEventProgram;
@class CFontInfo;
@class CExp;
@class CObjInfo;
@class CArrayList;
@class CValue;
@class CObject;
@class CExtension;
@class CBkd2;
@class CBackDraw;
@class CLayer;
@class CQuestion;
@class CBitmap;
class CRenderer;
@class CExtStorage;
@class CCndExtension;
@class CActExtension;
@class CCCA;
@class ObjectSelection;
@class CLoop;

@interface CRun : NSObject
{
@public
	CRunApp* rhApp;						/// Application info
	CRunFrame* rhFrame;					/// Frame info
	CObjInfo** rhOiList;					 		/// ObjectInfo list
	CArrayList* rh4FastLoops;
	NSString* rh4CurrentFastLoop;
	NSString* rh4PSaveFilename;
	CValue* rh4ExpValue1;				/// New V2
	CValue* rh4ExpValue2;
	CArrayList* rh4BackDrawRoutines;
	int rh4FrameRateArray[MAX_FRAMERATE];			 /// Framerate calculation buffer
	int* rhDestroyList;			/// Destroy list address
	CObject** rhObjectList;			/// Object list address

	int rhMaxOI;
	char rhStopFlag;					/// Current movement needs to be stopped
	char rhEvFlag; 					/// Event evaluation flag
	int rhNPlayers;					/// Number of players
	short rhGameFlags;					/// Game flags
	short rhFree;						/// Alignment
	unsigned char rhPlayer;					/// Current players entry
	short rhQuit;
	short rhQuitBis; 					/// Secondary quit (scrollings)
	int rhReturn;					/// Value to return to the editor
	int rhQuitParam;
	int rhNObjects;
	int rhMaxObjects;
	CEventProgram* rhEvtProg;
	int rhLevelSx;				/// Window size
	int rhLevelSy;
	int rhWindowX;   				/// Start of window in X/Y
	int rhWindowY;
	int rhVBLDeltaOld;				/// Number of VBL
	int rhVBLObjet;				/// For the objects
	int rhVBLOld;				/// For the counter
	short rhMT_VBLStep;   			/// Path movement variables
	short rhMT_VBLCount;
	int rhMT_MoveStep;
	int rhLoopCount;				/// Number of loops since start of level
	double rhTimer;				/// Timer in 1/50 since start of level
	double rhTimerOld;				/// For delta calculation
	int rhTimerDelta;				/// For delta calculation
	short rhFree4;				/// Alignment
	int rhOiListPtr;				/// OI list enumeration
	short rhObListNext;				/// Branch label
	char rhMouseUsed;					// Players using the mouse
	short rhDestroyPos;
	unsigned char rh2OldPlayer;				/// Previous player entries
	unsigned char rh2NewPlayer;				/// Modified player entries
	unsigned char rh2InputMask;				/// Inhibated players entries
	unsigned char rh2MouseKeys;				/// Mousekey entries
	short rh2CreationCount;			/// Number of objects created since beginning of frame
	short rh2Free;
	int rh2MouseX;				/// Mouse coordinate
	int rh2MouseY;				/// Mouse coordinate
	short rh2Free2;
	int rh2MouseSaveX;				/// Mouse saving when pause
	int rh2MouseSaveY;				/// Mouse saving when pause
	int rh2PauseCompteur;
	double rh2PauseTimer;
	int rh2PauseVbl;
	int rh4MouseXCenter;
	int rh4MouseYCenter;
	int rh3DisplayX;				/// To scroll
	int rh3DisplayY;
	int rh3WindowSx;   				/// Window size
	int rh3WindowSy;
	short rh3CollisionCount;			/// Collision counter
	char rh3Scrolling;				/// Flag: we need to scroll
	int rh3Panic;
	int rh3PanicBase;
	int rh3PanicPile;
	int rh3XMinimum;   				/// Object inactivation coordinates
	int rh3YMinimum;
	int rh3XMaximum;
	int rh3YMaximum;
	int rh3XMinimumKill;			/// Object destruction coordinates
	int rh3YMinimumKill;
	int rh3XMaximumKill;
	int rh3YMaximumKill;
	short rh3Graine;
	//	short		rh4KpxNumOfWindowProcs;					// Number of routines to call
	//	kpxMsg		rh4KpxWindowProc[KPX_MAXNUMBER];		// Message handle routines
	//	kpxLib		rh4KpxFunctions[KPX_MAXFUNCTIONS];		// Available internal routines
	//	CALLANIMATIONS	rh4Animations;
	//	CALLDIRATSTART	rh4DirAtStart;
	//	CALLMOVEIT		rh4MoveIt;
	//	CALLAPPROACHOBJECT rh4ApproachObject;
	//	CALLCOLLISIONS rh4Collisions;
	//	CALLTESTPOSITION rh4TestPosition;
	//	CALLGETJOYSTICK rh4GetJoystick;
	//	CALLCOLMASKTESTRECT rh4ColMaskTestRect;
	//	CALLCOLMASKTESTPOINT rh4ColMaskTestPoint;
	short rh4DemoMode;
	int rh4PauseKey;
	int rh4EndOfPause;
	int rh4SaveVersion;
	int rh4MusicHandle;
	int rh4MusicFlags;
	int rh4MusicLoops;
	int rh4LoadCount;
	//	LPDWORD		rh4TimerEventsBase;				// Timer events base

	//	short		rh4DroppedFlag;
	//	short		rh4NDroppedFiles;
	//	LPSTR		rh4DroppedFiles;
	//	LPSTR		rh4CreationErrorMessages;
	int rh4KpxReturn;				/// WindowProc return
	int rh4ObjectCurCreate;
	int rh4ObjectAddCreate;
	short rh4FakeKey;				/// For step through : fake key pressed
	char rh4DoUpdate;				/// Flag for screen update on first loop
	BOOL rh4MenuEaten;			/// Menu handled in an event?
	int rh4OnCloseCount;			/// For OnClose event
	short rh4ScrMode;				/// Current screen mode
	int rh4EventCount;
	short rh4LastQuickDisplay;			/// Quick - display list
	short rh4FirstQuickDisplay;			/// Quick-display object list
	int rh4WindowDeltaX;			/// For scrolling
	int rh4WindowDeltaY;
	int rh4TimeOut;				/// For time-out!
	int rh4TabCounter;				/// Objects with tabulation
	int rh4CurToken;
	int rh4FrameRatePos;						/// Position in buffer
	int rh4FrameRatePrevious;					/// Previous time
	int rh4SaveFrame;
	int rh4SaveFrameCount;
	double rh4MvtTimerCoef;
	int xScrolling;					// FRA: = 320;
	BOOL bOperande;
	//CQuestion* rhQuestionObjectOn;
	int rh4PosPile;				/// Expression evaluation pile position
	CValue* rh4Results[MAX_INTERMEDIATERESULTS];				/// Result pile
	int rh4Operators[MAX_INTERMEDIATERESULTS];				/// Operators pile
	LPEXP rh4ExpToken;
	NSString* rhTempString;
	CCndExtension* pCndExtension;
	CActExtension* pActExtension;
	CValue** rhTempValues;
	int rhCurTempValue;
	int rhMaxTempValues;
	int rhBaseTempValues;
	CCCA* subApps[MAX_SUBAPPS];
	int nSubApps;
	unsigned char rhJoystickMask;
	CValue* evaTmp;
	ObjectSelection* objectSelection;
	BOOL rh4Box2DSearched;
	void * rh4Box2DBase;
	BOOL bBodiesCreated;
	void * rh4TimerEvents;
	NSString* timerEventName;
	LPFOREACH rh4CurrentForEach;
	LPFOREACH rh4CurrentForEach2;
	LPFOREACH rh4ForEachs;
	void* rh4PosOnLoop;
	BOOL rh4ComplexOnLoop;
	CSpriteGen* spriteGen;
	std::vector< std::vector<saveSelection> > rhChildEventSelectionStack;
	event* rhChildEventToExecute;
	int rhLastCreatedObjectFixedValue;

	BOOL runtimeIsReady;
	BOOL appActive;
}



@end
