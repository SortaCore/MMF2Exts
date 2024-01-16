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

struct CRect
{
	int left;
	int top;
	int right;
	int bottom;
	int width() const;
	int height() const;
	BOOL isNil() const;
	BOOL containsPoint(int x, int y) const;
};

@interface CObject : NSObject <IDrawable>
{
@public
    short hoNumber;					/// Number of the object
    short hoNextSelected;				/// Selected object list!!! DO NOT CHANGE POSITION!!!
    CRun* hoAdRunHeader;                                  /// Run-header address
    short hoHFII;					/// Number of LevObj
    short hoOi;						/// Number of OI
    short hoNumPrev;					/// Same OI previous object
    short hoNumNext;					/// ... next
    short hoType;					/// Type of the object
    short hoCreationId;                                  /// Number of creation
    CObjInfo* hoOiList;                                   /// Pointer to OILIST information
    unsigned int* hoEvents;					/// Pointer to specific events
    void* hoPrevNoRepeat;                       /// One-shot event handling
    void* hoBaseNoRepeat;
    int hoMark1;                                         /// #of loop marker for the events
    int hoMark2;
    NSString* hoMT_NodeName;				/// Name fo the current node for path movements
    int hoEventNumber;                                   /// Number of the event called (for extensions)
    CObjectCommon* hoCommon;				/// Common structure address
    int hoCalculX;					/// Low weight value
    int hoX;                                             /// X coordinate
    int hoCalculY;					/// Low weight value
    int hoY;						/// Y coordinate
    int hoImgXSpot;					/// Hot spot of the current image
    int hoImgYSpot;
    int hoImgWidth;					/// Width of the current picture
    int hoImgHeight;
    CRect hoRect;        				/// Display rectangle
    int hoOEFlags;					/// Objects flags
    short hoFlags;					/// Flags
    unsigned char hoSelectedInOR;                                 /// Selection lors d'un evenement OR
    int hoOffsetValue;                                   /// Values structure offset
    int hoLayer;                                         /// Layer
    short hoLimitFlags;                                  /// Collision limitation flags
    short hoNextQuickDisplay;                            /// Quickdraw list
	short hoPreviousQuickDisplay;
    int hoCurrentParam;                                  /// Address of the current parameter
    int hoIdentifier;                                    /// ASCII identifier of the object
    BOOL hoCallRoutine;
	void* replacedColors;
	float controlScaleX;
	float controlScaleY;
	
    // Classes de gestion communes
    CRCom* roc;                   // The CRCom object
    CRMvt* rom;                   // The CRMvt object
    CRAni* roa;                   // The CRAni object
    CRVal* rov;                   // The CRVal object
    CRSpr* ros;                   // The CRSpr object
    
    // Effect code
    CEffectEx* hoEffect;
}
@end
