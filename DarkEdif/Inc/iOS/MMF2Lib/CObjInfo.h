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
// CObjInfo informations sur un objet
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>

#define OILIMITFLAGS_BORDERS 0x000F
#define OILIMITFLAGS_BACKDROPS 0x0010
#define OILIMITFLAGS_ONCOLLIDE 0x0080
#define OILIMITFLAGS_QUICKCOL 0x0100
#define OILIMITFLAGS_QUICKBACK 0x0200
#define OILIMITFLAGS_QUICKBORDER 0x0400
#define OILIMITFLAGS_QUICKSPR 0x0800
#define OILIMITFLAGS_QUICKEXT 0x1000
#define OILIMITFLAGS_ALL ((short)0xFFFF)

@class COI;

@interface CObjInfo : NSObject
{
@public
	short oilOi;  			/// THE oi
	short oilListSelected;			   /// First selection !!! DO NOT CHANGE POSITION !!!
	short oilType;			/// Type of the object
	short oilObject;			/// First objects in the game
	unsigned int oilEvents;			/// Events
	unsigned char oilWrap;			/// WRAP flags
	BOOL oilNextFlag;
	int oilNObjects;					 /// Current number
	int oilActionCount;			/// Action loop counter
	int oilActionLoopCount;			  /// Action loop counter
	int oilCurrentRoutine;			   /// Current routine for the actions
	int oilCurrentOi;			/// Current object
	int oilNext;				/// Pointer on the next
	int oilEventCount;			/// When the event list is done
	int oilNumOfSelected;				/// Number of selected objects
	int oilOEFlags;			/// Object's flags
	short oilLimitFlags;			/// Movement limitation flags
	signed long oilLimitList;	   /// Pointer to limitation list
	short oilOIFlags;			/// Objects preferences
	short oilOCFlags2;			/// Objects preferences II
	int oilInkEffect;			/// Ink effect
	int oilEffectParam;			/// Ink effect param
	short oilHFII;			/// First available frameitem
	int oilBackColor;			/// Background erasing color
	short oilQualifiers[8];			   /// Qualifiers for this object
	NSString* oilName;				 /// Name
	int oilEventCountOR;				 /// Selection in a list of events with OR
	short* oilColList;
 	short* oilRealQualifiers;

	int oilIndex;				/// Index in rhOiList
	int oilIndexEffect;
}
-(id)init;
-(void)dealloc;
-(void)copyData:(COI*)oiPtr;
-(NSString*)description;

@end
