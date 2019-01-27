#pragma once

#include "oxygine-framework.h"
#include "Box2d/Box2D.h"

namespace oxygine
{


class PhysDispConvert
{
public:
	PhysDispConvert();
	~PhysDispConvert();

	static b2Vec2 convert(const Vector2& pos, float scale);
	static Vector2 convert(const b2Vec2& pos, float scale);

};

} // namespace oxygine