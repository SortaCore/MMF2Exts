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
// CDEFCOUNTERS : DonnÈes d'un objet score / vies / counter
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>

// Display types
#define CTA_HIDDEN 0
#define CTA_DIGITS 1
#define CTA_VBAR 2
#define CTA_HBAR 3
#define CTA_ANIM 4
#define CTA_TEXT 5    
#define BARFLAG_INVERSE 0x0100
#define CTA_FILLTYPE_GRADIENT 2
#define CTA_FILLTYPE_SOLID 1
#define CTA_GRAD_HORIZONTAL 0
#define CTA_GRAD_VERTICAL 1

@class CFile;

@interface CDefCounters : NSObject 
{
@public 
	int odCx;					// Size: only lives & counters
    int odCy;
    short odPlayer;				// Player: only score & lives
    short odDisplayType;			// CTA_xxx
    short odDisplayFlags;			// BARFLAG_INVERSE
    short odFont;					// Font
    short ocBorderSize;			// Border
    int ocBorderColor;
    short ocShape;			// Shape
    short ocFillType;
    short ocLineFlags;			// Only for lines in non filled mode
    int ocColor1;			// Gradient
    int ocColor2;
    int ocGradientFlags;
    short nFrames;
    unsigned short* frames;
	
}
-(id)init;
-(void)dealloc;
-(void)load:(CFile*)file;
-(void)enumElements:(id)enumImages withFont:(id)enumFonts;

@end
