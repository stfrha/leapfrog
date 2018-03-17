#include "physdispconvert.h"
#include "scales.h"

#include "SDL.h"
#include "SDL_keyboard.h"

#include "leapfrog.h"

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

LfBody::LfBody(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_body"));

	// Size and position is in stage coordinates
	setSize(9.0, 9.6);
	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[7];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[6].Set(-4.5, 4.8);
	vertices[5].Set(-2.5, -3.7);
	vertices[4].Set(-1.5, -4.8);
	vertices[3].Set(1.5, -4.8);
	vertices[2].Set(2.5, -3.7);
	vertices[1].Set(4.5, 4.8);
	vertices[0].Set(-4.5, 4.8);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 7);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
	fixtureDef.filter.groupIndex = groupIndex;

	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

	m_body->ResetMassData();

}

LfBooster::LfBooster(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_boost"));

	// Size and position is in stage coordinates

	setSize(4, 1.6);
	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[7];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[6].Set(-2, 0.8);
	vertices[5].Set(-2, 0.2);
	vertices[4].Set(-1, -0.8);
	vertices[3].Set(1, -0.8);
	vertices[2].Set(2, 0.2);
	vertices[1].Set(2, 0.8);
	vertices[0].Set(-2, 0.8);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 7);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
	fixtureDef.filter.groupIndex = groupIndex;

	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

	m_body->ResetMassData();
}

LfBigLeg::LfBigLeg(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_big_leg"));

	// Size and position is in stage coordinates
	setSize(1.8, 7.3);
	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[5];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[4].Set(-0.7, 3.65);
	vertices[3].Set(-0.9, -3.65);
	vertices[2].Set(0.9, -3.65);
	vertices[1].Set(0.7, 3.65);
	vertices[0].Set(-0.7, 3.65);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 5);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
	fixtureDef.filter.groupIndex = groupIndex;

	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

	m_body->ResetMassData();
}

LfSmallLeg::LfSmallLeg(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_small_leg"));

	// Size and position is in stage coordinates
	setSize(1.4, 5.3);
	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[5];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[4].Set(-0.7, 2.65);
	vertices[3].Set(-0.6, -2.65);
	vertices[2].Set(0.6, -2.65);
	vertices[1].Set(0.7, 2.65);
	vertices[0].Set(-0.7, 2.65);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 5);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
	fixtureDef.filter.groupIndex = groupIndex;

	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

	m_body->ResetMassData();
}

LfFoot::LfFoot(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_foot"));

	// Size and position is in stage coordinates

	setSize(4, 0.6);

//	setSize(0.4, 0.06);
	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[5];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[4].Set(-1.4, 0.3);
	vertices[3].Set(-2, -0.3);
	vertices[2].Set(2, -0.3);
	vertices[1].Set(1.4, 0.3);
	vertices[0].Set(-1.4, 0.3);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 5);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
	fixtureDef.filter.groupIndex = groupIndex;


	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

	m_body->ResetMassData();
}

LfRightSteer::LfRightSteer(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_right_steer"));

	// Size and position is in stage coordinates

	setSize(1.2, 1.0);
	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[5];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[4].Set(-0.6, 0.5);
	vertices[3].Set(-0.6, -0.5);
	vertices[2].Set(0.6, -0.5);
	vertices[1].Set(0.6, 0.5);
	vertices[0].Set(-0.6, 0.5);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 5);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
	fixtureDef.filter.groupIndex = groupIndex;


	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

	m_body->ResetMassData();
}

LfLeftSteer::LfLeftSteer(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_left_steer"));

	// Size and position is in stage coordinates

	setSize(1.2, 1.0);
	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[5];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[4].Set(-0.6, 0.5);
	vertices[3].Set(-0.6, -0.5);
	vertices[2].Set(0.6, -0.5);
	vertices[1].Set(0.6, 0.5);
	vertices[0].Set(-0.6, 0.5);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 5);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
	fixtureDef.filter.groupIndex = groupIndex;


	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

	m_body->ResetMassData();
}


LeapFrog::LeapFrog(Resources& gameResources, b2World* world, Actor* actor, const Vector2& pos, float scale) :
	m_world(world),
	m_physDispScale(10.0f),
	m_boostMagnuitude(0.0f),
	m_steerMagnitude(0.0f),
	c_boostInc(900.0f),
	c_boostMaxMagnitude(3000.0f),
	c_steerImpulse(20000.0f),
	c_eveningMagnitude(10000.0f),
		c_deepSpaceMode(
		-187.5 * MATH_PI / 180, -3.5 * MATH_PI / 180, 
		-117.5 * MATH_PI / 180, 187 * MATH_PI / 180, 
		3.5 * MATH_PI / 180, 117.5 * MATH_PI / 180),
	c_landingMode(
		-65 * MATH_PI / 180, 65 * MATH_PI / 180, 
		0 * MATH_PI / 180, 65 * MATH_PI / 180, 
		-65 * MATH_PI / 180, 0 * MATH_PI / 180),
	c_reentryMode(
		-92 * MATH_PI / 180, -156.5 * MATH_PI / 180, 
		-35 * MATH_PI / 180, 92 * MATH_PI / 180, 
		156.5 * MATH_PI / 180, 35 * MATH_PI / 180),
	c_resetMode(9, 0, 0, 0, 0, 0),
   m_boostFireLastUpdate(false),
   m_rightSteerFireLastUpdate(false),
   m_leftSteerFireLastUpdate(false),
   m_gameResources(&gameResources)
{
	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);

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
	vertices[6].Set(-4.5, 4.8);
	vertices[5].Set(-2.5, -3.7);
	vertices[4].Set(-1.5, -4.8);
	vertices[3].Set(1.5, -4.8);
	vertices[2].Set(2.5, -3.7);
	vertices[1].Set(4.5, 4.8);
	vertices[0].Set(-4.5, 4.8);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 7);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
	fixtureDef.filter.groupIndex = -1;

	m_mainBody->CreateFixture(&fixtureDef);
	m_mainBody->SetUserData(this);

	m_mainBody->ResetMassData();
	
	//spLfBody lfBody = new LfBody(gameResources, world, pos, 1);
	//actor->addChild(lfBody);
	//m_leapFrogBody = lfBody;

	spLfBooster lfBoost = new LfBooster(gameResources, world, pos + Vector2(0, 4.8), 1, -1);
	actor->addChild(lfBoost);

	spLfBigLeg lfRightBigLeg = new LfBigLeg(gameResources, world, pos + Vector2(3, 4.9), 1, -1);
	actor->addChild(lfRightBigLeg);

	spLfBigLeg lfLeftBigLeg = new LfBigLeg(gameResources, world, pos + Vector2(-3.2, 4.9), 1, -1);
	actor->addChild(lfLeftBigLeg);

	spLfSmallLeg lfRightSmallLeg = new LfSmallLeg(gameResources, world, pos + Vector2(3.2, 12.2), 1, -1);
	actor->addChild(lfRightSmallLeg);

	spLfSmallLeg lfLeftSmallLeg = new LfSmallLeg(gameResources, world, pos + Vector2(-3.2, 12.2), 1, -1);
	actor->addChild(lfLeftSmallLeg);

	spLfFoot lfRightFoot = new LfFoot(gameResources, world, pos + Vector2(3.2, 17.5), 1, -1);
	actor->addChild(lfRightFoot);

	spLfFoot lfLeftFoot = new LfFoot(gameResources, world, pos + Vector2(-3.2, 17.5), 1, -1);
	actor->addChild(lfLeftFoot);

	spLfRightSteer lfRightSteer = new LfRightSteer(gameResources, world, pos + Vector2(3.9, 17.5), 1, -1);
	actor->addChild(lfRightSteer);

	spLfLeftSteer lfLeftSteer = new LfLeftSteer(gameResources, world, pos + Vector2(-3.9, 17.5), 1, -1);
	actor->addChild(lfLeftSteer);

	// Main Body and booster joint
	b2WeldJointDef	boostJointDef;
	boostJointDef.bodyA = m_mainBody;
	boostJointDef.bodyB = lfBoost->m_body;
	boostJointDef.localAnchorA.Set(0.0, 4.8);
	boostJointDef.localAnchorB.Set(0.0, -0.8);
	boostJointDef.collideConnected = false;
	m_boostJoint = (b2WeldJoint*)world->CreateJoint(&boostJointDef);

	// Main Body and right big leg joint
	b2RevoluteJointDef	rightLegJointDef;
	rightLegJointDef.bodyA = m_mainBody;
	rightLegJointDef.bodyB = lfRightBigLeg->m_body;
	rightLegJointDef.localAnchorA.Set(3.3, 5.8);
	rightLegJointDef.localAnchorB.Set(0.0, -2.75);
	rightLegJointDef.collideConnected = false;
	rightLegJointDef.enableMotor = true;
	rightLegJointDef.maxMotorTorque = 20000;
	rightLegJointDef.motorSpeed = 0;
	m_rightBigLegJoint = (b2RevoluteJoint*)world->CreateJoint(&rightLegJointDef);

	// Main Body and left big leg joint
	b2RevoluteJointDef	leftLegJointDef;
	leftLegJointDef.bodyA = m_mainBody;
	leftLegJointDef.bodyB = lfLeftBigLeg->m_body;
	leftLegJointDef.localAnchorA.Set(-3.3, 5.8);
	leftLegJointDef.localAnchorB.Set(0.0, -2.75);
	leftLegJointDef.collideConnected = false;
	leftLegJointDef.enableMotor = true;
	leftLegJointDef.maxMotorTorque = 20000;
	leftLegJointDef.motorSpeed = 0;
	m_leftBigLegJoint = (b2RevoluteJoint*)world->CreateJoint(&leftLegJointDef);

	// Right Big leg and right small leg joint
	b2RevoluteJointDef	rightSmallLegJointDef;
	rightSmallLegJointDef.bodyA = lfRightBigLeg->m_body;
	rightSmallLegJointDef.bodyB = lfRightSmallLeg->m_body;
	rightSmallLegJointDef.localAnchorA.Set(0.0, 2.95);
	rightSmallLegJointDef.localAnchorB.Set(0.0, -1.95);
	rightSmallLegJointDef.collideConnected = false;
	rightSmallLegJointDef.enableMotor = true;
	rightSmallLegJointDef.maxMotorTorque = 20000;
	rightSmallLegJointDef.motorSpeed = 0;
	m_rightSmallLegJoint = (b2RevoluteJoint*)world->CreateJoint(&rightSmallLegJointDef);

	// Left Big leg and left small leg joint
	b2RevoluteJointDef	leftSmallLegJointDef;
	leftSmallLegJointDef.bodyA = lfLeftBigLeg->m_body;
	leftSmallLegJointDef.bodyB = lfLeftSmallLeg->m_body;
	leftSmallLegJointDef.localAnchorA.Set(0.0, 2.95);
	leftSmallLegJointDef.localAnchorB.Set(0.0, -1.95);
	leftSmallLegJointDef.collideConnected = false;
	leftSmallLegJointDef.enableMotor = true;
	leftSmallLegJointDef.maxMotorTorque = 20000;
	leftSmallLegJointDef.motorSpeed = 0;
	m_leftSmallLegJoint = (b2RevoluteJoint*)world->CreateJoint(&leftSmallLegJointDef);

	// Right Small leg and right foot joint
	b2RevoluteJointDef	rightFootLegJointDef;
	rightFootLegJointDef.bodyA = lfRightSmallLeg->m_body;
	rightFootLegJointDef.bodyB = lfRightFoot->m_body;
	rightFootLegJointDef.localAnchorA.Set(0.0, 2.05);
	rightFootLegJointDef.localAnchorB.Set(0.0, -0.9);
	rightFootLegJointDef.collideConnected = false;
	rightFootLegJointDef.enableMotor = true;
	rightFootLegJointDef.maxMotorTorque = 20000;
	rightFootLegJointDef.motorSpeed = 0;
	m_rightFootLegJoint = (b2RevoluteJoint*)world->CreateJoint(&rightFootLegJointDef);

	// Left Small leg and left foot joint
	b2RevoluteJointDef	leftFootLegJointDef;
	leftFootLegJointDef.bodyA = lfLeftSmallLeg->m_body;
	leftFootLegJointDef.bodyB = lfLeftFoot->m_body;
	leftFootLegJointDef.localAnchorA.Set(0.0, 2.05);
	leftFootLegJointDef.localAnchorB.Set(0.0, -0.9);
	leftFootLegJointDef.collideConnected = false;
	leftFootLegJointDef.enableMotor = true;
	leftFootLegJointDef.maxMotorTorque = 20000;
	leftFootLegJointDef.motorSpeed = 0;
	m_leftFootLegJoint = (b2RevoluteJoint*)world->CreateJoint(&leftFootLegJointDef);

	// RIght small leg and steer booster joint
	b2WeldJointDef	rightSteerJointDef;
	rightSteerJointDef.bodyA = lfRightSmallLeg->m_body;
	rightSteerJointDef.bodyB = lfRightSteer->m_body;
	rightSteerJointDef.localAnchorA.Set(-0.7, 1.4);
	rightSteerJointDef.localAnchorB.Set(0.6, 0.0);
	rightSteerJointDef.collideConnected = false;
	m_rightSteerJoint = (b2WeldJoint*)world->CreateJoint(&rightSteerJointDef);

	// Left small leg and steer booster joint
	b2WeldJointDef	leftSteerJointDef;
	leftSteerJointDef.bodyA = lfLeftSmallLeg->m_body;
	leftSteerJointDef.bodyB = lfLeftSteer->m_body;
	leftSteerJointDef.localAnchorA.Set(0.7, 1.4);
	leftSteerJointDef.localAnchorB.Set(-0.6, 0.0);
	leftSteerJointDef.collideConnected = false;
	m_leftSteerJoint = (b2WeldJoint*)world->CreateJoint(&leftSteerJointDef);

   // Add main engine particle system
   m_boosterFlame = new FlameEmitter(
      gameResources, 
      lfBoost->m_body, 
      b2Vec2(0.0f, 3.0f), 
      90.0f * MATH_PI / 180.0f, 
      4.0f, 
      0,                               // Intensity
      250,                             // Lifetime [ms]
      70);                             // Impulse magnitude

   m_boosterFlame->attachTo(this);
	
   // Add main engine particle system
   m_boosterFlame = new FlameEmitter(
      gameResources,
      lfBoost->m_body,
      b2Vec2(0.0f, 3.0f),
      90.0f * MATH_PI / 180.0f,
      4.0f,
      0,                               // Intensity
      250,                             // Lifetime [ms]
      70);                             // Impulse magnitude

   m_boosterFlame->attachTo(this);

   
   // Add right steering engine particle system
   m_rightSteerFlame = new FlameEmitter(
      gameResources,
      lfRightSteer->m_body,
      b2Vec2(-1.0f, 0.0f),
      MATH_PI,
      1.0f,                            // Width
      0,                               // Intensity
      125,                             // Lifetime [ms]
      35);                             // Impulse magnitude

   m_rightSteerFlame->attachTo(this);

   // Add left steering engine particle system
   m_leftSteerFlame = new FlameEmitter(
      gameResources,
      lfLeftSteer->m_body,
      b2Vec2(1.0f, 0.0f),             // Origin
      0,                               // Angle 
      1.0f,                            // Width
      0,                               // Intensity
      125,                             // Lifetime [ms]
      35);                             // Impulse magnitude

   m_leftSteerFlame->attachTo(this);
}

LeapFrog::~LeapFrog()
{
	//b2Joint* joint = m_world->GetJointList();
	//while (joint)
	//{
	//	b2Joint* nextJ = joint->GetNext();
	//	m_world->DestroyJoint(joint);

	//	joint = nextJ;
	//}

	//m_boostJoint = 0;
	//m_rightSteerJoint = 0;
	//m_leftSteerJoint = 0;
	//m_rightBigLegJoint = 0;
	//m_leftBigLegJoint = 0;
	//m_rightSmallLegJoint = 0;
	//m_leftSmallLegJoint = 0;
	//m_rightFootLegJoint = 0;
	//m_leftFootLegJoint = 0;


	// b2Body* body = m_world->GetBodyList();
	//while (body)
	//{
	//	Actor* actor = (Actor*)body->GetUserData();
	//	b2Body* nextB = body->GetNext();
	//	if (actor)
	//	{
	//		body->SetUserData(0);
	//		m_world->DestroyBody(body);

	//		actor->detach();
	//	}

	//	body = nextB;
	//}
	delete m_world;
}


void LeapFrog::doUpdate(const UpdateState &us)
{
   float angle = m_mainBody->GetAngle();
   b2Vec2 boostForce = b2Vec2(m_boostMagnuitude * sin(angle), -m_boostMagnuitude * cos(angle));
   
   m_mainBody->ApplyTorque(m_steerMagnitude, true);
   m_mainBody->ApplyForceToCenter(boostForce, true);

   setJointMotor(m_rightBigLegJoint, m_modeAngleGoals.m_rightBigJointAngle, 15 * MATH_PI / 180);
   setJointMotor(m_rightSmallLegJoint, m_modeAngleGoals.m_rightSmallJointAngle, 15 * MATH_PI / 180);
   setJointMotor(m_rightFootLegJoint, m_modeAngleGoals.m_rightFootJointAngle, 15 * MATH_PI / 180);
   setJointMotor(m_leftBigLegJoint, m_modeAngleGoals.m_leftBigJointAngle, 15 * MATH_PI / 180);
   setJointMotor(m_leftSmallLegJoint, m_modeAngleGoals.m_leftSmallJointAngle, 15 * MATH_PI / 180);
   setJointMotor(m_leftFootLegJoint, m_modeAngleGoals.m_leftFootJointAngle, 15 * MATH_PI / 180);
};

void LeapFrog::setJointMotor(b2RevoluteJoint* joint, float goal, float speedMagnitude)
{
	float angle = joint->GetJointAngle();

	if (angle > goal)
	{
		if (angle - goal > 0.5 * MATH_PI / 180)
		{
			joint->SetMotorSpeed(-speedMagnitude);
		}
		else
		{
			joint->SetMotorSpeed(0);
		}
	}
	else if (angle < goal)
	{
		if (goal - angle > 0.5 * MATH_PI / 180)
		{
			joint->SetMotorSpeed(-speedMagnitude);
		}
		joint->SetMotorSpeed(speedMagnitude);
	}
}

void LeapFrog::goToMode(LeapFrogModeEnum mode)
{
	switch (mode)
	{
	case LFM_LANDING:
		m_modeAngleGoals = c_landingMode;
		break;
	case LFM_DEEP_SPACE:
		m_modeAngleGoals = c_deepSpaceMode;
		break;
	case LFM_ORBIT:
		m_modeAngleGoals = c_reentryMode;
		break;
	case LFM_REENTRY:
		m_modeAngleGoals = c_reentryMode;
		break;
	case LFM_RESET:
		m_modeAngleGoals = c_resetMode;
		break;
	}

}

void LeapFrog::fireMainBooster(bool fire)
{
	if (fire)
	{
      if (m_boostFireLastUpdate == false)
      {
         // Turn emitter on here
         m_boosterFlame->startEmitter();
      }

		m_boostMagnuitude += c_boostInc;

		if (m_boostMagnuitude > c_boostMaxMagnitude)
		{
			m_boostMagnuitude = c_boostMaxMagnitude;
		}
      m_boostFireLastUpdate = true;
	}
	else
	{
      if (m_boostFireLastUpdate == true)
      {
         // Turn emitter off here
         m_boosterFlame->stopEmitter();
      }

      m_boostMagnuitude = 0;
      m_boostFireLastUpdate = false;
   }

}

void LeapFrog::fireSteeringBooster(int dir)
{
	if (dir == -1)
	{
      m_leftSteerFlame->stopEmitter();

      if (m_rightSteerFireLastUpdate == false)
      {
         m_rightSteerFlame->startEmitter();
      }

      m_steerMagnitude = -c_steerImpulse;
      
      m_rightSteerFireLastUpdate = true;
	}
	else if (dir == 1)
	{
      m_rightSteerFlame->stopEmitter();
      
      if (m_leftSteerFireLastUpdate == false)
      {
         m_leftSteerFlame->startEmitter();
      }

      m_steerMagnitude = c_steerImpulse;

      m_leftSteerFireLastUpdate = true;
   }
	else if (dir == 0)
	{
		float angleVel = m_mainBody->GetAngularVelocity();

      m_rightSteerFlame->stopEmitter();
      m_leftSteerFlame->stopEmitter();

		if (angleVel > 0)
		{
			m_steerMagnitude = -c_eveningMagnitude;
		}
		else if (angleVel < 0)
		{
			m_steerMagnitude = c_eveningMagnitude;
		}

      m_rightSteerFireLastUpdate = false;
      m_leftSteerFireLastUpdate = false;
   }

}
