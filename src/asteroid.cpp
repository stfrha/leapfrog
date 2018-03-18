#include "asteroid.h"

using namespace oxygine;

Asteroid::Asteroid(
   Resources& gameResources,
   b2World* world,
   const b2Vec2& pos,
   AsteroidStateEnum state) :
   m_state(state),
   m_radius(generateRadius()),
   m_num(generateNum()),
   m_bitmapPxSize(512),
   m_asteroideMaxRadius(10.0f),
   m_bitmapScale(m_bitmapPxSize / 2 / m_asteroideMaxRadius)
{
   m_poly = new Polygon;

   //ResAnim "tiled" has only single frame and uses own separate atlas texture
   //it should have options trim=extend=false
   //and atlas with option clamp2edge=false to allow tiling
   /*
   <atlas clamp2edge="false">
   <image file="tiled.png" trim="false" extend="false"/>
   </atlas>
   */

   m_poly->setResAnim(gameResources.getResAnim("crater_rock"));

   createPolygon();

   m_poly->setScale(1/m_bitmapScale);

   float as = m_poly->getResAnim()->getAppliedScale();

   setAnchor(Vector2(0.5f, 0.5f));

   addChild(m_poly);

   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   bodyDef.position = pos;

   b2Body* body = world->CreateBody(&bodyDef);

   setUserData(body);

   b2Vec2* vertices = new b2Vec2[m_num + 1];

   // Polygon of a body shape is physical coordinates, i.e. in meters
   Vector2 tv;

   for (int i = 0; i < m_num ; i++)
   {
      generateVertex(tv, m_num - i - 1, m_num);
      vertices[i] = PhysDispConvert::convert(tv, m_bitmapScale);

      //vertices[i].Set(-0.6, 0.5);
      //vertices[3].Set(-0.6, -0.5);
      //vertices[2].Set(0.6, -0.5);
      //vertices[1].Set(0.6, 0.5);
      //vertices[0].Set(-0.6, 0.5);
   }

   generateVertex(tv, m_num - 1, m_num);
   vertices[m_num] = PhysDispConvert::convert(tv, m_bitmapScale);

   b2PolygonShape polyShape;

   polyShape.Set(vertices, m_num+1);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &polyShape;
   fixtureDef.density = 5.0f;
   fixtureDef.friction = 1.3f;
   
   body->CreateFixture(&fixtureDef);
   body->SetUserData(this);

   body->ResetMassData();

//   body->ApplyLinearImpulse(impulseForce, b2Vec2(0.5, 0.5), true);
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


float Asteroid::generateVertex(Vector2& v, int i, int num)
{
   float theta = 2.0f * MATH_PI / num * i;

   v = Vector2(m_radius * m_bitmapScale * cos(theta), 
      m_radius * m_bitmapScale * sin(theta));
   
   //   float rad = getStage()->getHeight() * 0.4f * (1 + scalar::sin(theta * 10) / 10);

   return theta;
}


vertexPCT2 Asteroid::getVertex(int i, int num)
{
   Vector2 p;
   
   float theta = generateVertex(p, i, num);

   Color c = Color::White;
   return initVertex(p, c.rgba());
}

vertexPCT2* Asteroid::createVertices(int num)
{
   int verticesCount = num * 4;

   vertexPCT2* vertices = new vertexPCT2[verticesCount];

   vertexPCT2* p = vertices;
   for (int n = 0; n < num; ++n)
   {
      //add centered vertex
      *p = initVertex(Vector2(0, 0), Color::White);
      ++p;

      *p = getVertex(n, num);
      ++p;

      *p = getVertex(n + 1, num);
      ++p;

      //Oxygine uses "triangles strip" rendering mode
      //dublicate last vertex (degenerate triangles)
      *p = getVertex(n + 1, num);
      ++p;
   }

   return vertices;
}

void Asteroid::createPolygon(void)
{
   vertexPCT2* vertices = createVertices(m_num);
   m_poly->setVertices(vertices, sizeof(vertexPCT2) * m_num * 4, vertexPCT2::FORMAT, true);
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
   }

   return 0;
}

int Asteroid::generateNum(void)
{
   switch (m_state)
   {
   case ASE_SMALL:
      return  5;
   case ASE_MIDDLE:
      return 6;
   case ASE_LARGE:
      return 7;
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