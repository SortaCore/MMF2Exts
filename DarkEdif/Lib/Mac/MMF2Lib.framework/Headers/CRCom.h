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
// CRCOM : Structure commune aux objets animes
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>


@class CSprite;

@interface CRCom : NSObject 
{
@public
	int rcPlayer;					// Player who controls
    int rcMovementType;				// Number of the current movement
    CSprite* rcSprite;					// Sprite ID if defined
    int rcAnim;						// Wanted animation
    unsigned short rcImage;					// Current frame
    float rcScaleX;
    float rcScaleY;
    float rcAngle;
    int rcDir;						// Current direction
    int rcSpeed;					// Current speed
    int rcMinSpeed;					// Minimum speed
    int rcMaxSpeed;					// Maximum speed
    BOOL rcChanged;					// Flag: modified object
    BOOL rcCheckCollides;			// For static objects
	
    int rcOldX;            			// Previous coordinates
    int rcOldY;
    unsigned short rcOldImage;
    float rcOldAngle;
    int rcOldDir;
    int rcOldX1;					// For zone detections
    int rcOldY1;
    int rcOldX2;
    int rcOldY2;
	
    int rcFadeIn;
    int rcFadeOut;
	
	BOOL rcCMoveChanged;
	
}
-(id)init;
-(void)kill:(BOOL)bFast;

@end
