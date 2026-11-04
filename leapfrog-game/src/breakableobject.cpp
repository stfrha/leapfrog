
#include "breakableobject.h"
#include "breakableobjectevents.h"

#include "sceneactor.h"
#include "freespaceactor.h"
#include "blastemitter.h"
#include "actoruserdata.h"


using namespace oxygine;
using namespace pugi;

BreakableObject::BreakableObject(
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& root,
   int groupIndex) :
   CompoundObject(sceneParent, parentObject),
   m_sceneActor(sceneParent),
   m_world(world)
{
   readBreakableObjectNode(root.child("behaviour").child("breakableObjectProperties"));
   m_spawnObjects = new SpawnObjectList();
     
   m_spawnObjects->readSpawnObjectsNode(root.child("behaviour").child("breakableObjectProperties"));

   initCompoundObjectParts(
      sceneParent,
      sceneParent,
      parentObject,
      world,
      pos,
      root,
      groupIndex);

   attachTo(m_sceneActor);

   // This CompoundObject is also an actor who normally has
   // a userData that points to its parent. However, the parent
   // of a CompoundObject is pointed by its m_parentObject 
   // member. The userData for this object should thus
   // be empty (=NULL)
   setUserData(NULL);

   if (m_sceneActor->getSceneType() == SceneActor::SceneTypeEnum::deepSpace)
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
         myBody->SetLinearDamping(m_linearDamping);
         myBody->SetAngularDamping(m_angularDamping);
      }

      if (myBody && !m_still)
      {

         // Find magnitude of nudge and pinch by evaluating
         // the mass of the body

         float mass = myBody->GetMass();

         // Rocks fly as fast as the scene says (rockSpeedMin and rockSpeedMax
         // on sceneProperties); a calm object just gets the small nudge
         float minSpeed = m_calm ? 0.0f : m_sceneActor->getRockSpeedMin();
         float maxSpeed = m_calm ? 8.0f : m_sceneActor->getRockSpeedMax();
         float maxAng = mass * 6.0f;

         // Randomise value between 0 and 2pi
         // A direction all the way round. (This was RAND_MAX / 2.0f * MATH_PI,
         // which only gave 0 to 37 degrees, so everything drifted the same way.)
         float angle = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * 2.0f * MATH_PI;


         // Randomise a speed between minSpeed and maxSpeed
         float speed = minSpeed + (maxSpeed - minSpeed) * (static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
         float magnitude = mass * speed;

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
   m_calm = node.attribute("calm").as_bool(false);
   m_still = node.attribute("still").as_bool(false);
   m_linearDamping = node.attribute("linearDamping").as_float(0.0f);
   m_angularDamping = node.attribute("angularDamping").as_float(0.0f);
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
   if (m_isDead)
   {
      return;
   }

   bool shattered = false;

   // Take damage
   m_resources->deltaDamage(bulletEqvDamage);

   if (m_resources->getDamage() >= m_breakAtDamage)
   {
      m_isDead = true;
      addMeToDeathList();

      spawnBreakableObjects();

      BreakableObjectBreakEvent event;
      dispatchEvent(&event);

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
   if (m_numberOfSpawns == 1)
   {
      m_sceneActor->addObjectToSpawnList(
         m_numberOfSpawns,
         getCompoundObjectPosition(),
         Vector2(2.0f, 2.0f),
         m_spawnObjects);

   }
   else
   {
      Vector2 center = getCompoundObjectPosition();
      float radius = m_shapes[0]->getWidth() / 4;

      // The pieces fly apart: evenly round the circle, turned at random,
      // each a little off its even share
      float circle = 2.0f * MATH_PI;
      float share = circle / m_numberOfSpawns;
      float turn = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX)) * circle;

      for (int i = 0; i < m_numberOfSpawns; i++)
      {
         float offset = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f) * 0.5f * share;
         float angle = turn + i * share + offset;
         Vector2 rayDir(sinf(angle), cosf(angle));
         Vector2 spawnPos = rayDir * radius + center;

         m_sceneActor->addObjectToSpawnList(
            1,
            spawnPos,
            Vector2(2.0f, 2.0f),
            m_spawnObjects,
            NULL,
            rayDir);
      }
   }

   BreakableObjectNewObjectSpawnedEvent event;
   dispatchEvent(&event);

}

void BreakableObject::initObjectResources(Actor* statusEventOriginator, ObjectResources* resources)
{
   m_resources = new ObjectResources();

   ObjectProperty* damage = new ObjectProperty(this, NULL, 0, 0.0f);

   m_resources->initObjectResources(statusEventOriginator, NULL, NULL, NULL, NULL, damage);

   m_properties.push_back(*damage);

   m_resources->registerShieldObject(NULL);
}
