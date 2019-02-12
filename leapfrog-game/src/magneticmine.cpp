
#include "magneticmine.h"

#include "bodyuserdata.h"
#include "sceneactor.h"
//#include "freespaceactor.h"
//#include "blastemitter.h"
//#include "actoruserdata.h"

//#include "groupindexsource.h"

using namespace oxygine;
using namespace pugi;

MagneticMine::MagneticMine(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& root,
   xml_node& propNode,
   int groupIndex) :
   ExplosiveObject(
      gameResources, 
      sceneParent, 
      parentObject, 
      world, 
      pos, 
      root, 
      propNode,
      groupIndex),
   m_magneticDistance(0.0f),
   m_magneticForce(0.0f),
   m_magenticMineBody(NULL)
{
   readMagneticMineNode(propNode);
     
   m_magenticMineBody = getBody("magenticMineBody");
}

void MagneticMine::readMagneticMineNode(const pugi::xml_node& node)
{
   m_magneticDistance = node.attribute("magneticDistance").as_float(0.0f);
   m_magneticForce = node.attribute("magneticForce").as_float(0.0f);
}


void MagneticMine::doUpdate(const oxygine::UpdateState& us)
{
   ExplosiveObject::doUpdate(us);

   if (!m_magenticMineBody)
   {
      return;
   }

   b2Body* currBody = m_world->GetBodyList();

   b2Vec2 force(0.0f, 0.0f);


   while (currBody)
   {
      b2Body* next = currBody->GetNext();

      CollisionEntity::CollisionEntityTypeEnum ce = BodyUserData::getCollisionType(currBody->GetUserData());

      // TODO: Add other collision entities that can be blasted by a mine
      if (ce == CollisionEntity::CollisionEntityTypeEnum::leapfrog)
      {

         b2Vec2 diff = currBody->GetPosition() - m_magenticMineBody->GetPosition();

         // Normalize and get length
         float distance = diff.Normalize();

         if (distance < m_magneticDistance)
         {
            float forceMag = (m_magneticDistance - distance) / m_magneticDistance * m_magneticForce;
            force += forceMag * diff;
         }
      }

      currBody = next;
   }

   m_magenticMineBody->ApplyForceToCenter(force, true);

}

