
#include "breakableobject.h"

#include "sceneactor.h"
#include "freespaceactor.h"
#include "blastemitter.h"


using namespace oxygine;

BreakableObject::BreakableObject(
   oxygine::Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const oxygine::Vector2& pos,
   pugi::xml_node& root) :
   CompoundObject(sceneParent),
   m_damage(0),
   m_sceneActor(sceneParent),
   m_gameResource(&gameResources),
   m_world(world)
{
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


// Fortsätt här!!!

CollisionEntityTypeEnum BreakableObject::getEntityType(void)
{
   return CET_BreakableObject;
}

void BreakableObject::addBreakableObjectSpawnInstruction(const BreakableObjectSpawnInstruction& inst)
{
   m_BreakableObjectSpawnList.push_back(inst);
}


//void BreakableObject::killActor(void)
//{
//   atDeathOfBreakableObject();
//}
//
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

   if (m_state == ASE_SMALL && m_damage >= 1)
   {
      m_sceneActor->addMeToDeathList(this);
      shattered = true;
   }

   b2Body* b = (b2Body*)getUserData();
   b2Vec2 pos = b->GetPosition();
   b2Vec2 spawnLeftTop = pos + b2Vec2(-1.0f, -1.0f);
   b2Vec2 spawnRightBottom = pos + b2Vec2(1.0f, 1.0f);

   if ((m_state == ASE_MIDDLE) && (m_damage >= 2))
   {
      // Spawn three small
      addBreakableObjectSpawnInstruction(BreakableObjectSpawnInstruction(3, ASE_SMALL, spawnLeftTop, spawnRightBottom));

      shattered = true;
   }

   if ((m_state == ASE_LARGE) && (m_damage >= 4))
   {
      // Spawn three middle 
      addBreakableObjectSpawnInstruction(BreakableObjectSpawnInstruction(3, ASE_MIDDLE, spawnLeftTop, spawnRightBottom));

      shattered = true;
   }

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

float BreakableObject::generateVertex(Vector2& v, int i, int num)
{
   float thetaNom = 2.0f * MATH_PI / num * i;

   // Randomise value between -2pi / num / 2 and 2pi / num / 2 
   float thetaBound = /*2.0f * */ MATH_PI / (float)num /* / 2.0f */;
   float thetaNoise = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / thetaBound)) - thetaBound;

   float theta = thetaNom + thetaNoise;

   v = Vector2(m_radius * m_bitmapScale * cos(theta),
      m_radius * m_bitmapScale * sin(theta));

   //   float rad = getStage()->getHeight() * 0.4f * (1 + scalar::sin(theta * 10) / 10);

   return theta;
}

float BreakableObject::getRadius()
{
   return m_radius;
}

void BreakableObject::generateVertices(Vector2* vertices)
{
   for (int i = 0; i < m_num; i++)
   {
      generateVertex(vertices[i], i, m_num);
   }
}

vertexPCT2 BreakableObject::initVertex(const Vector2& pos, unsigned int color)
{
   vertexPCT2 v;
   v.color = color;
   v.x = pos.x;
   v.y = pos.y;
   v.z = 0;
   v.u = v.x / m_bitmapPxSize;
   v.v = v.y / m_bitmapPxSize;

   return v;
}

vertexPCT2 BreakableObject::getVertex(Vector2* v2vertices, int i)
{
   Color c = Color::White;
   return initVertex(v2vertices[i], c.rgba());
}

vertexPCT2* BreakableObject::createVertices(Vector2* v2vertices, int num)
{
   int verticesCount = num * 4;

   vertexPCT2* vertices = new vertexPCT2[verticesCount];

   vertexPCT2* p = vertices;
   for (int n = 0; n < num; ++n)
   {
      int m = n + 1;

      // Handle wrap to first vertex
      if (m == num)
      {
         m = 0;
      }

      //add centered vertex
      *p = initVertex(Vector2(0, 0), Color::White);
      ++p;

      *p = getVertex(v2vertices, n);
      ++p;

      *p = getVertex(v2vertices, m);
      ++p;

      //Oxygine uses "triangles strip" rendering mode
      //dublicate last vertex (degenerate triangles)
      *p = getVertex(v2vertices, m);
      ++p;
   }

   return vertices;
}

void BreakableObject::createPolygon(Vector2* v2vertices, int num)
{
   vertexPCT2* vertices = createVertices(v2vertices, num);
   m_poly->setVertices(vertices, sizeof(vertexPCT2) * num * 4, vertexPCT2::FORMAT, true);
}

float BreakableObject::generateRadius(void)
{
   switch (m_state)
   {
   case ASE_SMALL:
      return 3.0f;
   case ASE_MIDDLE:
      return 5.0f;
   case ASE_LARGE:
      return 10.0f;
   case ASE_AUTO:
       return 10.0f;
   }

   return 0;
}

int BreakableObject::generateNum(void)
{
   switch (m_state)
   {
   case ASE_SMALL:
      return  4;
   case ASE_MIDDLE:
      return 5;
   case ASE_LARGE:
      return 5;
   case ASE_AUTO:
       return 5;
   }

   return 4;
}


// We want all BreakableObjects to display the same scale
// The png file is 512 px wide and tall. Lets use it for the 
// the biggest BreakableObjects and scale them all the same
// Now, to make this work we need the following values for
// a polygon radius:
// Size           Sprite radius        Body radius [m]
// ASE_SMALL      25.6                 2.0
// ASE_MIDDLE     64                   5.0
// ASE_LARGE      128                  10.0
// The scale factor is 12.8 and is used when we convert
// the sprite vertices to the bodie's. We use the same 
// scale factor to apply a scale to the sprite to get the
// sprite to the correct showing size.

void BreakableObject::doUpdate(const oxygine::UpdateState& us)
{
   spawnBreakableObjects();
}

void BreakableObject::atDeathOfBreakableObject(void)
{
   b2Body* myBody = (b2Body*)getUserData();
   
   myBody->GetWorld()->DestroyBody(myBody);
   
   this->detach();
}
//void BreakableObject::atParticleDeath(oxygine::Event* event)
//{
//   // Now I'm suppose to comit suicide. How to deregister me from actor and world?
//   b2Body* myBody = (b2Body*)getUserData();
//
//   myBody->GetWorld()->DestroyBody(myBody);
//
//   this->detach();
//
//}

void BreakableObject::spawnBreakableObjects(void)
{
   if (m_BreakableObjectSpawnList.size() > 0)
   {
      m_sceneActor->addMeToDeathList(this);

      for (auto it = m_BreakableObjectSpawnList.begin(); it != m_BreakableObjectSpawnList.end(); ++it)
      {
         for (int i = 0; i < it->m_num; i++)
         {
            // Randomise position within BreakableObject field
            oxygine::Vector2 pos;

            pos.x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (it->m_rightBottom.x - it->m_leftTop.x))) + it->m_leftTop.x;
            pos.y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (it->m_rightBottom.y - it->m_leftTop.y))) + it->m_leftTop.y;

            spBreakableObject BreakableObject =
               new BreakableObject(
                  *m_gameResource,
                  m_sceneActor,
                  m_world,
                  pos,
                  it->m_state);
            BreakableObject->attachTo(m_sceneActor);

            if (m_sceneActor->getSceneType() == STE_FREE_SPACE)
            {
               FreeSpaceActor* spaceActor = (FreeSpaceActor*)m_sceneActor;

               spaceActor->addBoundingBody((b2Body*)BreakableObject.get()->getUserData());
            }
         }
      }

      m_BreakableObjectSpawnList.clear();

   }

}
