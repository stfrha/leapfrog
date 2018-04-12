
#include "launchsite.h"

using namespace oxygine;
using namespace std;


LaunchSite::LaunchSite(
   Resources& gameResources,
   Actor* parent,
   b2World* world,
   const Vector2& pos,
   string& defXmlFileName) :
   CompoundObject(gameResources, parent, world, pos, defXmlFileName)
{
	b2Joint* leftHolderJoint = getJoint("leftHolderTankJoint");
	b2Joint* rightHolderJoint = getJoint("rightHolderTankJoint");
	b2RevoluteJoint* leftHolderTrolleyJoint = (b2RevoluteJoint*) getJoint("leftHolderJoint");
	b2RevoluteJoint* rightHolderTrolleyJoint = (b2RevoluteJoint*) getJoint("rightHolderJoint");

	world->DestroyJoint(leftHolderJoint);
	world->DestroyJoint(rightHolderJoint);
	leftHolderTrolleyJoint->EnableLimit(false);
	rightHolderTrolleyJoint->EnableLimit(false);
}

CollisionEntityTypeEnum LaunchSite::getEntityType(void)
{
   return CBT_LAUNCH_SITE_STATIC;
}

