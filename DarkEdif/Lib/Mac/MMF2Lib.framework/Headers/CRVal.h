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
// CRVAL : Alterable values et strings
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>

#define VALUES_NUMBEROF_ALTERABLE_DEFAULT 26
#define STRINGS_NUMBEROF_ALTERABLE_DEFAULT 10

@class CValue;
@class CObject;
@class CObjectCommon;
@class CCreateObjectInfo;

@interface CRVal : NSObject
{
@public
	int rvValueFlags;
    CValue** rvValues;
    NSString** rvStrings;
	int rvNumberOfValues;
    int rvNumberOfStrings;
}
-(void)dealloc;
-(id)initWithHO:(CObject*)ho andOC:(CObjectCommon*)ocPtr andCOB:(CCreateObjectInfo*)cob;
-(void)kill:(BOOL)bFast;
-(CValue*)getValue:(int)n;
-(NSString*)getString:(int)n;
-(void)setString:(int)n withString:(NSString*)s;

@end
