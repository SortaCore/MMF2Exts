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
// -----------------------------------------------------------------------------
//
// CBKD2 : objet paste dans le decor
//
// -----------------------------------------------------------------------------
#pragma once
#import <Foundation/Foundation.h>

@class CRun;
@class CRunApp;
@class COI;
@class CSprite;
@class CEffectEx;

@interface CBkd2 : NSObject
{
@public
	CRun* rhPtr;
	short loHnd;			// 0
    short oiHnd;			// 0
    int x;
    int y;
	int spotX;
	int spotY;
    short img;
    short colMode;
    short nLayer;
    short obstacleType;
    CSprite* pSpr[4];
    int inkEffect;
    int inkEffectParam;

    int bkdEffectShader;
    CEffectEx* bkdEffect;

	int spriteFlag;
    void* body;
}
-(id)initWithCRun:(CRun*)rh;
-(void)dealloc;
-(void)clearSprites;
-(int)checkOrCreateEffectIfNeeded:(CRunApp*)app andCOI:(COI*)oiPtr;
-(void)fillEffectData:(COI*)oiPtr;

@end
