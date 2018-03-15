#include "static.h"
#include "physdispconvert.h"
#include "scales.h"

using namespace oxygine;

// rc is in stage coordinates, i.e. meters
Static::Static(Resources& gameResources, b2World* world, const RectF& rc)
{
	//setHorizontalMode(Box9Sprite::TILING_FULL);
	//setVerticalMode(Box9Sprite::TILING_FULL);
	setResAnim(gameResources.getResAnim("ground"));

	// size and position here is in stage coordinate (same as rc)
	setSize(rc.getSize());
	setPosition(rc.getLeftTop());
	setAnchor(Vector2(0.5f, 0.5f));

	b2BodyDef groundBodyDef;

	groundBodyDef.position = PhysDispConvert::convert(getPosition(), 1.0);

	b2Body* groundBody = world->CreateBody(&groundBodyDef);

	b2PolygonShape groundBox;
	b2Vec2 sz = PhysDispConvert::convert(getSize() / Scales::c_physToStageScale / 2, 1);
	groundBox.SetAsBox(sz.x, sz.y);
	groundBody->CreateFixture(&groundBox, 0.0f);
}
