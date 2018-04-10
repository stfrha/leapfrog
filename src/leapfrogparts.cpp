#include "leapfrogparts.h"

using namespace oxygine;

LfBooster::LfBooster(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_boost"));

	// Size and position is in stage coordinates

	setSize(4.0f, 1.6f);
//	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);
   setPriority(128);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[7];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[6].Set(-2.0f, 0.8f);
	vertices[5].Set(-2.0f, 0.2f);
	vertices[4].Set(-1.0f, -0.8f);
	vertices[3].Set(1.0f, -0.8f);
	vertices[2].Set(2.0f, 0.2f);
	vertices[1].Set(2.0f, 0.8f);
	vertices[0].Set(-2.0f, 0.8f);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 7);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
   fixtureDef.filter.categoryBits = 4;
   fixtureDef.filter.maskBits = 40123;

	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

   m_body->GetFixtureList()->SetUserData((CollisionEntity*)this);

   m_body->ResetMassData();
}

CollisionEntityTypeEnum LfBooster::getEntityType(void)
{
   return CET_LF_BOOSTER;
}

LfBigLeg::LfBigLeg(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_big_leg"));

	// Size and position is in stage coordinates
	setSize(1.8f, 7.3f);
//	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);
   setPriority(128);

   b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[5];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[4].Set(-0.7f, 3.65f);
	vertices[3].Set(-0.9f, -3.65f);
	vertices[2].Set(0.9f, -3.65f);
	vertices[1].Set(0.7f, 3.65f);
	vertices[0].Set(-0.7f, 3.65f);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 5);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
   fixtureDef.filter.categoryBits = 4;
   fixtureDef.filter.maskBits = 40123;

	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

   m_body->GetFixtureList()->SetUserData((CollisionEntity*)this);

   m_body->ResetMassData();
}

CollisionEntityTypeEnum LfBigLeg::getEntityType(void)
{
   return CET_LF_BIG_LEG;
}

LfSmallLeg::LfSmallLeg(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_small_leg"));

	// Size and position is in stage coordinates
	setSize(1.4f, 5.3f);
//	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);
   setPriority(128);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0f);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[5];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[4].Set(-0.7f, 2.65f);
	vertices[3].Set(-0.6f, -2.65f);
	vertices[2].Set(0.6f, -2.65f);
	vertices[1].Set(0.7f, 2.65f);
	vertices[0].Set(-0.7f, 2.65f);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 5);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
   fixtureDef.filter.categoryBits = 4;
   fixtureDef.filter.maskBits = 40123;

	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

   m_body->GetFixtureList()->SetUserData((CollisionEntity*)this);

   m_body->ResetMassData();
}

CollisionEntityTypeEnum LfSmallLeg::getEntityType(void)
{
   return CET_LF_SMALL_LEG;
}

LfFoot::LfFoot(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_foot"));

	// Size and position is in stage coordinates

	setSize(4.0f, 0.6f);

//	setSize(0.4, 0.06);
//	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);
   setPriority(128);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0f);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[5];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[4].Set(-1.4f, 0.3f);
	vertices[3].Set(-2.0f, -0.3f);
	vertices[2].Set(2.0f, -0.3f);
	vertices[1].Set(1.4f, 0.3f);
	vertices[0].Set(-1.4f, 0.3f);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 5);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
   fixtureDef.filter.categoryBits = 4;
   fixtureDef.filter.maskBits = 40123;


	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

   m_body->GetFixtureList()->SetUserData((CollisionEntity*)this);

   m_body->ResetMassData();
}

CollisionEntityTypeEnum LfFoot::getEntityType(void)
{
   return CET_LF_FOOT;
}

LfRightSteer::LfRightSteer(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_right_steer"));

	// Size and position is in stage coordinates

	setSize(1.2f, 1.0f);
//	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);
   setPriority(128);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[5];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[4].Set(-0.6f, 0.5f);
	vertices[3].Set(-0.6f, -0.5f);
	vertices[2].Set(0.6f, -0.5f);
	vertices[1].Set(0.6f, 0.5f);
	vertices[0].Set(-0.6f, 0.5f);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 5);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
   fixtureDef.filter.categoryBits = 4;
   fixtureDef.filter.maskBits = 40123;


	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

   m_body->GetFixtureList()->SetUserData((CollisionEntity*)this);

   m_body->ResetMassData();
}

CollisionEntityTypeEnum LfRightSteer::getEntityType(void)
{
   return CET_LF_STEER_BOOSTER;
}

LfLeftSteer::LfLeftSteer(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_left_steer"));

	// Size and position is in stage coordinates

	setSize(1.2f, 1.0f);
//	setPosition(pos);
	setAnchor(Vector2(0.5f, 0.5f));
	setTouchChildrenEnabled(false);
   setPriority(128);

	b2BodyDef bodyDef;
	bodyDef.type = b2_dynamicBody;
	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0f);

	m_body = world->CreateBody(&bodyDef);

	setUserData(m_body);

	b2Vec2 vertices[5];

	// Polygon of a body shape is physical coordinates, i.e. in meters
	vertices[4].Set(-0.6f, 0.5f);
	vertices[3].Set(-0.6f, -0.5f);
	vertices[2].Set(0.6f, -0.5f);
	vertices[1].Set(0.6f, 0.5f);
	vertices[0].Set(-0.6f, 0.5f);

	b2PolygonShape polyShape;

	polyShape.Set(vertices, 5);

	b2FixtureDef fixtureDef;
	fixtureDef.shape = &polyShape;
	fixtureDef.density = 5.0f;
	fixtureDef.friction = 1.3f;
   fixtureDef.filter.categoryBits = 4;
   fixtureDef.filter.maskBits = 40123;


	m_body->CreateFixture(&fixtureDef);
	m_body->SetUserData(this);

   m_body->GetFixtureList()->SetUserData((CollisionEntity*)this);

   m_body->ResetMassData();
}

CollisionEntityTypeEnum LfLeftSteer::getEntityType(void)
{
   return CET_LF_STEER_BOOSTER;
}

