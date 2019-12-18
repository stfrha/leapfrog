
#include "launchsite.h"
#include "leapfrog.h"
#include "sceneactor.h"
#include "actoruserdata.h"
#include "launchsiteevents.h"
#include "headdowndisplay.h"

using namespace oxygine;
using namespace std;


LaunchSite::LaunchSite(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   pugi::xml_node& root,
   int groupIndex) :
   CompoundObject(sceneParent, parentObject),
   m_gameResources(&gameResources),
   m_world(world),
   m_sceneParent(sceneParent),
   m_state(idle),
   m_leftFootContact(false),
   m_rightFootContact(false),
   m_angleHoldStartTime(0),
   m_angleHoldDuration(16000)

{
	initCompoundObjectParts(gameResources, sceneParent, sceneParent, parentObject, world, pos, root, string(""), groupIndex);

   m_leftFootRestJoint = (b2RevoluteJoint*)getJoint("leftRestJoint");
   m_rightFootRestJoint = (b2RevoluteJoint*)getJoint("rightRestJoint");
   m_leftHolderJoint = getJoint("leftHolderTankJoint");
   m_rightHolderJoint = getJoint("rightHolderTankJoint");
   m_leftHolderTrolleyJoint = (b2RevoluteJoint*)getJoint("leftHolderJoint");
   m_rightHolderTrolleyJoint = (b2RevoluteJoint*)getJoint("rightHolderJoint");
   m_grabberJoint = (b2PrismaticJoint*)getJoint("grabberSpringJoint");
   m_grabberBody = getBody("grabber");
   m_tankActor = getActor("mainTank");
   m_mainTankBody = getBody("mainTank");
   m_leftBoosterBody = getBody("leftSupportBooster");
   m_rightBoosterBody = getBody("rightSupportBooster");
   m_leftSupportBoosterJoint = (b2WeldJoint*)getJoint("leftSupportBoosterJoint");
   m_rightSupportBoosterJoint = (b2WeldJoint*)getJoint("rightSupportBoosterJoint");

   // Add main engine particle system

   m_boosterFlame = static_cast<FlameEmitter*>(getSystem("boosterFlame"));
   // m_boosterFlame->attachTo(this);

   // Add right steering engine particle system
   m_rightFlame = static_cast<FlameEmitter*>(getSystem("rightFlame"));
   // m_rightFlame->attachTo(this);

   // Add left steering engine particle system
   m_leftFlame = static_cast<FlameEmitter*>(getSystem("leftFlame"));
   // m_leftFlame->attachTo(this);

   // Here we attach Launch Site object to tree so it gets updates etc.
   attachTo(sceneParent);

   m_properties.push_back(ObjectProperty(this, NULL, 2, 0.0f, true)); // State

   b2PrismaticJoint* grabberJoint = (b2PrismaticJoint*)getJoint("grabberSpringJoint");

   //logs::messageln("Grabber translation: %f", grabberJoint->GetJointTranslation());
   //logs::messageln("Grabber lower limit: %f", grabberJoint->GetLowerLimit());
   //logs::messageln("Grabber upper limit: %f", grabberJoint->GetUpperLimit());
   //if (grabberJoint->IsLimitEnabled())
   //{
   //   logs::messageln("Grabber limit enabled");
   //}
   //else
   //{
   //   logs::messageln("Grabber limit disabled");
   //}

   // This CompoundObject is also an actor who normally has
   // a userData that points to its parent. However, the parent
   // of a CompoundObject is pointed by its m_parentObject 
   // member. The userData for this object should thus
   // be empty (=NULL)
   setUserData(NULL);

}

void LaunchSite::doUpdate(const UpdateState &us)
{
   switch (m_state)
   {
   case idle:
      break;
   case leapfrogLanded:
      m_grabberJoint->EnableMotor(true);
      m_grabberJoint->SetMaxMotorForce(100000.0);
      m_grabberJoint->SetMotorSpeed(6.0);

      m_stateStartTime = us.time;
      m_state = extendGrabber;
      m_properties[state].setProperty((float)m_state);
      break;
   case extendGrabber:
      if (us.time >= m_stateStartTime + 2000)
      {
         m_grabLeapfrogJointDef1 = new b2RevoluteJointDef;
         m_grabLeapfrogJointDef1->bodyA = m_leapFrog->getBody("lfBooster");
         m_grabLeapfrogJointDef1->bodyB = m_grabberBody;
         m_grabLeapfrogJointDef1->localAnchorA.Set(-1.85f, 0.8f);
         m_grabLeapfrogJointDef1->localAnchorB.Set(-1.85f, -6.1f);
         m_grabLeapfrogJointDef1->collideConnected = false;
         m_grabLeapfrogJoint1 = (b2RevoluteJoint*)m_world->CreateJoint(m_grabLeapfrogJointDef1);

         m_grabLeapfrogJointDef2 = new b2RevoluteJointDef;
         m_grabLeapfrogJointDef2->bodyA = m_leapFrog->getBody("lfBooster");
         m_grabLeapfrogJointDef2->bodyB = m_grabberBody;
         m_grabLeapfrogJointDef2->localAnchorA.Set(1.85f, 0.8f);
         m_grabLeapfrogJointDef2->localAnchorB.Set(1.85f, -6.1f);
         m_grabLeapfrogJointDef2->collideConnected = false;
         m_grabLeapfrogJoint2 = (b2RevoluteJoint*)m_world->CreateJoint(m_grabLeapfrogJointDef2);

         // Lower foot rests, let them fall
         m_leftFootRestJoint->EnableLimit(false);
         m_rightFootRestJoint->EnableLimit(false);

         // Now Leapfrog is attached and we are launching into 
         // space, and don't want panorating to be limited to ground.
         m_sceneActor->enablePanorateLimit(false);
         m_sceneActor->setPanorateMode(SceneActor::PanorateModeEnum::midTop);

         m_stateStartTime = us.time;
         m_state = lowerFootRests;
         m_properties[state].setProperty((float)m_state);
      }
      break;
   case lowerFootRests:
      if (us.time >= m_stateStartTime + 4500)
      {
         m_grabberJoint->SetMotorSpeed(-2.0);

         m_leapFrog->goToMode(LFM_DEEP_SPACE);

         m_stateStartTime = us.time;
         m_state = attachLeapfrog;
         m_properties[state].setProperty((float)m_state);
      }
      break;
   case attachLeapfrog:
      if (us.time >= m_stateStartTime + 4000)
      {
         m_countdownCounter = 7;
         showCountdownNumber(m_countdownCounter);

         ((SceneActor*)m_sceneParent)->takeControlOfLeapfrog(true);

         m_stateStartTime = us.time;
         m_state = countdown;
         m_properties[state].setProperty((float)m_state);
      }
      break;
   case countdown:
      if (us.time >= m_stateStartTime + 1000)
      {
         if (m_countdownCounter > 3)
         {
            m_countdownCounter--;
            showCountdownNumber(m_countdownCounter);
            m_stateStartTime = us.time;
         }
         else
         {
            m_countdownCounter--;
            showCountdownNumber(m_countdownCounter);
            m_boosterFlame->startEmitter();
            m_leftFlame->startEmitter();
            m_rightFlame->startEmitter();
            m_stateStartTime = us.time;
            m_state = ignite;
            m_properties[state].setProperty((float)m_state);
         }
      }
      break;
   case ignite:
      if (us.time >= m_stateStartTime + 1000)
      {
         if (m_countdownCounter > 0)
         {
            m_countdownCounter--;
            showCountdownNumber(m_countdownCounter);
            m_stateStartTime = us.time;
         }
         else
         {
            m_mainTankBody->ApplyForceToCenter(b2Vec2(0.0f, -100000.0f), true);

            m_grabberJoint->SetLimits(m_grabberJoint->GetJointTranslation(), m_grabberJoint->GetJointTranslation());
            m_grabberJoint->EnableLimit(true);

            m_stateStartTime = us.time;
            m_state = tZero;
            m_properties[state].setProperty((float)m_state);
         }
      }
      break;
   case tZero:

      if (us.time >= m_stateStartTime + 1000)
      {
         m_angleHoldStartTime = us.time;
         updateHoldAngle(us.time);
         m_mainTankBody->ApplyForceToCenter(b2Vec2(0.0f, -100000.0f), true);

         m_world->DestroyJoint(m_leftHolderJoint);
         m_world->DestroyJoint(m_rightHolderJoint);
         m_leftHolderTrolleyJoint->EnableLimit(false);
         m_rightHolderTrolleyJoint->EnableLimit(false);

         m_stateStartTime = us.time;
         m_state = release;
         m_properties[state].setProperty((float)m_state);
      }
      break;
   case release:
      updateHoldAngle(us.time);
      m_mainTankBody->ApplyForceToCenter(b2Vec2(0.0f, -100000.0f), true);

      if (us.time >= m_stateStartTime + 8000)
      {
         m_leftFlame->stopEmitter();
         m_rightFlame->stopEmitter();

         m_stateStartTime = us.time;
         m_state = supportBoosterBurnout;
         m_properties[state].setProperty((float)m_state);
      }
      break;
   case supportBoosterBurnout:
      updateHoldAngle(us.time);
      m_mainTankBody->ApplyForceToCenter(b2Vec2(0.0f, -100000.0f), true);

      if (us.time >= m_stateStartTime + 1000)
      {
         m_world->DestroyJoint(m_leftSupportBoosterJoint);
         m_world->DestroyJoint(m_rightSupportBoosterJoint);
         //m_leftBoosterBody->ApplyAngularImpulse(-10000000.0f / 180.0f * MATH_PI, true);
         //m_leftBoosterBody->ApplyAngularImpulse(10000000.0f / 180.0f * MATH_PI, true);
         m_leftBoosterBody->ApplyLinearImpulse(b2Vec2(-5000.0f, -1500.0f), b2Vec2(0.0f, -8.0f), true);
         m_rightBoosterBody->ApplyLinearImpulse(b2Vec2(1000.0f, 50.0f), b2Vec2(0.0f, -8.0f), true);

         m_stateStartTime = us.time;
         m_state = dropSupportBooster;
         m_properties[state].setProperty((float)m_state);
      }
      break;
   case dropSupportBooster:
      updateHoldAngle(us.time);
      m_mainTankBody->ApplyForceToCenter(b2Vec2(0.0f, -100000.0f), true);

      if (us.time >= m_stateStartTime + 6000)
      {
         m_boosterFlame->stopEmitter();

         m_stateStartTime = us.time;
         m_state = mainBoosterBurnout;
         m_properties[state].setProperty((float)m_state);
      }
      break;
   case mainBoosterBurnout:
      updateHoldAngle(us.time);
      m_mainTankBody->ApplyForceToCenter(b2Vec2(0.0f, -100000.0f), true);

      if (us.time >= m_stateStartTime + 1000)
      {
         m_world->DestroyJoint(m_grabLeapfrogJoint1);
         m_world->DestroyJoint(m_grabLeapfrogJoint2);
         m_mainTankBody->ApplyForceToCenter(b2Vec2(0.0f, 10000.0f), true);
         m_leapFrog->fireMainBooster(true);

         m_stateStartTime = us.time;
         m_state = dropMainBooster;
         m_properties[state].setProperty((float)m_state);
      }
      break;
   case dropMainBooster:
      m_mainTankBody->ApplyForceToCenter(b2Vec2(0.0f, 10000.0f), true);
      m_leapFrog->fireMainBooster(true);
      m_leapFrog->setHoldAngle(MATH_PI / 2.0f);

      if (us.time >= m_stateStartTime + 4000)
      {
         LaunchSequenceCompleteEvent event;
         dispatchEvent(&event);

         m_stateStartTime = us.time;
         m_state = finishSequence;
         m_properties[state].setProperty((float)m_state);
      }
      break;
   case finishSequence:
      // We wait here idenfinetly for someone to kill us
      m_mainTankBody->ApplyForceToCenter(b2Vec2(0.0f, 10000.0f), true);
      m_leapFrog->fireMainBooster(true);
      m_leapFrog->setHoldAngle(MATH_PI / 2.0f);

      break;
   }
}

void LaunchSite::leapfrogFootTouch(b2Contact* contact, bool leftFoot)
{
   if (leftFoot)
   {
      m_leftFootContact = true;
   }
   else
   {
      m_rightFootContact = true;
   }

   if (m_leftFootContact && m_rightFootContact)
   {
      // Start timer and at the end of that, if both feet still
      // are in contact, send event for Leapfrog has landed.
      LaunchSiteLeapfrogLandedEvent event;
      dispatchEvent(&event);
   }
}

void LaunchSite::leapfrogFootLift(b2Contact* contact, bool leftFoot)
{
   if (leftFoot)
   {
      m_leftFootContact = false;
   }
   else
   {
      m_rightFootContact = false;
   }
}

void LaunchSite::startLaunchSequence(LeapFrog* leapFrog)
{
   // We do only want to start sequence once, so check that it is not 
   // already started.
   if (m_state == idle)
   {
      m_leapFrog = leapFrog;
      m_state = leapfrogLanded;
      m_properties[state].setProperty((float)m_state);
   }
}

void LaunchSite::showCountdownNumber(int n)
{
   spTextField t = new TextField();
   t->setAnchor(0.5f, 0.5f);
   t->setPriority(255);
   t->attachTo(m_sceneParent);

   TextStyle style = TextStyle(m_gameResources->getResFont("main")).withColor(Color::White).alignMiddle();

   t->setStyle(style);
   
   char text[5];

   sprintf(text, "%d", n);
   t->setText(text);

   Vector2 p = m_tankActor->getPosition();

   t->setPosition(p - Vector2(6.0f, 15.0f));

   t->setScale(10.0f);
   t->setAlpha(0);
   spTween tween = t->addTween(Actor::TweenScale(1.0f), 125);
   t->addTween(Actor::TweenAlpha(255), 125);
   tween->setDoneCallback(CLOSURE(this, &LaunchSite::countDownFadeInComplete));
}

void LaunchSite::countDownFadeInComplete(Event *event)
{
   TweenEvent* te = (TweenEvent*)event;

   Actor* ta = te->getActor();

   spTween tween = ta->addTween(Actor::TweenAlpha(0), 1500);

   tween->detachWhenDone();
}

void LaunchSite::updateHoldAngle(timeMS now)
{
   float wantedAngle = MATH_PI / 2.0f * ( 1.0f - cos( (float)(now - m_angleHoldStartTime) * MATH_PI / 2.0f / (float)m_angleHoldDuration )) ;

   float angle = m_mainTankBody->GetAngle();

   while (angle > 2.0f * MATH_PI)
   {
      angle -= 2.0f * MATH_PI;
   }

   while (angle < -2.0f * MATH_PI)
   {
      angle += 2.0f * MATH_PI;
   }

   // Calculate magnitude of angular velocity as a function of how
   // close to the goal angle we are 
   float magAngVel = fabs(wantedAngle - angle); // get there in one second

   // check if we have reached the gaol, i.e. one degree within goal
   if (magAngVel < 1.0f * MATH_PI / 180.0f)
   {
      m_mainTankBody->SetAngularVelocity(0.0f);
      //holdAngleReached();
   }
   else
   {
      if (magAngVel > 2.0f * MATH_PI)
      {
         magAngVel = 2.0f * MATH_PI;
      }

      if (angle < wantedAngle)
      {
         m_mainTankBody->SetAngularVelocity(magAngVel);
      }
      else if (angle > wantedAngle)
      {
         m_mainTankBody->SetAngularVelocity(-magAngVel);
      }
   }
}

void LaunchSite::registerToMap(void)
{
   g_HeadDownDisplay->addMeToMap(
      MapItem::MapItemTypeEnum::neutralStationary, 
      m_tankActor, 
      MapItem::MapItemStateEnum::hollow);
}

