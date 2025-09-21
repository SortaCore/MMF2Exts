#import <Foundation/Foundation.h>

#define _PI 3.1415926535897932

extern float degreesToRadians(float degrees);
extern float radiansToDegrees(float radians);
extern float Q_rsqrt(float number);

enum TextureQuality {
	RGBA8888,	//32 bit
	RGBA4444,	//16 bit
	RGBA5551,	//16 bit
	RGB888,		//24 bit
	RGB565		//16 bit
};

struct Vec2f {
	float x, y;
	Vec2f();
	Vec2f(float x, float y);
	static float distanceBetweenPositions(const Vec2f a, const Vec2f b);
	static Vec2f interpolate(const Vec2f a, const Vec2f b, double step);
	float distanceTo(const Vec2f point) const;
	float distanceToSquared(const Vec2f point) const;
	float angleTo(const Vec2f other) const;
	bool isCCWtoLine(Vec2f a, Vec2f b) const;
	bool isCCtoLine(Vec2f a, Vec2f b) const;
	float triangleAreaToLine(Vec2f a, Vec2f b) const;
	void normalize();
	Vec2f normalized() const;
	void normaliseFast();
	Vec2f normalizedFast() const;
	Vec2f operator+(const Vec2f &rhs) const;
	Vec2f operator-(const Vec2f &rhs) const;
	Vec2f operator+(const float &rhs) const;
	Vec2f operator-(const float &rhs) const;
	Vec2f operator*(const float rhs) const;
	Vec2f operator/(const float rhs) const;
	Vec2f operator/(const Vec2f &rhs) const;
	bool operator==(const Vec2f &rhs) const;
	bool operator!=(const Vec2f &rhs) const;
};

struct Vec2i {
	int x, y;
	Vec2i();
	Vec2i(int x, int y);
	Vec2i(float x, float y);
	Vec2i(Vec2f fVec);
	Vec2i operator+(const Vec2i &rhs) const;
	Vec2i operator-(const Vec2i &rhs) const;
	Vec2i operator+(const float &rhs) const;
	Vec2i operator-(const float &rhs) const;
	Vec2i operator*(const float rhs) const;
	Vec2i operator/(const float rhs) const;
	Vec2i operator/(const Vec2i &rhs) const;
	bool operator==(const Vec2i &rhs) const;
	bool operator!=(const Vec2i &rhs) const;
};

const Vec2f Vec2fZero(0,0);
const Vec2f Vec2fOne(1,1);
const Vec2i Vec2iZero(0,0);
const Vec2i Vec2iOne(1,1);

struct Mat3f {
	//float a,b,c,    d,e,f,   g,h,i;
	float a,d,g,    b,e,h,	c,f,i;		//Column major order   (rows are "abc, def, ghi")

	static Mat3f identity();
	static Mat3f zero();
	static Mat3f identityFlippedY();
	static Mat3f translationMatrix(float x, float y);
	static Mat3f translationMatrix(float x, float y, float z);
	static Mat3f scaleMatrix(float x, float y);
	static Mat3f scaleMatrix(float x, float y, float z);
	static Mat3f multiply(Mat3f &a, Mat3f &b);
	static Mat3f multiply(Mat3f &a, Mat3f &b, Mat3f &c, Mat3f &d);
	static Mat3f objectMatrix(const Vec2f &position, const Vec2f &size, const Vec2f &center);
	static Mat3f objectRotationMatrix(const Vec2f &position, const Vec2f &size, const Vec2f &scale, const Vec2f &center, float angle);
	static Mat3f orthogonalProjectionMatrix(int x, int y, int w, int h);
    static Mat3f orthogonalProjectionMatrix(int x, int y, int w, int h, bool flipY);
    
	static Mat3f textureMatrix(float x, float y, float width, float height, float textureWidth, float textureHeight);
	static Mat3f textureMatrixFlipped(float x, float y, float width, float height, float imageHeight, float textureWidth, float textureHeight);
	static Mat3f maskspaceToWorldspace(Vec2f position, Vec2f hotspot, Vec2f scale, float angle);
	static Mat3f worldspaceToMaskspace(Vec2f position, Vec2f hotspot, Vec2f scale, float angle);
	static Mat3f maskspaceToMaskspace(Vec2f positionA, Vec2f hotspotA, Vec2f scaleA, float angleA, Vec2f positionB, Vec2f hotspotB, Vec2f scaleB, float angleB);

	bool operator==(const Mat3f &rhs) const;
	bool operator!=(const Mat3f &rhs) const;

	Mat3f transpose() const;
	float determinant() const;
	Mat3f inverted() const;

	Mat3f flippedTexCoord(bool flipX, bool flipY);

	Vec2f transformPoint(Vec2f point) const;
};

struct Mat4f {
    float a,e,i,m,      b,f,j,n,      c,g,k,o,      d,h,l,p;    //Column major order   (rows are "abcd, efgh, ijkl, mnop")
    
    static Mat4f identity();
    static Mat4f zero();
    static Mat4f identityFlippedY();
    static Mat4f translationMatrix(float x, float y);
    static Mat4f translationMatrix(float x, float y, float z);
    static Mat4f scaleMatrix(float x, float y);
    static Mat4f scaleMatrix(float x, float y, float z);
    static Mat4f multiply(Mat4f &a, Mat4f &b);
    static Mat4f multiply(Mat4f &a, Mat4f &b, Mat4f &c, Mat4f &d);
    static Mat4f objectMatrix(const Vec2f &position, const Vec2f &size, const Vec2f &center);
    static Mat4f objectRotationMatrix(const Vec2f &position, const Vec2f &size, const Vec2f &scale, const Vec2f &center, float angle);
    static Mat4f orthogonalProjectionMatrix(int x, int y, int w, int h);
    
    static Mat4f textureMatrix(float x, float y, float width, float height, float textureWidth, float textureHeight);
    static Mat4f textureMatrixFlipped(float x, float y, float width, float height, float imageHeight, float textureWidth, float textureHeight);
    static Mat4f maskspaceToWorldspace(Vec2f position, Vec2f hotspot, Vec2f scale, float angle);
    static Mat4f worldspaceToMaskspace(Vec2f position, Vec2f hotspot, Vec2f scale, float angle);
    static Mat4f maskspaceToMaskspace(Vec2f positionA, Vec2f hotspotA, Vec2f scaleA, float angleA, Vec2f positionB, Vec2f hotspotB, Vec2f scaleB, float angleB);
    
    bool operator==(const Mat4f &rhs) const;
    bool operator!=(const Mat4f &rhs) const;
    
    Mat4f transpose() const;
    float determinant() const;
    Mat4f inverted() const;
    
    Mat4f flippedTexCoord(bool flipX, bool flipY);
    
    Vec2f transformPoint(Vec2f point) const;
};

struct ColorRGBA{
	float r, g, b, a;
	ColorRGBA();
	ColorRGBA(int color);
	ColorRGBA(float red, float green, float blue, float alpha);
	unsigned int getColorAsFormat(int format);
	unsigned int getRGBA8888();
	unsigned int getRGB888();
	unsigned short getRGBA4444();
	unsigned short getRGBA5551();
	unsigned short getRGB565();

	void premultiply();
	void unpremultiply();

	static ColorRGBA mix(ColorRGBA a, ColorRGBA b, float fraction);
};

struct GradientColor{
	ColorRGBA a, b, c, d;
	GradientColor();
	GradientColor(int color);
	GradientColor(ColorRGBA color);
	GradientColor(ColorRGBA a, ColorRGBA b, BOOL horizontal);
	GradientColor(ColorRGBA a, ColorRGBA b, ColorRGBA c, ColorRGBA d);
	GradientColor(int a, int b, int c, int d);
	GradientColor(int a, int b, BOOL horizontal);
	ColorRGBA getColorAtFraction(float x, float y);
    void toArray(float* colors);
};
