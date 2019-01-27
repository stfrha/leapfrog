#include "marker.h"
#include "physdispconvert.h"
#include "polygonvertices.h"

using namespace oxygine;
using namespace std;

// rc is in stage coordinates, i.e. meters
Marker::Marker(Resources& gameResources, const RectF& rc, b2World* world)
{
	setResAnim(gameResources.getResAnim("markers"));

   // The marker has one pixel per meter, i.e. the size of the bitmap 
   // in meters is the same as the size of the bitmap in pixels
   Vector2 mSize = getResAnim()->getSize();

	// size and position here is in stage coordinate (same as rc)
//	setSize(rc.getSize());
	setPosition(rc.getLeftTop());
//	setAnchor(Vector2(0.5f, 0.5f));

   vector<Vector2> vertices;
   vector<VectorT3<int> > triangles;
   
   vertices.push_back(Vector2(-rc.getSize().x / 2, rc.getSize().y / 2));
   vertices.push_back(Vector2(rc.getSize().x / 2, rc.getSize().y / 2));
   vertices.push_back(Vector2(rc.getSize().x / 2, -rc.getSize().y / 2));
   vertices.push_back(Vector2(-rc.getSize().x / 2, -rc.getSize().y / 2));

   triangles.push_back(VectorT3<int>(1, 3, 2));
   triangles.push_back(VectorT3<int>(1, 4, 3));

   vertexPCT2* vs = PolygonVertices::createTriangleVertices(vertices, triangles, mSize, Vector2(0.0f, 0.0f));
   setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

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

   Vector2 pos = rc.getLeftTop();

   b2BodyDef bodyDef;
   bodyDef.type = b2_staticBody;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f);
   bodyDef.position = bPos;
   b2Body* body = world->CreateBody(&bodyDef);

   b2PolygonShape polyShape;
   polyShape.Set(b2vertices, num - 1);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &polyShape;
   fixtureDef.density = 5.0f;
   fixtureDef.friction = 1.3f;

   body->CreateFixture(&fixtureDef);

   setUserData(body);

   body->SetUserData(this);
}

