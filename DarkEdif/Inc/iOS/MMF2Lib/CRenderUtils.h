/* Copyright (c) 1996-2023 Clickteam
 *
 * This source code is part of the Android exporter for Clickteam Multimedia Fusion 2.
 *
 * Permission is hereby granted to any person obtaining a legal copy
 * of Clickteam Multimedia Fusion 2 to use or modify this source code for
 * debugging, optimizing, or customizing applications created with
 * Clickteam Multimedia Fusion 2.  Any other use of this source code is prohibited.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#ifndef CRENDERUTILS_H
#define CRENDERUTILS_H

#include <stack>
#include <vector>

#include "CShader.h"
#include "CRenderToTexture.h"

inline unsigned char GetA(unsigned int rgb) {
    return (rgb >> 24) & 0xFF;
}

inline unsigned char GetR(unsigned int rgb) {
    return (rgb >> 16) & 0xFF;
}

inline unsigned char GetG(unsigned int rgb) {
    return (rgb >> 8) & 0xFF;
}

inline unsigned char GetB(unsigned int rgb) {
    return rgb & 0xFF;
}

inline float GetRed (unsigned int rgb)
{   return ((float) ((rgb >> 16) & 0xFF)) / 255.0f;
}

inline float GetGreen (unsigned int rgb)
{   return ((float) ((rgb >> 8) & 0xFF)) / 255.0f;
}

inline float GetBlue (unsigned int rgb)
{   return ((float) (rgb & 0xFF)) / 255.0f;
}

inline int calculateScale(int appD, int textSize, int screenD) {
    int scaleToFitText = textSize / appD;
    int scaleToMeetScreen = textSize / screenD;

    int chosenScale = MIN(scaleToFitText, scaleToMeetScreen);

    for (int scale = chosenScale; scale >= 1; --scale) {
        if (appD * scale < screenD) {
            return ++scale;
        }
    }

    // Unable to find a valid scale, return a default value or handle the case accordingly
    return 1; // Default to scale of 1
}

inline unsigned int swapColor(unsigned int rgb) {
    char r = (((rgb >> 16) & 0xFF));
    char g = (((rgb >> 8) & 0xFF));
    char b = ((rgb & 0xFF));
    char a = 0xFF;
    return (int) (a << 24 | b << 16 | g << 8 | r);
}


class Viewport
{
public:
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
	GLuint framebuffer;

    bool actProjstate;
    bool oldProjstate;
    bool newTransform;

	Vec2f contentSize;	//The size of the underlying game in game-units
	Mat3f transform;	//The transform matrix of this viewport (scroll, zoom, rotation ect. ect.)
	Mat3f projection;	    //The projection matrix of this viewport (based on the contentSize)
};

class SceneRender
{
public:
    CRenderToTexture* frameRtt;
    CRenderToTexture* layerRtt;
    CRenderToTexture* back1Rtt;

    int index, x, y, width, height;
    bool subScene, onLayer, onAlpha, onFrame, onScreen;
    bool bEffLayers, bEffFrame;
    int color, alpha;


    int colorRGBA();

    void reset();
};

class GLTexturePool
{
public:
    GLuint textureID;
    int width;
    int height;
    int mode;
    bool isUsed; // Indicates if the texture is currently being used
};

class CTexturePool
{
public:
    CTexture* texture;
    GLuint textureID;
    int width;
    int height;
    bool resampling;
    bool isUsed; // Indicates if the texture is currently being used
};

class RenderToTexturePool
{
public:
    CRenderToTexture* rtt;
    int width;
    int height;
    int mode;
    bool swapCoord;
    bool swapProj;

    bool isUsed; // Indicates if the texture is currently being used
};

class SubScenePool
{
public:

    int index;
    int w;
    int h;
    CRenderToTexture* frameRtt;
    CRenderToTexture* layerRtt;
    CRenderToTexture* back1Rtt;
    bool isUsed; // Indicates if the texture is currently being used
};

class Scissor
{
public:
    int x, y, w, h;
    void setScissor(GLint sb[4]);
};

class CShadersVector {
public:
    int AddShader(CShader* shader);
    void RemoveShaderByIndex(int index);
    void RemoveAllShaders();
    CShader* SearchShaderByIndex(int index) const;
    int LastPushedShaderIndex() const;
    int Size() const;
    
    void askEffectsToForgetCache();

private:
    std::vector<CShader*> shaderPointers;
};


#endif
