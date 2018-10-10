
#include "landingpad.h"

#include "sceneactor.h"

#include "gamestatus.h"

#include "actoruserdata.h"

using namespace oxygine;
using namespace std;


LandingPad::LandingPad(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   pugi::xml_node& root,
   int groupIndex) :
   CompoundObject(sceneParent, parentObject),
   m_leftFootContact(false),
   m_rightFootContact(false)
{
	initCompoundObjectParts(gameResources, sceneParent, parentObject, world, pos, root, string(""), groupIndex);

   // This CompoundObject is also an actor who normally has
   // a userData that points to its parent. However, the parent
   // of a CompoundObject is pointed by its m_parentObject 
   // member. The userData for this object should thus
   // be empty (=NULL)
   setUserData(NULL);

}

void LandingPad::leapfrogFootTouch(b2Contact* contact, bool leftFoot)
{
   if (leftFoot)
   {
      m_leftFootContact = true;
   }
   else
   {
      m_rightFootContact = true;
   }

   if (m_leftFootContact && m_rightFootContact)
   {
      // Start timer and at the end of that, if both feet still
      // are in contact, send event for Leapfrog has landed.
      logs::messageln("Landing Pad landing stable");
      g_GameStatus.deltaFuel(100.0f - g_GameStatus.getFuel());
   }
}

void LandingPad::leapfrogFootLift(b2Contact* contact, bool leftFoot)
{
   if (leftFoot)
   {
      m_leftFootContact = false;
   }
   else
   {
      m_rightFootContact = false;
   }
}

