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
// CSPRITE : Un sprite
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>
#import "IDrawable.h"
#import "CRect.h"

#define SF_TOKILL 0x00000001		// flag "a detruire"
#define SF_RECALCSURF 0x00000002		// Recalc surface (if rotation or stretch)
#define SF_PRIVATE 0x00000004		// flag priv� utilis� par le runtime pour la destruction des fade
#define SF_INACTIF 0x00000008		// flag "inactif" = reaffichage ssi intersection avec un autre
#define SF_TOHIDE 0x00000010		// flag "a cacher"
#define SF_RAMBO 0x00000020		// flag "rentre dans tout le monde"
#define SF_REAF 0x00000040		    // flag "a reafficher"
#define SF_HIDDEN 0x00000080		// flag "cache"
#define SF_COLBOX 0x00000100		// flag "collisions en mode box"
#define SF_NOSAVE 0x00000200		// flag "do not save background"
#define SF_FILLBACK 0x00000400		// flag "fill background using a solid colour (sprAdrBack)"
#define SF_DISABLED 0x00000800
#define SF_REAFINT 0x00001000		// Internal
#define SF_OWNERDRAW 0x00002000		// flag "owner draw"
#define SF_OWNERSAVE 0x00004000		// flag "owner save"
#define SF_FADE 0x00008000			// Private
#define SF_OBSTACLE 0x00010000		// Obstacle
#define SF_PLATFORM 0x00020000		// Platform
#define SF_BACKGROUND 0x00080000		// Backdrop object
#define SF_SCALE_RESAMPLE 0x00100000	// Resample when stretching
#define SF_ROTATE_ANTIA 0x00200000		// Antialiasing for rotations
#define SF_NOHOTSPOT 0x00400000		// No hot spot
#define SF_OWNERCOLMASK 0x00800000		// Owner-draw sprite supports collision masks
#define SF_UPDATECOLLIST 0x10000000
#define SF_NOKILLDATA 0x20000000
#define EFFECT_SEMITRANSP 1

@class CImageBank;
@class CMask;
@class CObject;
@class CArrayList;
@class CBitmap;
@class CImage;

@interface CSprite : NSObject 
{
@public
	
	// Donnees 
    CSprite* objPrev;
    CSprite* objNext;
    CImageBank* bank;
	CRect rect;
    
    int sprFlags;			/// Flags
    short sprLayer;			/// Sprite plane (layer)
    float sprAngle;			/// Angle
    int sprZOrder;			/// Z-order value
	
    // Coordinates
    int sprX;
    int sprY;
	
    // Bounding box
    int sprX1;
    int sprY1;
    int sprX2;
    int sprY2;
	
    // New coordinates
    int sprXnew;
    int sprYnew;
	
    // New bounding box
    int sprX1new;
    int sprY1new;
    int sprX2new;
    int sprY2new;
	
    // Background bounding box
    int sprX1z;
    int sprY1z;
    int sprX2z;
    int sprY2z;
	
    // Scale & Angle
    float sprScaleX;
    float sprScaleY;
	
    // Temporary values for collisions
    unsigned short sprTempImg;			// TODO: use DWORD later?
    short sprTempAngle;
    float sprTempScaleX;
    float sprTempScaleY;
	
    // Image or owner-draw routine
    unsigned short sprImg;		        /// Numero d'image
    unsigned short sprImgNew;			/// Nouvelle image
    id<IDrawable> sprRout;			/// Ownerdraw callback routine
	
    // Ink effect
    int sprEffect;			/// 0=normal, 1=semi-transparent, > 16 = routine
    int sprEffectParam;			/// parametre effet (coef transparence, etc...)
	
    // Effect Shader
    int sprEffectShader;
    
    // Fill color (wipe with color mode)
    int sprBackColor;
	
    // Surfaces
    CMask* sprColMask;			/// Collision mask (if stretched or rotated)
    CMask* sprTempColMask;		/// Temp collision mask (if stretched or rotated)
	
    // User data
    CObject* sprExtraInfo;
	
	CImage* sprSf;
	CImage* sprTempSf;
}
-(id)initWithBank:(CImageBank*)b; 
-(void)dealloc;
-(int)getSpriteLayer;
-(int)getSpriteFlags;
-(int)setSpriteFlags:(int)dwNewFlags;
-(int)setSpriteColFlag:(int)colMode;
-(float)getSpriteScaleX;
-(float)getSpriteScaleY;
-(BOOL)getSpriteScaleResample;
-(float)getSpriteAngle;
-(BOOL)getSpriteAngleAntiA;
-(void)draw:(CRenderer*)renderer;
-(CRect)getSpriteRect;
-(void)updateBoundingBox;
-(void)calcBoundingBox:(unsigned short)newImg withX:(int)newX andY:(int)newY andAngle:(float)newAngle andScaleX:(float)newScaleX andScaleY:(float)newScaleY andRect:(CRect)prc;
-(void)killSpriteZone;

@end
