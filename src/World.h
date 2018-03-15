#pragma once

#include "Box2D\Box2D.h"
#include "oxygine-framework.h"
#include "leapfrogbody.h"

using namespace oxygine;

class World : public Actor
{
public:
	World();
	~World();

	void doUpdate(const UpdateState&);

private:
	b2World * world;
	const float SCALE = 100.0f;
	const float FIXED_TIMESTEP = 1.0f / 60.0f;

	Vector2 convert(const b2Vec2& pos);
	b2Vec2 convert(const Vector2& pos);

};

