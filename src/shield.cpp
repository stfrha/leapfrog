#include "shield.h"

#include "gamestatus.h"

using namespace oxygine;

Shield::Shield(Resources& gameResources, b2World* world, const Vector2& pos)
{
   m_resAnim = gameResources.getResAnim("shield_exciter");
   setAnimFrame(m_resAnim, 0, 0);
   
   // Size need to be set like this since the tween animation will
   // screw up the size otherwise
   setSize(512.0f, 256.0f);
   setScale(16.0f / 512.0f);
   setAlpha(0);
   setAnchor(Vector2(0.5f, 1.0f));
   setPriority(147);


   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

   m_body = world->CreateBody(&bodyDef);

   setUserData(m_body);

   b2CircleShape shape;
   shape.m_radius = 8.0f;

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &shape;
   fixtureDef.density = 0.1f;
   fixtureDef.friction = 0.0f;
   fixtureDef.restitution = 0.3f;
   fixtureDef.filter.categoryBits = 16384;
   fixtureDef.filter.maskBits = 33819;

   m_body->CreateFixture(&fixtureDef);
   m_body->SetUserData(this);

   m_body->GetFixtureList()->SetUserData((CollisionEntity*)this);
}

CollisionEntityTypeEnum Shield::getEntityType(void)
{
   return CET_LF_SHIELD;
}

void Shield::setAngle(float angle)
{
   m_angle = angle;
}

float Shield::getAngle(void)
{
   return m_angle;
}

void Shield::shieldHit(b2Contact* contact, const b2ContactImpulse* impulse)
{
   b2WorldManifold m;
   contact->GetWorldManifold(&m);

   if (contact->GetManifold()->pointCount > 0)
   {
      b2Vec2 collVec = m.points[0] - m_body->GetPosition();

      float angle = atan2(collVec.y, collVec.x) + MATH_PI / 2.0f;

      setAngle(angle);
   }

   setAlpha(255);
   spTween animTween = addTween(TweenAnim(m_resAnim), 500);
   spTween alphaTween = addTween(Actor::TweenAlpha(64), 500);

   animTween->setDoneCallback(CLOSURE(this, &Shield::atHitComplete));




   float normalImpulses = 0.0f;

   for (int i = 0; i < impulse->count; i++)
   {
      normalImpulses += impulse->normalImpulses[i];
   }

   g_GameStatus.deltaShield(-normalImpulses / 1000.0f);

   if (g_GameStatus.getShield() <= 0.0f)
   {
      // Remove shield from collisions
      b2Filter filt;
      filt.categoryBits = 0;
      filt.maskBits = 65535;

      b2Fixture* bf = m_body->GetFixtureList();
      bf->SetFilterData(filt);
   }


}


void Shield::doUpdate(const oxygine::UpdateState& us)
{
}

void Shield::atHitComplete(oxygine::Event* event)
{
   setAlpha(0);
}