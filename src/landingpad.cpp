
#include "LandingPad.h"

using namespace oxygine;
using namespace std;


LandingPad::LandingPad(
   Resources& gameResources,
   Actor* parent,
   b2World* world,
   const Vector2& pos,
   string& defXmlFileName) 
{
	initCompoundObject(gameResources, parent, world, pos, defXmlFileName, string(""));
}

CollisionEntityTypeEnum LandingPad::getEntityType(void)
{
   return CET_NOT_APPLICABLE;
}

void LandingPad::leapfrogLanded(b2Contact* contact)
{
}