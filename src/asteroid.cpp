
#include "asteroid.h"

#include "sceneactor.h"
#include "freespaceactor.h"
#include "blastemitter.h"


using namespace std;
using namespace oxygine;
using namespace pugi;

Asteroid::Asteroid(
   Resources& gameResources,
   SceneActor* sceneActor,
   b2World* world,
   const Vector2& pos,
   const xml_node& root) :
   CompoundObject(sceneActor),
   m_num(generateNum()),
   m_bitmapPxSize(512),
   m_asteroideMaxRadius(10.0f),
   m_bitmapScale(m_bitmapPxSize / 2 / m_asteroideMaxRadius),
   m_damage(0),
   m_sceneActor(sceneActor),
   m_gameResource(&gameResources),
   m_world(world)
{




   //----------------------------------------------------------------------
   // Below is replaced by XML-defined Asteroids
   //----------------------------------------------------------------------

//
//   m_poly = new oxygine::Polygon;
//
//   //ResAnim "tiled" has only single frame and uses own separate atlas texture
//   //it should have options trim=extend=false
//   //and atlas with option clamp2edge=false to allow tiling
//   /*
//   <atlas clamp2edge="false">
//   <image file="tiled.png" trim="false" extend="false"/>
//   </atlas>
//   */
//
//   m_poly->setResAnim(gameResources.getResAnim("crater_rock"));
//
//   if (m_state == ASE_AUTO)
//   {
//      float size = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 3.0f));
//
//      if (size < 0.5f)
//      {
//         m_state = ASE_SMALL;
//      }
//      else if (size < 1.5f)
//      {
//         m_state = ASE_MIDDLE;
//      }
//      else
//      {
//         m_state = ASE_LARGE;
//      }
//   }
//
//   m_radius = generateRadius();
//
//   setPriority(160);
//
//   Vector2*  v2vertices = new Vector2[m_num];
//
//   generateVertices(v2vertices);
//
//   createPolygon(v2vertices, m_num);
//
//   m_poly->setScale(1/m_bitmapScale);
//
//   float as = m_poly->getResAnim()->getAppliedScale();
//
//   setAnchor(Vector2(0.5f, 0.5f));
//
//   addChild(m_poly);
//
//   b2BodyDef bodyDef;
//   bodyDef.type = b2_dynamicBody;
//   bodyDef.position = PhysDispConvert::convert(pos, 1.0f);
//
//   b2Body* body = world->CreateBody(&bodyDef);
//
//   setUserData(body);
//
//   b2Vec2* b2vertices = new b2Vec2[m_num + 1];
//
//   // Polygon of a body shape is physical coordinates, i.e. in meters
//   Vector2 tv;
//
//   for (int i = 0; i < m_num ; i++)
//   {
//      tv = v2vertices[m_num - i - 1];
//      b2vertices[i] = PhysDispConvert::convert(tv, m_bitmapScale);
//   }
//
//   tv = v2vertices[m_num - 1];
//   b2vertices[m_num] = PhysDispConvert::convert(tv, m_bitmapScale);
//
//   b2PolygonShape polyShape;
//
//   polyShape.Set(b2vertices, m_num+1);
//
//   b2FixtureDef fixtureDef;
//   fixtureDef.shape = &polyShape;
//   fixtureDef.density = 5.0f;
//   fixtureDef.friction = 1.3f;
//   fixtureDef.filter.categoryBits = 8;
//   fixtureDef.filter.maskBits = 64703;
//   fixtureDef.userData = (CollisionEntity*)this;
//
//   body->CreateFixture(&fixtureDef);
//   body->SetUserData(this);
//
////   body->GetFixtureList()->SetUserData((CollisionEntity*)this);
//   body->ResetMassData();

   //----------------------------------------------------------------------
   // ...until here
   //----------------------------------------------------------------------



   // Randomise value between 0 and 2pi
   float angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f * MATH_PI));

   float maxImp = 1000.0f;
   float maxAng = 1000.0f;

   switch (m_state)
   {
   case ASE_SMALL:
      maxImp = 2000.0f;
      maxAng = 1000.0f;
      break;
   case ASE_MIDDLE:
      maxImp = 5000.0f;
      maxAng = 3000.0f;
      break;
   case ASE_LARGE:
      maxImp = 10000.0f;
      maxAng = 20000.0f;
      break;
   case ASE_AUTO:
      maxImp = 10000.0f;
      maxAng = 20000.0f;
      break;
   }

   // Randomise value between 0 and maxImp
   float magnitude = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / maxImp));

   b2Vec2 impulseForce = b2Vec2(magnitude * cos(angle), magnitude * sin(angle));

//   body->ApplyLinearImpulseToCenter(impulseForce, true);

   // Randomise value between 0 and maxAng
   float angImpulse = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / maxAng));


//   body->ApplyAngularImpulse(angImpulse * 10.0f, true);

   attachTo(m_sceneActor);
}

Asteroid::~Asteroid()
{
   delete m_smallCoNodeHolder;
   delete m_mediumCoNodeHolder;
   delete m_bigCoNodeHolder;
}

void Asteroid::readAsteroidNode(const xml_node& objectNode)
{
   m_smallCoNode = objectNode.child("smallAsteroid");
   m_smallCoNodeHolder = new xml_document();
   m_smallCoNodeHolder->append_copy(m_smallCoNode);
   m_smallCoNode = m_smallCoNodeHolder->child("smallAsteroid");

   m_mediumCoNode = objectNode.child("mediumAsteroid");
   m_mediumCoNodeHolder = new xml_document();
   m_mediumCoNodeHolder->append_copy(m_mediumCoNode);
   m_mediumCoNode = m_mediumCoNodeHolder->child("mediumAsteroid");

   m_bigCoNode = objectNode.child("bigAsteroid");
   m_bigCoNodeHolder = new xml_document();
   m_bigCoNodeHolder->append_copy(m_bigCoNode);
   m_bigCoNode = m_bigCoNodeHolder->child("bigAsteroid");

   string size = objectNode.child("behaviour").attribute("size").as_string();
   if (size == "small")
   {
      m_state = ASE_SMALL;
   }
   else if (size == "medium")
   {
      m_state = ASE_MIDDLE;
   }
   else if (size == "big")
   {
      m_state = ASE_LARGE;
   }
   else
   {
      m_state = ASE_AUTO;
   }

}


CollisionEntityTypeEnum Asteroid::getEntityType(void)
{
   return CET_ASTEROID;
}

void Asteroid::addAsteroidSpawnInstruction(const AsteroidSpawnInstruction& inst)
{
   m_asteroidSpawnList.push_back(inst);
}


//void Asteroid::killActor(void)
//{
//   atDeathOfAsteroid();
//}
//
void Asteroid::hitByBullet(b2Contact* contact)
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
      addAsteroidSpawnInstruction(AsteroidSpawnInstruction(3, ASE_SMALL, spawnLeftTop, spawnRightBottom));

      shattered = true;
   }

   if ((m_state == ASE_LARGE) && (m_damage >= 4))
   {
      // Spawn three middle 
      addAsteroidSpawnInstruction(AsteroidSpawnInstruction(3, ASE_MIDDLE, spawnLeftTop, spawnRightBottom));

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

void Asteroid::hitShield(b2Contact* contact)
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


void Asteroid::hitByLepfrog(b2Contact* contact)
{
   // Take damage like two bullets
}

float Asteroid::generateVertex(Vector2& v, int i, int num)
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

float Asteroid::getRadius()
{
   return m_radius;
}

void Asteroid::generateVertices(Vector2* vertices)
{
   for (int i = 0; i < m_num; i++)
   {
      generateVertex(vertices[i], i, m_num);
   }
}

vertexPCT2 Asteroid::initVertex(const Vector2& pos, unsigned int color)
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

vertexPCT2 Asteroid::getVertex(Vector2* v2vertices, int i)
{
   Color c = Color::White;
   return initVertex(v2vertices[i], c.rgba());
}

vertexPCT2* Asteroid::createVertices(Vector2* v2vertices, int num)
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

void Asteroid::createPolygon(Vector2* v2vertices, int num)
{
   vertexPCT2* vertices = createVertices(v2vertices, num);
   m_poly->setVertices(vertices, sizeof(vertexPCT2) * num * 4, vertexPCT2::FORMAT, true);
}

float Asteroid::generateRadius(void)
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

int Asteroid::generateNum(void)
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


// We want all asteroids to display the same scale
// The png file is 512 px wide and tall. Lets use it for the 
// the biggest asteroids and scale them all the same
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

void Asteroid::doUpdate(const oxygine::UpdateState& us)
{
   spawnAsteroids();
}

void Asteroid::atDeathOfAsteroid(void)
{
   b2Body* myBody = (b2Body*)getUserData();
   
   myBody->GetWorld()->DestroyBody(myBody);
   
   this->detach();
}
//void Asteroid::atParticleDeath(oxygine::Event* event)
//{
//   // Now I'm suppose to comit suicide. How to deregister me from actor and world?
//   b2Body* myBody = (b2Body*)getUserData();
//
//   myBody->GetWorld()->DestroyBody(myBody);
//
//   this->detach();
//
//}

void Asteroid::spawnAsteroids(void)
{
   if (m_asteroidSpawnList.size() > 0)
   {
      m_sceneActor->addMeToDeathList(this);

      for (auto it = m_asteroidSpawnList.begin(); it != m_asteroidSpawnList.end(); ++it)
      {
         for (int i = 0; i < it->m_num; i++)
         {
            // Randomise position within asteroid field
            oxygine::Vector2 pos;

            pos.x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (it->m_rightBottom.x - it->m_leftTop.x))) + it->m_leftTop.x;
            pos.y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (it->m_rightBottom.y - it->m_leftTop.y))) + it->m_leftTop.y;

            spAsteroid asteroid =
               new Asteroid(
                  *m_gameResource,
                  m_sceneActor,
                  m_world,
                  pos,
                  it->m_state);
            asteroid->attachTo(m_sceneActor);

            if (m_sceneActor->getSceneType() == STE_FREE_SPACE)
            {
               FreeSpaceActor* spaceActor = (FreeSpaceActor*)m_sceneActor;

               spaceActor->addBoundingBody((b2Body*)asteroid.get()->getUserData());
            }
         }
      }

      m_asteroidSpawnList.clear();

   }

}
