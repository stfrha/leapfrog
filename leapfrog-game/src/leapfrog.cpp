#include "leapfrog.h"

#include <algorithm>
#include "leapfrogevents.h"

#include "physdispconvert.h"
#include "scales.h"

#include "SDL.h"
#include "SDL_keyboard.h"

#include "shield.h"

#include "mainactor.h"
#include "sceneactor.h"

#include "gamestatus.h"

#include "bodyuserdata.h"
#include "actoruserdata.h"

#include "blastemitter.h"
#include "headdowndisplay.h"

#include "headupdisplay.h"

#include "objectresourcesevents.h"


using namespace oxygine;
using namespace std;

void setModePropCallback(void)
{

}

ModeAngles::ModeAngles() : 
	m_rightBigJointAngle(0),
	m_rightSmallJointAngle(0),
	m_rightFootJointAngle(0),
	m_leftBigJointAngle(0),
	m_leftSmallJointAngle(0),
	m_leftFootJointAngle(0)
{ }

ModeAngles::ModeAngles(float rgtBigLeg, float rgtSmallLeg, float rgtFoot,
	float lftBigLeg, float lftSmallLeg, float lftFoot) :
	m_rightBigJointAngle(rgtBigLeg),
	m_rightSmallJointAngle(rgtSmallLeg),
	m_rightFootJointAngle(rgtFoot),
	m_leftBigJointAngle(lftBigLeg),
	m_leftSmallJointAngle(lftSmallLeg),
	m_leftFootJointAngle(lftFoot)
	{  }


LeapFrog::LeapFrog(
	SceneActor* sceneParent,
   CompoundObject* parentObject,
	b2World* world,
	const oxygine::Vector2& pos,
   pugi::xml_node& root,
   int groupIndex) :
   CompoundObject(sceneParent, parentObject),

	m_world(world),
   m_boostMagnuitude(0.0f),
   m_steerMagnitude(0.0f),
   m_boostInc(900.0f),
   m_boostMaxMagnitude(6000.0f),
   m_steerMaxMagnitude(10000.0f),
   m_eveningMagnitude(10000.0f),
   m_boostFireLastUpdate(false),
   m_rightSteerFireLastUpdate(false),
   m_leftSteerFireLastUpdate(false),
   m_state(LFS_NORMAL),
   m_wantedAngle(0.0f),
   m_modeInTransit(false),
   m_armInitialRegistration(true),
   m_reportedLegHealth(-1.0f),
   m_feetOnPad(0),
   m_serviceLiftPhase(-1),
   m_serviceLiftTime(0.0f),
   m_serviceLiftStartY(0.0f),
   m_serviceLiftTarget(0.0f),
   m_serviceLiftMass(0.0f)
{
	initCompoundObjectParts(sceneParent, sceneParent, parentObject, world, pos, root, groupIndex);

   m_leapfrogActor = getActor("lfMainBody");
   m_lfRightBigLeg = static_cast<Sprite*>(getActor("lfRightBigLeg").get());
   m_lfLeftBigLeg = static_cast<Sprite*>(getActor("lfLeftBigLeg").get());
   m_mainBody = ActorUserData::getBody(m_leapfrogActor->getUserData());
   m_rightBigLegBody = ActorUserData::getBody(m_lfRightBigLeg->getUserData());
   m_leftBigLegBody = ActorUserData::getBody(m_lfLeftBigLeg->getUserData());

	m_rightSteerBody = getBody("lfRightSteer");
	m_leftSteerBody = getBody("lfLeftSteer");

   JointUserData* jd;

   m_rightSteerJoint = static_cast<b2WeldJoint*>(getJoint("rightSteerJoint"));
   jd = new JointUserData(false);
   m_rightSteerJoint->SetUserData(jd);
   
   m_leftSteerJoint = static_cast<b2WeldJoint*>(getJoint("leftSteerJoint"));
   jd = new JointUserData(false);
   m_leftSteerJoint->SetUserData(jd);
   
   m_rightBigLegJoint = static_cast<b2RevoluteJoint*>(getJoint("rightLegJoint"));
   jd = new JointUserData(false);
   m_rightBigLegJoint->SetUserData(jd);
   
   m_leftBigLegJoint = static_cast<b2RevoluteJoint*>(getJoint("leftLegJoint"));
   jd = new JointUserData(false);
   m_leftBigLegJoint->SetUserData(jd);
   
   m_rightSmallLegJoint = static_cast<b2RevoluteJoint*>(getJoint("rightSmallLegJoint"));
   jd = new JointUserData(false);
   m_rightSmallLegJoint->SetUserData(jd);
   
   m_leftSmallLegJoint = static_cast<b2RevoluteJoint*>(getJoint("leftSmallLegJoint"));
   jd = new JointUserData(false);
   m_leftSmallLegJoint->SetUserData(jd);
   
   m_rightFootLegJoint = static_cast<b2RevoluteJoint*>(getJoint("rightFootLegJoint"));
   jd = new JointUserData(false);
   m_rightFootLegJoint->SetUserData(jd);
   
   m_leftFootLegJoint = static_cast<b2RevoluteJoint*>(getJoint("leftFootLegJoint"));
   jd = new JointUserData(false);
   m_leftFootLegJoint->SetUserData(jd);

   // Shield only exists in deep space but for simplicity it is always
   // there. In deep space it gets its normal size but is other 
   // environments it is turned very small

   m_shield = static_cast<Shield*>(getSystem("lfShield"));

   if (m_shield)
   {
      m_shieldJoint = m_shield->m_shieldJoint;
   }

   m_boosterFlame = static_cast<FlameEmitter*>(getSystem("boosterFlame"));
   //m_boosterFlame->attachTo(this);
	
   m_leftSteerFlame = static_cast<FlameEmitter*>(getSystem("leftSteeringFlame"));
   // m_leftSteerFlame->attachTo(this);

   m_rightSteerFlame = static_cast<FlameEmitter*>(getSystem("rightSteeringFlame"));
   // m_rightSteerFlame->attachTo(this);

   m_gun = static_cast<Gun*>(getSystem("lfGun"));
   // m_gun->attachTo(this);

   m_reentryFlameEmitterBooster = static_cast<ReentryFlameEmitter*>(getSystem("centerReentryFlame"));
   // m_reentryFlameEmitterBooster->attachTo(this);

   m_reentryFlameEmitterRightLeg = static_cast<ReentryFlameEmitter*>(getSystem("rightReentryFlame"));
   // m_reentryFlameEmitterRightLeg->attachTo(this);

   m_reentryFlameEmitterLeftLeg = static_cast<ReentryFlameEmitter*>(getSystem("leftReentryFlame"));
   // m_reentryFlameEmitterLeftLeg->attachTo(this);


   // Register all properties:
   m_properties.push_back(ObjectProperty(this, new LeapfrogExtSetModeEvent, 0, 0.0f)); // Mode
   m_properties.push_back(ObjectProperty(this, new LeapfrogExtSetEnvEvent, 1, 0.0f)); // Environment
   m_properties.push_back(ObjectProperty(this, NULL, 2, 0.0f, true)); // State
   m_properties.push_back(ObjectProperty(this, NULL, 3, 0.0f, true)); // xPos
   m_properties.push_back(ObjectProperty(this, NULL, 4, 0.0f, true)); // yPos

   addEventListener(LeapfrogExtSetModeEvent::EVENT, CLOSURE(this, &LeapFrog::setModePropHandler));
   addEventListener(LeapfrogExtSetEnvEvent::EVENT, CLOSURE(this, &LeapFrog::setEnvPropHandler));
   addEventListener(LeapfrogExtSetLegHealthEvent::EVENT, CLOSURE(this, &LeapFrog::setLegHealthPropHandler));


   // Here we attach Leapfrog object to tree so it gets updates etc.
   attachTo(sceneParent);

   setWeakJoints();

   // Leg health comes from the game status (a limp leg stays limp until
   // repaired, also across scenes)
   initLegJoints();

   // This CompoundObject is also an actor who normally has
   // a userData that points to its parent. However, the parent
   // of a CompoundObject is pointed by its m_parentObject 
   // member. The userData for this object should thus
   // be empty (=NULL)
   setUserData(NULL);

   m_sceneActor->addBoundingBody(m_mainBody);

   // Register leapfrog on map
   g_HeadDownDisplay->addMeToMap(
      MapItem::MapItemTypeEnum::me,
      m_leapfrogActor,
      MapItem::MapItemStateEnum::filled);


   // Register leapfrog to connect (and create) status bars
   MainActor* ma = (MainActor*)(getStage()->getFirstChild().get());
   if (ma != NULL)
   {
      // Init game status in leapfrog object
      initObjectResources(ma, g_GameStatus.getResources());
   }

}

void LeapFrog::setModePropHandler(oxygine::Event *ev)
{
   // dumpParts();

   float value = m_properties[LeapfroPropertiesEnum::propMode].getProperty();
   
   if ((value > -0.5) && (value < 0.5))
   {
      goToMode(LFM_RESET);
   }
   else if ((value > 0.5) && (value < 1.5))
   {
      goToMode(LFM_DEEP_SPACE);
   }
   else if ((value > 1.5) && (value < 2.5))
   {
      goToMode(LFM_LANDING);
   }
   else if ((value > 2.5) && (value < 3.5))
   {
      goToMode(LFM_ORBIT);
   }
   else if ((value > 3.5) && (value < 4.5))
   {
      goToMode(LFM_REENTRY);
   }
}

void LeapFrog::setEnvPropHandler(oxygine::Event *ev)
{
   float value = m_properties[LeapfroPropertiesEnum::propMode].getProperty();

   goToEnvironment(static_cast<EnvironmentEnum>((int)value));
}

// Velocity of the centre of mass of the whole craft (main body, legs,
// feet, steering boosters). The main body alone also moves when the
// craft rotates, since its centre is not the centre of the assembly,
// which made the HDD speed bars react to steering.
b2Vec2 LeapFrog::getCraftVelocity(void)
{
   b2Vec2 momentum(0.0f, 0.0f);
   float mass = 0.0f;
   std::vector<b2Body*> counted;

   for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it)
   {
      b2Body* body = ActorUserData::getBody((const void*)(*it)->getUserData());

      if ((body == NULL) || (body->GetType() != b2_dynamicBody) || !body->IsActive())
      {
         continue;
      }

      if (std::find(counted.begin(), counted.end(), body) != counted.end())
      {
         continue;
      }

      counted.push_back(body);
      momentum += body->GetMass() * body->GetLinearVelocity();
      mass += body->GetMass();
   }

   if (mass <= 0.0f)
   {
      return m_mainBody->GetLinearVelocity();
   }

   return (1.0f / mass) * momentum;
}

oxygine::Actor* LeapFrog::getMainActor(void)
{
   return m_leapfrogActor.get();
}

void LeapFrog::doUpdate(const UpdateState &us)
{
   // Register Leapfrog in various places. But only the very first update
   if (m_armInitialRegistration)
   {
      m_armInitialRegistration = false;
      
      // Start leapfrog handling in scene
      m_sceneActor->startLeapfrogInScene(getName());

      g_headUpDisplay->registerLeapfrog(this, (m_mode == LeapFrogModeEnum::LFM_LANDING));
   }

   // Update properties with the values that where determined by 
   // the physics engine
   b2Vec2 pos = m_mainBody->GetPosition();

//   logs::messageln("(%f,%f)", pos.x, pos.y);

   m_properties[LeapfroPropertiesEnum::propXPos].setProperty(pos.x);
   m_properties[LeapfroPropertiesEnum::propYPos].setProperty(pos.y);

   // Feed the speed bars and resource warnings of the head-down display
   if (m_properties.size() > LeapfroPropertiesEnum::damage)
   {
      LeapfrogTelemetry telemetry;
      telemetry.m_velocity = getCraftVelocity();
      telemetry.m_shieldInUse = (m_environment != ENV_GROUND) && (m_environment != ENV_LAUNCH);
      telemetry.m_ammo = m_properties[LeapfroPropertiesEnum::ammo].getProperty();
      telemetry.m_shield = m_properties[LeapfroPropertiesEnum::shield].getProperty();
      telemetry.m_fuel = m_properties[LeapfroPropertiesEnum::fuel].getProperty();
      telemetry.m_damage = m_properties[LeapfroPropertiesEnum::damage].getProperty();
      g_HeadDownDisplay->setLeapfrogTelemetry(telemetry);
   }

   float angle = 0.0f;
   b2Vec2 boostForce = b2Vec2(0.0f, 0.0f);
   float magAngVel = 0.0f;


   switch (m_state)
   {
   case LFS_NORMAL:
      angle = m_mainBody->GetAngle();
      boostForce = b2Vec2(m_boostMagnuitude * sin(angle), -m_boostMagnuitude * cos(angle));

      if ((m_environment != ENV_DEEP_SPACE) && (m_environment != ENV_ORBIT))
      {
         m_mainBody->ApplyTorque(m_steerMagnitude, true);
      }

      m_mainBody->ApplyForceToCenter(boostForce, true);

      setJointMotor(m_rightBigLegJoint, m_modeAngleGoals.m_rightBigJointAngle, c_normalJointMotorSpeed);
      setJointMotor(m_rightSmallLegJoint, m_modeAngleGoals.m_rightSmallJointAngle, c_normalJointMotorSpeed);
      setJointMotor(m_rightFootLegJoint, m_modeAngleGoals.m_rightFootJointAngle, c_normalJointMotorSpeed);
      setJointMotor(m_leftBigLegJoint, m_modeAngleGoals.m_leftBigJointAngle, c_normalJointMotorSpeed);
      setJointMotor(m_leftSmallLegJoint, m_modeAngleGoals.m_leftSmallJointAngle, c_normalJointMotorSpeed);
      setJointMotor(m_leftFootLegJoint, m_modeAngleGoals.m_leftFootJointAngle, c_normalJointMotorSpeed);
      break;

   case LFS_HOLD_ANGLE:

      // In hold angle state the steering is disabled but booster still works
      angle = m_mainBody->GetAngle();
      boostForce = b2Vec2(m_boostMagnuitude * sin(angle), -m_boostMagnuitude * cos(angle));
      m_mainBody->ApplyForceToCenter(boostForce, true);

      angle = m_mainBody->GetAngle();

      while (angle > 2.0f * MATH_PI)
      {
         angle -= 2.0f * MATH_PI;
      }

       while (angle < -2.0f * MATH_PI)
      {
         angle += 2.0f * MATH_PI;
      }

      // Calculate magnitude of angular velocity as a function of how
      // close to the goal angle we are but not more than m_maxAngularVelocity
      magAngVel = fabs(m_wantedAngle - angle); // get there in one second

      // check if we have reached the gaol, i.e. one degree within goal
      if (magAngVel < 1.0f * MATH_PI / 180.0f)
      {
         m_mainBody->SetAngularVelocity(0.0f);
         holdAngleReached();
      }
      else
      {
         if (magAngVel > 2.0f * MATH_PI)
         {
            magAngVel = 2.0f * MATH_PI;
         }

         if (angle < m_wantedAngle)
         {
            m_mainBody->SetAngularVelocity(magAngVel);
         }
         else if (angle > m_wantedAngle)
         {
            m_mainBody->SetAngularVelocity(-magAngVel);
         }
      }

      setJointMotor(m_rightBigLegJoint, m_modeAngleGoals.m_rightBigJointAngle, c_normalJointMotorSpeed);
      setJointMotor(m_rightSmallLegJoint, m_modeAngleGoals.m_rightSmallJointAngle, c_normalJointMotorSpeed);
      setJointMotor(m_rightFootLegJoint, m_modeAngleGoals.m_rightFootJointAngle, c_normalJointMotorSpeed);
      setJointMotor(m_leftBigLegJoint, m_modeAngleGoals.m_leftBigJointAngle, c_normalJointMotorSpeed);
      setJointMotor(m_leftSmallLegJoint, m_modeAngleGoals.m_leftSmallJointAngle, c_normalJointMotorSpeed);
      setJointMotor(m_leftFootLegJoint, m_modeAngleGoals.m_leftFootJointAngle, c_normalJointMotorSpeed);

      break;
   }

   updateLegStrain();
   updateServiceLift();
};

void LeapFrog::setJointMotor(b2RevoluteJoint* joint, float goal, float speedMagnitude)
{
   if (joint == NULL)
   {
      return;
   }

   JointUserData* jd = legData(joint);

   if ((jd != NULL) && jd->m_limp)
   {
      // A limp joint has no motor to drive. It counts as having reached
      // the mode so that mode changes still complete.
      if (m_modeInTransit)
      {
         jd->setModeReached(true);
         if (modeIsReached())
         {
            modeReached();
         }
      }
      return;
   }

	float angle = joint->GetJointAngle();

	if (angle > goal)
	{
		if (angle - goal > 2.0f * MATH_PI / 180.0f)
		{
			joint->SetMotorSpeed(-speedMagnitude);
		}
		else
		{
			joint->SetMotorSpeed(0);
         
         // Here the goal is reached. Lets call a function
         // that can do some things at this time, for instance
         // lock leg joints
         if (m_modeInTransit)
         {
            ((JointUserData*)joint->GetUserData())->setModeReached(true);
            if (modeIsReached())
            { 
               modeReached();
            }
         }
		}
	}
	else if (angle < goal)
	{
		if (goal - angle > 2.0f * MATH_PI / 180.0f)
		{
			joint->SetMotorSpeed(-speedMagnitude);
		}
      else
      {

         // Here the goal is reached. Lets call a function
         // that can do some things at this time, for instance
         // lock leg joints
         if (m_modeInTransit)
         {
            ((JointUserData*)joint->GetUserData())->setModeReached(true);
            if (modeIsReached())
            {
               modeReached();
            }
         }
      }
      joint->SetMotorSpeed(speedMagnitude);
   }
}

void LeapFrog::lockJoints(bool onPad)
{
   // Lock all legs and disable motors. Limp joints keep swinging. On a
   // landing pad, weakened joints aren't locked either: their tired motors
   // hold (or fail to hold) the craft, so it sags on a damaged leg.
   b2RevoluteJoint* joints[c_numLegJoints];
   float goals[c_numLegJoints];
   getLegJoints(joints, goals);

   for (int i = 0; i < c_numLegJoints; i++)
   {
      b2RevoluteJoint* joint = joints[i];

      if (joint == NULL)
      {
         continue;
      }

      JointUserData* jd = legData(joint);

      if ((jd != NULL) && jd->m_limp)
      {
         continue;
      }

      if (onPad && (jd != NULL) && (jd->m_health < c_legLockHealth))
      {
         applyLegStrength(joint);
         joint->EnableLimit(false);
         joint->EnableMotor(true);
         continue;
      }

      joint->EnableMotor(false);
      joint->SetLimits(goals[i], goals[i]);
      joint->EnableLimit(true);
   }
}

void LeapFrog::unlockJoints(void)
{
   // Unlock all legs and enable motors (limp joints stay limp)
   b2RevoluteJoint* joints[c_numLegJoints];
   float goals[c_numLegJoints];
   getLegJoints(joints, goals);

   for (int i = 0; i < c_numLegJoints; i++)
   {
      b2RevoluteJoint* joint = joints[i];

      if (joint == NULL)
      {
         continue;
      }

      JointUserData* jd = legData(joint);

      if ((jd != NULL) && jd->m_limp)
      {
         continue;
      }

      joint->EnableMotor(true);
      joint->EnableLimit(false);
   }
}

void LeapFrog::setStrongJoints(void)
{
   m_rightBigLegJoint->SetMaxMotorTorque(c_instantJointMotorTorque);
   m_leftBigLegJoint->SetMaxMotorTorque(c_instantJointMotorTorque);
   m_rightSmallLegJoint->SetMaxMotorTorque(c_instantJointMotorTorque);
   m_leftSmallLegJoint->SetMaxMotorTorque(c_instantJointMotorTorque);
   m_rightFootLegJoint->SetMaxMotorTorque(c_instantJointMotorTorque);
   m_leftFootLegJoint->SetMaxMotorTorque(c_instantJointMotorTorque);
}

void LeapFrog::setWeakJoints(void)
{
   // Normal motor torque, reduced for damaged leg joints
   for (int i = 0; i < c_numLegJoints; i++)
   {
      applyLegStrength(getLegJoint(i));
   }
}

bool LeapFrog::modeIsReached(void)
{
   if (!((JointUserData*)m_rightBigLegJoint->GetUserData())->getModeReached())
   {
      return false;
   }

   if (!((JointUserData*)m_leftBigLegJoint->GetUserData())->getModeReached())
   {
      return false;
   }

   if (!((JointUserData*)m_rightSmallLegJoint->GetUserData())->getModeReached())
   {
      return false;
   }

   if (!((JointUserData*)m_leftSmallLegJoint->GetUserData())->getModeReached())
   {
      return false;
   }

   if (!((JointUserData*)m_rightFootLegJoint->GetUserData())->getModeReached())
   {
      return false;
   }

   if (!((JointUserData*)m_leftFootLegJoint->GetUserData())->getModeReached())
   {
      return false;
   }

   return true;
}

void LeapFrog::resetModeReached(void)
{
   ((JointUserData*)m_rightBigLegJoint->GetUserData())->setModeReached(false);

   ((JointUserData*)m_leftBigLegJoint->GetUserData())->setModeReached(false);

   ((JointUserData*)m_rightSmallLegJoint->GetUserData())->setModeReached(false);

   ((JointUserData*)m_leftSmallLegJoint->GetUserData())->setModeReached(false);

   ((JointUserData*)m_rightFootLegJoint->GetUserData())->setModeReached(false);

   ((JointUserData*)m_leftFootLegJoint->GetUserData())->setModeReached(false);

}

void LeapFrog::stopAllJointMotors(void)
{
   if (m_rightBigLegJoint)
   {
      m_rightBigLegJoint->SetMotorSpeed(0.0f);
   }

   if (m_leftBigLegJoint)
   {
      m_leftBigLegJoint->SetMotorSpeed(0.0f);
   }

   if (m_rightSmallLegJoint)
   {
      m_rightSmallLegJoint->SetMotorSpeed(0.0f);
   }

   if (m_leftSmallLegJoint)
   {
      m_leftSmallLegJoint->SetMotorSpeed(0.0f);
   }

   if (m_rightFootLegJoint)
   {
      m_rightFootLegJoint->SetMotorSpeed(0.0f);
   }

   if (m_leftFootLegJoint)
   {
      m_leftFootLegJoint->SetMotorSpeed(0.0f);
   }
}

void LeapFrog::setHoldAngle(float angle)
{
   m_wantedAngle = angle;

   m_state = LFS_HOLD_ANGLE;
   m_properties[LeapfroPropertiesEnum::propState].setProperty((float)m_state);
}

void LeapFrog::releaseHoldAngle(void)
{
   m_state = LFS_NORMAL;
   m_properties[LeapfroPropertiesEnum::propState].setProperty((float)m_state);
}

void LeapFrog::breakJoints(void)
{
   if (m_rightSteerJoint)
   {
      m_world->DestroyJoint(m_rightSteerJoint);
   }

   if (m_leftSteerJoint)
   {
      m_world->DestroyJoint(m_leftSteerJoint);
   }

   if (m_rightFootLegJoint)
   {
      m_world->DestroyJoint(m_rightFootLegJoint);
   }

   if (m_leftFootLegJoint)
   {
      m_world->DestroyJoint(m_leftFootLegJoint);
   }

   if (m_rightSmallLegJoint)
   {
      m_world->DestroyJoint(m_rightSmallLegJoint);
   }

   if (m_leftSmallLegJoint)
   {
      m_world->DestroyJoint(m_leftSmallLegJoint);
   }

   if (m_rightBigLegJoint)
   {
      m_world->DestroyJoint(m_rightBigLegJoint);
   }

   if (m_leftBigLegJoint)
   {
      m_world->DestroyJoint(m_leftBigLegJoint);
   }

   m_rightSteerJoint = NULL;
   m_leftSteerJoint = NULL;
   m_rightBigLegJoint = NULL;
   m_leftBigLegJoint = NULL;
   m_rightSmallLegJoint = NULL;
   m_leftSmallLegJoint = NULL;
   m_rightFootLegJoint = NULL;
   m_leftFootLegJoint = NULL;
}

void LeapFrog::goToMode(LeapFrogModeEnum mode)
{
   m_modeInTransit = true;
   m_properties[LeapfroPropertiesEnum::propState].setProperty((float)m_state);
   m_mode = mode;
   m_properties[LeapfroPropertiesEnum::propMode].setProperty((float)m_mode);

   unlockJoints();

   switch (mode)
   {
   case LFM_LANDING:
      m_modeAngleGoals = c_landingModeAngles;
      break;

   case LFM_DEEP_SPACE:
      m_modeAngleGoals = c_deepSpaceModeAngles;
      break;

   case LFM_ORBIT:
      m_modeAngleGoals = c_orbitModeAngles;
      break;

   case LFM_REENTRY:
      m_modeAngleGoals = c_reentryModeAngles;
      break;

   case LFM_RESET:
      m_modeAngleGoals = c_resetModeAngles;
      break;
   }
}

void LeapFrog::goToEnvironment(EnvironmentEnum env)
{
   m_environment = env;

   switch (env)
   {
   case ENV_DEEP_SPACE:
      m_boostInc = 10000.0f;
      m_boostMaxMagnitude = 30000.0f;
      m_steerMaxMagnitude = 30000.0f;;
      m_eveningMagnitude = 60000.0f;
      m_maxVelocity = 50.0f;    // [m/s]
      m_maxAngularVelocity = MATH_PI; // [rad/s]
      m_boosterFlame->setParameters(500, 90.0f, 10.0f);
      m_shield->m_body->SetActive(true);

      break;

   case ENV_ORBIT:
      m_boostInc = 2000.0f;
      m_boostMaxMagnitude = 4000.0f;
      m_steerMaxMagnitude = 30000.0f;;
      m_eveningMagnitude = 60000.0f;
      m_maxVelocity = 80.0f;    // [m/s]
      m_maxAngularVelocity = 4.0f * MATH_PI; // [rad/s]
      m_boosterFlame->setParameters(500, 90.0f, 10.0f);
      m_shield->m_body->SetActive(true);
      break;

   case ENV_GROUND:
      m_boostInc = 900.0f;
      m_boostMaxMagnitude = 6000.0f;
      m_steerMaxMagnitude = 40000.0f;;
      m_eveningMagnitude = 10000.0f;
      m_maxVelocity = 120.0f;    // [m/s]
      m_maxAngularVelocity = MATH_PI; // [rad/s]
      m_boosterFlame->setParameters(250, 70.0f, 10.0f);
      m_shield->m_body->SetActive(false);
      break;


   case ENV_LAUNCH:
      m_boostInc = 900.0f;
      m_boostMaxMagnitude = 3000.0f;
      m_steerMaxMagnitude = 30000.0f;;
      m_eveningMagnitude = 10000.0f;
      m_maxVelocity = 400.0f;    // [m/s]
      m_maxAngularVelocity = MATH_PI; // [rad/s]
      m_boosterFlame->setParameters(250, 70.0f, 10.0f);
      m_shield->m_body->SetActive(false);
      break;
   }

}

void LeapFrog::modeReached(void)
{
   resetModeReached();

   if (m_modeInTransit)
   {
      // Don't know anything special here...
   }

   if (m_mode == LFM_DEEP_SPACE)
   {
      // For deep space we lock all legs so that 
      // the central movement of fast spinning
      // not causes them to fling out
      lockJoints();

   }
   else
   {
      unlockJoints();
   }

   m_modeInTransit = false;
   m_properties[LeapfroPropertiesEnum::propState].setProperty((float)m_state);

   LeapfrogModeReachedEvent event("Test");
   dispatchEvent(&event);

}

void LeapFrog::holdAngleReached(void)
{
   //Don't know what to do here
   int a = 10;

}


void LeapFrog::fireMainBooster(bool burn, bool flamesOnly, bool freeFuel)
// Will be called each doUpdate when burn is pressed
{
   // Handle booster flame particles

   if (!freeFuel && (m_environment == ENV_GROUND) && (m_resources->getFuel() <= 0.0f))
   {
      // No burn here
   }
   else
   {
      if (burn)
      {
         if (m_boostFireLastUpdate == false)
         {
            // Turn emitter on here
            m_boosterFlame->startEmitter();
         }
      }
      else
      {
         if (m_boostFireLastUpdate == true)
         {
            // Turn emitter off here
            m_boosterFlame->stopEmitter();
         }
      }
   }

   //// Below override any booster settings on ground if no fuel
   //if ((m_environment == ENV_GROUND) && (m_resources->getFuel() <= 0.0f))
   //{
   //   // Turn emitter off here, will be turned off each update that 
   //   // the booster is pressed with no fule, in land
   //   m_boosterFlame->stopEmitter();
   //}


   if (!flamesOnly)
   {
      // Handle forces of Leapfrog
      // This is done differently for Deep-space and ground environment
      // In Deep space we accellerate to a maximum speed and decellerate
      // fast when booster is turned off.
      // Steering is fast and near instantaneous and stop turning directly

      // In ground mode we apply force independently of the velocity
      // and rotation is slowly decellerating

      if ((m_environment == ENV_DEEP_SPACE) || (m_environment == ENV_ORBIT))
      {
         float vel = m_mainBody->GetLinearVelocity().Length();

         // We start damping when booster is released
         // and remove damping when it is fired
         if (burn)
         {
            if (m_boostFireLastUpdate == false)
            {
               // Turn damping off

               // TODO: Should this not be zero?
               m_mainBody->SetLinearDamping(1.0f);
            }
         }
         else
         {
            if (m_boostFireLastUpdate == true)
            {
               // Turn damping on
               m_mainBody->SetLinearDamping(1.0f);
            }
         }

         if (burn)
         {
            // Increment force with m_boostInc until more than
            // m_boostMaxMagnitude and remain this max magnitude
            // force until speed is more than 90 of max speed, 
            // drop force then. Then, when no boost burn, 
            // break with a negative force linear with speed
            // making it regulate itself to stand-still
            //if (vel < m_maxVelocity * 0.95f)
            //{
            //   m_boostMagnuitude += m_boostInc;

            //   if (m_boostMagnuitude > m_boostMaxMagnitude)
            //   {
            //      m_boostMagnuitude = m_boostMaxMagnitude;
            //   }
            //}
            if (vel < m_maxVelocity * 0.95f)
            {
               m_boostMagnuitude = m_boostMaxMagnitude;
            }
            else
            {
               m_boostMagnuitude = 0.0f;
            }

         }
         else
         {
            m_boostMagnuitude = 0;
            //m_boostMagnuitude = vel * -m_boostInc;
         }

      }
      else if (m_environment == ENV_GROUND)
      {
         if (burn && (freeFuel || (m_resources->getFuel() > 0.0f)))
         {
            m_boostMagnuitude += m_boostInc;
            if (m_boostMagnuitude > m_boostMaxMagnitude)
            {
                m_boostMagnuitude = m_boostMaxMagnitude;
            }

            if (!freeFuel && (m_boostMagnuitude >= m_boostMaxMagnitude))
            {
                // Fuel per scene update. The 240000.0f was calibrated at 60
                // updates per second, which is now always the rate
                // (Scales::c_updateStep)
                m_resources->deltaFuel(-m_boostMagnuitude / 240000.0f * 60.0f * Scales::c_updateStep);
                //m_resources->deltaFuel(-m_boostMagnuitude / 30000.0f);
            }
         }
         else
         {
            m_boostMagnuitude = 0;
         }
      }
   }

   if (burn)
   {
      m_boostFireLastUpdate = true;
   }
   else
   {
      m_boostFireLastUpdate = false;
   }

}

void LeapFrog::fireSteeringBooster(int dir)
{
   // Handle booster flame particles

   if ((m_environment == ENV_GROUND) && (m_resources->getFuel() <= 0.0f))
   {
      // No fire here
   }
   else
   {
      if (dir == -1)
      {

         if (m_rightSteerFireLastUpdate == false)
         {
            m_leftSteerFlame->stopEmitter();
            m_rightSteerFlame->startEmitter();
         }
      }
      else if (dir == 1)
      {

         if (m_leftSteerFireLastUpdate == false)
         {
            m_rightSteerFlame->stopEmitter();
            m_leftSteerFlame->startEmitter();
         }
      }
      else if (dir == 0)
      {
         if ((m_leftSteerFireLastUpdate == true) || (m_rightSteerFireLastUpdate == true))
         {
            m_rightSteerFlame->stopEmitter();
            m_leftSteerFlame->stopEmitter();
         }
      }
   }

   //if (m_environment == ENV_GROUND)
   //{
   //   if (m_resources->getFuel() == 0.0f)
   //   {
   //      // Turn emitter off here, will be turned off each update that 
   //      // the booster is pressed with no fule, in land
   //      m_rightSteerFlame->stopEmitter();
   //      m_leftSteerFlame->stopEmitter();
   //   }
   //}

   if ((m_environment == ENV_DEEP_SPACE) || (m_environment == ENV_ORBIT))
   {
      // In deep space we accellerate fast to a maximum
      // angular speed and hold that speed until steering
      // is released. We then immediately stop turning
      if (dir == -1)
      {
         m_mainBody->SetAngularVelocity(-m_maxAngularVelocity);
         //if (vel > m_maxAngularVelocity * 0.95f)
         //{
         //   m_steerMagnitude += m_steerInc;

         //   if (m_steerMagnitude > m_steerMaxMagnitude)
         //   {
         //      m_steerMagnitude = m_steerMaxMagnitude;
         //   }
         //}

         //m_steerForce = -m_steerMagnitude;
      }
      else if (dir == 1)
      {
         m_mainBody->SetAngularVelocity(m_maxAngularVelocity);
      }
      else
      {
         m_mainBody->SetAngularVelocity(0.0f);
      }
   }
   else if (m_environment == ENV_GROUND)
   {
      if (m_resources->getFuel() > 0.0f)
      {
         if ((dir == -1) && (m_resources->getFuel() > 0.0f))
         {
            m_steerMagnitude = -m_steerMaxMagnitude;
         }
         else if ((dir == 1) && (m_resources->getFuel() > 0.0f))
         {
            m_steerMagnitude = m_steerMaxMagnitude;
         }
         else if (dir == 0)
         {
            float angleVel = m_mainBody->GetAngularVelocity();

            if (angleVel > 0.5 * MATH_PI / 180.0f)
            {
               m_steerMagnitude = -m_eveningMagnitude;
            }
            else if (angleVel < -0.5 * MATH_PI / 180.0f)
            {
               m_steerMagnitude = m_eveningMagnitude;
            }
         }
      }
      else
      {
         // Even out rotation due to no more fuel
         float angleVel = m_mainBody->GetAngularVelocity();

         if (angleVel > 0.5 * MATH_PI / 180.0f)
         {
            m_steerMagnitude = -m_eveningMagnitude;
         }
         else if (angleVel < -0.5 * MATH_PI / 180.0f)
         {
            m_steerMagnitude = m_eveningMagnitude;
         }
      }
   }

   if (dir == -1)
   {
      m_rightSteerFireLastUpdate = true;
   }
   else if (dir == 1)
   {
      m_leftSteerFireLastUpdate = true;
   }
   else if (dir == 0)
   {
      m_rightSteerFireLastUpdate = false;
      m_leftSteerFireLastUpdate = false;
   }
}

void LeapFrog::fireGun(bool fire)
{
   if (fire)
   {
      m_gun->startGun();
   }
   else
   {
      m_gun->stopGun();
   }
}

void LeapFrog::fireReentryFlames(bool fire)
{
   if (fire)
   {
      m_reentryFlameEmitterBooster->startEmitter();
      m_reentryFlameEmitterRightLeg->startEmitter();
      m_reentryFlameEmitterLeftLeg->startEmitter();
   }
   else
   {
      m_reentryFlameEmitterBooster->stopEmitter();
      m_reentryFlameEmitterRightLeg->stopEmitter();
      m_reentryFlameEmitterLeftLeg->stopEmitter();
   }
}

void LeapFrog::reentrySetHeat(int heatAmount)
{
   if (heatAmount > 255)
   {
      heatAmount = 255;
   }
   else if (heatAmount < 0)
   {
      heatAmount = 0;
   }

   float intensity = 60.0f / 255.0f * heatAmount;
   float hSize = 0.5f + 1.0f / 255.0f * heatAmount;
   float vSize = 5.0f + 25.0f / 255.0f * heatAmount;
   int white = 128 + heatAmount / 2;

   m_lfRightBigLeg->setColor(255, white, white, 255);
   m_lfLeftBigLeg->setColor(255, white, white, 255);
   m_reentryFlameEmitterBooster->setParameters(intensity, 500, b2Vec2(hSize, vSize));
   m_reentryFlameEmitterRightLeg->setParameters(intensity, 500, b2Vec2(hSize, vSize));
   m_reentryFlameEmitterLeftLeg->setParameters(intensity, 500, b2Vec2(hSize, vSize));
}

//void LeapFrog::setBoundedWallsActor(FreeSpaceActor* actor)
//{
////   m_gun->setBoundedWallsActor(actor);
//}
//

const ModeAngles LeapFrog::c_resetModeAngles =
   ModeAngles(9.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

const ModeAngles LeapFrog::c_landingModeAngles =
   ModeAngles(
      -65.0f * MATH_PI / 180.0f, 65.0f * MATH_PI / 180.0f,
      0.0f * MATH_PI / 180.0f, 65.0f * MATH_PI / 180.0f,
      -65.0f * MATH_PI / 180.0f, 0.0f * MATH_PI / 180.0f);

const ModeAngles LeapFrog::c_deepSpaceModeAngles =
   ModeAngles(
      -187.5f * MATH_PI / 180.0f, -3.5f * MATH_PI / 180.0f,
      -117.5f * MATH_PI / 180.0f, 187.0f * MATH_PI / 180.0f,
      3.5f * MATH_PI / 180.0f, 117.5f * MATH_PI / 180.0f);

const ModeAngles LeapFrog::c_orbitModeAngles =
   ModeAngles(
      -65.0f * MATH_PI / 180.0f, 65.0f * MATH_PI / 180.0f,
      0.0f * MATH_PI / 180.0f, 65.0f * MATH_PI / 180.0f,
      -65.0f * MATH_PI / 180.0f, 0.0f * MATH_PI / 180.0f);

const ModeAngles LeapFrog::c_reentryModeAngles =
   ModeAngles(
      -92.0f * MATH_PI / 180.0f, -156.5f * MATH_PI / 180.0f,
      -35.0f * MATH_PI / 180.0f, 92.0f * MATH_PI / 180.0f,
      156.5f * MATH_PI / 180.0f, 35.0f * MATH_PI / 180.0f);


const float LeapFrog::c_normalJointMotorTorque = 20000.0f;
const float LeapFrog::c_normalJointMotorSpeed = 30.0f * MATH_PI / 180.0f;
const float LeapFrog::c_instantJointMotorTorque = 2000000.0f;
const float LeapFrog::c_instnatJointMotorSpeed = 150.0f * MATH_PI / 180.0f;

// Leg joint damage. Tuning values: the log prints each strain event's peak
// next to these (search for "Leg strain").
//
// Overload: the torque a joint takes (limit torque when locked, motor torque
// when driven) above c_legSustainTorque is summed as an angular impulse over
// about c_legStrainMemory seconds, so one impact counts the same however
// Box2D spreads it over steps. The part of an event's peak above
// c_legYieldImpulse costs health, 1.0 per c_legToughnessImpulse.
// The craft weighs about 620 x 3 = 1860; standing, each hip carries about
// 4700. A hard set-down at 4 m/s on locked legs gives each hip roughly 6000.
const float LeapFrog::c_legSustainTorque = 15000.0f;
const float LeapFrog::c_legStrainMemory = 0.15f;               // [s]
const float LeapFrog::c_legYieldImpulse = 7000.0f;
const float LeapFrog::c_legToughnessImpulse = 7000.0f;

// Over-bending: a motor-held joint forced further than c_legBendYield from
// where it should be costs health, 1.0 per c_legBendToughness beyond it.
const float LeapFrog::c_legBendYield = 15.0f * MATH_PI / 180.0f;
const float LeapFrog::c_legBendToughness = 30.0f * MATH_PI / 180.0f;

// Below c_legLockHealth a joint isn't locked on a landing pad (its motor
// holds it); below c_legWeakHealth the HUD reports it. Motor torque scales
// with health, down to c_legMinTorqueScale of normal just before failure.
const float LeapFrog::c_legLockHealth = 0.5f;
const float LeapFrog::c_legWeakHealth = 0.6f;
const float LeapFrog::c_legMinTorqueScale = 0.15f;

// A limp joint: a weak motor at speed 0 acts as friction so the leg swings
// heavily rather than flapping. Limits: the joint's whole range of travel
// (all mode poses, and hanging straight down) widened by c_legLimpMargin.
const float LeapFrog::c_legLimpFriction = 250.0f;
const float LeapFrog::c_legLimpMargin = 20.0f * MATH_PI / 180.0f;

// Service lift: rise c_liftHeight at c_liftRiseSpeed, wait for the legs'
// pose, sink at c_liftLowerSpeed until the legs carry the craft. The main
// body follows a moving target like a critically damped spring of angular
// frequency c_liftOmega, and is held level.
const float LeapFrog::c_liftHeight = 4.0f;
const float LeapFrog::c_liftRiseSpeed = 2.0f;          // [units/s]
const float LeapFrog::c_liftLowerSpeed = 1.5f;         // [units/s]
const float LeapFrog::c_liftOmega = 3.0f;              // [rad/s]
const float LeapFrog::c_liftLevelStiffness = 60000.0f; // per rad
const float LeapFrog::c_liftLevelDamping = 30000.0f;   // per rad/s
const float LeapFrog::c_liftTimeout = 12.0f;           // [s]


void LeapFrog::dumpPart(std::string name, b2Body* body, b2Body* mainBody)
{
   b2Vec2 p;
   b2Vec2 mp;
   float a;
   float ma;

   mp = mainBody->GetPosition();
   p = body->GetPosition() - mp;
   ma = mainBody->GetAngle();
   a = body->GetAngle() - ma /* + MATH_PI / 2.0f*/;
   logs::messageln("%s: pos: %f, %f, angle: %f", name.c_str(), p.x, p.y, a);
}

void LeapFrog::dumpParts(void)
{
   dumpPart("Main Body:", m_mainBody, m_mainBody);
   dumpPart("Right Big leg:", m_rightBigLegBody, m_mainBody);
   dumpPart("Left Big leg:", m_leftBigLegBody, m_mainBody);
   dumpPart("Right Small leg:", getBody("lfRightSmallLeg"), m_mainBody);
   dumpPart("Left Small leg:", getBody("lfLeftSmallLeg"), m_mainBody);
   dumpPart("Right foot:", getBody("lfRightFoot"), m_mainBody);
   dumpPart("Left foot:", getBody("lfLeftFoot"), m_mainBody);
   dumpPart("Right Steer Booster:", m_rightSteerBody, m_mainBody);
   dumpPart("Left Steer Booster:", m_leftSteerBody, m_mainBody);
}

void LeapFrog::dumpReentryFlamees(void)
{
   logs::message("Left reentry flame: ");
   logs::messageln(m_reentryFlameEmitterLeftLeg->dumpData().c_str());
   logs::message("Center reentry flame: ");
   logs::messageln(m_reentryFlameEmitterBooster->dumpData().c_str());
   logs::message("Right reentry flame: ");
   logs::messageln(m_reentryFlameEmitterRightLeg->dumpData().c_str());
}


void LeapFrog::collisionBlast(b2Contact* contact, bool small)
// Sparks blastemitter, small is default, can also be big
{
   // Assume small blast
   int emitterLifetime = 150;
   int particleLifetime = 500;
   float particleDistance = 30.0f;
   float particleSize = 0.75f;
   float blastIntensity = 200.0f;

   if (!small)
   {
      emitterLifetime = 250;
      particleLifetime = 500;
      particleDistance = 60.0f;
      particleSize = 0.9f;
      blastIntensity = 300.0f;
   }

   b2WorldManifold m;
   contact->GetWorldManifold(&m);

   if (contact->GetManifold()->pointCount > 0)
   {
      spBlastEmitter blast = new BlastEmitter(
         PhysDispConvert::convert(m.points[0], 1.0f),
         blastIntensity,                                     // Intensity, particles / sec
         emitterLifetime,                                    // Emitter Lifetime
         particleLifetime,                                   // Particle lifetime
         particleDistance,                                   // Particle distance
         particleSize);                                      // Particle spawn size
      blast->attachTo(m_sceneActor);
   }
}

void LeapFrog::evaluateLepfrogDamage(void)
{
   // TODO: Evaluate game over

}

void LeapFrog::hitImpulse(b2Contact* contact, const b2ContactImpulse* impulse)
{
   collisionBlast(contact, true);

   float normalImpulses = 0.0f;

   for (int i = 0; i < impulse->count; i++)
   {
      normalImpulses += impulse->normalImpulses[i];
   }

   m_resources->deltaDamage(normalImpulses / 100.0f);

   evaluateLepfrogDamage();

}

void LeapFrog::hitByBullet(b2Contact* contact, float bulletEqvDamage)
{
   collisionBlast(contact, true);

   m_resources->deltaDamage(25.0f);

   evaluateLepfrogDamage();
}

void LeapFrog::footOnLandingPad(bool resting)
{
   m_feetOnPad = std::max(0, m_feetOnPad + (resting ? 1 : -1));

   if (resting)
   {
      lockJoints(true);
   }
   else
   {
      unlockJoints();
   }
}

// The resources of the leapfrog is saved in the game status and uses a pointer to that instance. 
// This means that the initObjectResources is called with the mainactor gamestatus resources as
// a reference.
void LeapFrog::initObjectResources(Actor* statusEventOriginator, ObjectResources* resources)
{
   m_resources = resources;

   // The event to send on property change must contain a pointer to the ObjectProperty.
   // The ObjectProperty is created without an event (set to NULL) and the event is 
   // then defined with the registerPropertySetEvent method

   ObjectProperty ammo(this, NULL, 5, 0.0f);
   ObjectProperty shield(this, NULL, 6, 0.0f);
   ObjectProperty fuel(this, NULL, 7, 0.0f);
   ObjectProperty credits(this, NULL, 8, 0.0f);
   ObjectProperty damage(this, NULL, 9, 0.0f);
   
   m_properties.push_back(ammo);
   m_properties.push_back(shield);
   m_properties.push_back(fuel);
   m_properties.push_back(credits);
   m_properties.push_back(damage);

   if (m_properties.size() == LeapfroPropertiesEnum::legHealth)
   {
      // Leg health, set externally (e.g. Lua) to repair the legs
      m_properties.push_back(ObjectProperty(this, new LeapfrogExtSetLegHealthEvent, 10, getLowestLegHealth()));
   }

   m_resources->initObjectResources(
      statusEventOriginator, 
      &m_properties[LeapfroPropertiesEnum::ammo], 
      &m_properties[LeapfroPropertiesEnum::shield],
      &m_properties[LeapfroPropertiesEnum::fuel],
      &m_properties[LeapfroPropertiesEnum::credits],
      &m_properties[LeapfroPropertiesEnum::damage]);

   g_GameStatus.restoreLeapfrogResources();

   m_resources->registerShieldObject(m_shield.get());
}


// ---------------------------------------------------------------------------
// Leg joint damage
//
// In landing configuration the leg joints, not only the hull, can be hurt.
// Each of the six leg joints has a health (1 = sound). A joint that is
// overloaded or bent too far loses health; a weakened joint's motor gets
// weaker, so it sags; at 0 the joint goes limp: still attached, but its
// motor is gone and the leg swings like a dead limb. Health is kept in the
// game status, so a limp leg stays limp until repaired (property 10).

namespace
{
   const char* const c_legNames[LeapFrog::c_numLegJoints] =
   {
      "Left hip", "Left knee", "Left foot", "Right hip", "Right knee", "Right foot"
   };

   float ModeAngles::* const c_legGoal[LeapFrog::c_numLegJoints] =
   {
      &ModeAngles::m_leftBigJointAngle,
      &ModeAngles::m_leftSmallJointAngle,
      &ModeAngles::m_leftFootJointAngle,
      &ModeAngles::m_rightBigJointAngle,
      &ModeAngles::m_rightSmallJointAngle,
      &ModeAngles::m_rightFootJointAngle
   };
}

b2RevoluteJoint* LeapFrog::getLegJoint(int index)
{
   switch (index)
   {
   case 0: return m_leftBigLegJoint;
   case 1: return m_leftSmallLegJoint;
   case 2: return m_leftFootLegJoint;
   case 3: return m_rightBigLegJoint;
   case 4: return m_rightSmallLegJoint;
   case 5: return m_rightFootLegJoint;
   }

   return NULL;
}

int LeapFrog::getLegJoints(b2RevoluteJoint* joints[c_numLegJoints], float goals[c_numLegJoints])
{
   for (int i = 0; i < c_numLegJoints; i++)
   {
      joints[i] = getLegJoint(i);
      goals[i] = m_modeAngleGoals.*c_legGoal[i];
   }

   return c_numLegJoints;
}

JointUserData* LeapFrog::legData(b2RevoluteJoint* joint)
{
   if (joint == NULL)
   {
      return NULL;
   }

   JointUserData* jd = (JointUserData*)joint->GetUserData();

   if ((jd == NULL) || (jd->m_legIndex < 0))
   {
      return NULL;
   }

   return jd;
}

void LeapFrog::initLegJoints(void)
{
   const ModeAngles* poses[] =
   {
      &c_landingModeAngles, &c_deepSpaceModeAngles, &c_orbitModeAngles, &c_reentryModeAngles
   };

   for (int i = 0; i < c_numLegJoints; i++)
   {
      b2RevoluteJoint* joint = getLegJoint(i);

      if (joint == NULL)
      {
         continue;
      }

      JointUserData* jd = (JointUserData*)joint->GetUserData();

      if (jd == NULL)
      {
         continue;
      }

      jd->m_legIndex = i;
      jd->m_legName = c_legNames[i];

      // Limp range: the joint's whole travel (all poses, and hanging
      // straight down, which is angle 0) plus a margin
      float lower = 0.0f;
      float upper = 0.0f;

      for (const ModeAngles* pose : poses)
      {
         float a = pose->*c_legGoal[i];
         lower = std::min(lower, a);
         upper = std::max(upper, a);
      }

      jd->m_limpLower = lower - c_legLimpMargin;
      jd->m_limpUpper = upper + c_legLimpMargin;

      jd->m_health = std::max(0.0f, std::min(1.0f, g_GameStatus.getLegHealth(i)));

      if (jd->m_health <= 0.0f)
      {
         makeLegLimp(joint, false);
      }
      else
      {
         applyLegStrength(joint);
      }
   }

   reportLegHealth();
}

void LeapFrog::applyLegStrength(b2RevoluteJoint* joint)
{
   JointUserData* jd = legData(joint);

   if (joint == NULL)
   {
      return;
   }

   if (jd == NULL)
   {
      joint->SetMaxMotorTorque(c_normalJointMotorTorque);
      return;
   }

   if (jd->m_limp)
   {
      return;
   }

   float scale = c_legMinTorqueScale + (1.0f - c_legMinTorqueScale) * jd->m_health;
   joint->SetMaxMotorTorque(c_normalJointMotorTorque * scale);
}

void LeapFrog::makeLegLimp(b2RevoluteJoint* joint, bool announce)
{
   JointUserData* jd = legData(joint);

   if (jd == NULL)
   {
      return;
   }

   bool wasLimp = jd->m_limp;

   jd->m_limp = true;
   jd->m_health = 0.0f;
   g_GameStatus.setLegHealth(jd->m_legIndex, 0.0f);

   // Keep the current angle inside the limits so the leg doesn't snap
   float angle = joint->GetJointAngle();
   joint->SetLimits(std::min(jd->m_limpLower, angle), std::max(jd->m_limpUpper, angle));
   joint->EnableLimit(true);

   // A weak motor holding speed 0 is friction: the leg swings heavily
   joint->SetMotorSpeed(0.0f);
   joint->SetMaxMotorTorque(c_legLimpFriction);
   joint->EnableMotor(true);

   if (announce && !wasLimp)
   {
      logs::messageln("Leg joint failed: %s", jd->m_legName);

      if (g_HeadDownDisplay)
      {
         g_HeadDownDisplay->logMessage(std::string(jd->m_legName) + " actuator failed", true);
      }

      // Sparks where the joint gave way
      spBlastEmitter blast = new BlastEmitter(
         PhysDispConvert::convert(joint->GetAnchorA(), 1.0f),
         250.0f,                                             // Intensity, particles / sec
         250,                                                // Emitter Lifetime
         500,                                                // Particle lifetime
         40.0f,                                              // Particle distance
         0.8f);                                              // Particle spawn size
      blast->attachTo(m_sceneActor);
   }
}

void LeapFrog::repairLeg(b2RevoluteJoint* joint)
{
   JointUserData* jd = legData(joint);

   if (jd == NULL)
   {
      return;
   }

   jd->m_limp = false;
   jd->m_strain = 0.0f;
   jd->m_strainCounted = 0.0f;
   jd->m_strainPeak = 0.0f;
   jd->m_bendCounted = 0.0f;
   jd->m_bendPeak = 0.0f;

   joint->EnableLimit(false);
   joint->EnableMotor(true);
   applyLegStrength(joint);
}

float LeapFrog::getLegHealth(int index)
{
   JointUserData* jd = legData(getLegJoint(index));

   if (jd != NULL)
   {
      return jd->m_health;
   }

   return g_GameStatus.getLegHealth(index);
}

void LeapFrog::setLegHealth(int index, float health)
{
   health = std::max(0.0f, std::min(1.0f, health));

   b2RevoluteJoint* joint = getLegJoint(index);
   JointUserData* jd = legData(joint);

   if (jd == NULL)
   {
      g_GameStatus.setLegHealth(index, health);
      return;
   }

   bool wasLimp = jd->m_limp;
   float healthBefore = jd->m_health;

   if (health <= 0.0f)
   {
      makeLegLimp(joint, true);
   }
   else
   {
      jd->m_health = health;
      g_GameStatus.setLegHealth(index, health);

      if (wasLimp)
      {
         repairLeg(joint);

         // Drive the repaired leg back to the current pose
         goToMode(m_mode);
      }
      else
      {
         applyLegStrength(joint);
      }

      // Standing on a pad, a repaired leg may be curled under the craft's
      // weight; lift the craft so it can straighten
      if (health > healthBefore + 0.0001f)
      {
         requestServiceLift();
      }
   }

   reportLegHealth();
}

void LeapFrog::setAllLegHealth(float health)
{
   for (int i = 0; i < c_numLegJoints; i++)
   {
      setLegHealth(i, health);
   }
}

float LeapFrog::getLowestLegHealth(void)
{
   float lowest = 1.0f;

   for (int i = 0; i < c_numLegJoints; i++)
   {
      lowest = std::min(lowest, getLegHealth(i));
   }

   return lowest;
}

void LeapFrog::reportLegHealth(void)
{
   if (m_properties.size() <= LeapfroPropertiesEnum::legHealth)
   {
      return;
   }

   float lowest = getLowestLegHealth();

   if (fabs(lowest - m_reportedLegHealth) > 0.001f)
   {
      m_reportedLegHealth = lowest;
      m_properties[LeapfroPropertiesEnum::legHealth].setProperty(lowest);
   }
}

void LeapFrog::setLegHealthPropHandler(oxygine::Event *ev)
{
   float value = m_properties[LeapfroPropertiesEnum::legHealth].getProperty();

   // Repair: raise every joint to at least the value (a sound joint is
   // never made worse, so 0.3 patches limp joints without hurting the rest)
   for (int i = 0; i < c_numLegJoints; i++)
   {
      if (getLegHealth(i) < value)
      {
         setLegHealth(i, value);
      }
   }

   // Make sure the property is written back even if nothing changes
   m_reportedLegHealth = -1.0f;
   reportLegHealth();
}

void LeapFrog::damageLeg(b2RevoluteJoint* joint, float damage)
{
   JointUserData* jd = legData(joint);

   if ((jd == NULL) || jd->m_limp || (damage <= 0.0f))
   {
      return;
   }

   float before = jd->m_health;
   float after = std::max(0.0f, before - damage);

   logs::messageln("Leg damage: %s %.2f -> %.2f", jd->m_legName, before, after);

   if (after <= 0.0f)
   {
      makeLegLimp(joint, true);
      reportLegHealth();
      return;
   }

   jd->m_health = after;
   g_GameStatus.setLegHealth(jd->m_legIndex, after);
   applyLegStrength(joint);

   if ((before >= c_legWeakHealth) && (after < c_legWeakHealth) && g_HeadDownDisplay)
   {
      g_HeadDownDisplay->logMessage(std::string(jd->m_legName) + " strained");
   }

   // Too weak to be locked: if it is locked on a pad, hand it to its motor
   if ((before >= c_legLockHealth) && (after < c_legLockHealth) &&
      joint->IsLimitEnabled() && !joint->IsMotorEnabled())
   {
      joint->EnableLimit(false);
      joint->EnableMotor(true);
   }

   reportLegHealth();
}

void LeapFrog::updateLegStrain(void)
{
   bool counting = (m_mode == LFM_LANDING) && !m_modeInTransit;

   if (g_MainActor && g_MainActor->isInPause())
   {
      // Box2D isn't stepping; its last impulses are stale
      return;
   }

   const float dt = Scales::c_updateStep;
   const float invDt = 1.0f / dt;
   const float decay = expf(-dt / c_legStrainMemory);

   b2RevoluteJoint* joints[c_numLegJoints];
   float goals[c_numLegJoints];
   getLegJoints(joints, goals);

   for (int i = 0; i < c_numLegJoints; i++)
   {
      b2RevoluteJoint* joint = joints[i];
      JointUserData* jd = legData(joint);

      if ((jd == NULL) || jd->m_limp)
      {
         continue;
      }

      if (!counting)
      {
         // Folding legs or other modes: no damage, forget recent strain
         jd->m_strain = 0.0f;
         jd->m_strainCounted = 0.0f;
         jd->m_strainPeak = 0.0f;
         jd->m_bendCounted = 0.0f;
         jd->m_bendPeak = 0.0f;
         continue;
      }

      float damage = 0.0f;

      // 1. Overload: torque through the limit (locked) and the motor
      float torque = 0.0f;

      if (joint->IsLimitEnabled())
      {
         torque += fabs(joint->GetReactionTorque(invDt));
      }

      if (joint->IsMotorEnabled())
      {
         torque += fabs(joint->GetMotorTorque(invDt));
      }

      float excess = std::max(0.0f, torque - c_legSustainTorque) * dt;

      jd->m_strain = jd->m_strain * decay + excess;
      jd->m_strainCounted *= decay;

      // Charge each event once, for how far its peak goes above yield
      float above = jd->m_strain - std::max(c_legYieldImpulse, jd->m_strainCounted);

      if (above > 0.0f)
      {
         damage += above / c_legToughnessImpulse;
         jd->m_strainCounted = jd->m_strain;
      }

      if (jd->m_strain > jd->m_strainPeak)
      {
         jd->m_strainPeak = jd->m_strain;
      }
      else if (jd->m_strain < 0.1f * c_legYieldImpulse)
      {
         if (jd->m_strainPeak > 0.3f * c_legYieldImpulse)
         {
            logs::messageln("Leg strain: %s peak %.0f (yield %.0f, toughness %.0f)",
               jd->m_legName, jd->m_strainPeak, c_legYieldImpulse, c_legToughnessImpulse);
         }

         jd->m_strainPeak = 0.0f;
      }

      // 2. Over-bending: a motor-held joint forced away from its pose
      if (joint->IsMotorEnabled() && !joint->IsLimitEnabled())
      {
         float bend = fabs(joint->GetJointAngle() - goals[i]);

         if (bend > c_legBendYield)
         {
            float base = std::max(c_legBendYield, jd->m_bendCounted);

            if (bend > base)
            {
               damage += (bend - base) / c_legBendToughness;
               jd->m_bendCounted = bend;
            }
         }
         else
         {
            jd->m_bendCounted = 0.0f;
         }

         if (bend > jd->m_bendPeak)
         {
            jd->m_bendPeak = bend;
         }
         else if (bend < 0.5f * c_legBendYield)
         {
            if (jd->m_bendPeak > 0.7f * c_legBendYield)
            {
               logs::messageln("Leg bend: %s peak %.1f deg (yield %.1f, toughness %.1f)",
                  jd->m_legName, jd->m_bendPeak * 180.0f / MATH_PI,
                  c_legBendYield * 180.0f / MATH_PI, c_legBendToughness * 180.0f / MATH_PI);
            }

            jd->m_bendPeak = 0.0f;
         }
      }

      if (damage > 0.0f)
      {
         damageLeg(joint, damage);
      }
   }
}


// ---------------------------------------------------------------------------
// Service lift
//
// A leg repaired while the craft stands on a pad may be folded under it, and
// the motors can't straighten a leg that carries the craft. So after a
// repair the craft is lifted a few units without the booster (an invisible
// hand, as a crane would), held until the legs have their pose, then set
// down gently. Landing pads freeze meanwhile (LandingPad::doUpdate), so the
// lift sends no take-off or landing events. Firing the booster ends it.

bool LeapFrog::legsAtPose(void)
{
   // Every working leg joint within a few degrees of its pose
   b2RevoluteJoint* joints[c_numLegJoints];
   float goals[c_numLegJoints];
   getLegJoints(joints, goals);

   for (int i = 0; i < c_numLegJoints; i++)
   {
      JointUserData* jd = legData(joints[i]);

      if ((jd == NULL) || jd->m_limp)
      {
         continue;
      }

      if (fabs(joints[i]->GetJointAngle() - goals[i]) > 3.0f * MATH_PI / 180.0f)
      {
         return false;
      }
   }

   return true;
}

void LeapFrog::requestServiceLift(void)
{
   if ((m_serviceLiftPhase >= 0) || (m_mode != LFM_LANDING) || (m_feetOnPad <= 0))
   {
      return;
   }

   const char* parts[] =
   {
      "lfMainBody", "lfLeftBigLeg", "lfRightBigLeg", "lfLeftSmallLeg", "lfRightSmallLeg",
      "lfLeftFoot", "lfRightFoot", "lfLeftSteer", "lfRightSteer"
   };

   m_serviceLiftMass = 0.0f;

   for (const char* part : parts)
   {
      b2Body* body = getBody(part);
      if (body != NULL)
      {
         m_serviceLiftMass += body->GetMass();
      }
   }

   m_serviceLiftPhase = 0;
   m_serviceLiftTime = 0.0f;
   m_serviceLiftStartY = m_mainBody->GetPosition().y;
   m_serviceLiftTarget = m_serviceLiftStartY;

   logs::messageln("Service lift: start (mass %.0f)", m_serviceLiftMass);
}

void LeapFrog::endServiceLift(const char* why)
{
   if (m_serviceLiftPhase >= 0)
   {
      logs::messageln("Service lift: end (%s) after %.1f s", why, m_serviceLiftTime);
   }

   m_serviceLiftPhase = -1;
}

void LeapFrog::updateServiceLift(void)
{
   if (m_serviceLiftPhase < 0)
   {
      return;
   }

   if (g_MainActor && g_MainActor->isInPause())
   {
      return;
   }

   if (m_boostFireLastUpdate)
   {
      endServiceLift("booster");
      return;
   }

   if (m_mode != LFM_LANDING)
   {
      endServiceLift("mode");
      return;
   }

   const float dt = Scales::c_updateStep;
   m_serviceLiftTime += dt;

   if (m_serviceLiftTime > c_liftTimeout)
   {
      endServiceLift("timeout");
      return;
   }

   b2Vec2 pos = m_mainBody->GetPosition();
   b2Vec2 vel = m_mainBody->GetLinearVelocity();

   // The game's y axis points down: up is negative y
   float goal = m_serviceLiftStartY - c_liftHeight;
   float targetVel = 0.0f;

   if (m_serviceLiftPhase == 0)
   {
      if (m_serviceLiftTarget > goal)
      {
         targetVel = -c_liftRiseSpeed;
         m_serviceLiftTarget = std::max(goal, m_serviceLiftTarget + targetVel * dt);
      }
      else if ((fabs(pos.y - goal) < 0.5f) && !m_modeInTransit && legsAtPose())
      {
         // Up, and the legs have their pose: set it down
         m_serviceLiftPhase = 1;
      }
   }
   else
   {
      targetVel = c_liftLowerSpeed;
      m_serviceLiftTarget += targetVel * dt;

      // The target has gone on below the body: the legs carry the craft
      if (m_serviceLiftTarget - pos.y > 1.0f)
      {
         endServiceLift("down");
         return;
      }
   }

   // Critically damped spring towards the moving target, plus gravity
   float w = c_liftOmega;
   float g = m_world->GetGravity().y;
   float ay = w * w * (m_serviceLiftTarget - pos.y) + 2.0f * w * (targetVel - vel.y) - g;
   float ax = -2.0f * w * vel.x;

   m_mainBody->ApplyForceToCenter(b2Vec2(m_serviceLiftMass * ax, m_serviceLiftMass * ay), true);

   // Hold it level
   float angle = m_mainBody->GetAngle();
   while (angle > MATH_PI) angle -= 2.0f * MATH_PI;
   while (angle < -MATH_PI) angle += 2.0f * MATH_PI;

   m_mainBody->ApplyTorque(
      -c_liftLevelStiffness * angle - c_liftLevelDamping * m_mainBody->GetAngularVelocity(), true);
}
