
#include "oxygine-framework.h"
#include "Box2DDebugDraw.h"
#include "Box2D/Box2D.h"
#include "flameemitter.h"


namespace oxygine
{

DECLARE_SMART(LfBody, spLfBody);

class LfBody : public Sprite
{
public:
	LfBody(Resources& gameResources, b2World* world, const Vector2& pos, float scalem, int16 groupIndex);
	b2Body * m_body;
};

DECLARE_SMART(LfBooster, spLfBooster);

class LfBooster : public Sprite
{
public:
	LfBooster(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex);
	b2Body * m_body;
};

DECLARE_SMART(LfBigLeg, spLfBigLeg);

class LfBigLeg : public Sprite
{
public:
	LfBigLeg(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex);
	b2Body * m_body;
};

DECLARE_SMART(LfSmallLeg, spLfSmallLeg);

class LfSmallLeg : public Sprite
{
public:
	LfSmallLeg(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex);
	b2Body * m_body;
};

DECLARE_SMART(LfFoot, spLfFoot);

class LfFoot : public Sprite
{
public:
	LfFoot(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex);
	b2Body * m_body;
};

DECLARE_SMART(LfRightSteer, spLfRightSteer);

class LfRightSteer : public Sprite
{
public:
	LfRightSteer(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex);
	b2Body * m_body;
};

DECLARE_SMART(LfLeftSteer, spLfLeftSteer);

class LfLeftSteer : public Sprite
{
public:
	LfLeftSteer(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex);
	b2Body * m_body;
};

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

class LeapFrog : public Sprite
{
private:
	const float c_boostInc;
	const float c_boostMaxMagnitude;
	const float c_steerImpulse;
	const float c_eveningMagnitude;

	const ModeAngles c_resetMode;
	const ModeAngles c_deepSpaceMode;
	const ModeAngles c_landingMode;
	const ModeAngles c_reentryMode;

	b2World * m_world;
	b2Body* m_mainBody;

	float	m_boostMagnuitude;
	float	m_steerMagnitude;
	float	m_physDispScale;
	spBox2DDraw m_debugDraw;

	ModeAngles m_modeAngleGoals;

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
	int		m_steeringBoosterFire;

   bool  m_boostFireLastUpdate;
   bool  m_rightSteerFireLastUpdate;
   bool  m_leftSteerFireLastUpdate;

   spFlameEmitter m_boosterFlame;
   spFlameEmitter m_leftSteerFlame;
   spFlameEmitter m_rightSteerFlame;

   Resources* m_gameResources;

public:
	LeapFrog(Resources& gameResources, b2World* world, Actor* actor, const Vector2& pos, float scale);
	~LeapFrog();
	void goToMode(LeapFrogModeEnum mode);
	void fireMainBooster(bool fire);
	void fireSteeringBooster(int dir); // -1 is counter clockwise, 1 is clockwise

private:
	void setJointMotor(b2RevoluteJoint* joint, float goal, float speedMagnitude);

protected:
   void doUpdate(const UpdateState &us);
};

} // namespace oxygine