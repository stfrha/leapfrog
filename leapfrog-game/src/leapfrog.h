#pragma once

#include "oxygine-framework.h"
#include "Box2DDebugDraw.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "flameemitter.h"
#include "reentryflameemitter.h"
#include "gun.h"
#include "collisionentity.h"
#include "shield.h"

class SceneActor;

//void setModePropCallback(void);


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

class LeapFrog : public CompoundObject
{
public:
   /*
   Lets define properties:
   Prop     Description
   0     =  Mode
   1     =  Environment
   2     =  State (read only)
   3     =  X-pos (read only)
   4     =  Y-pos (read only)
   */

   enum properties
   {
      propMode    = 0,
      propEnv     = 1,
      propState   = 2,
      propXPos    = 3,
      propYPos    = 4
   };

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
   bool m_modeInTransit;

	b2World * m_world;
	b2Body* m_mainBody;
   b2Body* m_boostBody;
	b2Body* m_rightBigLegBody;
	b2Body* m_leftBigLegBody;
	b2Body* m_rightSteerBody;
	b2Body* m_leftSteerBody;

   oxygine::spActor m_mainActor;
   oxygine::Sprite* m_lfRightBigLeg;
   oxygine::Sprite* m_lfLeftBigLeg;

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

   spGun m_gun;

   spBox2DDraw m_debugDraw;
   

   oxygine::Resources* m_gameResources;

   void collisionBlast(b2Contact* contact, bool small = true);
   void evaluateLepfrogDamage(void);

public:

   spShield m_shield;
   spFlameEmitter m_boosterFlame;
   spFlameEmitter m_leftSteerFlame;
   spFlameEmitter m_rightSteerFlame;
   spReentryFlameEmitter m_reentryFlameEmitterBooster;
   spReentryFlameEmitter m_reentryFlameEmitterRightLeg;
   spReentryFlameEmitter m_reentryFlameEmitterLeftLeg;

	LeapFrog(
		oxygine::Resources& gameResources,
		SceneActor* sceneParent,
      CompoundObject* parentObject,
		b2World* world,
		const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);


   void hitImpulse(b2Contact* contact, const b2ContactImpulse* impulse);
   void hitByBullet(b2Contact* contact, float bulletEqvDamage);
   void footOnLandingPad(bool resting);


   oxygine::Actor* getMainActor(void);

   /*
   To be controlled by Game Manager:
   goToMode
   goToEnvironment
   */

   void goToMode(LeapFrogModeEnum mode);
   void goToEnvironment(EnvironmentEnum env);
   void fireMainBooster(bool fire, bool flamesOnly = false);
	void fireSteeringBooster(int dir); // -1 is counter clockwise, 1 is clockwise
   void fireGun(bool fire);
   void fireReentryFlames(bool fire);
   void reentrySetHeat(int heatAmount);   // 0 = off, 1 = coldest, 255 = hottest, most intense
   void setHoldAngle(float angle);
   void releaseHoldAngle(void);
   void breakJoints(void);
   
   // Leapfrog specialisation of map registration
   void registerToMap(void) override;
   void initGameStatus(spGameStatus status) override;

   void dumpPart(std::string name, b2Body* body, b2Body* mainBody);
   void dumpParts(void);
   void dumpReentryFlamees(void);


private:
	void setJointMotor(b2RevoluteJoint* joint, float goal, float speedMagnitude);
   void setStrongJoints(void);
   void setWeakJoints(void);
   void lockJoints(void);
   void unlockJoints(void);
   bool modeIsReached(void);
   void resetModeReached(void);
   void modeReached(void);
   void holdAngleReached(void);
   void stopAllJointMotors(void);

   void setModePropHandler(oxygine::Event *ev);
   void setEnvPropHandler(oxygine::Event *ev);

protected:
   void doUpdate(const UpdateState &us);
};

