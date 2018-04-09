#include "polygonvertices.h"
#include "staticpolygon.h"
#include "Polygon.h"
#include "pugixml\pugixml.hpp"

using namespace oxygine;
using namespace std;
using namespace pugi;

StaticPolygon::StaticPolygon(
   Resources& gameResources,
   string fileName,
   b2World* world) :
   m_bitmapPxSize(512.0f),
   m_bitmapScale(1.0f),
   m_gameResource(&gameResources)
{

   // Read map file
   loadPolygonFile(fileName);


   m_poly->setScale(1/m_bitmapScale);

   float as = m_poly->getResAnim()->getAppliedScale();

   setAnchor(Vector2(0.5f, 0.5f));

   addChild(m_poly);

   int num = m_vertices.size() + 1;

   b2Vec2* b2vertices = new b2Vec2[num];

   // Polygon of a body shape is physical coordinates, i.e. in meters
   Vector2 tv;

   for (int i = 0; i < num-1 ; i++)
   {
      tv = m_vertices[i];
      b2vertices[i] = PhysDispConvert::convert(tv, m_bitmapScale);
   }

   tv = m_vertices[0];
   b2vertices[num-1] = PhysDispConvert::convert(tv, m_bitmapScale);

   b2BodyDef bodyDef;
   bodyDef.type = b2_staticBody;
   bodyDef.position = b2Vec2(0.0f, 0.0f);
   b2Body* body = world->CreateBody(&bodyDef);

   b2ChainShape chain;
   chain.CreateChain(b2vertices, num);

   body->CreateFixture(&chain, 1);

   setUserData(body);


   body->SetUserData(this);
}


void StaticPolygon::createPolygon(void)
{
   vertexPCT2* vertices = PolygonVertices::createTriangleVertices();
   m_poly->setVertices(vertices, sizeof(vertexPCT2) *  m_triangles.size() * 4, vertexPCT2::FORMAT, true);
}

// We want all StaticPolygons to display the same scale
// The png file is 512 px wide and tall. Lets use it for the 
// the biggest StaticPolygons and scale them all the same
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

//void StaticPolygon::doUpdate(const oxygine::UpdateState& us)
//{
//}




void StaticPolygon::loadPolygonFile(Resources& gameResources, Actor* parent, b2World* world, const Vector2& pos, string fileName)
{
   vector<Vector2> vertices;
   vector<VectorT3<int> > triangles;

   m_poly = new Polygon;

   m_poly->setResAnim(gameResources.getResAnim(m_textureName));


   xml_document doc;

   xml_parse_result result = doc.load_file(fileName.c_str());

   //if (result != status_ok)
   //{
   //   // Handle error
   //   return;
   //}
   xml_node root = doc.child("landingMap");
   xml_node map = root.child("staticObject");
   m_textureName = map.attribute("texture").as_string();

   for (auto it = map.child("vertices").children("vertex").begin();
      it != map.child("vertices").children("vertex").end();
      ++it)
   {
      vertices.push_back(Vector2(it->attribute("x").as_float(), it->attribute("y").as_float()));
   }

   for (auto it = map.child("triangles").children("triangle").begin();
      it != map.child("triangles").children("triangle").end();
      ++it)
   {
      triangles.push_back(VectorT3<int>(
         it->attribute("v1").as_int(), 
         it->attribute("v2").as_int(),
         it->attribute("v3").as_int()));
   }

   vertexPCT2* vs = PolygonVertices::createTriangleVertices(vertices, triangles, bmSize, Vector2(0.0f, 0.0f));
   m_poly->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

   m_poly->attachTo(parent);

   int num = vertices.size() + 1;

   b2Vec2* b2vertices = new b2Vec2[num];

   // Polygon of a body shape is physical coordinates, i.e. in meters
   Vector2 tv;

   for (int i = 0; i < num - 1; i++)
   {
      tv = vertices[i];
      b2vertices[i] = PhysDispConvert::convert(tv, 1.0f);
   }

   tv = vertices[0];
   b2vertices[num - 1] = PhysDispConvert::convert(tv, 1.0f);

   b2BodyDef bodyDef;
   bodyDef.type = b2_staticBody;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f)/* + b2Vec2(staticNode.attribute("posX").as_float(), staticNode.attribute("posY").as_float())*/;
   bodyDef.position = bPos;
   b2Body* body = world->CreateBody(&bodyDef);

   b2ChainShape chain;
   chain.CreateChain(b2vertices, num);

   body->CreateFixture(&chain, 1);

   staticObject->setUserData(body);

   body->SetUserData(staticObject.get());

   m_staticPolygons.push_back(staticObject);


}