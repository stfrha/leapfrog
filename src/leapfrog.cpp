#include "physdispconvert.h"
#include "scales.h"

#include "SDL.h"
#include "SDL_keyboard.h"

#include "leapfrog.h"
#include "shield.h"

using namespace oxygine;

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
   Resources& gameResources,
   SceneActor* sceneActor,
   b2World* world,
   Actor* actor,
   const Vector2& pos /*,
   float assemblyAngle*/) :
   m_world(world),
   m_physDispScale(10.0f),
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
   m_sceneActor(sceneActor),
   m_state(LFS_NORMAL),
   m_wantedAngle(0.0f),
   m_initiating(false)
{
	// The leapfrog main body is the main sprite of the leapfrog

	setResAnim(gameResources.getResAnim("lf_body"));

	// Size and position is in stage coordinates
	setSize(9.0, 9.6);
	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

	m_mainBody = world->CreateBody(&bodyDef);

	setUserData(m_mainBody);

	b2Vec2 vertices[7];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[6].Set(-4.5f, 4.8f);
	vertices[5].Set(-2.5f, -3.7f);
	vertices[4].Set(-1.5f, -4.8f);
	vertices[3].Set(1.5f, -4.8f);
	vertices[2].Set(2.5f, -3.7f);
	vertices[1].Set(4.5f, 4.8f);
	vertices[0].Set(-4.5f, 4.8f);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 7);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 3.0f;
	fixtureDef.friction = 1.3f;
	fixtureDef.filter.groupIndex = -1;

	m_mainBody->CreateFixture(&fixtureDef);
	m_mainBody->SetUserData(this);

   m_mainBody->GetFixtureList()->SetUserData((CollisionEntity*)this);

	m_mainBody->ResetMassData();
	
	//spLfBody lfBody = new LfBody(gameResources, world, pos, 1);
	//actor->addChild(lfBody);
	//m_leapFrogBody = lfBody;

	m_lfBoost = new LfBooster(gameResources, world, pos + Vector2(0.0f, 4.8f), 1, -1);
	actor->addChild(m_lfBoost);

	m_lfRightBigLeg = new LfBigLeg(gameResources, world, pos + Vector2(3.0f, 4.9f), 1, -1);
	actor->addChild(m_lfRightBigLeg);

	m_lfLeftBigLeg = new LfBigLeg(gameResources, world, pos + Vector2(-3.2f, 4.9f), 1, -1);
	actor->addChild(m_lfLeftBigLeg);

	m_lfRightSmallLeg = new LfSmallLeg(gameResources, world, pos + Vector2(3.2f, 12.2f), 1, -1);
	actor->addChild(m_lfRightSmallLeg);

	m_lfLeftSmallLeg = new LfSmallLeg(gameResources, world, pos + Vector2(-3.2f, 12.2f), 1, -1);
	actor->addChild(m_lfLeftSmallLeg);

	m_lfRightFoot = new LfFoot(gameResources, world, pos + Vector2(3.2f, 17.5f), 1, -1);
	actor->addChild(m_lfRightFoot);

	m_lfLeftFoot = new LfFoot(gameResources, world, pos + Vector2(-3.2f, 17.5f), 1, -1);
	actor->addChild(m_lfLeftFoot);

	m_lfRightSteer = new LfRightSteer(gameResources, world, pos + Vector2(3.9f, 17.5f), 1, -1);
	actor->addChild(m_lfRightSteer);

	m_lfLeftSteer = new LfLeftSteer(gameResources, world, pos + Vector2(-3.9f, 17.5f), 1, -1);
	actor->addChild(m_lfLeftSteer);

   m_shield = new Shield(gameResources, world, pos);
   // Shield will be attached as a post-production part (createLeapFrog() in sceneactor.cpp)

	// Main Body and booster joint
	b2WeldJointDef	boostJointDef;
	boostJointDef.bodyA = m_mainBody;
	boostJointDef.bodyB = m_lfBoost->m_body;
	boostJointDef.localAnchorA.Set(0.0f, 4.8f);
	boostJointDef.localAnchorB.Set(0.0f, -0.8f);
	boostJointDef.collideConnected = false;
	m_boostJoint = (b2WeldJoint*)world->CreateJoint(&boostJointDef);


	// Main Body and right big leg joint
	b2RevoluteJointDef	rightLegJointDef;
	rightLegJointDef.bodyA = m_mainBody;
	rightLegJointDef.bodyB = m_lfRightBigLeg->m_body;
	rightLegJointDef.localAnchorA.Set(3.3f, 5.8f);
	rightLegJointDef.localAnchorB.Set(0.0f, -2.75f);
	rightLegJointDef.collideConnected = false;
	rightLegJointDef.enableMotor = true;
	rightLegJointDef.maxMotorTorque = c_normalJointMotorTorque;
	rightLegJointDef.motorSpeed = 0;
	m_rightBigLegJoint = (b2RevoluteJoint*)world->CreateJoint(&rightLegJointDef);
   JointUserData* jd1 = new JointUserData(false);
   m_rightBigLegJoint->SetUserData(jd1);


	// Main Body and left big leg joint
	b2RevoluteJointDef	leftLegJointDef;
	leftLegJointDef.bodyA = m_mainBody;
	leftLegJointDef.bodyB = m_lfLeftBigLeg->m_body;
	leftLegJointDef.localAnchorA.Set(-3.3f, 5.8f);
	leftLegJointDef.localAnchorB.Set(0.0f, -2.75f);
	leftLegJointDef.collideConnected = false;
	leftLegJointDef.enableMotor = true;
	leftLegJointDef.maxMotorTorque = c_normalJointMotorTorque;
	leftLegJointDef.motorSpeed = 0;
	m_leftBigLegJoint = (b2RevoluteJoint*)world->CreateJoint(&leftLegJointDef);
   JointUserData* jd2 = new JointUserData(false);
   m_leftBigLegJoint->SetUserData(jd2);


	// Right Big leg and right small leg joint
	b2RevoluteJointDef	rightSmallLegJointDef;
	rightSmallLegJointDef.bodyA = m_lfRightBigLeg->m_body;
	rightSmallLegJointDef.bodyB = m_lfRightSmallLeg->m_body;
	rightSmallLegJointDef.localAnchorA.Set(0.0f, 2.95f);
	rightSmallLegJointDef.localAnchorB.Set(0.0f, -1.95f);
	rightSmallLegJointDef.collideConnected = false;
	rightSmallLegJointDef.enableMotor = true;
	rightSmallLegJointDef.maxMotorTorque = c_normalJointMotorTorque;
	rightSmallLegJointDef.motorSpeed = 0;
	m_rightSmallLegJoint = (b2RevoluteJoint*)world->CreateJoint(&rightSmallLegJointDef);
   JointUserData* jd3 = new JointUserData(false);
   m_rightSmallLegJoint->SetUserData(jd3);


	// Left Big leg and left small leg joint
	b2RevoluteJointDef	leftSmallLegJointDef;
	leftSmallLegJointDef.bodyA = m_lfLeftBigLeg->m_body;
	leftSmallLegJointDef.bodyB = m_lfLeftSmallLeg->m_body;
	leftSmallLegJointDef.localAnchorA.Set(0.0f, 2.95f);
	leftSmallLegJointDef.localAnchorB.Set(0.0f, -1.95f);
	leftSmallLegJointDef.collideConnected = false;
	leftSmallLegJointDef.enableMotor = true;
	leftSmallLegJointDef.maxMotorTorque = c_normalJointMotorTorque;
	leftSmallLegJointDef.motorSpeed = 0;
	m_leftSmallLegJoint = (b2RevoluteJoint*)world->CreateJoint(&leftSmallLegJointDef);
   JointUserData* jd4 = new JointUserData(false);
   m_leftSmallLegJoint->SetUserData(jd4);


	// Right Small leg and right foot joint
	b2RevoluteJointDef	rightFootLegJointDef;
	rightFootLegJointDef.bodyA = m_lfRightSmallLeg->m_body;
	rightFootLegJointDef.bodyB = m_lfRightFoot->m_body;
	rightFootLegJointDef.localAnchorA.Set(0.0f, 2.05f);
	rightFootLegJointDef.localAnchorB.Set(0.0f, -0.9f);
	rightFootLegJointDef.collideConnected = false;
	rightFootLegJointDef.enableMotor = true;
	rightFootLegJointDef.maxMotorTorque = c_normalJointMotorTorque;
	rightFootLegJointDef.motorSpeed = 0;
	m_rightFootLegJoint = (b2RevoluteJoint*)world->CreateJoint(&rightFootLegJointDef);
   JointUserData* jd5 = new JointUserData(false);
   m_rightFootLegJoint->SetUserData(jd5);


	// Left Small leg and left foot joint
	b2RevoluteJointDef	leftFootLegJointDef;
	leftFootLegJointDef.bodyA = m_lfLeftSmallLeg->m_body;
	leftFootLegJointDef.bodyB = m_lfLeftFoot->m_body;
	leftFootLegJointDef.localAnchorA.Set(0.0f, 2.05f);
	leftFootLegJointDef.localAnchorB.Set(0.0f, -0.9f);
	leftFootLegJointDef.collideConnected = false;
	leftFootLegJointDef.enableMotor = true;
	leftFootLegJointDef.maxMotorTorque = c_normalJointMotorTorque;
	leftFootLegJointDef.motorSpeed = 0;
	m_leftFootLegJoint = (b2RevoluteJoint*)world->CreateJoint(&leftFootLegJointDef);
   JointUserData* jd6 = new JointUserData(false);
   m_leftFootLegJoint->SetUserData(jd6);


	// Right small leg and steer booster joint
	b2WeldJointDef	rightSteerJointDef;
	rightSteerJointDef.bodyA = m_lfRightSmallLeg->m_body;
	rightSteerJointDef.bodyB = m_lfRightSteer->m_body;
	rightSteerJointDef.localAnchorA.Set(-0.7f, 1.4f);
	rightSteerJointDef.localAnchorB.Set(0.6f, 0.0f);
	rightSteerJointDef.collideConnected = false;
	m_rightSteerJoint = (b2WeldJoint*)world->CreateJoint(&rightSteerJointDef);

	// Left small leg and steer booster joint
	b2WeldJointDef	leftSteerJointDef;
	leftSteerJointDef.bodyA = m_lfLeftSmallLeg->m_body;
	leftSteerJointDef.bodyB = m_lfLeftSteer->m_body;
	leftSteerJointDef.localAnchorA.Set(0.7f, 1.4f);
	leftSteerJointDef.localAnchorB.Set(-0.6f, 0.0f);
	leftSteerJointDef.collideConnected = false;
	m_leftSteerJoint = (b2WeldJoint*)world->CreateJoint(&leftSteerJointDef);

   // Shield only exists in deep space but for simplicity it is always
   // there. In deep space it gets its normal size but is other 
   // environments it is turned very small
   b2RevoluteJointDef shieldJointDef;
   shieldJointDef.bodyA = m_mainBody;
   shieldJointDef.bodyB = m_shield->m_body;
   shieldJointDef.localAnchorA.Set(0.0f, 1.0f);
   shieldJointDef.localAnchorB.Set(0.0f, 0.0f);
   shieldJointDef.collideConnected = false;
   shieldJointDef.enableMotor = false;
   m_shieldJoint = (b2RevoluteJoint*)m_world->CreateJoint(&shieldJointDef);

   // Add main engine particle system
   m_boosterFlame = new FlameEmitter(
      gameResources, 
      m_lfBoost->m_body,
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
      m_lfRightSteer->m_body,
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
      m_lfLeftSteer->m_body,
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
      4,                               // Intensity [bullets per second]
      250.0f,                          // Lifetime [ms]
      1000.0f,                         // Bullet speed
      false);                          // Bouncy

   m_gun->attachTo(this);

//   setAlpha(0);
}

LeapFrog::~LeapFrog()
{
   //delete ((JointUserData*)m_rightBigLegJoint->GetUserData());

   //delete ((JointUserData*)m_leftBigLegJoint->GetUserData());

   //delete ((JointUserData*)m_rightSmallLegJoint->GetUserData());

   //delete ((JointUserData*)m_leftSmallLegJoint->GetUserData());

   //delete ((JointUserData*)m_rightFootLegJoint->GetUserData());

   //delete ((JointUserData*)m_leftFootLegJoint->GetUserData());

}

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

      // check if we have reached the gaol
      if (magAngVel < 1.0f * MATH_PI / 180.0f)
      {
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
   m_rightBigLegJoint->EnableMotor(false);
   m_rightBigLegJoint->SetLimits(m_modeAngleGoals.m_rightBigJointAngle, m_modeAngleGoals.m_rightBigJointAngle);
   m_rightBigLegJoint->EnableLimit(true);

   m_leftBigLegJoint->EnableMotor(false);
   m_leftBigLegJoint->SetLimits(m_modeAngleGoals.m_leftBigJointAngle, m_modeAngleGoals.m_leftBigJointAngle);
   m_leftBigLegJoint->EnableLimit(true);

   m_rightSmallLegJoint->EnableMotor(false);
   m_rightSmallLegJoint->SetLimits(m_modeAngleGoals.m_rightSmallJointAngle, m_modeAngleGoals.m_rightSmallJointAngle);
   m_rightSmallLegJoint->EnableLimit(true);

   m_leftSmallLegJoint->EnableMotor(false);
   m_leftSmallLegJoint->SetLimits(m_modeAngleGoals.m_leftSmallJointAngle, m_modeAngleGoals.m_leftSmallJointAngle);
   m_leftSmallLegJoint->EnableLimit(true);

   m_rightFootLegJoint->EnableMotor(false);
   m_rightFootLegJoint->SetLimits(m_modeAngleGoals.m_rightFootJointAngle, m_modeAngleGoals.m_rightFootJointAngle);
   m_rightFootLegJoint->EnableLimit(true);

   m_leftFootLegJoint->EnableMotor(false);
   m_leftFootLegJoint->SetLimits(m_modeAngleGoals.m_leftFootJointAngle, m_modeAngleGoals.m_leftFootJointAngle);
   m_leftFootLegJoint->EnableLimit(true);
}

void LeapFrog::unlockJoints(void)
{
   // Unlock all legs and enable motors
   m_rightBigLegJoint->EnableMotor(true);
   m_rightBigLegJoint->EnableLimit(false);

   m_leftBigLegJoint->EnableMotor(true);
   m_leftBigLegJoint->EnableLimit(false);

   m_rightSmallLegJoint->EnableMotor(true);
   m_rightSmallLegJoint->EnableLimit(false);

   m_leftSmallLegJoint->EnableMotor(true);
   m_leftSmallLegJoint->EnableLimit(false);

   m_rightFootLegJoint->EnableMotor(true);
   m_rightFootLegJoint->EnableLimit(false);

   m_leftFootLegJoint->EnableMotor(true);
   m_leftFootLegJoint->EnableLimit(false);

}

void LeapFrog::weldJoints(void)
{
   // Main Body and right big leg joint
   b2WeldJointDef	rightLegJointDef;
   rightLegJointDef.bodyA = m_mainBody;
   rightLegJointDef.bodyB = m_lfRightBigLeg->m_body;
   rightLegJointDef.localAnchorA.Set(3.3, 5.8);
   rightLegJointDef.localAnchorB.Set(0.0, -2.75);
   rightLegJointDef.collideConnected = false;
   m_rightBigLegWeldJoint = (b2WeldJoint*)m_world->CreateJoint(&rightLegJointDef);

   // Main Body and left big leg joint
   b2WeldJointDef	leftLegJointDef;
   leftLegJointDef.bodyA = m_mainBody;
   leftLegJointDef.bodyB = m_lfLeftBigLeg->m_body;
   leftLegJointDef.localAnchorA.Set(-3.3, 5.8);
   leftLegJointDef.localAnchorB.Set(0.0, -2.75);
   leftLegJointDef.collideConnected = false;
   m_leftBigLegWeldJoint = (b2WeldJoint*)m_world->CreateJoint(&leftLegJointDef);

   // Right Big leg and right small leg joint
   b2WeldJointDef	rightSmallLegJointDef;
   rightSmallLegJointDef.bodyA = m_lfRightBigLeg->m_body;
   rightSmallLegJointDef.bodyB = m_lfRightSmallLeg->m_body;
   rightSmallLegJointDef.localAnchorA.Set(0.0, 2.95);
   rightSmallLegJointDef.localAnchorB.Set(0.0, -1.95);
   rightSmallLegJointDef.collideConnected = false;
   m_rightSmallLegWeldJoint = (b2WeldJoint*)m_world->CreateJoint(&rightSmallLegJointDef);

   // Left Big leg and left small leg joint
   b2WeldJointDef	leftSmallLegJointDef;
   leftSmallLegJointDef.bodyA = m_lfLeftBigLeg->m_body;
   leftSmallLegJointDef.bodyB = m_lfLeftSmallLeg->m_body;
   leftSmallLegJointDef.localAnchorA.Set(0.0, 2.95);
   leftSmallLegJointDef.localAnchorB.Set(0.0, -1.95);
   leftSmallLegJointDef.collideConnected = false;
   m_leftSmallLegWeldJoint = (b2WeldJoint*)m_world->CreateJoint(&leftSmallLegJointDef);

   // Right Small leg and right foot joint
   b2WeldJointDef	rightFootLegJointDef;
   rightFootLegJointDef.bodyA = m_lfRightSmallLeg->m_body;
   rightFootLegJointDef.bodyB = m_lfRightFoot->m_body;
   rightFootLegJointDef.localAnchorA.Set(0.0, 2.05);
   rightFootLegJointDef.localAnchorB.Set(0.0, -0.9);
   rightFootLegJointDef.collideConnected = false;
   m_rightFootLegWeldJoint = (b2WeldJoint*)m_world->CreateJoint(&rightFootLegJointDef);

   // Left Small leg and left foot joint
   b2WeldJointDef	leftFootLegJointDef;
   leftFootLegJointDef.bodyA = m_lfLeftSmallLeg->m_body;
   leftFootLegJointDef.bodyB = m_lfLeftFoot->m_body;
   leftFootLegJointDef.localAnchorA.Set(0.0, 2.05);
   leftFootLegJointDef.localAnchorB.Set(0.0, -0.9);
   leftFootLegJointDef.collideConnected = false;
   m_leftFootLegWeldJoint = (b2WeldJoint*)m_world->CreateJoint(&leftFootLegJointDef);

}


void LeapFrog::unweldJoints(void)
{
   if (m_rightBigLegWeldJoint)
   {
      m_world->DestroyJoint(m_rightBigLegWeldJoint);
   }
   m_world->DestroyJoint(m_leftBigLegWeldJoint);
   m_world->DestroyJoint(m_rightSmallLegWeldJoint);
   m_world->DestroyJoint(m_leftSmallLegWeldJoint);
   m_world->DestroyJoint(m_rightFootLegWeldJoint);
   m_world->DestroyJoint(m_leftFootLegWeldJoint);
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
   m_rightBigLegJoint->SetMotorSpeed(0.0f);
   m_leftBigLegJoint->SetMotorSpeed(0.0f);
   m_rightSmallLegJoint->SetMotorSpeed(0.0f);
   m_leftSmallLegJoint->SetMotorSpeed(0.0f);
   m_rightFootLegJoint->SetMotorSpeed(0.0f);
   m_leftFootLegJoint->SetMotorSpeed(0.0f);
}

void LeapFrog::setInstantAngle(float angle)
{
   m_wantedAngle = angle;

   stopAllJointMotors();
   lockJoints();
   m_state = LFS_INSTANTLY_ROTATING;
}

void LeapFrog::setHoldAngle(float angle)
{
   m_wantedAngle = angle;
   m_state = LFS_HOLD_ANGLE;
}

void LeapFrog::releaseHoldAngle(void)
{
   m_state = LFS_NORMAL;
}


void LeapFrog::initLeapfrog(LeapFrogModeEnum mode, float angle)
{
   m_initiating = true;
   m_mode = mode;

   setInstantAngle(angle);

   m_initiating = true;
}

void LeapFrog::initMode(LeapFrogModeEnum mode)
{
   m_mode = mode;

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

   //// Now set all joints angles directly
   //m_rightBigLegJoint->SetLimits(m_modeAngleGoals.m_rightBigJointAngle, m_modeAngleGoals.m_rightBigJointAngle);
   //m_rightSmallLegJoint->SetLimits(m_modeAngleGoals.m_rightSmallJointAngle, m_modeAngleGoals.m_rightSmallJointAngle);
   //m_rightFootLegJoint->SetLimits(m_modeAngleGoals.m_rightFootJointAngle, m_modeAngleGoals.m_rightFootJointAngle);
   //m_leftBigLegJoint->SetLimits(m_modeAngleGoals.m_leftBigJointAngle, m_modeAngleGoals.m_leftBigJointAngle);
   //m_leftSmallLegJoint->SetLimits(m_modeAngleGoals.m_leftSmallJointAngle, m_modeAngleGoals.m_leftSmallJointAngle);
   //m_leftFootLegJoint->SetLimits(m_modeAngleGoals.m_leftFootJointAngle, m_modeAngleGoals.m_leftFootJointAngle);
   //m_rightBigLegJoint->EnableLimit(true);
   //m_leftBigLegJoint->EnableLimit(true);
   //m_rightSmallLegJoint->EnableLimit(true);
   //m_leftSmallLegJoint->EnableLimit(true);
   //m_rightFootLegJoint->EnableLimit(true);
   //m_leftFootLegJoint->EnableLimit(true);

}

void LeapFrog::goToMode(LeapFrogModeEnum mode)
{
   m_state = LFS_MODE_IN_TRANSIT;
   m_mode = mode;

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
}

void LeapFrog::instantAngleReached(void)
{
   m_mainBody->SetAngularVelocity(0.0f);
   m_steerMagnitude = 0.0f;
 
   m_state = LFS_GET_TO_EQUILIBRIUM;
 
   //if (m_initiating)
   //{
   //   initMode(m_mode);
   //}
   //else
   //{ 
   //   m_state = LFS_GET_TO_EQUILIBRIUM;
   //}
}

void LeapFrog::equilibriumReached(void)
{
   m_state = LFS_NORMAL;

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

   //if (dir == -1)
   //{
   //   m_leftSteerFlame->stopEmitter();

   //   if (m_rightSteerFireLastUpdate == false)
   //   {
   //      m_rightSteerFlame->startEmitter();
   //   }

   //   m_steerMagnitude = -m_steerImpulse;

   //   m_rightSteerFireLastUpdate = true;
   //}
   //else if (dir == 1)
   //{
   //   m_rightSteerFlame->stopEmitter();

   //   if (m_leftSteerFireLastUpdate == false)
   //   {
   //      m_leftSteerFlame->startEmitter();
   //   }

   //   m_steerMagnitude = m_steerImpulse;

   //   m_leftSteerFireLastUpdate = true;
   //}
   //else if (dir == 0)
   //{
   //   float angleVel = m_mainBody->GetAngularVelocity();

   //   m_rightSteerFlame->stopEmitter();
   //   m_leftSteerFlame->stopEmitter();

   //   if (angleVel > 0)
   //   {
   //      m_steerMagnitude = -m_eveningMagnitude;
   //   }
   //   else if (angleVel < 0)
   //   {
   //      m_steerMagnitude = m_eveningMagnitude;
   //   }

   //   m_rightSteerFireLastUpdate = false;
   //   m_leftSteerFireLastUpdate = false;
   //}

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

void LeapFrog::setBoundedWallsActor(FreeSpaceActor* actor)
{
   m_gun->setBoundedWallsActor(actor);
}


const ModeAngles LeapFrog::c_resetModeAngles =
   ModeAngles(9.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);

const ModeAngles LeapFrog::c_landingModeAngles =
   ModeAngles(
      -65 * MATH_PI / 180, 65 * MATH_PI / 180,
      0 * MATH_PI / 180, 65 * MATH_PI / 180,
      -65 * MATH_PI / 180, 0 * MATH_PI / 180);

const ModeAngles LeapFrog::c_deepSpaceModeAngles =
   ModeAngles(
      -187.5 * MATH_PI / 180, -3.5 * MATH_PI / 180,
      -117.5 * MATH_PI / 180, 187 * MATH_PI / 180,
      3.5 * MATH_PI / 180, 117.5 * MATH_PI / 180);

const ModeAngles LeapFrog::c_orbitModeAngles =
   ModeAngles(
      -65 * MATH_PI / 180, 65 * MATH_PI / 180,
      0 * MATH_PI / 180, 65 * MATH_PI / 180,
      -65 * MATH_PI / 180, 0 * MATH_PI / 180);

const ModeAngles LeapFrog::c_reentryModeAngles =
   ModeAngles(
      -92 * MATH_PI / 180, -156.5 * MATH_PI / 180,
      -35 * MATH_PI / 180, 92 * MATH_PI / 180,
      156.5 * MATH_PI / 180, 35 * MATH_PI / 180);


const float LeapFrog::c_normalJointMotorTorque = 20000.0f;
const float LeapFrog::c_normalJointMotorSpeed = 30 * MATH_PI / 180;
const float LeapFrog::c_instantJointMotorTorque = 2000000.0f;
const float LeapFrog::c_instnatJointMotorSpeed = 150 * MATH_PI / 180;;





