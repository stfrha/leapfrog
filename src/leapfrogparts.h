
#include "oxygine-framework.h"
#include "Box2DDebugDraw.h"
#include "Box2D/Box2D.h"
#include "flameemitter.h"
#include "collisionentity.h"


//DECLARE_SMART(LfBody, spLfBody);
//
//class LfBody : public oxygine::Sprite, CollisionEntity
//{
//public:
//	LfBody(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, float scalem, int16 groupIndex);
//   virtual CollisionEntityTypeEnum getEntityType(void);
//   b2Body * m_body;
//};
//

DECLARE_SMART(LfBooster, spLfBooster);

class LfBooster : public oxygine::Sprite, CollisionEntity
{
public:
	LfBooster(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, float scale, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LfBigLeg, spLfBigLeg);

class LfBigLeg : public oxygine::Sprite, CollisionEntity
{
public:
	LfBigLeg(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, float scale, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LfSmallLeg, spLfSmallLeg);

class LfSmallLeg : public oxygine::Sprite, CollisionEntity
{
public:
	LfSmallLeg(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, float scale, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LfFoot, spLfFoot);

class LfFoot : public oxygine::Sprite, CollisionEntity
{
public:
	LfFoot(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, float scale, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LfRightSteer, spLfRightSteer);

class LfRightSteer : public oxygine::Sprite, CollisionEntity
{
public:
	LfRightSteer(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, float scale, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};

DECLARE_SMART(LfLeftSteer, spLfLeftSteer);

class LfLeftSteer : public oxygine::Sprite, CollisionEntity
{
public:
	LfLeftSteer(Resources& gameResources, b2World* world, const oxygine::Vector2& pos, float scale, int16 groupIndex);
   virtual CollisionEntityTypeEnum getEntityType(void);
   b2Body * m_body;
};


