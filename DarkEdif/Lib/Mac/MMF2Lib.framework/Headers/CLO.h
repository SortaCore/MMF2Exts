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
// CLO : un levobj
//
//----------------------------------------------------------------------------------

#import <Foundation/Foundation.h>

#define PARENT_NONE 0
#define PARENT_FRAME 1
#define PARENT_FRAMEITEM 2
#define PARENT_QUALIFIER 3

@class CSprite;
@class CFile;

@interface CLO : NSObject
{
@public
    short loHandle;			// Le handle
    short loOiHandle;			// HOI
    int loX;				// Coords
    int loY;
    short loParentType;			// Parent type
    short loValue;		// HOI Parent
    short loLayer;			// Layer
    short loType;
    CSprite* loSpr[4];			// Sprite handles for backdrop objects from layers > 1
}

-(id)init;
-(void)load:(CFile*)file;

@end
