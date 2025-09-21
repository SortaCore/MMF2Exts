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
// CFONTBANK : stockage des fontes
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "IEnum.h"

@class CRunApp;
@class CFont;
@class CFontInfo;
@class CFile;

@interface CFontBank : NSObject <IEnum>
{
@public
	CRunApp* runApp;
    CFont** fonts;          /// List of CFonts
    int* offsetsToFonts;
    int nFonts;
    short* handleToIndex;
    int maxHandlesReel;
    int maxHandlesTotal;
    short* useCount;
    CFont* nullFont;
}

-(id)initWithApp:(CRunApp*)app;
-(void)dealloc;
-(void)preLoad;
-(void)load;
-(CFont*)getFontFromHandle:(short)handle;
-(CFont*)getFontFromIndex:(short)index;
-(CFontInfo*)getFontInfoFromHandle:(short)handle;
-(void)setToLoad:(short)handle;
-(void)resetToLoad;
-(short)addFont:(CFontInfo*)info;
-(short)enumerate:(short)num;

@end
