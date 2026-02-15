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
// COI
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>

// Flags
#define OILF_OCLOADED 0x0001
#define OILF_ELTLOADED 0x0002
#define OILF_TOLOAD 0x0004
#define OILF_TODELETE 0x0008
#define OILF_CURFRAME 0x0010
#define OILF_TORELOAD 0x0020
#define OILF_IGNORELOADONCALL 0x0040
#define OIF_LOADONCALL 0x0001
#define OIF_DISCARDABLE 0x0002
#define OIF_GLOBAL 0x0004

#define NUMBEROF_SYSTEMTYPES 7
#define OBJ_PLAYER -7
#define OBJ_KEYBOARD -6
#define OBJ_CREATE -5
#define OBJ_TIMER -4
#define OBJ_GAME -3
#define OBJ_SPEAKER -2
#define OBJ_SYSTEM -1
#define OBJ_BOX 0
#define OBJ_BKD 1
#define OBJ_SPR 2
#define OBJ_TEXT 3
#define OBJ_QUEST 4
#define OBJ_SCORE 5
#define OBJ_LIVES 6
#define OBJ_COUNTER 7
#define OBJ_RTF 8
#define OBJ_CCA 9
#define NB_SYSOBJ 10
#define OBJ_LAST 10
#define KPX_BASE 32
#define OIFLAG_QUALIFIER 0x8000

@class COC;
@class CFile;

@interface COI : NSObject
{
@public
	// objInfoHeader
    short oiHandle;
    short oiType;
    short oiFlags;			/// Memory flags
	//  public short oiReserved=0;			/// No longer used
    int oiInkEffect;			/// Ink effect
    int oiInkEffectParam;	        /// Ink effect param

    int oiIndexEffect;
    NSUInteger oiEffectOffset;
    int* oiEffectData;
    int oiEffectNParams;

	NSString* oiName;

    // OI
    COC* oiOC;			/// ObjectsCommon
    NSUInteger oiFileOffset;
    int oiLoadFlags;
    short oiLoadCount;
    short oiCount;

}

-(id)init;
-(void)dealloc;
-(void)loadHeader:(CFile*)file;
-(void)load:(CFile*)file;
-(void)unLoad;
-(void)enumElements:(id)enumImages withFont:(id)enumFonts;
-(void)loadEffectData:(CFile*)file;
@end
