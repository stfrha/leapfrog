#include "physdispconvert.h"
#include "scales.h"

#include "SDL.h"
#include "SDL_keyboard.h"

#include "leapfrog.h"
#include "shield.h"

#include "leapfrogevents.h"

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
	oxygine::Resources& gameResources,
	oxygine::Actor* parent,
	b2World* world,
	const oxygine::Vector2& pos,
	std::string& defXmlFileName) :
	m_world(world),
   m_boostMagnuitude(0.0f),
   m_steerMagnitude(0.0f),
   m_boostInc(900.0f),
   m_boostMaxMagnitude(3000.0f),
   m_steerMaxMagnitude(10000.0f),
   m_eveningMagnitude(10000.0f),
   m_boostFireLastUpdate(false),
   m_rightSteerFireLastUpdate(false),
   m_leftSteerFireLastUpdate(false),
   m_gameResources(&gameResources),
   m_state(LFS_NORMAL),
   m_wantedAngle(0.0f),
   m_initiating(false)
{
	initCompoundObject(gameResources, parent, world, pos, defXmlFileName, string(""));

   m_sceneActor = (SceneActor*) parent;

   m_mainObject = getObject("lfMainBody");
   m_lfRightBigLeg = getObject("lfRightBigLeg");
   m_lfLeftBigLeg = getObject("lfLeftBigLeg");
   m_mainBody = (b2Body*)m_mainObject->getUserData();
   m_rightBigLegBody = (b2Body*)m_lfRightBigLeg->getUserData();
   m_leftBigLegBody = (b2Body*)m_lfLeftBigLeg->getUserData();

   m_mainBody = getBody("lfMainBody");
	m_boostBody = getBody("lfBooster");
	m_rightSteerBody = getBody("lfRightSteer");
	m_leftSteerBody = getBody("lfLeftSteer");

   JointUserData* jd;


	m_boostJoint = static_cast<b2WeldJoint*>(getJoint("boostJoint"));
   jd = new JointUserData(false);
   m_boostJoint->SetUserData(jd);

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
   m_shield = new Shield(gameResources, world, pos);
   m_shield->attachTo(parent);
   m_shield->setPriority(147);


   b2RevoluteJointDef shieldJointDef;
   shieldJointDef.bodyA = m_mainBody;
   shieldJointDef.bodyB = m_shield->m_body;
   shieldJointDef.localAnchorA.Set(0.0f, 1.0f);
   shieldJointDef.localAnchorB.Set(0.0f, 0.0f);
   shieldJointDef.collideConnected = false;
   shieldJointDef.enableMotor = false;
   m_shieldJoint = (b2RevoluteJoint*)m_world->CreateJoint(&shieldJointDef);
   
   //	m_shieldJoint = static_cast<b2RevoluteJoint*>(getJoint("boostJoint"));

   // Add main engine particle system
   m_boosterFlame = new FlameEmitter(
      gameResources, 
      m_boostBody,
      b2Vec2(0.0f, 3.0f), 
      90.0f * MATH_PI / 180.0f, 
      4.0f,                            // Emitter width
      0,                               // Intensity
      250,                             // Lifetime [ms]
      70.0f,                           // Impulse magnitude
      10.0f);                          // Radius

   m_boosterFlame->attachTo(this);
	  
   // Add right steering engine particle system
   m_rightSteerFlame = new FlameEmitter(
      gameResources,
	   m_rightSteerBody,
      b2Vec2(-1.0f, 0.0f),
      MATH_PI,
      1.0f,                            // Width
      0,                               // Intensity
      125,                             // Lifetime [ms]
      35.0f,                           // Impulse magnitude
      5.0f);                           // Radius


   m_rightSteerFlame->attachTo(this);

   // Add left steering engine particle system
   m_leftSteerFlame = new FlameEmitter(
      gameResources,
	   m_leftSteerBody,
      b2Vec2(1.0f, 0.0f),             // Origin
      0,                               // Angle 
      1.0f,                            // Width
      0,                               // Intensity
      125,                             // Lifetime [ms]
      35.0f,                           // Impulse magnitude
      5.0f);                           // Radius

   m_leftSteerFlame->attachTo(this);

   m_gun = new Gun(
      gameResources,
      m_sceneActor,
      m_mainBody,
      b2Vec2(0.0f, -4.0f),             // Origin
      3.0f * MATH_PI / 2.0f,           // Angle 
      4.0f,                            // Intensity [bullets per second]
      2000,                            // Lifetime [ms]
      1000.0f,                         // Bullet speed
      false);                          // Bouncy

   m_gun->attachTo(this);

   m_reentryFlameEmitterBooster = new ReentryFlameEmitter(
      gameResources,
      m_boostBody,
      b2Vec2(-2.0, 0.8f),
      b2Vec2(2.0f, 0.8f),
      MATH_PI / 2.0f,
      4.0f,
      500,
      b2Vec2(0.1f, 5.0f));

   m_reentryFlameEmitterBooster->attachTo(this);

   m_reentryFlameEmitterRightLeg = new ReentryFlameEmitter(
      gameResources,
      m_rightBigLegBody,
      b2Vec2(2.0, 1.7f),
      b2Vec2(8.0f, 1.7f),
      MATH_PI / 2.0f,
      4.0f,
      500,
      b2Vec2(0.1f, 5.0f));

   m_reentryFlameEmitterRightLeg->attachTo(this);

   m_reentryFlameEmitterLeftLeg = new ReentryFlameEmitter(
      gameResources,
	   m_leftBigLegBody,
      b2Vec2(-8.0, 1.7f),
      b2Vec2(-2.0f, 1.7f),
      MATH_PI / 2.0f,
      4.0f,
      500,
      b2Vec2(0.1f, 5.0f));

   m_reentryFlameEmitterLeftLeg->attachTo(this);

   // Register all properties:
   m_properties.push_back(ObjectProperty(this, new LeapfrogExtSetModeEvent, 0, 0.0f)); // Mode
   m_properties.push_back(ObjectProperty(this, new LeapfrogExtSetEnvEvent, 1, 0.0f)); // Environment
   m_properties.push_back(ObjectProperty(this, NULL, 2, 0.0f, true)); // State
   m_properties.push_back(ObjectProperty(this, NULL, 3, 0.0f, true)); // xPos
   m_properties.push_back(ObjectProperty(this, NULL, 4, 0.0f, true)); // yPos

   addEventListener(LeapfrogExtSetModeEvent::EVENT, CLOSURE(this, &LeapFrog::setModePropHandler));
   addEventListener(LeapfrogExtSetEnvEvent::EVENT, CLOSURE(this, &LeapFrog::setEnvPropHandler));

   // Here we attach Leapfrog object to tree so it gets updates etc.
   attachTo(parent);
   setWeakJoints();
}

void LeapFrog::setModePropHandler(oxygine::Event *ev)
{
   dumpParts();

   float value = m_properties[propMode].getProperty();
   
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
   //float value = m_properties[propMode].getProperty();

   //goToEnvironment((int) value);
}

LeapFrog::~LeapFrog()
{
}

oxygine::Actor* LeapFrog::getMainActor(void)
{
   return m_mainObject;
}

//void LeapFrog::setPropertyImpl(int propId, float value)
//{
//   if (propId == propMode)
//   {
//      if ((value > -0.5) && (value < 0.5))
//      {
//         goToMode(LFM_RESET);
//      }
//      else if ((value > 0.5) && (value < 1.5))
//      {
//         goToMode(LFM_DEEP_SPACE);
//      }
//      else if ((value > 1.5) && (value < 2.5))
//      {
//         goToMode(LFM_LANDING);
//      }
//      else if ((value > 2.5) && (value < 3.5))
//      {
//         goToMode(LFM_ORBIT);
//      }
//      else if ((value > 3.5) && (value < 4.5))
//      {
//         goToMode(LFM_REENTRY);
//      }
//   }
//}
//

//void LeapFrog::initLeapfrog(SceneActor* sceneActor)
//{
//	m_sceneActor = sceneActor;
//}


CollisionEntityTypeEnum LeapFrog::getEntityType(void)
{
   return CET_LEAPFROG;
}

void LeapFrog::hitByAsteroid(b2Contact* contact)
{
   // Damage to LeapFrog!
}


void LeapFrog::doUpdate(const UpdateState &us)
{
   b2Vec2 pos = m_mainBody->GetPosition();
   m_properties[propXPos].setProperty(pos.x);
   m_properties[propYPos].setProperty(pos.y);

   float angle = 0.0f;
   b2Vec2 boostForce = b2Vec2(0.0f, 0.0f);
   float magAngVel = 0.0f;


   switch (m_state)
   {
   case LFS_NORMAL:
   case LFS_MODE_IN_TRANSIT:
      angle = m_mainBody->GetAngle();
      boostForce = b2Vec2(m_boostMagnuitude * sin(angle), -m_boostMagnuitude * cos(angle));

      if (m_environment != ENV_DEEP_SPACE)
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

   case LFS_INITIATING_MODE:
      setJointMotor(m_rightBigLegJoint, m_modeAngleGoals.m_rightBigJointAngle, c_instnatJointMotorSpeed);
      setJointMotor(m_rightSmallLegJoint, m_modeAngleGoals.m_rightSmallJointAngle, c_instnatJointMotorSpeed);
      setJointMotor(m_rightFootLegJoint, m_modeAngleGoals.m_rightFootJointAngle, c_instnatJointMotorSpeed);
      setJointMotor(m_leftBigLegJoint, m_modeAngleGoals.m_leftBigJointAngle, c_instnatJointMotorSpeed);
      setJointMotor(m_leftSmallLegJoint, m_modeAngleGoals.m_leftSmallJointAngle, c_instnatJointMotorSpeed);
      setJointMotor(m_leftFootLegJoint, m_modeAngleGoals.m_leftFootJointAngle, c_instnatJointMotorSpeed);
      break;

   case LFS_INSTANTLY_ROTATING:
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
      magAngVel =  4.0f * fabs(m_wantedAngle - angle); // get there in one second

      // check if we have reached the gaol
      if (magAngVel < 1.0f * MATH_PI / 180.0f)
      {
         instantAngleReached();
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

      break;

   case LFS_GET_TO_EQUILIBRIUM:

      if (fabs(m_mainBody->GetAngularVelocity()) < 0.1f)
      {
         equilibriumReached();
      }
      else
      {
         m_mainBody->SetAngularVelocity(0.0f);
      }
      break;

   case LFS_HOLD_ANGLE:
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
};

void LeapFrog::setJointMotor(b2RevoluteJoint* joint, float goal, float speedMagnitude)
{
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
         if ((m_state == LFS_MODE_IN_TRANSIT) || (m_state == LFS_INITIATING_MODE))
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
         if ((m_state == LFS_MODE_IN_TRANSIT) || (m_state == LFS_INITIATING_MODE))
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

void LeapFrog::lockJoints(void)
{
   // Lock all legs and disable motors
   if (m_rightBigLegJoint)
   {
      m_rightBigLegJoint->EnableMotor(false);
      m_rightBigLegJoint->SetLimits(m_modeAngleGoals.m_rightBigJointAngle, m_modeAngleGoals.m_rightBigJointAngle);
      m_rightBigLegJoint->EnableLimit(true);
   }

   if (m_leftBigLegJoint)
   {
      m_leftBigLegJoint->EnableMotor(false);
      m_leftBigLegJoint->SetLimits(m_modeAngleGoals.m_leftBigJointAngle, m_modeAngleGoals.m_leftBigJointAngle);
      m_leftBigLegJoint->EnableLimit(true);
   }

   if (m_rightSmallLegJoint)
   {
      m_rightSmallLegJoint->EnableMotor(false);
      m_rightSmallLegJoint->SetLimits(m_modeAngleGoals.m_rightSmallJointAngle, m_modeAngleGoals.m_rightSmallJointAngle);
      m_rightSmallLegJoint->EnableLimit(true);
   }

   if (m_leftSmallLegJoint)
   {
      m_leftSmallLegJoint->EnableMotor(false);
      m_leftSmallLegJoint->SetLimits(m_modeAngleGoals.m_leftSmallJointAngle, m_modeAngleGoals.m_leftSmallJointAngle);
      m_leftSmallLegJoint->EnableLimit(true);
   }

   if (m_rightFootLegJoint)
   {
      m_rightFootLegJoint->EnableMotor(false);
      m_rightFootLegJoint->SetLimits(m_modeAngleGoals.m_rightFootJointAngle, m_modeAngleGoals.m_rightFootJointAngle);
      m_rightFootLegJoint->EnableLimit(true);
   }

   if (m_leftFootLegJoint)
   {
      m_leftFootLegJoint->EnableMotor(false);
      m_leftFootLegJoint->SetLimits(m_modeAngleGoals.m_leftFootJointAngle, m_modeAngleGoals.m_leftFootJointAngle);
      m_leftFootLegJoint->EnableLimit(true);
   }
}

void LeapFrog::unlockJoints(void)
{
   // Unlock all legs and enable motors
   if (m_rightBigLegJoint)
   {
      m_rightBigLegJoint->EnableMotor(true);
      m_rightBigLegJoint->EnableLimit(false);
   }

   if (m_leftBigLegJoint)
   {
      m_leftBigLegJoint->EnableMotor(true);
      m_leftBigLegJoint->EnableLimit(false);
   }

   if (m_rightSmallLegJoint)
   {
      m_rightSmallLegJoint->EnableMotor(true);
      m_rightSmallLegJoint->EnableLimit(false);
   }

   if (m_leftSmallLegJoint)
   {
      m_leftSmallLegJoint->EnableMotor(true);
      m_leftSmallLegJoint->EnableLimit(false);
   }

   if (m_rightFootLegJoint)
   {
      m_rightFootLegJoint->EnableMotor(true);
      m_rightFootLegJoint->EnableLimit(false);
   }

   if (m_leftFootLegJoint)
   {
      m_leftFootLegJoint->EnableMotor(true);
      m_leftFootLegJoint->EnableLimit(false);
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
   m_rightBigLegJoint->SetMaxMotorTorque(c_normalJointMotorTorque);
   m_leftBigLegJoint->SetMaxMotorTorque(c_normalJointMotorTorque);
   m_rightSmallLegJoint->SetMaxMotorTorque(c_normalJointMotorTorque);
   m_leftSmallLegJoint->SetMaxMotorTorque(c_normalJointMotorTorque);
   m_rightFootLegJoint->SetMaxMotorTorque(c_normalJointMotorTorque);
   m_leftFootLegJoint->SetMaxMotorTorque(c_normalJointMotorTorque);
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

void LeapFrog::setInstantAngle(float angle)
{
   m_wantedAngle = angle;

   stopAllJointMotors();
   lockJoints();

   m_state = LFS_INSTANTLY_ROTATING;
   m_properties[propState].setProperty((float)m_state);

}

void LeapFrog::setHoldAngle(float angle)
{
   m_wantedAngle = angle;

   m_state = LFS_HOLD_ANGLE;
   m_properties[propState].setProperty((float)m_state);
}

void LeapFrog::releaseHoldAngle(void)
{
   m_state = LFS_NORMAL;
   m_properties[propState].setProperty((float)m_state);
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


void LeapFrog::initLeapfrog(LeapFrogModeEnum mode, float angle)
{
   m_initiating = true;
   m_mode = mode;
   m_properties[propMode].setProperty((float)m_mode);

   setInstantAngle(angle);
}

void LeapFrog::initMode(LeapFrogModeEnum mode)
{
   m_mode = mode;
   m_properties[propMode].setProperty((float)m_mode);

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

   setStrongJoints();
   m_state = LFS_INITIATING_MODE;
   m_properties[propState].setProperty((float)m_state);
}

void LeapFrog::goToMode(LeapFrogModeEnum mode)
{
   m_state = LFS_MODE_IN_TRANSIT;
   m_properties[propState].setProperty((float)m_state);
   m_mode = mode;
   m_properties[propMode].setProperty((float)m_mode);

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
      m_maxVelocity = 200.0f;    // [m/s]
      m_maxAngularVelocity = MATH_PI; // [rad/s]
      m_boosterFlame->setParameters(0, 500, 90.0f, 10.0f);
      m_shield->m_body->SetActive(true);

      break;

   case ENV_GROUND:
      m_boostInc = 900.0f;
      m_boostMaxMagnitude = 3000.0f;
      m_steerMaxMagnitude = 40000.0f;;
      m_eveningMagnitude = 10000.0f;
      m_maxVelocity = 400.0f;    // [m/s]
      m_maxAngularVelocity = MATH_PI; // [rad/s]
      m_boosterFlame->setParameters(0, 250, 70.0f, 10.0f);
      m_shield->m_body->SetActive(false);
      break;

   case ENV_ORBIT:
      m_boostInc = 900.0f;
      m_boostMaxMagnitude = 3000.0f;
      m_steerMaxMagnitude = 30000.0f;;
      m_eveningMagnitude = 10000.0f;
      m_maxVelocity = 400.0f;    // [m/s]
      m_maxAngularVelocity = MATH_PI; // [rad/s]
      m_boosterFlame->setParameters(0, 250, 70.0f, 10.0f);
      m_shield->m_body->SetActive(false);
      break;

   case ENV_REENTRY:
      m_boostInc = 900.0f;
      m_boostMaxMagnitude = 3000.0f;
      m_steerMaxMagnitude = 30000.0f;;
      m_eveningMagnitude = 10000.0f;
      m_maxVelocity = 400.0f;    // [m/s]
      m_maxAngularVelocity = MATH_PI; // [rad/s]
      m_boosterFlame->setParameters(0, 250, 70.0f, 10.0f);
      m_shield->m_body->SetActive(false);
      break;

   case ENV_LAUNCH:
      m_boostInc = 900.0f;
      m_boostMaxMagnitude = 3000.0f;
      m_steerMaxMagnitude = 30000.0f;;
      m_eveningMagnitude = 10000.0f;
      m_maxVelocity = 400.0f;    // [m/s]
      m_maxAngularVelocity = MATH_PI; // [rad/s]
      m_boosterFlame->setParameters(0, 250, 70.0f, 10.0f);
      m_shield->m_body->SetActive(false);
      break;
   }

}

void LeapFrog::modeReached(void)
{
   resetModeReached();

   if (m_state == LFS_MODE_IN_TRANSIT)
   {
      // Don't know anything special here...
   }

   if (m_state == LFS_INITIATING_MODE)
   {
      stopAllJointMotors();
      setWeakJoints();
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

   if (m_initiating)
   {
//      setAlpha(255);
      

      m_initiating = false;
   }

   m_state = LFS_NORMAL;
   m_properties[propState].setProperty((float)m_state);

   LeapfrogModeReachedEvent event("Test");
   dispatchEvent(&event);

}

void LeapFrog::instantAngleReached(void)
{
   m_mainBody->SetAngularVelocity(0.0f);
   m_steerMagnitude = 0.0f;
 
   m_state = LFS_GET_TO_EQUILIBRIUM;
   m_properties[propState].setProperty((float)m_state);
}

void LeapFrog::equilibriumReached(void)
{
   m_state = LFS_NORMAL;
   m_properties[propState].setProperty((float)m_state);

   if (m_initiating)
   {
      initMode(m_mode);
   }
   else
   {
      if (m_mode != LFM_DEEP_SPACE)
      {
         unlockJoints();
      }
   }

}

void LeapFrog::holdAngleReached(void)
{
   //Don't know what to do here
   int a = 10;

}


void LeapFrog::fireMainBooster(bool fire)
{


   // Handle booster flame particles

   if (fire)
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


   // Handle forces of Leapfrog
   // This is done differently for Deep-space and ground environment
   // In Deep space we accellerate to a maximum speed and decellerate
   // fast when booster is turned off.
   // Steering is fast and near instantaneous and stop turning directly

   // In ground mode we apply force independently of the velocity
   // and rotation is slowly decellerating

   if (m_environment == ENV_DEEP_SPACE)
   {
      float vel = m_mainBody->GetLinearVelocity().Length();

      // We start damping when booster is released
      // and remove damping when it is fired
      if (fire)
      {
         if (m_boostFireLastUpdate == false)
         {
            // Turn damping off
            m_mainBody->SetLinearDamping(0.0f);
         }
      }
      else
      {
         if (m_boostFireLastUpdate == true)
         {
            // Turn damping on
            m_mainBody->SetLinearDamping(2.0f);
         }
      }

      if (fire)
      {
         // Increment force with m_boostInc until more than
         // m_boostMaxMagnitude and remain this max magnitude
         // force until speed is more than 90 of max speed, 
         // drop force then. Then, when no boost fire, 
         // break with a negative force linear with speed
         // making it regulate itself to stand-still
         if (vel < m_maxVelocity * 0.95f)
         {
            m_boostMagnuitude += m_boostInc;

            if (m_boostMagnuitude > m_boostMaxMagnitude)
            {
               m_boostMagnuitude = m_boostMaxMagnitude;
            }
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
      if (fire)
      {
         m_boostMagnuitude += m_boostInc;
         if (m_boostMagnuitude > m_boostMaxMagnitude)
         {
            m_boostMagnuitude = m_boostMaxMagnitude;
         }
      }
      else
      {
         m_boostMagnuitude = 0;
      }
   }

   if (fire)
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

   if (m_environment == ENV_DEEP_SPACE)
   {
      // In deep space we accellerate fast to a maximum
      // angular speed and hold that speed until steering
      // is released. We the immediately stop turning
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
      if (dir == -1)
      {
         m_steerMagnitude = -m_steerMaxMagnitude;
      }
      else if (dir == 1)
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

void LeapFrog::reentrySetHeat(unsigned char heatAmount)
{
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

void LeapFrog::setBoundedWallsActor(FreeSpaceActor* actor)
{
   m_gun->setBoundedWallsActor(actor);
}


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
   dumpPart("Booster:", m_boostBody, m_mainBody);
   dumpPart("Right Big leg:", m_rightBigLegBody, m_mainBody);
   dumpPart("Left Big leg:", m_leftBigLegBody, m_mainBody);
   dumpPart("Right Small leg:", getBody("lfRightSmallLeg"), m_mainBody);
   dumpPart("Left Small leg:", getBody("lfLeftSmallLeg"), m_mainBody);
   dumpPart("Right foot:", getBody("lfRightFoot"), m_mainBody);
   dumpPart("Left foot:", getBody("lfLeftFoot"), m_mainBody);
   dumpPart("Right Steer Booster:", m_rightSteerBody, m_mainBody);
   dumpPart("Left Steer Booster:", m_leftSteerBody, m_mainBody);
}


