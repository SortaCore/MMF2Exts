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
// COBJECTCOMMON : Donnees d'un objet normal
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "COC.h"
#import "COI.h"


// Flags
#define OEFLAG_DISPLAYINFRONT 0x0001
#define OEFLAG_BACKGROUND 0x0002
#define OEFLAG_BACKSAVE 0x0004
#define OEFLAG_RUNBEFOREFADEIN 0x0008
#define OEFLAG_MOVEMENTS 0x0010
#define OEFLAG_ANIMATIONS 0x0020
#define OEFLAG_TABSTOP 0x0040
#define OEFLAG_WINDOWPROC 0x0080
#define OEFLAG_VALUES 0x0100
#define OEFLAG_SPRITES 0x0200
#define OEFLAG_INTERNALBACKSAVE 0x0400
#define OEFLAG_SCROLLINGINDEPENDANT 0x0800
#define OEFLAG_QUICKDISPLAY 0x1000
#define OEFLAG_NEVERKILL 0x2000
#define OEFLAG_NEVERSLEEP 0x4000
#define OEFLAG_MANUALSLEEP 0x8000
#define OEFLAG_TEXT 0x10000
#define OEFLAG_DONTCREATEATSTART 0x20000
#define OEFLAG_DONTRESETANIMCOUNTER 0x100000		// do not reset current frame duration when the animation is modified
#define OCFLAGS2_DONTSAVEBKD 0x0001
#define OCFLAGS2_SOLIDBKD 0x0002
#define OCFLAGS2_COLBOX 0x0004
#define OCFLAGS2_VISIBLEATSTART 0x0008
#define OCFLAGS2_OBSTACLESHIFT 4
#define OCFLAGS2_OBSTACLEMASK 0x0030
#define OCFLAGS2_OBSTACLE_SOLID 0x0010
#define OCFLAGS2_OBSTACLE_PLATFORM 0x0020
#define OCFLAGS2_OBSTACLE_LADDER 0x0030
#define OCFLAGS2_AUTOMATICROTATION 0x0040
#define	OCFLAGS2_INITFLAGS 0x0080

// Flags modifiable by the program
#define OEPREFS_BACKSAVE 0x0001
#define OEPREFS_SCROLLINGINDEPENDANT 0x0002
#define OEPREFS_QUICKDISPLAY 0x0004
#define OEPREFS_SLEEP 0x0008
#define OEPREFS_LOADONCALL 0x0010
#define OEPREFS_GLOBAL 0x0020
#define OEPREFS_BACKEFFECTS 0x0040
#define OEPREFS_KILL 0x0080
#define OEPREFS_INKEFFECTS 0x0100
#define OEPREFS_TRANSITIONS 0x0200
#define OEPREFS_FINECOLLISIONS 0x0400

@class CFile;
@class CDefValues;
@class CDefStrings;
@class CAnimHeader;
@class CDefCounters;
@class CDefObject;
@class CMoveDefList;
@class CDefCounter;
@class CDefTexts;
@class COI;
@class CTransitionData;

@interface CObjectCommon : COC
{
@public
	int ocOEFlags;		    /// New flags
    short ocQualifiers[8];	    /// Qualifier list
    short ocFlags2;		    /// New news flags, before was ocEvents
    short ocOEPrefs;		    /// Automatically modifiable flags
    int ocIdentifier;		    /// Identifier d'objet
    int ocBackColor;		    /// Background color
	CTransitionData* ocFadeIn;                    /// Fade in
	CTransitionData* ocFadeOut;                   /// Fade out
    CMoveDefList* ocMovements;     /// La liste des mouvements
    CDefValues* ocValues;          /// Les alterable values par defaut
    CDefStrings* ocStrings;        /// Les alterable strings
    CAnimHeader* ocAnimations;     /// Les animations
    CDefCounters* ocCounters;   /// Settings lives / scores / counter
    CDefObject* ocObject;          /// L'objet lui meme'
    unsigned char* ocExtension;	/// Les donn�es objets extension
	int ocExtLength;
    int ocVersion;
    int ocID;
    int ocPrivate;
	COI* pCOI;

}
-(id)init;
-(void)dealloc;
-(void)load:(CFile*)file withType:(short)type andCOI:(COI*)pOI;
-(void)enumElements:(id)enumImages withFont:(id)enumFonts;

@end
