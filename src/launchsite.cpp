
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
}

CollisionEntityTypeEnum LaunchSite::getEntityType(void)
{
   return CBT_LAUNCH_SITE_STATIC;
}

