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
// COBJECT : Classe de base d'un objet'
//
//----------------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>
#import "IDrawable.h"
#import "CoreMath.h"
#import "CRect.h"

@class CRun;
@class CRCom;
@class CRAni;
@class CRMvt;
@class CRVal;
@class CRSpr;
@class CObjInfo;
@class CArrayList;
@class CObjectCommon;
@class CMask;
@class CSprite;
@class CImageBank;
@class CCreateObjectInfo;
@class CBitmap;
@class CEffectEx;
class CRenderer;

@interface CObject : NSObject <IDrawable>
{
@public
	short hoNumber;					/// Number of the object
	short hoNextSelected;				/// Selected object list!!! DO NOT CHANGE POSITION!!!
	CRun* hoAdRunHeader;								  /// Run-header address
	short hoHFII;					/// Number of LevObj
	short hoOi;						/// Number of OI
	short hoNumPrev;					/// Same OI previous object
	short hoNumNext;					/// ... next
	short hoType;					/// Type of the object
	short hoCreationId;								  /// Number of creation
	CObjInfo* hoOiList;								   /// Pointer to OILIST information
	unsigned int* hoEvents;					/// Pointer to specific events
	void* hoPrevNoRepeat;					   /// One-shot event handling
	void* hoBaseNoRepeat;
	int hoMark1;										 /// #of loop marker for the events
	int hoMark2;
	NSString* hoMT_NodeName;				/// Name fo the current node for path movements
	int hoEventNumber;								   /// Number of the event called (for extensions)
	CObjectCommon* hoCommon;				/// Common structure address
	int hoCalculX;					/// Low weight value
	int hoX;											 /// X coordinate
	int hoCalculY;					/// Low weight value
	int hoY;						/// Y coordinate
	int hoImgXSpot;					/// Hot spot of the current image
	int hoImgYSpot;
	int hoImgWidth;					/// Width of the current picture
	int hoImgHeight;
	CRect hoRect;						/// Display rectangle
	int hoOEFlags;					/// Objects flags
	short hoFlags;					/// Flags
	unsigned char hoSelectedInOR;								 /// Selection lors d'un evenement OR
	int hoOffsetValue;								   /// Values structure offset
	int hoLayer;										 /// Layer
	short hoLimitFlags;								  /// Collision limitation flags
	short hoNextQuickDisplay;							/// Quickdraw list
	short hoPreviousQuickDisplay;
	int hoCurrentParam;								  /// Address of the current parameter
	int hoIdentifier;									/// ASCII identifier of the object
	BOOL hoCallRoutine;
	void* replacedColors;
	float controlScaleX;
	float controlScaleY;

	// Classes de gestion communes
	CRCom* roc;				   // The CRCom object
	CRMvt* rom;				   // The CRMvt object
	CRAni* roa;				   // The CRAni object
	CRVal* rov;				   // The CRVal object
	CRSpr* ros;				   // The CRSpr object

	// Effect code
	CEffectEx* hoEffect;
}
-(void)dealloc;
-(void)setScale:(float)fScaleX withScaleY:(float)fScaleY andFlag:(BOOL)bResample;
-(void)setBoundingBoxFromWidth:(int)cx andHeight:(int)cy andXSpot:(int)hsx andYSpot:(int)hsy;
-(void)initObject:(CObjectCommon*)ocPtr withCOB:(CCreateObjectInfo*)cob;
-(int)getX;
-(int)getY;
-(int)globalToLayerX:(int)X;
-(int)globalToLayerY:(int)Y;
-(Vec2i)getPosition;
-(int)getWidth;
-(int)getHeight;
-(void)setX:(int)x;
-(void)setY:(int)y;
-(void)setPosition:(int)x withY:(int)y;
-(void)setLayer:(int)newLayer;
-(void)setWidth:(int)width;
-(void)setHeight:(int)height;
-(void)generateEvent:(int)code withParam:(int)param;
-(void)pushEvent:(int)code withParam:(int)param;
-(bool)isPaused;
-(void)pause;
-(void)resume;
-(void)redisplay;
-(void)redraw;
-(void)destroy;
-(int)fixedValue;
-(void)handle;
-(void)modif;
-(void)display;
-(BOOL)kill:(BOOL)bFast;
-(void)getZoneInfos;
-(void)draw:(CRenderer*)renderer;
-(CMask*)getCollisionMask:(int)flags;
-(void)spriteDraw:(CRenderer*)renderer withSprite:(CSprite*)spr andImageBank:(CImageBank*)bank andX:(int)x andY:(int)y;
-(void)spriteKill:(CSprite*)spr;
-(CMask*)spriteGetMask;
-(NSString*)description;
-(CObject*)getObjectFromFixed:(int)fixed;
-(BOOL)isOfType:(short)OiList;
-(void)runtimeIsReady;
-(int)checkOrCreateEffectIfNeededByIndex:(int)index andEffectParam:(int)rgba;
-(int)checkOrCreateEffectIfNeededByName:(NSString*)name andEffectParam:(int)rgba;
-(void)fillEffectData;
@end
