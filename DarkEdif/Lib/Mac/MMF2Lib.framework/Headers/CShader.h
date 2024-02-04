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
//  CShader.h
//  RuntimeIPhone
//
//  Created by Anders Riggelsen on 6/10/10.
//  Copyright 2010 Clickteam. All rights reserved.
//

#pragma once
#import <Foundation/Foundation.h>
#import "CoreMath.h"

@class CTexture;

class CRenderer;

enum {
    UNIFORM_TEXTURE,
    UNIFORM_PROJECTIONMATRIX,
    UNIFORM_TRANSFORMMATRIX,
    UNIFORM_OBJECTMATRIX,
    UNIFORM_TEXTUREMATRIX,
    UNIFORM_INKEFFECT,
    UNIFORM_RGBA,
    UNIFORM_GRADIENT,
    UNIFORM_VAR1,
    UNIFORM_VAR2,
    UNIFORM_VAR3,
    UNIFORM_VAR4,
    UNIFORM_VAR5,
    UNIFORM_VAR6,
    UNIFORM_VAR7,
    UNIFORM_VAR8,
    UNIFORM_VAR9,
    UNIFORM_VAR10,
    UNIFORM_VAR11,
    UNIFORM_VAR12,
    UNIFORM_VAR13,
    UNIFORM_VAR14,
    UNIFORM_VAR15,
    UNIFORM_VAR16,
    UNIFORM_VAR17,
    UNIFORM_VAR18,
    UNIFORM_VAR19,
    UNIFORM_VAR20,
    NUM_UNIFORMS
};

#define ATTRIB_VERTEX 0

#define NUM_XTRATEX 8

class CShader
{
public:
	GLuint program;
	GLuint fragmentProgram;
	GLuint vertexProgram;

	int uniforms[NUM_UNIFORMS];
	BOOL usesTexCoord;
	BOOL usesColor;
	CRenderer* render;

	Mat3f prevTransform;
	Mat3f prevProjection;
	Mat3f prevTexCoord;

	int currentEffect;
	float currentR, currentG, currentB, currentA;
	NSString* sname;

    int extraTexID[NUM_XTRATEX];
    BOOL hasExtras;

	CShader(CRenderer* renderer);
	~CShader();

    void checkError();

	bool loadShader(NSString* name, NSString* vertexShader, NSString* fragmentShader, bool useTexCoord, bool useColors);
	bool loadShader(NSString* shaderName, bool useTexCoord, bool useColors);

	GLuint compileShader(GLuint* shader, NSString* shaderSource, GLint type);
	bool linkProgram(GLuint prog);
	bool validateProgram(GLuint prog);
    void detachShader();

	void setTexture(CTexture* texture);
	void setTexture(CTexture* texture, Mat3f &textureMatrix);
	void setTexCoord(Mat3f &texCoord);
	void setRGBCoeff(float red, float green, float blue, float alpha);
	void setInkEffect(int effect);
	void forgetCachedState();

	void bindShader();
    void unbindShader();
    void bindVertexArray();
    void unbindVertexArray();
	void setProjectionMatrix(const Mat3f &matrix);
	void setTransformMatrix(const Mat3f &matrix);
	void setObjectMatrix(const Mat3f &matrix);

    void setSurfaceTextureAtIndex(CTexture* texture, const GLchar* name, int index);
    void updateSurfaceTexture();

	void setGradientColors(int color);
	void setGradientColors(int a, int b, BOOL horizontal);
	void setGradientColors(int a, int b, int c, int d);
	void setGradientColors(GradientColor gradient);

    void setVariable1i(const GLchar* field, int value);
    void setVariable1f(const GLchar* field, float value);
    void setVariable2i(const GLchar* field, int value0, int value1);
    void setVariable2f(const GLchar* field, float value0, float value1);
    void setVariable3i(const GLchar* field, int value0, int value1, int value2);
    void setVariable3f(const GLchar* field, float value0, float value1, float value2);
    void setVariable4i(const GLchar* field, int value0, int value1, int value2, int value3);
    void setVariable4f(const GLchar* field, float value0, float value1, float value2, float value3);

    void setVariable1i(int nfield, int value);
    void setVariable1f(int nfield, float value);
    void setVariable2i(int nfield, int value0, int value1);
    void setVariable2f(int nfield, float value0, float value1);
    void setVariable3i(int nfield, int value0, int value1, int value2);
    void setVariable3f(int nfield, float value0, float value1, float value2);
    void setVariable4i(int nfield, int value0, int value1, int value2, int value3);
    void setVariable4f(int nfield, float value0, float value1, float value2, float value3);


};



