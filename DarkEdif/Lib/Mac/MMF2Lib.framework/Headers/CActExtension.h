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
// CACTEXTENSION
//
// -----------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "CEvents.h"

#define MAX_ACTPARAMS 16

@class CValue;
@class CRun;
@class CFile;

@interface CActExtension : NSObject
{
@public
	LPEVP pParams[MAX_ACTPARAMS];
	LPEVT pEvent;
}
-(void)initialize:(LPEVT)evtPtr;
-(CObject*)getParamObject:(CRun*)rhPt withNum:(int)num;
-(short)getParamObjectType:(CRun*)rhPt withNum:(int)num;
-(int)getParamTime:(CRun*)rhPtr withNum:(int)num;
-(short)getParamBorder:(CRun*)rhPtr withNum:(int)num;
-(short)getParamAltValue:(CRun*)rhPtr withNum:(int)num;
-(short)getParamDirection:(CRun*)rhPtr withNum:(int)num;
-(int)getParamAnimation:(CRun*)rhPtr withNum:(int)num;
-(short)getParamPlayer:(CRun*)rhPtr withNum:(int)num;
-(LPEVP)getParamEvery:(CRun*)rhPtrw withNum:(int)num;
-(int)getParamSpeed:(CRun*)rhPtr withNum:(int)num;
-(unsigned int)getParamPosition:(CRun*)rhPtr withNum:(int)num;
-(short)getParamJoyDirection:(CRun*)rhPtr withNum:(int)num;
-(short)getParamMenu:(CRun*)rhPtr withNum:(int)num;
-(int)getParamExpression:(CRun*)rhPtr withNum:(int)num;
-(int)getParamColour:(CRun*)rhPtr withNum:(int)num;
-(short)getParamFrame:(CRun*)rhPtr withNum:(int)num;
-(int)getParamNewDirection:(CRun*)rhPtr withNum:(int)num;
-(short)getParamClick:(CRun*)rhPtr withNum:(int)num;
-(NSString*)getParamFilename:(CRun*)rhPtr withNum:(int)num;
-(NSString*)getParamExpString:(CRun*)rhPtr withNum:(int)num;
-(double)getParamExpDouble:(CRun*)rhPtr withNum:(int)num;
-(NSString*)getParamFilename2:(CRun*)rhPtr withNum:(int)num;
-(CFile*)getParamExtension:(CRun*)rhPtr withNum:(int)num;
-(short*)getParamZone:(CRun*)rhPtr withNum:(int)num;

@end
