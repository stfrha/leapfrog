
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
   m_slowTurningAfterHit(false)
{
   Vector2 oSize = Vector2(24.0f, 12.6f);

   float maxImp = 100000.0f;
   float maxAng = 100000.0f;

   spSprite sprite = new Sprite();

   sprite->setResAnim(gameResources.getResAnim("hammer"));
   sprite->setSize(oSize);
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

   m_steeringBody = new SteeringBody(m_body);

   m_steeringManager = new SteeringManager(m_steeringBody, m_sceneActor);
   m_steeringManager->m_wanderAngle = MATH_PI;

   //// Start velocity
   //b2Vec2 vel = m_body->GetLinearVelocity();
   //b2Vec2 desiredVel = b2Vec2(10.0f, 0.0f);   // Set to 5 m/s
   //b2Vec2 velChange = desiredVel - vel;
   //b2Vec2 impulse = m_body->GetMass() * velChange;
   //m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

   float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1000.0));
   float y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 500.0));

   m_seekTarget = b2Vec2(x, y);

   //// Randomise value between 0 and maxImp
   //float magnitude = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / maxImp));

   //// Randomise value between 0 and 2pi
   //float angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f * MATH_PI));

   //b2Vec2 impulseForce = b2Vec2(maxImp * cos(angle), maxImp * sin(angle));

   //body->ApplyLinearImpulseToCenter(impulseForce, true);
   //


   //m_aheadCircle = new Sprite();
   //m_aheadCircle->setResAnim(gameResources.getResAnim("circle"));
   //m_aheadCircle->setSize(40.0f, 40.0f);
   //m_aheadCircle->setPosition(PhysDispConvert::convert(m_steeringManager->m_debugAhead, 1.0f));
   //m_aheadCircle->setAnchor(0.5f, 0.5f);
   //m_aheadCircle->setTouchChildrenEnabled(false);
   //m_aheadCircle->setPriority(160);
   //m_aheadCircle->attachTo(m_sceneActor);

   //m_ahead2Circle = new Sprite();
   //m_ahead2Circle->setResAnim(gameResources.getResAnim("circle"));
   //m_ahead2Circle->setSize(40.0f, 40.0f);
   //m_ahead2Circle->setPosition(PhysDispConvert::convert(m_steeringManager->m_debugAhead2, 1.0f));
   //m_ahead2Circle->setAnchor(0.5f, 0.5f);
   //m_ahead2Circle->setTouchChildrenEnabled(false);
   //m_ahead2Circle->setPriority(160);
   //m_ahead2Circle->attachTo(m_sceneActor);
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
//   m_steeringManager->wander();

//   m_steeringManager->seek(b2Vec2(100.0f, 450.0f));

   if (((FreeSpaceActor*)m_sceneActor)->m_leapfrogBody != NULL)
   {
      // m_steeringManager->pursuit(((FreeSpaceActor*)m_sceneActor)->m_leapfrogBody);
      m_steeringManager->wanderHunt(((FreeSpaceActor*)m_sceneActor)->m_leapfrogBody, 100.0f);
   }

   m_steeringManager->update();

   if (us.time >= m_stateStartTime + 5000)
   {
      float x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1000.0));
      float y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 500.0));

      m_seekTarget = b2Vec2(x, y);


      m_stateStartTime = us.time;
   }

   float bodyAngle = m_body->GetAngle();
   b2Vec2 toTarget = m_body->GetLinearVelocity();
   float desiredAngle = atan2f(-toTarget.x, toTarget.y) - MATH_PI / 2.0f;

   float nextAngle = bodyAngle + m_body->GetAngularVelocity() / 60.0f;
   float totalRotation = desiredAngle - nextAngle;
   while (totalRotation < -MATH_PI) totalRotation += 2.0f * MATH_PI;
   while (totalRotation >  MATH_PI) totalRotation -= 2.0f * MATH_PI;

   if (fabs(totalRotation) < 5.0f / 180.0f *  MATH_PI)
   {
      // Now a slow turn has found the desired angle for forward
      // movement, lets go back to fast rotations
      if (m_slowTurningAfterHit)
      {
         logs::messageln("Turning fast...");
      }

      m_slowTurningAfterHit = false;
   }

   float desiredAngularVelocity = totalRotation * 60;

   if (m_slowTurningAfterHit)
   {
      float change = 2.5f / 180.0f * MATH_PI; //allow 1 degree rotation per time step
      desiredAngularVelocity = fmin(change, fmax(-change, desiredAngularVelocity));
   }
   float impulse = m_body->GetInertia() * desiredAngularVelocity;// disregard time factor
   m_body->ApplyAngularImpulse(impulse, true);

   //m_aheadCircle->setPosition(PhysDispConvert::convert(m_steeringManager->m_debugAhead, 1.0f));
   //m_aheadCircle->setSize(m_steeringManager->m_debugRadius * 2.0f, m_steeringManager->m_debugRadius * 2.0f);
   //m_ahead2Circle->setPosition(PhysDispConvert::convert(m_steeringManager->m_debugAhead2, 1.0f));
   //m_ahead2Circle->setSize(m_steeringManager->m_debugRadius * 2.0f, m_steeringManager->m_debugRadius * 2.0f);


   //switch (m_state)
   //{
   //case start:
   //   m_stateStartTime = us.time;
   //   m_state = goRight;

   //   break;
   //case goRight:
   //   if (us.time >= m_stateStartTime + 5000)
   //   {
   //      // Start going up
   //      b2Vec2 vel = m_body->GetLinearVelocity();
   //      b2Vec2 desiredVel = b2Vec2(0.0f, -15.0f);   // Set to 5 m/s
   //      b2Vec2 velChange = desiredVel - vel;
   //      b2Vec2 impulse = m_body->GetMass() * velChange;
   //      m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

   //      m_stateStartTime = us.time;
   //      m_state = goUp;
   //   }
   //   break;
   //case goUp:
   //   if (us.time >= m_stateStartTime + 5000)
   //   {
   //      // Start going left
   //      b2Vec2 vel = m_body->GetLinearVelocity();
   //      b2Vec2 desiredVel = b2Vec2(-15.0f, 0.0f);   // Set to 5 m/s
   //      b2Vec2 velChange = desiredVel - vel;
   //      b2Vec2 impulse = m_body->GetMass() * velChange;
   //      m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

   //      m_stateStartTime = us.time;
   //      m_state = goLeft;
   //   }
   //   break;
   //case goLeft:
   //   if (us.time >= m_stateStartTime + 5000)
   //   {
   //      // Start going down
   //      b2Vec2 vel = m_body->GetLinearVelocity();
   //      b2Vec2 desiredVel = b2Vec2(0.0f, 15.0f);   // Set to 5 m/s
   //      b2Vec2 velChange = desiredVel - vel;
   //      b2Vec2 impulse = m_body->GetMass() * velChange;
   //      m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

   //      m_stateStartTime = us.time;
   //      m_state = goDown;
   //   }
   //   break;
   //case goDown:
   //   if (us.time >= m_stateStartTime + 5000)
   //   {
   //      // Start going right
   //      b2Vec2 vel = m_body->GetLinearVelocity();
   //      b2Vec2 desiredVel = b2Vec2(15.0f, 0);   // Set to 5 m/s
   //      b2Vec2 velChange = desiredVel - vel;
   //      b2Vec2 impulse = m_body->GetMass() * velChange;
   //      m_body->ApplyLinearImpulse(impulse, m_body->GetWorldCenter(), true);

   //      m_stateStartTime = us.time;
   //      m_state = goRight;
   //   }
   //   break;
   //}
}

void Hammer::hitByAnything(b2Contact* contact)
{
   m_slowTurningAfterHit = true;
   logs::messageln("Turning slow...");

}