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
// CEffect: effect code
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>

@class CFile;
@class CRunApp;
@class CEffectParam;

#define EFFECTPARAM_INT 0
#define EFFECTPARAM_FLOAT 1
#define EFFECTPARAM_INTFLOAT4 2
#define EFFECTPARAM_SURFACE 3

#define EFFECTOPT_BKDTEXTUREMASK 0x000F
#define EFFECTOPT_ZBUFFER 0x0100
#define EFFECTOPT_WANTSPIXELSIZE 0x0200

#define EFFECTFLAG_INTERNALMASK  0xF0000000
#define EFFECTFLAG_HASTEXTUREPARAMS 0x10000000
#define EFFECTFLAG_WANTSPIXELSIZE 0x00000001
#define EFFECTFLAG_WANTSMATRIX 0x00000002
#define EFFECTFLAG_MODIFIED 0x00000004

@interface CEffect : NSObject
{
@public
     CRunApp* app;
     int handle;
     NSString* name;

     NSString* vertexData;
     NSString* fragData;

     int nParams;
     int options;

     CEffectParam** effectParams;
     bool hasExtras;
}
-(void)dealloc;
-(id)initWithApp:(CRunApp*)app;
-(void)fillParams:(CFile*)file StartAt:(long)startParams OffsetName:(int)paramNameOffset andTypeOffSet:(int)paramTypeOffset;
-(void)fillValues:(CFile*)file andOffset:(int)fileOffset;
-(CEffectParam**)copyParams;
@end

