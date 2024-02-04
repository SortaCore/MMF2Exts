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
//
//  CPPRenderer.h
//  RuntimeIPhone
//
//  Created by Anders Riggelsen on 8/20/13.
//  Copyright (c) 2013 Clickteam. All rights reserved.
//

#pragma once
#import <QuartzCore/QuartzCore.h>
#import <OpenGL/glu.h>
#import <OpenGL/gl3.h>
#import "CTexture.h"
#import "CoreMath.h"
#import "CRect.h"

#import "Array.h"

#define NUM_SHADER      250

@class CRenderToTexture;
@class CRunView;
@class CArrayList;
@class CRunApp;
@class CLayer;
class CShader;

struct Viewport
{
	Viewport();
	Viewport(int x, int y, int width, int height);
	Viewport(Vec2i position, Vec2i size);

	bool operator==(const Viewport &rhs) const;
	bool operator!=(const Viewport &rhs) const;

	float aspect();		//The current aspect ratio of this viewport

	Vec2i position;		//Position in pixels
	Vec2i size;			//Size in pixels
};

class RenderingState
{
public:
	Viewport viewport;
	Vec2i framebufferSize;
	unsigned int framebuffer;

	Vec2f contentSize;	//The size of the underlying game in game-units
	Mat3f transform;	//The transform matrix of this viewport (scroll, zoom, rotation ect. ect.)
	Mat3f projection;	//The projection matrix of this viewport (based on the contentSize)
};

class CRenderer
{
public:
	CRunView* view;

    // The pixel dimensions of the CAEAGLLayer
    GLint backingWidth;
    GLint backingHeight;
    GLint backingProgram;

	RenderingState currentRenderState;
	Array<RenderingState> renderStateStack;

	GLuint buffer;
    GLuint vao;

	//OpenGL capabilities
	GLint maxTextureSize;

	int originX, originY;
	CGPoint topLeft;

	int textureUsage;
	NSMutableSet* usedTextures;

    // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view
    GLuint colorRenderbuffer;

	CGSize windowSize;
	CArrayList* texturesToRemove;

	//Current OpenGL state to minimize redundant state changes
	int currentTextureID;
	int currentBlendEquationA;
	int currentBlendEquationB;
	int currentBlendFunctionA;
	int currentBlendFunctionB;
	float cR, cG, cB, cA;
	BOOL usesBlending;
	BOOL usesScissor;

	CLayer* currentLayer;
	int currentEffect;
	float currentParam;
	Viewport currentViewport;

	CShader* defaultShader;
	CShader* gradientShader;
	CShader* currentShader;

    CShader* perspectiveShader;
    CShader* sinewaveShader;

    CShader* effectShader;
    CShader** indexedShaders;
    int iShader;

	CRenderer(CRunView* runView);
	~CRenderer();

	void pushRenderingState(Vec2f offset);
	void pushRenderingState();
	void popRenderingState();

	void destroyFrameBuffers();
	void clear(float red, float green, float blue);
	void clear();
	void clearWithRunApp(CRunApp* app);
	void swapBuffers();
	void flush();
	void forgetCachedState();
	void forgetShader();
	void setCurrentShader(CShader* shader);

	void bindRenderBuffer();
    void pushShader();
    void popShader();
	void updateViewport();
	void setViewport(Viewport viewport);

	void setCurrentLayer(CLayer* layer);

	void setInkEffect(int effect, int effectParam, CShader* shader);
	void setProjectionMatrix(int x, int y, int width, int height);

    void setSurfaceTextureAtIndex(CTexture* image, NSString* name, int index);
    void updateSurfaceTexture();

	void renderSimpleImage(int x, int y, int w, int h);
	void renderImage(CTexture* image, int x, int y, int w, int h, int inkEffect, int inkEffectParam);
	void renderPattern(CTexture* image, int x, int y, int w, int h, int inkEffect, int inkEffectParam);
	void renderPattern(CTexture* image, int x, int y, int w, int h, int inkEffect, int inkEffectParam, bool flipX, bool flipY, float scaleX = 1.0f, float scaleY = 1.0f);
	void renderScaledRotatedImage(CTexture* image, float angle, float sX, float sY, int hX, int hY, int x, int y, int w, int h, int inkEffect, int inkEffectParam);
	void renderLine(Vec2f a, Vec2f b, int color, float thickness);

	void renderGradient(GradientColor gradient, int x, int y, int w, int h, int inkEffect, int inkEffectParam);
	void renderGradient(GradientColor gradient, CRect rect, int inkEffect, int inkEffectParam);
	void renderSolidColor(int color, int x, int y, int w, int h, int inkEffect, int inkEffectParam);
	void renderSolidColor(int color, CRect rect, int inkEffect, int inkEffectParam);
    void renderPerspective(CRenderToTexture* image, int x, int y, int w, int h, float fA, float fB, int dir, int inkEffect, int inkEffectParam);
    void renderSinewave(CRenderToTexture* image, int x, int y, int w, int h, float zoom, float wave, float offset, int dir, int inkEffect, int inkEffectParam);


	//For transitions
	void setOrigin(int x, int y);
	void renderBlitFull(CRenderToTexture* source);
	void renderBlit(CRenderToTexture* source, int xDst, int yDst, int xSrc, int ySrc, int width, int height);
	void renderStretch(CRenderToTexture* source, int xDst, int yDst, int wDst, int hDst, int xSrc, int ySrc, int wSrc, int hSrc, int inkEffect = 0, int inkEffectParam = 0);
	void renderFade(CRenderToTexture* source, int alpha);

	void useBlending(BOOL useBlending);

	void setBlendEquation(GLenum equation);
	void setBlendEquationSeperate(GLenum equationA, GLenum equationB);
	void setBlendFunction(GLenum sFactor, GLenum dFactor);
	void setBlendColor(float red, float green, float blue, float alpha);

	//Clipping
	void setClip(int x, int y, int w, int h);
	void resetClip();

	void startStencilDefinition();
	void startStencilMasking();
	void stopStencilMasking();

	static void checkForError();
	static CRenderer* getRenderer();

	void uploadTexture(CTexture* texture);
    void removeTextureFromPrune(CTexture* texture);
	void removeTexture(CTexture* texture, BOOL cleanMemory);
	void cleanMemory();
	void cleanUnused();
	void pruneTexture();
	void clearPruneList();

    // to be used with RTT
    void screenAreaToTexture(CTexture* texture, int x , int y, int w, int h, int mode);
    void screenPixelsToTexture(CTexture* texture, int x , int y, int w, int h);
    CIImage* screenAreaToImage(int x , int y, int width, int height);

    //Shader operation
    int addShader(NSString* shaderName, NSString* vertexShader, NSString* fragmentShader, NSArray* shaderVariables, bool useTexCoord, bool useColors);
    int addShader(NSString* shaderName, NSArray* shaderVariables, bool useTexCoord, bool useColors);
    void removeShader(int shaderIndex);
    void setEffectShader(int shaderIndex);
    void removeEffectShader();

    void updateVariable1i(NSString* varName, int value);
    void updateVariable1i(int varIndex, int value);
    void updateVariable1f(NSString* varName, float value);
    void updateVariable1f(int varIndex, float value);

    void updateVariable2i(NSString* varName, int value0, int value1);
    void updateVariable2i(int varIndex, int value0, int value1);
    void updateVariable2f(NSString* varName, float value0, float value1);
    void updateVariable2f(int varIndex, float value0, float value1);

    void updateVariable3i(NSString* varName, int value0, int value1, int value2);
    void updateVariable3i(int varIndex, int value0, int value1, int value2);
    void updateVariable3f(NSString* varName, float value0, float value1, float value2);
    void updateVariable3f(int varIndex, float value0, float value1, float value2);

    void updateVariable4i(NSString* varName, int value0, int value1, int value2, int value3);
    void updateVariable4i(int varIndex, int value0, int value1, int value2, int value3);
    void updateVariable4f(NSString* varName, float value0, float value1, float value2, float value3);
    void updateVariable4f(int varIndex, float value0, float value1, float value2, float value3);


};
