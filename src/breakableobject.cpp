
#include "breakableobject.h"

#include "sceneactor.h"
#include "freespaceactor.h"
#include "blastemitter.h"
#include "actoruserdata.h"


using namespace oxygine;
using namespace pugi;

BreakableObject::BreakableObject(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& root,
   int groupIndex) :
   CompoundObject(sceneParent, parentObject),
   m_gameResource(&gameResources),
   m_sceneActor(sceneParent),
   m_world(world),
   m_damage(0)
{
   readBreakableObjectNode(root);
   m_spawnObjects = new SpawnObjectList();
     
   m_spawnObjects->readSpawnObjectsNode(root);

   initCompoundObjectParts(
      gameResources,
      sceneParent,
      parentObject,
      world,
      pos,
      root,
      "",
      groupIndex);

   attachTo(m_sceneActor);

   // This CompoundObject is also an actor who normally has
   // a userData that points to its parent. However, the parent
   // of a CompoundObject is pointed by its m_parentObject 
   // member. The userData for this object should thus
   // be empty (=NULL)
   setUserData(NULL);

   if (m_sceneActor->getSceneType() == STE_FREE_SPACE)
   {
      FreeSpaceActor* spaceActor = (FreeSpaceActor*)m_sceneActor;

      setAllBodiesToBounding(spaceActor);
   }

}

void BreakableObject::readBreakableObjectNode(const pugi::xml_node& node)
{
   m_breakAtDamage = node.child("behaviour").attribute("breakBulletDamage").as_int(0);
   m_numberOfSpawns = node.child("behaviour").attribute("numberOfSpawns").as_int(0);
}

void BreakableObject::collisionBlast(b2Contact* contact, bool small)
// Sparks blastemitter, small is default, can also be big
{
   // Assume small blast
   int emitterLifetime = 150;
   int particleLifetime = 500;
   float particleDistance = 30.0f;
   float particleSize = 0.75f;
   float blastIntensity = 200.0f;

   if (!small)
   {
      emitterLifetime = 250;
      particleLifetime = 500;
      particleDistance = 60.0f;
      particleSize = 0.9f;
      blastIntensity = 300.0f;
   }

   b2WorldManifold m;
   contact->GetWorldManifold(&m);

   if (contact->GetManifold()->pointCount > 0)
   {
      spBlastEmitter blast = new BlastEmitter(
         m_sceneActor->getResources(),
         PhysDispConvert::convert(m.points[0], 1.0f),
         blastIntensity,                                     // Intensity, particles / sec
         emitterLifetime,                                    // Emitter Lifetime
         particleLifetime,                                   // Particle lifetime
         particleDistance,                                   // Particle distance
         particleSize);                                      // Particle spawn size
      blast->attachTo(m_sceneActor);
   }
}

void BreakableObject::damageCollision(b2Contact* contact, float bulletEqvDamage)
// Create blast emitter and check for breaking
{
   bool shattered = false;

   // Take damage
   m_damage += (int)(25.0f * bulletEqvDamage);

   if (m_damage >= m_breakAtDamage)
   {
      addShapesToDeathList();

      spawnBreakableObjects();

      collisionBlast(contact, false);
   }

   collisionBlast(contact, true);
}

void BreakableObject::doUpdate(const oxygine::UpdateState& us)
{
   //if (m_spawnCount > 0)
   //{
   //   spawnBreakableObjects();
   //}
}


// TODO: Should be called addMeToDeathList, also move to base class CompoundObject
void BreakableObject::addShapesToDeathList(void)
{
   for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it)
   {
      m_sceneActor->addMeToDeathList(*it);
   }
   
   m_sceneActor->addMeToDeathList(this);
}

void BreakableObject::atDeathOfBreakableObject(void)
{
   b2Body* myBody = ActorUserData::getBody(getUserData());

   myBody->GetWorld()->DestroyBody(myBody);
   
   this->detach();
}

void BreakableObject::spawnBreakableObjects(void)
{
   m_sceneActor->addObjectToSpawnList(
      m_numberOfSpawns,
      getCompoundObjectPosition(),
      Vector2(2.0f, 2.0f),
      m_spawnObjects);
}
