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
// CEffectEx: effect executable code
//
//----------------------------------------------------------------------------------
#import <Foundation/Foundation.h>

@class CFile;
@class CRunApp;
@class CEffectParam;
@class CEffect;
@class CValue;
class Renderer;

#define EFFECTPARAM_INT 0
#define EFFECTPARAM_FLOAT 1
#define EFFECTPARAM_INTFLOAT4 2
#define EFFECTPARAM_SURFACE 3

@interface CEffectEx : NSObject
{
@public
    CRunApp* app;
    
    int handle;
    NSString* name;

    NSString* vertexData;
    NSString* fragData;

    int nParams;
    int options;

    CEffectParam** eParams;

    int indexShader;
    int blendColor;
    bool hasExtras;
    bool useBackground;
    bool usePixelSize;
}
-(void)dealloc;
-(id)initWithApp:(CRunApp*)App;
-(bool)initializeByIndex:(int)index withEffectParam:(int)rgba;
-(bool)initializeByName:(NSString*)effectName withEffectParam:(int)rgba;
-(bool)initializeShader;
-(bool)setEffectData:(int*)values andSize:(int)size;
-(bool)removeShader;
-(bool)destroyShader;
-(NSString*)getName;
-(unsigned int)getRGBA;
-(void)setRGBA:(int)color;
-(int)getIndexShader;
-(int)getParamType:(int)paramIdx;
-(int)getParamIndex:(NSString*)name;
-(NSString*)getParamName:(int)paramIdx;
-(int)getParamInt:(int)paramIdx;
-(float)getParamFloat:(int)paramIdx;
-(bool)setParamValue:(int)index andValue:(CValue*)value;
-(void)setParamAt:(int)paramIdx withIntValue:(int)value;
-(void)setParamAt:(int)paramIdx withFloatValue:(float)value;
-(void)setParamAt:(int)paramIdx withFloat4Value:(int)value;
-(void)setParamAt:(int)paramIdx withTexture:(unsigned short)image_handle;
-(bool)updateShader;
-(bool)updateParamTexture;
-(bool)refreshParamSurface;
@end


