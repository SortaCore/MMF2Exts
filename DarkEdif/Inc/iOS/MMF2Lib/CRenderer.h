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

#import <UIKit/UIKit.h>

#include <OpenGLES/EAGL.h>
#include <OpenGLES/EAGLDrawable.h>

#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>

#import "CTexture.h"
#import "CoreMath.h"
#import "CRect.h"
#import "CRenderUtils.h"

#import "Array.h"

#define NUM_INIT_SHADER      50
#define MAX_TEXPOOL          40
#define MAX_SSCPOOL          30
#define MAX_RTTPOOL          30



@class CRenderToTexture;
@class CRunView;
@class CArrayList;
@class CRunApp;
@class CRunFrame;
@class CLayer;
class CShader;

class DownScaling {
public:
    int scale;
    bool active;
};

class CRenderer
{
public:
	EAGLContext* context;
	CRunView* view;
    bool bOpenGL2;

    // The pixel dimensions of the CAEAGLLayer
    GLint backingWidth;
    GLint backingHeight;

    GLint backingProgram;

	RenderingState currentRenderState;
    std::stack<RenderingState> renderStateStack;

    GLuint vao;
    GLuint buffer;
    GLuint indexbuffer;

	//OpenGL capabilities
	GLint maxTextureSize;

	int originX, originY;
    int baseX, baseY;
    float scaleX, scaleY;

	CGPoint topLeft;

	int textureUsage;
	NSMutableSet* usedTextures;
	NSMutableSet* supportedExtensions;

    int backColor;
    int frameColor;
    int frameAlpha;

    std::vector<GLTexturePool> glTexturePool;
    std::vector<CTexturePool> cTexturePool;
    std::vector<RenderToTexturePool> renderTexturePool;
    std::vector<SubScenePool>subScenePool;

    DownScaling downScaling;

    // The OpenGL ES names for the framebuffer and renderbuffer used to render to this view
    GLuint defaultFramebuffer, colorRenderbuffer, stencilbuffer;

	CAEAGLLayer* glLayer;
	CGSize windowSize;
	CArrayList* texturesToRemove;

	//Current OpenGL state to minimize redundant state changes
	int currentTextureID;
	int currentBlendEquationA;
	int currentBlendEquationB;
	int currentBlendFunctionA;
	int currentBlendFunctionB;
    int currentBlendFunctionC;
    int currentBlendFunctionD;
	float cR, cG, cB, cA;
	BOOL usesBlending;
	BOOL usesScissor;

	CLayer* currentLayer;
	int currentEffect;
	float currentParam;
	Viewport currentViewport;
    Viewport screenViewport;
    Mat3f currentTransform;
    Mat3f currentProjection;
    GLuint currentProgram;

    bool bPreMultiply;
    bool useClipping;
    int nLayers, clipLeft, clipBottom, clipWidth, clipHeight;

    bool downgrade, onLayer, onAlpha, onScreen, onFrame;
    bool newProjection, newTransform;

	CShader* defaultShader;
	CShader* gradientShader;
    CShader* defaultEllipseShader;
    CShader* gradientEllipseShader;
    CShader* solidColorShader;
	CShader* currentShader;

    CShader* perspectiveShader;
    CShader* sinewaveShader;

    CShader* effectShader;
    std::stack<CShader*> shaderStack;
    CShadersVector shaders;
    int iShader;

    SceneRender renderScene;

    std::stack<SceneRender> sceneStack;
    std::stack<Scissor> scissors;

    NSObject* lock; // Lock object to avoid possible racing over cleanUnused texture function.

	CRenderer(CRunView* runView);
	~CRenderer();

	void pushRenderingState(Vec2f offset);
	void pushRenderingState();
	void popRenderingState();

	void destroyFrameBuffers();

    void beginFrame(CRunApp * app);
    void beginLayer(int inkEffect);
    void layerToFrame(int shaderIndex, int inkEffect, int inkEffectParam);
    void renderAScene(CRunFrame* frame);
    void renderToScreen(bool bResampling, int bckcolor, int shaderIndex, int inkEffect, int inkEffectParam);
    void renderBackground(int x, int y, int w, int h);
    void renderColor(int x, int y, int w, int h, int color);

	bool resizeFromLayer(CAEAGLLayer* layer);

	void clear(float red, float green, float blue);
	void clear();
	void clearWithRunApp(CRunApp* app);
	void swapBuffers();
	void flush();
	void forgetCachedState();
	void forgetShader();

    void setCurrentShader(CShader* shader);

	void bindRenderBuffer();
	void updateViewport();
	void setViewport(Viewport viewport);

    void resetTransformation();
    void setCurrentLayerByBool(bool mode);
    void setCurrentLayer(CLayer* layer);

    void setInkBlendEffect(int effect, int effectParam, CShader* pShader);
    void setProjectionMatrix(int x, int y, int width, int height, bool flipY=false);
	void setTransformMatrix(const Mat3f matrix);
    void flipProjection();
    void restoreProjection();

    void setSurfaceTextureAtIndex(CTexture* image, NSString* name, int index);
    void updateSurfaceTexture();

	void renderSimpleImage(int x, int y, int w, int h);
	void renderImage(CTexture* image, int x, int y, int w, int h, int shaderIndex = -1, int inkEffect = 1, int inkEffectParam = -1);
	void renderPattern(CTexture* image, int x, int y, int w, int h, int shaderIndex, int inkEffect, int inkEffectParam);
	void renderPattern(CTexture* image, int x, int y, int w, int h, int shaderIndex, int inkEffect, int inkEffectParam, bool flipX, bool flipY, float scaleX = 1.0f, float scaleY = 1.0f);
    void renderPatternEllipse(CTexture* image, int x, int y, int w, int h, int shaderIndex, int inkEffect, int inkEffectParam);
    void renderPatternEllipse(CTexture* image, int x, int y, int w, int h, int shaderIndex, int inkEffect, int inkEffectParam, bool flipX, bool flipY, float scaleX = 1.0f, float scaleY = 1.0f);

	void renderScaledRotatedImage(CTexture* image, float angle, float sX, float sY, int hX, int hY, int x, int y, int w, int h, int shaderIndex = -1, int inkEffect = 0, int inkEffectParam = -1);
    void renderScaledRotatedImageWrapAndFlip(CTexture* image, float angle, float sX, float sY, int hX,
                                            int hY, int x, int y, int w, int h, int offsetX, int offsetY, int wrap, int flipH, int flipV,
                                             int resample, int transp, int shaderIndex = -1, int inkEffect = 0, int inkEffectParam = -1);
	void renderLine(Vec2f a, Vec2f b, int color, float thickness);

	void renderGradient(GradientColor gradient, int x, int y, int w, int h, int shaderIndex, int inkEffect, int inkEffectParam);
	void renderGradient(GradientColor gradient, CRect rect, int inkEffect, int inkEffectParam);
	void renderSolidColor(int color, int x, int y, int w, int h, int inkEffect, int inkEffectParam);
	void renderSolidColor(int color, CRect rect, int inkEffect, int inkEffectParam);
    void renderGradientEllipse(GradientColor gradient, int x, int y, int w, int h, int shaderIndex, int inkEffect, int inkEffectParam);
    void renderRect(int x, int y, int w, int h, int color, int thickness, int inkEffect, int inkEffectParam);

    void renderPerspective(CRenderToTexture* image, int x, int y, int w, int h, float fA, float fB, int dir, int inkEffect, int inkEffectParam);
    void renderSinewave(CRenderToTexture* image, int x, int y, int w, int h, float zoom, float wave, float offset, int dir, int inkEffect, int inkEffectParam);
	//For transitions
	void setOrigin(int x, int y);
	void renderBlitFull(CRenderToTexture* source);
	void renderBlit(CRenderToTexture* source, int xDst, int yDst, int xSrc, int ySrc, int width, int height);
	void renderStretch(CRenderToTexture* source, int xDst, int yDst, int wDst, int hDst, int xSrc, int ySrc, int wSrc, int hSrc, int shaderIndex = -1, int inkEffect = 0, int inkEffectParam = 0);
	void renderFade(CRenderToTexture* source, int alpha);

    void draw();
    void drawBackground(bool clearBck, bool flip);

	void useBlending(BOOL useBlending);
    BOOL getBlending();

	void setBlendEquation(GLenum equation);
	void setBlendEquationSeparate(GLenum equationA, GLenum equationB);
	void setBlendFunction(GLenum sFactor, GLenum dFactor);
    void setBlendFunctionSeparate(GLenum sFactor, GLenum dFactor, GLenum sFactorAlpha, GLenum dFactorAlpha);
	void setBlendColor(float red, float green, float blue, float alpha);

	//Clipping
	void setClip(int x, int y, int w, int h);
	void resetClip();

	static void checkForError();
	static CRenderer* getRenderer();

	void uploadTexture(CTexture* texture);
	void removeTexture(CTexture* texture, BOOL cleanMemory);
	void cleanMemory();
	void cleanUnused();
	void pruneTexture();
	void clearPruneList();

    // to be used with RTT
    void screenAreaToTexture(CRenderToTexture* rtt, int x , int y, int w, int h, int mode);
    void screenPixelsToTexture(CTexture* texture, int x , int y, int w, int h);
    UIImage* screenAreaToImage(int x , int y, int width, int height);

    void copyRttToRtt(CRenderToTexture* src, CRenderToTexture* dest, int effect, int effectParam, int flipY);
    void copyRttToTextureAt(CRenderToTexture *src, CTexture *texture, int x, int y);
    void filterLayerRtt(CRenderToTexture *src, CRenderToTexture *dest, float comp);


    //Shader operation
    int addShader(NSString* shaderName, NSString* vertexShader, NSString* fragmentShader, NSArray* shaderVariables, bool useTexCoord, bool useColors);
    int addShader(NSString* shaderName, NSArray* shaderVariables, bool useTexCoord, bool useColors);
    void removeShader(int shaderIndex);
    void removeAllShaders();
    void setEffectShader(int shaderIndex);
    void setEffectShaderByIndex(int shaderIndex);
    void removeEffectShader();

    void pushScene(int x, int y, int w, int h);
    void popScene();

    //Use Background
    void setBackgroundUse(int shaderIndex);
    //Use PixelSize
    void setPixelSizeUse(int shaderIndex);
    void setCurrentView(int x, int y, float sX, float sY);
    UIImage* readFBO(int texScene);
    UIImage* readFBO(RenderingState* currentState);
    UIImage* readAreaToImage(int x, int y, int w, int h);
    UIImage* readTextureToImage(GLuint textureID, int w, int h);

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

    GLuint getOrCreatePoolGLTexture(int width, int height, int glMode=GL_RGBA);
    void releaseGLPoolTexture(GLuint textureID);
    void deleteGLPoolTexture(GLuint textureID);
    void clearGLPoolTexture();

    CTexture* getOrCreatePoolCTexture(int width, int height, bool resampling);
    void releasePoolCTexture(CTexture* texture);
    void clearPoolCTexture();

    CRenderToTexture* getOrCreatePoolRenderTexture(int width, int height, bool swapCoord, bool swapProj, int openGLmode = GL_RGBA);
    void releasePoolRenderTexture(CRenderToTexture* rtt);
    void deletePoolRenderTexture(CRenderToTexture* rtt);
    void clearPoolRenderTexture();

    int getOrCreatePoolSubScene(int w, int h);
    void releasePoolSubScene(int idx);
    void clearPoolSubScene();

    void Mat3fXVec3f(const float mat[9], const float vec[3], float result[3]);

};
