#pragma once

#include "Box2D/Box2D.h"
#include "sceneactor.h"

class LanderContactListener : public b2ContactListener
{
public:
   void BeginContact(b2Contact* contact);
   void EndContact(b2Contact* contact);
   void ContactHandler(b2Contact* contact, bool begin); // begin = false means the end of the contact
};
