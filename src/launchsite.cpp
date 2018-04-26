
#include "launchsite.h"

#include "launchsiteevents.h"

using namespace oxygine;
using namespace std;


LaunchSite::LaunchSite(
   Resources& gameResources,
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   string& defXmlFileName) :
   m_world(world),
   m_launch(false),
   m_started(false)
{
	initCompoundObject(gameResources, sceneParent, parentObject, world, pos, defXmlFileName, string(""));

   // Here we attach Launch Site object to tree so it gets updates etc.
   attachTo(sceneParent);

}

CollisionEntityTypeEnum LaunchSite::getEntityType(void)
{
   return CET_NOT_APPLICABLE;
}

void LaunchSite::doUpdate(const UpdateState &us)
{
   if (m_launch && !m_started)
   {
      b2Joint* leftHolderJoint = getJoint("leftHolderTankJoint");
      b2Joint* rightHolderJoint = getJoint("rightHolderTankJoint");
      b2RevoluteJoint* leftHolderTrolleyJoint = (b2RevoluteJoint*)getJoint("leftHolderJoint");
      b2RevoluteJoint* rightHolderTrolleyJoint = (b2RevoluteJoint*)getJoint("rightHolderJoint");

      m_world->DestroyJoint(leftHolderJoint);
      m_world->DestroyJoint(rightHolderJoint);
      leftHolderTrolleyJoint->EnableLimit(false);
      rightHolderTrolleyJoint->EnableLimit(false);

      m_started = true;

   }
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
      LaunchSiteLeapfrogLandedEvent event;
      dispatchEvent(&event);
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

void LaunchSite::startLaunchSequence(void)
{
   m_launch = true;
}