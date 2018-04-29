
#include "launchsite.h"
#include "leapfrog.h"

#include "launchsiteevents.h"

using namespace oxygine;
using namespace std;


LaunchSite::LaunchSite(
   Resources& gameResources,
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   string& defXmlFileName) :
   m_gameResources(&gameResources),
   m_world(world),
   m_sceneParent(sceneParent),
   m_state(idle),
   m_leftFootContact(false),
   m_rightFootContact(false)
{
	initCompoundObject(gameResources, sceneParent, parentObject, world, pos, defXmlFileName, string(""));

   m_leftFootRestJoint = (b2RevoluteJoint*)getJoint("leftRestJoint");
   m_rightFootRestJoint = (b2RevoluteJoint*)getJoint("rightRestJoint");
   m_leftHolderJoint = getJoint("leftHolderTankJoint");
   m_rightHolderJoint = getJoint("rightHolderTankJoint");
   m_leftHolderTrolleyJoint = (b2RevoluteJoint*)getJoint("leftHolderJoint");
   m_rightHolderTrolleyJoint = (b2RevoluteJoint*)getJoint("rightHolderJoint");
   m_grabberJoint = (b2PrismaticJoint*)getJoint("grabberSpringJoint");
   m_grabberBody = getBody("grabber");
   m_tankObject = getObject("mainTank");
   m_mainTankBody = getBody("mainTank");
   m_leftBoosterBody = getBody("leftSupportBooster");
   m_rightBoosterBody = getBody("rightSupportBooster");

   // Add main engine particle system
   m_boosterFlame = new FlameEmitter(
      gameResources,
      m_mainTankBody,
      b2Vec2(0.0f, 28.0f),
      90.0f * MATH_PI / 180.0f,
      12.0f,                            // Emitter width
      0,                               // Intensity
      1000,                            // Lifetime [ms]
      1500.0f,                          // Impulse magnitude
      30.0f);                          // Radius

   m_boosterFlame->attachTo(this);

   // Add right steering engine particle system
   m_rightFlame = new FlameEmitter(
      gameResources,
      m_rightBoosterBody,
      b2Vec2(0.0f, 19.0f),
      90.0f * MATH_PI / 180.0f,
      2.0f,                            // Width
      0,                               // Intensity
      1000,                             // Lifetime [ms]
      1500.0f,                           // Impulse magnitude
      20.0f);                           // Radius


   m_rightFlame->attachTo(this);

   // Add left steering engine particle system
   m_leftFlame = new FlameEmitter(
      gameResources,
      m_leftBoosterBody,
      b2Vec2(0.0f, 19.0f),             // Origin
      90.0f * MATH_PI / 180.0f,                               // Angle 
      2.0f,                            // Width
      0,                               // Intensity
      1000,                             // Lifetime [ms]
      1500.0f,                           // Impulse magnitude
      20.0f);                           // Radius

   m_leftFlame->attachTo(this);

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
}

CollisionEntityTypeEnum LaunchSite::getEntityType(void)
{
   return CET_NOT_APPLICABLE;
}

void LaunchSite::doUpdate(const UpdateState &us)
{
   switch (m_state)
   {
   case lfLanded:
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
         m_jointDef = new b2WeldJointDef;
         m_jointDef->bodyA = m_leapFrog->getBody("lfBooster");
         m_jointDef->bodyB = m_grabberBody;
         m_jointDef->localAnchorA.Set(0.0f, 0.8f);
         m_jointDef->localAnchorB.Set(0.0f, -6.1f);
         m_jointDef->collideConnected = false;

         m_leapfrogJoint = (b2WeldJoint*)m_world->CreateJoint(m_jointDef);

         // Lower foot rests, let them fall
         m_leftFootRestJoint->EnableLimit(false);
         m_rightFootRestJoint->EnableLimit(false);

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
      m_mainTankBody->ApplyForceToCenter(b2Vec2(0.0f, -100000.0f), true);

      if (us.time >= m_stateStartTime + 4000)
      {
         m_stateStartTime = us.time;
         m_state = release;
         m_properties[state].setProperty((float)m_state);
      }
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

   Vector2 p = m_tankObject->getPosition();

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
