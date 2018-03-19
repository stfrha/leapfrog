
#include "oxygine-framework.h"
#include "Box2DDebugDraw.h"
#include "Box2D/Box2D.h"
#include "flameemitter.h"


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

enum LeapFrogModeEnum
{
   LFM_RESET,
   LFM_DEEP_SPACE,
   LFM_LANDING,
   LFM_ORBIT,
   LFM_REENTRY
};

DECLARE_SMART(LeapFrog, spLeapFrog);

class LeapFrog : public oxygine::Sprite
{
private:
	b2World * m_world;
	b2Body* m_mainBody;

   LeapFrogModeEnum  m_mode;

	float	m_boostMagnuitude;
	float	m_steerMagnitude;
	float	m_physDispScale;
	spBox2DDraw m_debugDraw;

	b2WeldJoint* m_boostJoint;
	b2WeldJoint* m_rightSteerJoint;
	b2WeldJoint* m_leftSteerJoint;
	b2RevoluteJoint* m_rightBigLegJoint;
	b2RevoluteJoint* m_leftBigLegJoint;
	b2RevoluteJoint* m_rightSmallLegJoint;
	b2RevoluteJoint* m_leftSmallLegJoint;
	b2RevoluteJoint* m_rightFootLegJoint;
	b2RevoluteJoint* m_leftFootLegJoint;

	bool	m_mainBoosterFire;
	int	m_steeringBoosterFire;

   bool  m_boostFireLastUpdate;
   bool  m_rightSteerFireLastUpdate;
   bool  m_leftSteerFireLastUpdate;

   spFlameEmitter m_boosterFlame;
   spFlameEmitter m_leftSteerFlame;
   spFlameEmitter m_rightSteerFlame;

   oxygine::Resources* m_gameResources;

   // Data below is depending on the mode
   float m_boostInc;
   float m_boostMaxMagnitude;
   float m_steerImpulse;
   float m_eveningMagnitude;
   ModeAngles m_modeAngleGoals;

public:
	LeapFrog(
      oxygine::Resources& gameResources, 
      b2World* world, 
      oxygine::Actor* actor, 
      const oxygine::Vector2& pos, 
      float scale);
	~LeapFrog();
	void goToMode(LeapFrogModeEnum mode);
   void modeReached(void);
	void fireMainBooster(bool fire);
	void fireSteeringBooster(int dir); // -1 is counter clockwise, 1 is clockwise

private:
	void setJointMotor(b2RevoluteJoint* joint, float goal, float speedMagnitude);

protected:
   void doUpdate(const UpdateState &us);
};

