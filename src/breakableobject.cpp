
#include "breakableobject.h"

#include "sceneactor.h"
#include "freespaceactor.h"
#include "blastemitter.h"


using namespace oxygine;
using namespace pugi;

BreakableObject::BreakableObject(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& root) :
   CompoundObject(sceneParent, parentObject),
   m_gameResource(&gameResources),
   m_sceneActor(sceneParent),
   m_world(world),
   m_damage(0),
   m_spawnCount(0)
{
   readBreakableObjectNode(root);

   m_spawnObjects.readSpawnObjectsNode(root);

   initCompoundObjectParts(
      gameResources,
      sceneParent,
      parentObject,
      world,
      pos,
      root,
      "");

   attachTo(m_sceneActor);
}

void BreakableObject::readBreakableObjectNode(const pugi::xml_node& node)
{
   m_breakAtDamage = node.child("behaviour").attribute("breakBulletDamage").as_int(0);
   m_numberOfSpawns = node.child("behaviour").attribute("numberOfSpawns").as_int(0);;
}

CollisionEntityTypeEnum BreakableObject::getEntityType(void)
{
   return CET_BREAKABLE_OBJECT;
}

void BreakableObject::hitByBullet(b2Contact* contact)
{
   // Assume unshattered blast
   int emitterLifetime = 150;
   int particleLifetime = 500;
   float particleDistance = 30.0f;
   float particleSize = 0.75f;
   float blastIntensity = 200.0f;

   bool shattered = false;

   // Take damage
   m_damage += 1;

   if (m_damage >= m_breakAtDamage)
   {
      m_sceneActor->addMeToDeathList(this);
      shattered = true;
      m_spawnCount += m_numberOfSpawns;
   }

   b2Vec2 pos = getCompoundObjectPosition();

   if (shattered)
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

void BreakableObject::hitShield(b2Contact* contact)
{
   int emitterLifetime = 250;
   int particleLifetime = 350;
   float particleDistance = 40.0f;
   float particleSize = 0.9f;

   b2WorldManifold m;
   contact->GetWorldManifold(&m);

   spBlastEmitter blast = new BlastEmitter(
      m_sceneActor->getResources(),
      PhysDispConvert::convert(m.points[0], 1.0f),
      150.0f,                                             // Intensity, particles / sec
      emitterLifetime,                                    // Emitter Lifetime
      particleLifetime,                                   // Particle lifetime
      particleDistance,                                   // Particle distance
      particleSize);                                      // Particle spawn size
   blast->attachTo(m_sceneActor);
}


void BreakableObject::hitByLepfrog(b2Contact* contact)
{
   // Take damage like two bullets
}

void BreakableObject::doUpdate(const oxygine::UpdateState& us)
{
   if (m_spawnCount > 0)
   {
      spawnBreakableObjects();
   }
}

void BreakableObject::atDeathOfBreakableObject(void)
{
   b2Body* myBody = (b2Body*)getUserData();
   
   myBody->GetWorld()->DestroyBody(myBody);
   
   this->detach();
}

void BreakableObject::spawnBreakableObjects(void)
{
   for (int i = 0; i < m_spawnCount; i++)
   {
      xml_node* spawnNode = m_spawnObjects.getSpawnObjectNode();
      
      defineChildObject(
         *m_gameResource, 
         m_sceneActor, 
         m_parentObject, 
         m_world, 
         PhysDispConvert::convert(getCompoundObjectPosition(), 1.0f), 
         *spawnNode, 
         "");

   }

   m_spawnCount = 0;
}