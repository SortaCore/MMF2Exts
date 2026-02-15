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
// CEVENTPROGRAM : Programme des evenements
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>
#import "CEvents.h"
//#import "COI.h"
//#import "IEnum.h"
#import "vector"

@class CRunApp2;
@class CLoadQualifiers;
@class CQualToOiList;
@class CObject;
@class CObjInfo;
@class CArrayList;
@class CRun;
@class CSoundBank;

@interface CEventProgram : NSObject
{
@public
	CRunApp2* app;
	CRun* rhPtr;
	short maxObjects;
	short maxOi;
	short nPlayers;
	short nConditions[7 + 10];		 // NUMBEROF_SYSTEMTYPES+OBJ_LAST
	short nQualifiers;
	void** qualifiers;
	LPEVG pEvents;
	CQualToOiList** qualToOiList;
	CQualToOiList** qualToOiListFull;
	DWORD* listPointers;
	DWORD* eventPointers;
//  BYTE eventPointersCnd[] = null;
	short* limitBuffer;
	LPDWORD rhEvents[7 + 1];
	LPDWORD rhEventAlways;
	LPDWORD rh4TimerEventsBase;
	short* colBuffer;
	int qualOilPtr;
	int qualOilPos;
	int qualOilPtr2;
	int qualOilPos2;
	BOOL rh4CheckDoneInstart;			/// Build92 to correct start of frame with fade in
	LPEVG rhEventGroup;				/// Current group
	int rhCurCode;									   /// Current event
	int rh4PickFlags[4];				/// 00-31
	BOOL rh2ActionLoop;			/// Actions flag
	BOOL rh2ActionOn;			/// Flag are we in actions?
	BOOL rh2EnablePick;  			/// Are we in pick for actions?
	int rh2EventCount;				/// Number of the event
	int rh2ActionCount;				/// Action counter
	int rh2ActionLoopCount;	  		/// Action loops counter
	int rh4EventCountOR;						 /// Number of the event for OR conditions
	BOOL rh4ConditionsFalse;
	BOOL rh3DoStop;			/// Force the test of stop actions
	void* rh2EventQualPos;		  /// ***Position in event objects
	int rh2EventQualPosNum;			/// ***Position in event objects
	CObject* rh2EventPos;				/// ***Position in event objects
	int rh2EventPosOiList; 		/// ***Position in oilist for TYPE exploration
	CObject* rh2EventPrev;				/// ***Previous object address
	CObjInfo* rh2EventPrevOiList;				/// ***Previous object address
	int evtNSelectedObjects;
	BOOL repeatFlag;
	short rh2EventType;
	short rhCurOi;
	int rhCurParam[2];
	int rh3CurrentMenu;				/// For menu II events
	short rh2CurrentClick;			/// For click events II
	CObject* rh4_2ndObject;	 		/// Collision object address
	BOOL bReady;
	CArrayList* rh2ShuffleBuffer;
	short rhCurObjectNumber;	 		/// Object number
	short rh1stObjectNumber;		  /// Number, for collisions
	CArrayList* rh2PushedEvents;
	BOOL rh2ActionEndRoutine;		/// End of action routine
	LPEVT rh4ActionStart;
	CArrayList* allocatedStrings;
	BOOL bEndForEach;
	int gEvgFlagsDefaultMask;
	NSMutableDictionary* onEachLoopConditionIndexes;
	NSMutableDictionary* onEachLoopConditionIndexesActives;
}

@end
