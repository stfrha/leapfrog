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
   // Leg joint health (landing configuration only takes damage). 1 = sound,
   // 0 = limp: the joint loses its motor and swings freely within wide
   // limits, with a little friction, like a dead limb.
   int   m_legIndex;          // index in GameStatus leg health (-1 = not a leg joint)
   const char* m_legName;     // for log messages
   float m_health;
   bool  m_limp;
   float m_strain;            // recent overload impulse (decays)
   float m_strainCounted;     // part of m_strain already charged as damage
   float m_strainPeak;        // for the tuning log
   float m_bendCounted;       // over-bend [rad] already charged as damage
   float m_bendPeak;          // for the tuning log
   float m_limpLower;         // limits while limp [rad]
   float m_limpUpper;

   JointUserData()
   {
      init(false);
   }

   JointUserData(bool modeReached)
   {
      init(modeReached);
   }

   void init(bool modeReached)
   {
      m_modeReached = modeReached;
      m_legIndex = -1;
      m_legName = "";
      m_health = 1.0f;
      m_limp = false;
      m_strain = 0.0f;
      m_strainCounted = 0.0f;
      m_strainPeak = 0.0f;
      m_bendCounted = 0.0f;
      m_bendPeak = 0.0f;
      m_limpLower = 0.0f;
      m_limpUpper = 0.0f;
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
   5     =  Ammo
   6     =  Shield
   7     =  Fuel
   8     =  Credits
   9     =  Damage
   10    =  Leg health: lowest health of the six leg joints, 0..1 (0 =
            a joint is limp). Writing raises every joint to at least the
            value (repair: 1; a field fix: e.g. 0.3).
   */

   enum LeapfroPropertiesEnum
   {
      propMode    = 0,
      propEnv     = 1,
      propState   = 2,
      propXPos    = 3,
      propYPos    = 4,
      ammo        = 5,
      shield      = 6,
      fuel        = 7,
      credits     = 8,
      damage      = 9,
      legHealth   = 10
   };

   static const int c_numLegJoints = 6;

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

   // Leg joint damage (see updateLegStrain)
   const static float c_legSustainTorque;
   const static float c_legStrainMemory;
   const static float c_legYieldImpulse;
   const static float c_legToughnessImpulse;
   const static float c_legBendYield;
   const static float c_legBendToughness;
   const static float c_legLockHealth;
   const static float c_legWeakHealth;
   const static float c_legMinTorqueScale;
   const static float c_legLimpFriction;
   const static float c_legLimpMargin;

   // Service lift (see updateServiceLift)
   const static float c_liftHeight;
   const static float c_liftRiseSpeed;
   const static float c_liftLowerSpeed;
   const static float c_liftOmega;
   const static float c_liftLevelStiffness;
   const static float c_liftLevelDamping;
   const static float c_liftTimeout;

   LeapFrogStateEnum m_state;
   LeapFrogModeEnum  m_mode;
   EnvironmentEnum m_environment;
   bool m_armInitialRegistration;
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

	b2World* m_world;
	b2Body* m_mainBody;
	b2Body* m_rightBigLegBody;
	b2Body* m_leftBigLegBody;
	b2Body* m_rightSteerBody;
	b2Body* m_leftSteerBody;

   oxygine::spActor m_leapfrogActor;
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
   float m_reportedLegHealth;

   // Service lift: after a repair on a pad, the craft is lifted a little
   // (no booster) so the repaired legs can straighten, then set down again
   int m_feetOnPad;              // foot contacts with landing pads
   int m_serviceLiftPhase;       // -1 none, 0 rising/waiting, 1 lowering
   float m_serviceLiftTime;      // [s]
   float m_serviceLiftStartY;
   float m_serviceLiftTarget;    // wanted main body y, moves smoothly
   float m_serviceLiftMass;

   spGun m_gun;

   spBox2DDraw m_debugDraw;
   
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
		SceneActor* sceneParent,
      CompoundObject* parentObject,
		b2World* world,
		const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);


   void hitImpulse(b2Contact* contact, const b2ContactImpulse* impulse);
   void hitByBullet(b2Contact* contact, float bulletEqvDamage);
   void footOnLandingPad(bool resting);

   // Leg joint health, index as in GameStatus: 0 left hip, 1 left knee,
   // 2 left foot, 3 right hip, 4 right knee, 5 right foot
   float getLegHealth(int index);
   void setLegHealth(int index, float health);
   void setAllLegHealth(float health);
   float getLowestLegHealth(void);

   // True while the craft is being lifted for a leg repair. Landing pads
   // freeze meanwhile: no hold, no events, no fuel.
   bool isInServiceLift(void) const { return m_serviceLiftPhase >= 0; }


   oxygine::Actor* getMainActor(void);
   b2Vec2 getCraftVelocity(void);

   /*
   To be controlled by Game Manager:
   goToMode
   goToEnvironment
   */

   void goToMode(LeapFrogModeEnum mode);
   void goToEnvironment(EnvironmentEnum env);
   // freeFuel: burn without using fuel (and also with an empty tank), as
   // when the launch site's sequence fires the booster
   void fireMainBooster(bool fire, bool flamesOnly = false, bool freeFuel = false);
   bool isMainBoosterFiring(void) const { return m_boostFireLastUpdate; }   // the player is pressing boost
	void fireSteeringBooster(int dir); // -1 is counter clockwise, 1 is clockwise
   void fireGun(bool fire);
   void fireReentryFlames(bool fire);
   void reentrySetHeat(int heatAmount);   // 0 = off, 1 = coldest, 255 = hottest, most intense
   void setHoldAngle(float angle);
   void releaseHoldAngle(void);
   void breakJoints(void);
   
   void initObjectResources(oxygine::Actor* statusEventOriginator, ObjectResources* resources = NULL) override;

   void dumpPart(std::string name, b2Body* body, b2Body* mainBody);
   void dumpParts(void);
   void dumpReentryFlamees(void);


private:
	void setJointMotor(b2RevoluteJoint* joint, float goal, float speedMagnitude);
   int getLegJoints(b2RevoluteJoint* joints[c_numLegJoints], float goals[c_numLegJoints]);
   b2RevoluteJoint* getLegJoint(int index);
   static JointUserData* legData(b2RevoluteJoint* joint);
   void initLegJoints(void);
   void applyLegStrength(b2RevoluteJoint* joint);
   void makeLegLimp(b2RevoluteJoint* joint, bool announce);
   void repairLeg(b2RevoluteJoint* joint);
   void damageLeg(b2RevoluteJoint* joint, float damage);
   void updateLegStrain(void);
   void reportLegHealth(void);
   void setLegHealthPropHandler(oxygine::Event *ev);
   bool legsAtPose(void);
   void requestServiceLift(void);
   void updateServiceLift(void);
   void endServiceLift(const char* why);
   void setStrongJoints(void);
   void setWeakJoints(void);
   void lockJoints(bool onPad = false);
   void unlockJoints(void);
   bool modeIsReached(void);
   void resetModeReached(void);
   void modeReached(void);
   void holdAngleReached(void);
   void stopAllJointMotors(void);

   void setModePropHandler(oxygine::Event *ev);
   void setEnvPropHandler(oxygine::Event *ev);

protected:
   void doUpdate(const UpdateState &us) override;
};

