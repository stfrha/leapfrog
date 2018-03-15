#include "World.h"



World::World()
{
	// set size of this object to the size of the window
	setSize(getStage()->getSize());

	// create the hysics world
	world = new b2World(b2Vec2(0, 150));

	spSprite = new Sprite()
}


World::~World()
{
}
