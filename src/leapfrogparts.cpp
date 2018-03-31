#include "leapfrogparts.h"

using namespace oxygine;

//LfBody::LfBody(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
//{
//	setResAnim(gameResources.getResAnim("lf_body"));
//
//	// Size and position is in stage coordinates
//	setSize(9.0, 9.6);
//	setPosition(pos);
//	setAnchor(Vector2(0.5f, 0.5f));
//	setTouchChildrenEnabled(false);
//
//	b2BodyDef bodyDef;
//	bodyDef.type = b2_dynamicBody;
//	bodyDef.position = PhysDispConvert::convert(pos / Scales::c_physToStageScale, 1.0);
//
//	m_body = world->CreateBody(&bodyDef);
//
//	setUserData(m_body);
//
//	b2Vec2 vertices[7];
//
//	// Polygon of a body shape is physical coordinates, i.e. in meters
//	vertices[6].Set(-4.5, 4.8);
//	vertices[5].Set(-2.5, -3.7);
//	vertices[4].Set(-1.5, -4.8);
//	vertices[3].Set(1.5, -4.8);
//	vertices[2].Set(2.5, -3.7);
//	vertices[1].Set(4.5, 4.8);
//	vertices[0].Set(-4.5, 4.8);
//
//	b2PolygonShape polyShape;
//
//	polyShape.Set(vertices, 7);
//
//	b2FixtureDef fixtureDef;
//	fixtureDef.shape = &polyShape;
//	fixtureDef.density = 5.0f;
//	fixtureDef.friction = 1.3f;
//	fixtureDef.filter.groupIndex = groupIndex;
//
//	m_body->CreateFixture(&fixtureDef);
//	m_body->SetUserData(this);
//
//   m_body->GetFixtureList()->SetUserData((CollisionEntity*)this);
//
//   m_body->ResetMassData();
//
//}
//
//CollisionEntityTypeEnum LfBody::getEntityType(void)
//{
//   return CET_ASTEROID;
//}


LfBooster::LfBooster(Resources& gameResources, b2World* world, const Vector2& pos, float scale, int16 groupIndex)
{
	setResAnim(gameResources.getResAnim("lf_boost"));

	// Size and position is in stage coordinates

	setSize(4, 1.6);
//	setPosition(pos);
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
	setSize(1.8, 7.3);
//	setPosition(pos);
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
	setSize(1.4, 5.3);
//	setPosition(pos);
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

	setSize(4, 0.6);

//	setSize(0.4, 0.06);
//	setPosition(pos);
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

	setSize(1.2, 1.0);
//	setPosition(pos);
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

	setSize(1.2, 1.0);
//	setPosition(pos);
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

   m_body->GetFixtureList()->SetUserData((CollisionEntity*)this);

   m_body->ResetMassData();
}

CollisionEntityTypeEnum LfLeftSteer::getEntityType(void)
{
   return CET_LF_STEER_BOOSTER;
}

