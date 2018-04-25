
#include "launchsite.h"

using namespace oxygine;
using namespace std;


LaunchSite::LaunchSite(
   Resources& gameResources,
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   string& defXmlFileName) 
{
	initCompoundObject(gameResources, sceneParent, parentObject, world, pos, defXmlFileName, string(""));
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
   return CET_NOT_APPLICABLE;
}

void LaunchSite::leapfrogFootTouch(b2Contact* contact, bool leftFoot)
{
   if (leftFoot)
   {
      m_leftFootContact = true;
   }

   if (!leftFoot)
   {
      m_rightFootContact = true;
   }

   if (m_leftFootContact && m_rightFootContact)
   {
      // Start timer and at the end of that, if both feet still
      // are in contact, send event for Leapfrog has landed.
      logs::messageln("Launch Site landing stable");

   }
}

void LaunchSite::leapfrogFootLift(b2Contact* contact, bool leftFoot)
{
   if (leftFoot)
   {
      m_leftFootContact = false;
   }

   if (!leftFoot)
   {
      m_rightFootContact = false;
   }
}

