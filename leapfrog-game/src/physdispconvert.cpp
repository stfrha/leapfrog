#include "physdispconvert.h"

using namespace oxygine;

PhysDispConvert::PhysDispConvert()
{
}


PhysDispConvert::~PhysDispConvert()
{
}


b2Vec2 PhysDispConvert::convert(const Vector2& pos, float scale)
{
	return b2Vec2(pos.x / scale, pos.y / scale);
}

Vector2 PhysDispConvert::convert(const b2Vec2& pos, float scale)
{
	return Vector2(pos.x * scale, pos.y * scale);
}
