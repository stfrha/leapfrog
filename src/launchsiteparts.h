
#include "oxygine-framework.h"
#include "Box2DDebugDraw.h"
#include "Box2D/Box2D.h"
#include "flameemitter.h"
#include "collisionentity.h"


DECLARE_SMART(LsMainBooster, spLsMainBooster);

class LsMainBooster : public oxygine::Sprite, CollisionEntity
{
public:
	LsMainBooster(Resources& gameResources, b2World* world, const oxygine::Vector2& pos,  int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LsFoundation, spLsFoundation);

class LsFoundation : public oxygine::Sprite, CollisionEntity
{
public:
   LsFoundation(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LsPullSpring, spLsPullSpring);

class LsPullSpring : public oxygine::Sprite, CollisionEntity
{
public:
   LsPullSpring(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LsSupportBooster, spLsSupportBooster);

class LsSupportBooster : public oxygine::Sprite, CollisionEntity
{
public:
   LsSupportBooster(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LsTower, spLsTower);

class LsTower : public oxygine::Sprite, CollisionEntity
{
public:
   LsTower(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};


DECLARE_SMART(LsRightLegRest, spLsRightLegRest);

class LsRightLegRest : public oxygine::Sprite, CollisionEntity
{
public:
	LsRightLegRest(Resources& gameResources, b2World* world, const oxygine::Vector2& pos,  int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LsRightTankHolder, spLsRightTankHolder);

class LsRightTankHolder : public oxygine::Sprite, CollisionEntity
{
public:
   LsRightTankHolder(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LsRightTrolley, spLsRightTrolley);

class LsRightTrolley : public oxygine::Sprite, CollisionEntity
{
public:
   LsRightTrolley(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LsRightWire, spLsRightWire);

class LsRightWire : public oxygine::Sprite, CollisionEntity
{
public:
   LsRightWire(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LsLeftLegRest, spLsLeftLegRest);

class LsLeftLegRest : public oxygine::Sprite, CollisionEntity
{
public:
   LsLeftLegRest(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LsLeftTankHolder, spLsLeftTankHolder);

class LsLeftTankHolder : public oxygine::Sprite, CollisionEntity
{
public:
   LsLeftTankHolder(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LsLeftTrolley, spLsLeftTrolley);

class LsLeftTrolley : public oxygine::Sprite, CollisionEntity
{
public:
   LsLeftTrolley(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LsLeftWire, spLsLeftWire);

class LsLeftWire : public oxygine::Sprite, CollisionEntity
{
public:
   LsLeftWire(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};







