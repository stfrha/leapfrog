#include "shield.h"
#include "sceneactor.h"
#include "compoundobject.h"
#include "bodyuserdata.h"

#include "gamestatus.h"

using namespace oxygine;
using namespace pugi;

class SceneActor;
class CompoundObject;

Shield::Shield(
   Resources* gameResources,
   SceneActor* sceneActor,
   CompoundObject* parentObject,
   b2World* world,
   const xml_node& objectNode,
   int groupIndex) :
   System(gameResources, sceneActor,  world, parentObject)
{
   readShieldNode(objectNode);

   m_resAnim = m_gameResources->getResAnim("shield_exciter");

   m_sprite = new Sprite();

   m_sprite->setAnimFrame(m_resAnim, 0, 0);

   // Size need to be set like this since the tween animation will
   // screw up the size otherwise
   m_sprite->setSize(512.0f, 256.0f);
   m_sprite->setScale(m_radius / 512.0f);
//   shSp->setScale(16.0f / 512.0f);
   m_sprite->setAlpha(0);
   m_sprite->setAnchor(Vector2(0.5f, 1.0f));
   addChild(m_sprite);

   // Seems that only the center of the attachedBody is used.
   // Therefore we don't use origin now, lets see if this 
   // can be corrected
   // b2Vec2 shieldPos = m_attachedBody->GetWorldPoint(m_attachedOrigin);

   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   bodyDef.position = m_attachedBody->GetPosition();

   m_body = world->CreateBody(&bodyDef);

   ActorUserData* aud = new ActorUserData();
   aud->m_body = m_body;
   aud->m_parentCo = parentObject;

   setUserData(aud);

   b2CircleShape shape;
   shape.m_radius = m_radius/2.0f;
//   shape.m_radius = 8.0f;

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &shape;
   fixtureDef.density = 0.1f;
   fixtureDef.friction = 0.0f;
   fixtureDef.restitution = 0.3f;
   fixtureDef.filter.categoryBits = 1;
   fixtureDef.filter.maskBits = 65533;
   fixtureDef.filter.groupIndex = -groupIndex;

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = this;
   bud->m_collisionType = CollisionEntity::shield;

   fixtureDef.userData = (CollisionEntity*)&bud->m_collisionType;

   m_body->CreateFixture(&fixtureDef);
   m_body->SetUserData(bud);

   attachTo(sceneActor);
   setPriority(objectNode.attribute("zLevel").as_int());

   b2RevoluteJointDef shieldJointDef;
   shieldJointDef.bodyA = m_attachedBody;
   shieldJointDef.bodyB = m_body;
   shieldJointDef.localAnchorA.Set(0.0f, 1.0f);
   shieldJointDef.localAnchorB.Set(0.0f, 0.0f);
   shieldJointDef.collideConnected = false;
   shieldJointDef.enableMotor = false;
   m_shieldJoint = (b2RevoluteJoint*)m_world->CreateJoint(&shieldJointDef);

}

void Shield::readShieldNode(const xml_node& objectNode)
{
   m_attachedBody = m_parent->getBody(objectNode.attribute("body").as_string());

   // Seems that only the center of the attachedBody is used.
   // Therefore we don't use origin now, lets see if this 
   // can be corrected
   // m_attachedOrigin.x = objectNode.attribute("emitterOriginX").as_float();
   // m_attachedOrigin.y = objectNode.attribute("emitterOriginY").as_float();

   m_radius = objectNode.attribute("radius").as_float();
}


void Shield::setAngle(float angle)
{
   m_angle = angle;
}

float Shield::getAngle(void)
{
   return m_angle;
}


void Shield::shieldHit(b2Contact* contact)
{
   b2WorldManifold m;
   contact->GetWorldManifold(&m);

   if (contact->GetManifold()->pointCount > 0)
   {
      b2Vec2 collVec = m.points[0] - m_body->GetPosition();

      float angle = atan2(collVec.y, collVec.x) + MATH_PI / 2.0f;

      setAngle(angle);
   }

   m_sprite->setAlpha(255);
   spTween animTween = m_sprite->addTween(TweenAnim(m_resAnim), 500);
   spTween alphaTween = m_sprite->addTween(Actor::TweenAlpha(64), 500);

   animTween->setDoneCallback(CLOSURE(this, &Shield::atHitComplete));
}

void Shield::evaluatShieldDamage(void)
{
   if (m_parent->m_gameStatus->getShield() <= 0.0f)
   {
      // Remove shield from collisions... only, this  didn't work
      b2Filter filt;
      filt.groupIndex = -filt.groupIndex;
      filt.categoryBits = 0;
      b2Fixture* bf = m_body->GetFixtureList();
      bf->SetFilterData(filt);
   }
}

void Shield::shieldHitImpulse(b2Contact* contact, const b2ContactImpulse* impulse)
{
   shieldHit(contact);

   float normalImpulses = 0.0f;

   for (int i = 0; i < impulse->count; i++)
   {
      normalImpulses += impulse->normalImpulses[i];
   }

   m_parent->m_gameStatus->deltaShield(-normalImpulses / 1000.0f);

   evaluatShieldDamage();
}

void Shield::shieldHitByBullet(b2Contact* contact, float bulletEqvDamage)
{

   shieldHit(contact);

   m_parent->m_gameStatus->deltaShield(-25.0f);

   evaluatShieldDamage();
}

void Shield::doUpdate(const oxygine::UpdateState& us)
{
}

void Shield::atHitComplete(oxygine::Event* event)
{
   m_sprite->setAlpha(0);
}