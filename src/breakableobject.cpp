
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
   readBreakableObjectNode(root.child("behaviour").child("breakableObjectProperties"));
   m_spawnObjects = new SpawnObjectList();
     
   m_spawnObjects->readSpawnObjectsNode(root.child("behaviour").child("breakableObjectProperties"));

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

   // Give body a little nudge and pinch
   if (m_shapes.size() > 0)
   {
      b2Body* myBody = ActorUserData::getBody(m_shapes[0]->getUserData());

      if (myBody)
      {

         // Find magnitude of nudge and pinch by evaluating
         // the mass of the body

         float mass = myBody->GetMass();

         float maxImp = mass * 8.0f;
         float maxAng = mass * 6.0f;

         // Randomise value between 0 and 2pi
         float angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f * MATH_PI));


         // Randomise value between 0 and maxImp
         float magnitude = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / maxImp));

         b2Vec2 impulseForce = b2Vec2(magnitude * cos(angle), magnitude * sin(angle));

         myBody->ApplyLinearImpulseToCenter(impulseForce, true);

         // Randomise value between 0 and maxAng
         float angImpulse = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / maxAng));

         myBody->ApplyAngularImpulse(angImpulse * 10.0f, true);
      }
   }
}

void BreakableObject::readBreakableObjectNode(const pugi::xml_node& node)
{
   m_breakAtDamage = node.attribute("breakBulletDamage").as_int(0);
   m_numberOfSpawns = node.attribute("numberOfSpawns").as_int(0);
}

void BreakableObject::collisionBlast(b2Contact* contact, bool small)
// Sparks blastemitter, small is default, can also be big
{
   // Assume small blast
   int emitterLifetime = 150;
   int particleLifetime = 700;
   float particleDistance = 30.0f;
   float particleSize = 0.75f;
   float blastIntensity = 100.0f;

   if (!small)
   {
      emitterLifetime = 250;
      particleLifetime = 900;
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
   m_gameStatus->deltaDamage(25.0f * bulletEqvDamage);

   if (m_gameStatus->getDamage() >= m_breakAtDamage)
   {
      addMeToDeathList();

      spawnBreakableObjects();

      collisionBlast(contact, false);
   }
   else
   {
      collisionBlast(contact, true);
   }
}

void BreakableObject::doUpdate(const oxygine::UpdateState& us)
{
   //if (m_spawnCount > 0)
   //{
   //   spawnBreakableObjects();
   //}
}


void BreakableObject::spawnBreakableObjects(void)
{
   m_sceneActor->addObjectToSpawnList(
      m_numberOfSpawns,
      getCompoundObjectPosition(),
      Vector2(2.0f, 2.0f),
      m_spawnObjects);
}
