
#include "hammer.h"

#include "sceneactor.h"
#include "freespaceactor.h"
#include "blastemitter.h"


using namespace oxygine;

Hammer::Hammer(
   Resources& gameResources,
   SceneActor* sceneActor,
   b2World* world,
   const Vector2& pos) :
   CompoundObject(sceneActor),
   m_gameResource(&gameResources),
   m_world(world),
   m_sceneActor(sceneActor),
   m_damage(0),
   m_state(start),
   m_steeringManager(NULL)
{
   Vector2 oSize = Vector2(24.0f, 12.6f);

   float maxImp = 100000.0f;
   float maxAng = 100000.0f;

   spSprite sprite = new Sprite();

   sprite->setResAnim(gameResources.getResAnim("hammer"));
   sprite->setSize(oSize);
   sprite->setAnchor(0.0f, 0.0f);
   sprite->setTouchChildrenEnabled(false);
   sprite->attachTo(this);
   sprite->setAnchor(0.5f, 0.5f);


   setPriority(160);

   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   bodyDef.position = PhysDispConvert::convert(pos, 1.0f);
   m_body = world->CreateBody(&bodyDef);

   b2PolygonShape boxShape;
   boxShape.SetAsBox(oSize.x / 2.0f, oSize.y / 2.0f);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &boxShape;
   fixtureDef.density = 1.0f;
   fixtureDef.friction = 1.0f;
   fixtureDef.filter.categoryBits = 8;
   fixtureDef.filter.maskBits = 64703;
   fixtureDef.userData = (CollisionEntity*)this;

   m_body->CreateFixture(&fixtureDef);
   m_body->SetUserData(this);
   setUserData(m_body);

   attachTo(m_sceneActor);

   m_steeringManager = new SteeringManager(m_body);

   // Start going right
   b2Vec2 vel = m_body->GetLinearVelocity();
   b2Vec2 desiredVel = b2Vec2(15.0f, 0);   // Set to 5 m/s
   b2Vec2 velChange = desiredVel - vel;
   b2Vec2 impulse = m_body->GetMass() * velChange;
   m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);


   //// Randomise value between 0 and maxImp
   //float magnitude = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / maxImp));

   //// Randomise value between 0 and 2pi
   //float angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f * MATH_PI));

   //b2Vec2 impulseForce = b2Vec2(maxImp * cos(angle), maxImp * sin(angle));

   //body->ApplyLinearImpulseToCenter(impulseForce, true);
   //
}

CollisionEntityTypeEnum Hammer::getEntityType(void)
{
   return CET_HAMMER;
}

void Hammer::hitByBullet(b2Contact* contact)
{
}

void Hammer::hitShield(b2Contact* contact)
{
}


void Hammer::hitByLepfrog(b2Contact* contact)
{
   // Take damage like two bullets
}



void Hammer::doUpdate(const oxygine::UpdateState& us)
{
   switch (m_state)
   {
   case start:
      m_stateStartTime = us.time;
      m_state = goRight;

      break;
   case goRight:
      if (us.time >= m_stateStartTime + 5000)
      {
         // Start going up
         b2Vec2 vel = m_body->GetLinearVelocity();
         b2Vec2 desiredVel = b2Vec2(0.0f, -15.0f);   // Set to 5 m/s
         b2Vec2 velChange = desiredVel - vel;
         b2Vec2 impulse = m_body->GetMass() * velChange;
         m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

         m_stateStartTime = us.time;
         m_state = goUp;
      }
      break;
   case goUp:
      if (us.time >= m_stateStartTime + 5000)
      {
         // Start going left
         b2Vec2 vel = m_body->GetLinearVelocity();
         b2Vec2 desiredVel = b2Vec2(-15.0f, 0.0f);   // Set to 5 m/s
         b2Vec2 velChange = desiredVel - vel;
         b2Vec2 impulse = m_body->GetMass() * velChange;
         m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

         m_stateStartTime = us.time;
         m_state = goLeft;
      }
      break;
   case goLeft:
      if (us.time >= m_stateStartTime + 5000)
      {
         // Start going down
         b2Vec2 vel = m_body->GetLinearVelocity();
         b2Vec2 desiredVel = b2Vec2(0.0f, 15.0f);   // Set to 5 m/s
         b2Vec2 velChange = desiredVel - vel;
         b2Vec2 impulse = m_body->GetMass() * velChange;
         m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

         m_stateStartTime = us.time;
         m_state = goDown;
      }
      break;
   case goDown:
      if (us.time >= m_stateStartTime + 5000)
      {
         // Start going right
         b2Vec2 vel = m_body->GetLinearVelocity();
         b2Vec2 desiredVel = b2Vec2(15.0f, 0);   // Set to 5 m/s
         b2Vec2 velChange = desiredVel - vel;
         b2Vec2 impulse = m_body->GetMass() * velChange;
         m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

         m_stateStartTime = us.time;
         m_state = goRight;
      }
      break;
   }
}

void Hammer::atDeathOfAsteroid(void)
{
}

