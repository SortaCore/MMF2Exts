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
//  CBox2D.h
//  RuntimeIPhone
//
//  Created by Francois Lionet on 14/11/13.
//  Copyright (c) 2013 Clickteam. All rights reserved.
//
#import <Foundation/Foundation.h>
#import "Box2D.h"
#import "CRunExtension.h"
#import "CRunMvtExtension.h"
#import "CRect.h"
#import "CArrayList.h"

typedef unsigned int DWORD;
typedef unsigned short WORD;
@class CRun;
@class CObject;
@class CSprite;

#define BASEIDENTIFIER 0x42324547
#define FANIDENTIFIER 0x42324641
#define TREADMILLIDENTIFIER	0x4232544D
#define MAGNETIDENTIFIER 0x42369856
#define GROUNDIDENTIFIER 0x42324E4F
#define PARTICULESIDENTIFIER 0x42326AF3
#define ROPEANDCHAINIDENTIFIER 0x4232EFFA

#define MTYPE_OBJECT 0
#define MTYPE_PARTICULE 1
#define MTYPE_ELEMENT 2
#define MTYPE_BORDERLEFT 3
#define MTYPE_BORDERRIGHT 4
#define MTYPE_BORDERTOP 5
#define MTYPE_BORDERBOTTOM 6
#define MTYPE_OBSTACLE 7
#define MTYPE_PLATFORM 8
#define MTYPE_FAKEOBJECT 9

#define MSUBTYPE_OBJECT 0
#define MSUBTYPE_BOTTOM 1
#define MSUBTYPE_TOP 2
#define MSUBTYPE_LEFT 3
#define MSUBTYPE_RIGHT 4


class ContactListener : public b2ContactListener
{
public:
    BOOL bWorking;
    ContactListener()
    {
        bWorking = NO;
    }
	void BeginContact(b2Contact* contact){ }
	void EndContact(b2Contact* contact){ }
	void PreSolve(b2Contact* contact, const b2Manifold* oldManifold);
	void PostSolve(b2Contact* contact, const b2ContactImpulse* impulse)	{ }
};

@interface CRunBox2DParent : CRunExtension
{
@public
    void* m_object;
}
-(int)getNumberOfConditions;
-(BOOL)createRunObject:(CFile*)file withCOB:(CCreateObjectInfo*)cob andVersion:(int)version;
-(int)handleRunObject;
-(void)destroyRunObject:(BOOL)bFast;
-(BOOL)condition:(int)num withCndExtension:(CCndExtension*)cnd;
-(void)action:(int)num withActExtension:(CActExtension*)act;
-(CValue*)expression:(int)num;
@end

typedef void (* AddObject)(void* rdPtr, void* pHo);
typedef void (* RemoveObject)(void* rdPtr, void* pHo);
typedef BOOL (* StartObject)(void* rdPtr);
typedef void (* WorldToFrame)(void* rdPtr, b2Vec2* pVect);
typedef void (* FrameToWorld)(void* rdPtr, b2Vec2* pVect);
typedef b2Body* (*CreateBody)(void* ptr, b2BodyType type, int x, int y, float angle, float gravity, void* userData, DWORD flags, float deceleration);
typedef b2Fixture* (* BodyCreateBoxFixture)(void* ptr, b2Body* body, void* base, int x, int y, int sx, int sy, float	density, float friction, float restitution);
typedef b2Fixture* (* BodyCreateCircleFixture)(void* ptr, b2Body* body, void* base, int x, int y, int radius, float density, float friction, float restitution);
typedef b2Fixture* (* BodyCreateShapeFixture)(void* ptr, b2Body* body, void* base, int x, int y, unsigned short image, float density, float friction, float restitution, float scaleX, float scaleY);
typedef void (*GetBodyPosition)(void* rdPtr, b2Body* body, int* pX, int* pY, float* pAngle);
typedef void (* BodyApplyForce)(void* ptr, b2Body* pBody, float force, float angle);
typedef void (* BodyApplyImpulse)(void* ptr, b2Body* pBody, float force, float angle);
typedef void (* BodyApplyMMFImpulse)(void* ptr, b2Body* pBody, float force, float angle);
typedef void (* BodySetLinearVelocity)(void* ptr, b2Body* pBody, float force, float angle);
typedef void (* BodySetLinearVelocityAdd)(void* ptr, b2Body* pBody, float force, float angle, float vx, float vy);
typedef void (* GetImageDimensions)(void* ptr, unsigned short img, int* x1, int* x2, int* y1, int* y2);
typedef void (* DestroyBody)(void* ptr, b2Body* body);
typedef void (* CreateDistanceJoint)(void* ptr, b2Body* pBody1, b2Body* pBody2, float dampingRatio, float frequency, int x, int y);
typedef void (* BodyCreatePlatformFixture)(void* ptr, b2Body* pBody, void* base, unsigned short img, int x, int y, float density, float friction, float restitution, b2Fixture** pFixture, int* pOffsetX, int* pOffsetY, float scaleX, float scaleY, float maskWidth);
typedef void (* BodyAddVelocity)(void* ptr, b2Body* pBody, float vx, float vy);
typedef b2Body*(* CreateBullet)(void* ptr, float angle, float speed, void* userData);
typedef void (* BodySetTransform)(void* ptr, b2Body* pBody, b2Vec2 position, float angle);
typedef void (* BodyAddLinearVelocity)(void* ptr, b2Body* pBody, float speed, float angle);
typedef void (* BodySetPosition)(void* ptr, b2Body* pBody, int x, int y);
typedef void (* BodySetAngle)(void* ptr, b2Body* pBody, float angle);
typedef void (* BodyResetMassData)(void* ptr, b2Body* pBody);
typedef void (* BodyApplyAngularImpulse)(void* ptr, b2Body* pBody, float torque);
typedef void (* BodyApplyAngularImpulse)(void* ptr, b2Body* pBody, float torque);
typedef void (* BodyApplyTorque)(void* ptr, b2Body* pBody, float torque);
typedef void (* BodySetAngularVelocity)(void* ptr, b2Body* pBody, float torque);
typedef void (* BodyStopForce)(void* ptr, b2Body* pBody);
typedef void (* BodyStopTorque)(void* ptr, b2Body* pBody);
typedef void (* AddNormalObject)(void* ptr, CObject* pHo);
typedef void (* BodyDestroyFixture)(void* ptr, b2Body* pBody, b2Fixture* pFixture);
typedef b2Joint* (* WorldCreateRevoluteJoint)(void* ptr, b2RevoluteJointDef* jointDef, b2Body* body1, b2Body* body2, b2Vec2 position);
typedef void (* BodySetLinearVelocityVector)(void* ptr, b2Body* pBody, b2Vec2 velocity);
typedef b2Vec2 (* BodyGetLinearVelocity)(void* ptr, b2Body* pBody);
typedef void (* BodySetLinearDamping)(void* ptr, b2Body* pBody, float deceleration);
typedef b2Vec2 (* BodyGetPosition)(void* ptr, b2Body* pBody);
typedef float (* BodyGetAngle)(void* ptr, b2Body* pBody);
typedef void (* BodySetGravityScale)(void* ptr, b2Body* pBody, float gravity);
typedef void (* FixtureSetRestitution)(void* ptr, b2Fixture* pBody, float restitution);
typedef void (* BodySetAngularDamping)(void* ptr, b2Body* pBody, float damping);
typedef float (* BodyGetMass)(void* ptr, b2Body* pBody);
typedef void* (* GetMBaseRout)(void* rdPtr, CObject* pHo);
typedef void (* DestroyJoint)(void* rdPtr, b2Joint* joint);
typedef void (* BodySetFixedRotation)(void* ptr, b2Body* pBody, BOOL flag);
typedef b2Joint* (* JointCreate)(void* ptr, void* pBase1, short jointType, short jointAnchor, NSString* jointName, NSString* jointObject, float param1, float param2);
typedef b2Body* (* AddABackdrop)(void* ptr, int x, int y, unsigned short img, int obstacle);
typedef void (* SubABackdrop)(void* ptr, b2Body* body);
typedef void (* RJointSetMotor)(void* rdPtr, b2RevoluteJoint* pJoint, int t, int s);
typedef void (* RJointSetLimits)(void* rdPtr, b2RevoluteJoint* pJoint, int angle1, int angle2);
typedef BOOL (* AddFan)(void* ptr, void* pObject);
typedef BOOL (* AddMagnet)(void* ptr, void* pObject);
typedef BOOL (* AddTreadmill)(void* ptr, void* pObject);
typedef BOOL (* AddRope)(void* ptr, void* pObject);

typedef struct tagRDATABASE
{
	DWORD identifier;								// 0
	StartObject	pStartObject;
	BodyApplyForce pBodyApplyForce;
	BodyApplyImpulse pBodyApplyImpulse;
	BodyAddVelocity pBodyAddVelocity;
	CreateBullet pCreateBullet;						// 5
	GetBodyPosition pGetBodyPosition;
	DestroyBody	pDestroyBody;
	WorldToFrame			pWorldToFrame;
	FrameToWorld			pFrameToWorld;
	CreateBody				pCreateBody;			// 10
	BodyCreateBoxFixture	pBodyCreateBoxFixture;
	BodyCreateCircleFixture	pBodyCreateCircleFixture;
	BodyCreateShapeFixture	pBodyCreateShapeFixture;
	BodySetLinearVelocity	pBodySetLinearVelocity;
	BodySetLinearVelocityAdd pBodySetLinearVelocityAdd;		// 15
	GetImageDimensions		pGetImageDimensions;
	CreateDistanceJoint		pCreateDistanceJoint;
	BodyCreatePlatformFixture pBodyCreatePlatformFixture;
	BodySetTransform		pBodySetTransform;
	BodyAddLinearVelocity	pBodyAddLinearVelocity;			// 20
	BodySetPosition			pBodySetPosition;
	BodySetAngle			pBodySetAngle;
	BodyResetMassData		pBodyResetMassData;
	BodyApplyAngularImpulse pBodyApplyAngularImpulse;
	BodyApplyTorque			pBodyApplyTorque;				// 25
	BodySetAngularVelocity	pBodySetAngularVelocity;
	BodyStopForce			pBodyStopForce;
	BodyStopTorque			pBodyStopTorque;
	AddNormalObject			pAddNormalObject;				// 29
	BodyDestroyFixture		pBodyDestroyFixture;			// 30
	WorldCreateRevoluteJoint	pWorldCreateRevoluteJoint;
	BodySetLinearVelocityVector	pBodySetLinearVelocityVector;
	BodyGetLinearVelocity	pBodyGetLinearVelocity;
	BodySetLinearDamping	pBodySetLinearDamping;
	BodyGetPosition			pBodyGetPosition;
	BodyGetAngle			pBodyGetAngle;
	BodySetGravityScale		pBodySetGravityScale;
	FixtureSetRestitution	pFixtureSetRestitution;
	BodySetAngularDamping	pBodySetAngularDamping;
	BodyGetMass				pBodyGetMass;
	GetMBaseRout			pGetMBase;
	DestroyJoint			pDestroyJoint;
	BodyApplyMMFImpulse		pBodyApplyMMFImpulse;
	BodySetFixedRotation	pBodySetFixedRotation;
    JointCreate				pJointCreate;
    AddABackdrop            pAddABackdrop;
    SubABackdrop            pSubABackdrop;
    RJointSetLimits         pRJointSetLimits;
    RJointSetMotor          pRJointSetMotor;
    AddFan		            pAddFan;
    AddMagnet	            pAddMagnet;
    AddTreadmill            pAddTreadmill;
    AddRope					pAddRope;
    
	float			angle;
	DWORD			flags;
	b2World*		world;
	float			gravity;
	float			factor;
	int				angleBase;
	int				xBase;
	int				yBase;
	int				velocityIterations;
	int				positionIterations;
	float			friction;
	float			restitution;
	int				shapeStep;
	ContactListener*		contactListener;
	BOOL			started;
	float			bulletGravity;
	float			bulletDensity;
	float			bulletRestitution;
	float			bulletFriction;
	CCArrayList*    objects;
	CCArrayList*	objectIDs;
	CCArrayList*	joints;
	CCArrayList*	forces;
	CCArrayList*	torques;
	CCArrayList*	treadmills;
	CCArrayList*    fans;
	CCArrayList*    magnets;
    CCArrayList*    backgroundBases;
    CCArrayList*    bodiesToDestroy;
	CCArrayList*	ropes;
	float			npDensity;
	float			npFriction;
    CRun*           rh;
    CExtension*     ho;
    bool            bPaused;
    bool            bListener;
} RUNDATABASE;
typedef	RUNDATABASE	*			LPRDATABASE;

typedef struct tagRDATABOX2DPARENT
{
	AddObject		pAddObject;
	RemoveObject	pRemoveObject;
	StartObject		pStartObject;
    DWORD           identifier;
}RUNDATABOX2DPARENT;
typedef	RUNDATABOX2DPARENT*			LPRDATABOX2DPARENT;

#define LPHO CObject*
#define LPRH CRun*
class CRunMvt
{
public:
	virtual ~CRunMvt() {}
	virtual void		Initialize(LPHO pHo, CFile* file) {}
    virtual void		Delete(void) { delete this; }
    
	virtual BOOL		Move(CObject* pHo) { return NO; }
	virtual void		SetPosition(CObject* pHo, int x, int y) {}
	virtual void		SetXPosition(CObject* pHo, int x) {}
	virtual void		SetYPosition(CObject* pHo, int y) {}
    
	virtual void		Stop(CObject* pHo, BOOL bCurrent) {}
	virtual void		Bounce(CObject* pHo, BOOL bCurrent) {}
	virtual void		Reverse(CObject* pHo) {}
	virtual void		Start(CObject* pHo) {}
	virtual void		SetSpeed(CObject* pHo, int speed) {}
	virtual void		SetMaxSpeed(CObject* pHo, int speed) {}
	virtual void		SetDir(CObject* pHO, int dir) {}
	virtual void		SetAcc(CObject* pHo, int acc) {}
	virtual void		SetDec(CObject* pHo, int dec) {}
	virtual void		SetRotSpeed(CObject* pHo, int speed) {}
	virtual void		Set8Dirs(CObject* pHo, int dirs) {}
	virtual void		SetGravity(CObject* hoPtr, int gravity) {}
	virtual double		ActionEntry(CObject* hoPtr, int action, double param1, double param2) {return 0;}
	virtual int			GetSpeed(CObject* hoPtr) {return 0;}
	virtual int			GetAcceleration(CObject* hoPtr) {return 0;}
	virtual int			GetDeceleration(CObject* hoPtr) {return 0;}
	virtual int			GetGravity(CObject* hoPtr) {return 0;}
	virtual int			GetDir(CObject* hoPtr) {return 0;}
public:
	CRunMvt() {}
};

class CRunMBase;
class CRunMvtPhysics : CRunMvt
{
public:
    CRunMBase*          m_mBase;
    DWORD               m_identifier;
    CRunMvtExtension*   m_movement;
    
public:
    virtual void		Initialize(CFile* file, CObject* hoPtr) {}
    virtual void		Delete(void) { delete this; }
	virtual BOOL		Move(CObject* pHo) { return NO; }
	virtual void		SetPosition(CObject* pHo, int x, int y) {}
	virtual void		SetXPosition(CObject* pHo, int x) {}
	virtual void		SetYPosition(CObject* pHo, int y) {}
    
	virtual void		Stop(CObject* pHo, BOOL bCurrent) {}
	virtual void		Bounce(CObject* pHo, BOOL bCurrent) {}
	virtual void		Reverse(CObject* pHo) {}
	virtual void		Start(CObject* pHo) {}
	virtual void		SetSpeed(CObject* pHo, int speed) {}
	virtual void		SetMaxSpeed(CObject* pHo, int speed) {}
	virtual void		SetDir(CObject* pHO, int dir) {}
	virtual void		SetAcc(CObject* pHo, int acc) {}
	virtual void		SetDec(CObject* pHo, int dec) {}
	virtual void		SetRotSpeed(CObject* pHo, int speed) {}
	virtual void		Set8Dirs(CObject* pHo, int dirs) {}
	virtual void		SetGravity(CObject* hoPtr, int gravity) {}
	virtual double		ActionEntry(CObject* hoPtr, int action, double param1, double param2) {return 0;}
	virtual int			GetSpeed(CObject* hoPtr) {return 0;}
	virtual int			GetAcceleration(CObject* hoPtr) {return 0;}
	virtual int			GetDeceleration(CObject* hoPtr) {return 0;}
	virtual int			GetGravity(CObject* hoPtr) {return 0;}
	virtual int			GetDir(CObject* hoPtr) {return 0;}
    
	virtual void SetCollidingObject(CRunMBase* object){};
	virtual BOOL CreateBody(CObject* pHo){return NO;};
	virtual void CreateJoint(CObject* pHo){};
	virtual void SetFriction(int friction){}
	virtual void SetRestitution(int restitution){}
	virtual void SetGravity(int gravity){}
	virtual void SetDensity(int density){}
	virtual WORD GetType(){return MTYPE_OBJECT;}
	virtual void SetAngle(float angle){}
	virtual float GetAngle(){return 0;}
};
#define ANGLE_MAGIC 123456789.0f

@interface CRunMvtBox2D : CRunMvtExtension
{
@public
	CRunMvtPhysics* m_movement;
}
-(void)initialize:(CFile*)file;
-(void)kill;
-(BOOL)move;
-(void)setPosition:(int)x withY:(int)y;
-(void)setXPosition:(int)x;
-(void)setYPosition:(int)y;
-(void)stop:(BOOL)bCurrent;
-(void)bounce:(BOOL)bCurrent;
-(void)start;
-(void)setSpeed:(int)speed;
-(void)setMaxSpeed:(int)speed;
-(void)setDir:(int)dir;
-(void)setAcc:(int)acc;
-(void)setDec:(int)dec;
-(void)setRotSpeed:(int)speed;
-(void)setGravity:(int)gravity;
-(double)actionEntry:(int)action;
-(int)getSpeed;
-(int)getAcceleration;
-(int)getDeceleration;
-(int)getGravity;
-(int)getDir;

@end

class CRunMBase
{
public:
	DWORD m_identifier;
	BOOL m_stopFlag;
	CObject* m_pHo;
	b2Body* m_body;
	float m_currentAngle;
	DWORD m_eventCount;
    unsigned short m_image;
	BOOL stopped;
	DWORD m_type;
	DWORD m_subType;
	float m_addVX;
	float m_addVY;
	BOOL m_addVFlag;
	LPRDATABASE m_base;
	void* m_particule;
	void* m_element;
	CRunMvtPhysics* m_movement;
	float m_setVX;
	float m_setVY;
	BOOL m_setVFlag;
	BOOL m_platform;
	CRect rc;
    void ResetAddVelocity();
    void SetVelocity(float vx, float vy);
    void AddVelocity(float vx, float vy);
    void SetStopFlag(BOOL flag);
    BOOL IsStop();
    void PrepareCondition();
    BOOL m_background;
    CRunMBase(LPRDATABASE base, CObject* pHo, WORD type);    
};

//#endif
