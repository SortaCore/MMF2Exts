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
// COBJECTCOMMON : Donn�es d'un objet normal
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "COC.h"
#import "IDrawable.h"

#define LINEF_INVX 0x0001
#define LINEF_INVY 0x0002

@class COI;

enum {
	SHAPE_LINE = 1,
	SHAPE_RECTANGLE = 2,
	SHAPE_ELLIPSE = 3
};

enum {
	FILLTYPE_NONE = 0,
	FILLTYPE_SOLID = 1,
	FILLTYPE_GRADIENT = 2,
	FILLTYPE_MOTIF = 3
};

enum {
	GRADIENT_HORIZONTAL = 0,
	GRADIENT_VERTICAL
};

@interface COCQBackdrop : COC <IDrawable>
{
@public
	short ocBorderSize;			// Border
    int ocBorderColor;
    short ocShape;			// Shape
    short ocFillType;
    short ocLineFlags;			// Only for lines in non filled mode
    int ocColor1;			// Gradient
    int ocColor2;
    int ocGradientFlags;
    unsigned short ocImage;				// Image
	COI* pCOI;
}
-(id)init;
-(void)dealloc;
-(void)load:(CFile*)file withType:(short)type andCOI:(COI*)pOi;
-(void)enumElements:(id)enumImages withFont:(id)enumFonts;
-(void)spriteDraw:(CRenderer*)renderer withSprite:(CSprite*)spr andImageBank:(CImageBank*)bank andX:(int)x andY:(int)y;
-(void)spriteKill:(CSprite*)spr;
-(CMask*)spriteGetMask;

@end
