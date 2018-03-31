#pragma once

#include "oxygine-framework.h"
#include "Box2DDebugDraw.h"
#include "Box2D/Box2D.h"
#include "flameemitter.h"
#include "gun.h"
#include "collisionentity.h"
#include "shield.h"
#include "leapfrogparts.h"

class SceneActor;

class ModeAngles
{
public:
	ModeAngles();
	ModeAngles(float rgtBigLeg, float rgtSmallLeg, float rgtFoot,
		float lftBigLeg, float lftSmallLeg, float lftFoot);
	float   m_rightBigJointAngle;
	float   m_rightSmallJointAngle;
	float   m_rightFootJointAngle;
	float   m_leftBigJointAngle;
	float   m_leftSmallJointAngle;
	float   m_leftFootJointAngle;
};

enum LeapFrogStateEnum
{
   LFS_NORMAL,
   LFS_MODE_IN_TRANSIT,
   LFS_INITIATING_MODE,
   LFS_INSTANTLY_ROTATING,
   LFS_GET_TO_EQUILIBRIUM,
   LFS_HOLD_ANGLE
};

enum LeapFrogModeEnum
{
   LFM_RESET,
   LFM_DEEP_SPACE,
   LFM_LANDING,
   LFM_ORBIT,
   LFM_REENTRY
};

enum EnvironmentEnum
{
   ENV_DEEP_SPACE,
   ENV_GROUND,
   ENV_ORBIT,
   ENV_REENTRY,
   ENV_LAUNCH
};

DECLARE_SMART(JointUserData, spJointUserData);

class JointUserData
{
private:
   bool m_modeReached;
public:
   JointUserData()
   {
      m_modeReached = false;
   }

   JointUserData(bool modeReached)
   {
      m_modeReached = modeReached;
   }

   void setModeReached(bool modeReached)
   {
      m_modeReached = modeReached;
   }

   bool getModeReached(void)
   {
      return m_modeReached;
   }
};

DECLARE_SMART(LeapFrog, spLeapFrog);

class LeapFrog : public oxygine::Sprite, CollisionEntity
{
private:
   const static ModeAngles c_resetModeAngles;
   const static ModeAngles c_landingModeAngles;
   const static ModeAngles c_deepSpaceModeAngles;
   const static ModeAngles c_orbitModeAngles;
   const static ModeAngles c_reentryModeAngles;
   const static float c_normalJointMotorTorque;
   const static float c_normalJointMotorSpeed;
   const static float c_instantJointMotorTorque;
   const static float c_instnatJointMotorSpeed;

   LeapFrogStateEnum m_state;
   bool m_initiating;
   LeapFrogModeEnum  m_mode;
   EnvironmentEnum m_environment;
   bool	m_mainBoosterFire;
   int	m_steeringBoosterFire;
   bool  m_boostFireLastUpdate;
   bool  m_rightSteerFireLastUpdate;
   bool  m_leftSteerFireLastUpdate;

   float	m_boostMagnuitude;
   float	m_steerMagnitude;       // The magnitude of the angular force (without sign)

   // Parameters that can be changed
   // depending on the environment
   float m_maxVelocity;
   float m_boostInc;
   float m_boostMaxMagnitude;
   float m_maxAngularVelocity;
   float m_steerMaxMagnitude;
   float m_eveningMagnitude;

   // Parameters that can be changed 
   // depending on mode
   ModeAngles m_modeAngleGoals;
   float m_wantedAngle;

	b2World * m_world;
	b2Body* m_mainBody;

   spLfBooster m_lfBoost;
   spLfBigLeg m_lfRightBigLeg;
   spLfBigLeg m_lfLeftBigLeg;
   spLfSmallLeg m_lfRightSmallLeg;
   spLfSmallLeg m_lfLeftSmallLeg;
   spLfFoot m_lfRightFoot;
   spLfFoot m_lfLeftFoot;
   spLfRightSteer m_lfRightSteer;
   spLfLeftSteer m_lfLeftSteer;
   b2WeldJoint* m_boostJoint;
   b2WeldJoint* m_rightSteerJoint;
   b2WeldJoint* m_leftSteerJoint;
   b2RevoluteJoint* m_rightBigLegJoint;
   b2RevoluteJoint* m_leftBigLegJoint;
   b2RevoluteJoint* m_rightSmallLegJoint;
   b2RevoluteJoint* m_leftSmallLegJoint;
   b2RevoluteJoint* m_rightFootLegJoint;
   b2RevoluteJoint* m_leftFootLegJoint;
   b2RevoluteJoint* m_shieldJoint;

   //  Weld joints are for locking arms and legs during
   // instant rotation, they are supposed to be removed
   // after the instant rotation
   b2WeldJoint* m_rightBigLegWeldJoint;
   b2WeldJoint* m_leftBigLegWeldJoint;
   b2WeldJoint* m_rightSmallLegWeldJoint;
   b2WeldJoint* m_leftSmallLegWeldJoint;
   b2WeldJoint* m_rightFootLegWeldJoint;
   b2WeldJoint* m_leftFootLegWeldJoint;
   b2WeldJoint* m_shieldWeldJoint;

   spFlameEmitter m_boosterFlame;
   spFlameEmitter m_leftSteerFlame;
   spFlameEmitter m_rightSteerFlame;

   spGun m_gun;

   float	m_physDispScale;
	spBox2DDraw m_debugDraw;
   

   oxygine::Resources* m_gameResources;

   SceneActor* m_sceneActor;


public:

   spShield m_shield;


	LeapFrog(
      oxygine::Resources& gameResources, 
      SceneActor* sceneActor,
      b2World* world, 
      oxygine::Actor* actor, 
      const oxygine::Vector2& pos /*,
      float assemblyAngle*/);   // Indicates if the actor pointer implements the bounded bodies
	~LeapFrog();

   virtual CollisionEntityTypeEnum getEntityType(void);



   void hitByAsteroid(b2Contact* contact);

   void initLeapfrog(LeapFrogModeEnum mode, float angle);
	void initMode(LeapFrogModeEnum mode);
   void goToMode(LeapFrogModeEnum mode);
   void goToEnvironment(EnvironmentEnum env);
   void fireMainBooster(bool fire);
	void fireSteeringBooster(int dir); // -1 is counter clockwise, 1 is clockwise
   void fireGun(bool fire);
   void setBoundedWallsActor(FreeSpaceActor* actor);
   void setInstantAngle(float angle);
   void setHoldAngle(float angle);
   void releaseHoldAngle(void);


private:
	void setJointMotor(b2RevoluteJoint* joint, float goal, float speedMagnitude);
   void setStrongJoints(void);
   void setWeakJoints(void);
   void lockJoints(void);
   void unlockJoints(void);
   void weldJoints(void);
   void unweldJoints(void);
   bool modeIsReached(void);
   void resetModeReached(void);
   void equilibriumReached(void);
   void modeReached(void);
   void instantAngleReached(void);
   void holdAngleReached(void);
   void stopAllJointMotors(void);

protected:
   void doUpdate(const UpdateState &us);
};

