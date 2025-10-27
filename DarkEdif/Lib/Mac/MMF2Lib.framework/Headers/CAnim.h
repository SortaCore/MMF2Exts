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
// CANIM : definition d'une animation
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>

@class CFile;

// Definition of animation codes
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#define ANIMID_STOP 0
#define ANIMID_WALK 1
#define ANIMID_RUN 2
#define ANIMID_APPEAR 3
#define ANIMID_DISAPPEAR 4
#define ANIMID_BOUNCE 5
#define ANIMID_SHOOT 6
#define ANIMID_JUMP 7
#define ANIMID_FALL 8
#define ANIMID_CLIMB 9
#define ANIMID_CROUCH 10
#define ANIMID_UNCROUCH 11
#define ANIMID_USER1 12

@class CFile;
@class CAnimDir;

@interface CAnim : NSObject
{
@public
	CAnimDir* anDirs[32];
    unsigned char anTrigo[32];
    unsigned char anAntiTrigo[32];
}
-(void)dealloc;
-(void)load:(CFile*)file;
-(void)enumElements:(id)enumImages;
-(void)approximate:(int)nAnim;

@end
