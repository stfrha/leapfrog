#include "polygonvertices.h"

#include "compoundobject.h"

using namespace oxygine;
using namespace std;
using namespace pugi;


CompoundObject::CompoundObject(oxygine::Resources& gameResources, Actor* parent, b2World* world, const oxygine::Vector2& pos, std::string& defXmlFileName)
{
   readDefinitionXmlFile(gameResources, parent, world, pos, defXmlFileName);
}

CompoundObject::~CompoundObject()
{
   m_boxedSprites.clear();
   m_polygonSprites.clear();
   m_staticPolygons.clear();
   m_weldJoints.clear();
   m_revoluteJoints.clear();
   m_prismaticJoints.clear();

}

bool CompoundObject::readDefinitionXmlFile(Resources& gameResources, Actor* parent, b2World* world, const Vector2& pos, std::string& fileName)
{
   xml_document doc;

   xml_parse_result result = doc.load_file(fileName.c_str());

   xml_node root = doc.child("compoundObject");

   for (auto it = root.children("staticPolygon").begin();
      it != root.children("staticPolygon").end();
      ++it)
   {
      // define a staticObject
      defineStaticPolygon(gameResources, parent, world, pos, *it);
   }

   for (auto it = root.children("staticBox").begin();
      it != root.children("staticBox").end();
      ++it)
   {
      // define a staticObject
      defineStaticBox(gameResources, parent, world, pos, *it);
   }

   for (auto it = root.children("dynamicBox").begin();
      it != root.children("dynamicBox").end();
      ++it)
   {
      // define a box object
      defineBoxedObject(gameResources, parent, world, pos, *it);
   }

   for (auto it = root.children("dynamicPolygon").begin();
      it != root.children("dynamicPolygon").end();
      ++it)
   {
      // define a polygon object
      definePolygonObject(gameResources, parent, world, pos, *it);
   }

   return true;
}

// Remove below when new oxygine is pulled
#include "Polygon.h"

void CompoundObject::defineStaticPolygon(Resources& gameResources, Actor* parent, b2World* world, const Vector2& pos, xml_node& staticNode)
{
   vector<Vector2> vertices;
   vector<VectorT3<int> > triangles;

   spPolygon staticObject = new Polygon();
   staticObject->setName(staticNode.attribute("name").as_string());
   staticObject->setResAnim(gameResources.getResAnim(staticNode.attribute("texture").as_string()));
   Vector2 bmSize = Vector2(staticObject->getResAnim()->getWidth(), staticObject->getResAnim()->getHeight());

   for (auto it = staticNode.child("vertices").children("vertex").begin();
      it != staticNode.child("vertices").children("vertex").end();
      ++it)
   {
      vertices.push_back(Vector2(it->attribute("x").as_float(), it->attribute("y").as_float()));
   }

   for (auto it = staticNode.child("triangles").children("triangle").begin();
      it != staticNode.child("triangles").children("triangle").end();
      ++it)
   {
      triangles.push_back(VectorT3<int>(
         it->attribute("v1").as_int(),
         it->attribute("v2").as_int(),
         it->attribute("v3").as_int()));
   }

   vertexPCT2* vs = PolygonVertices::createTriangleVertices(vertices, triangles, bmSize, Vector2(0.0f, 0.0f));
   staticObject->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

   staticObject->attachTo(parent);

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
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(staticNode.attribute("posX").as_float(), staticNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   b2Body* body = world->CreateBody(&bodyDef);

   b2ChainShape chain;
   chain.CreateChain(b2vertices, num);

   body->CreateFixture(&chain, 1);

   staticObject->setUserData(body);
   
   body->SetUserData(staticObject.get());

   m_staticPolygons.push_back(staticObject);
}

void CompoundObject::defineStaticBox(Resources& gameResources, Actor* parent, b2World* world, const Vector2& pos, xml_node& staticNode)
{
   spSprite boxObject = new Sprite();
   boxObject->setName(staticNode.attribute("name").as_string());
   boxObject->setResAnim(gameResources.getResAnim(staticNode.attribute("texture").as_string()));

   boxObject->setSize(staticNode.attribute("width").as_float(), staticNode.attribute("height").as_float());
   boxObject->setAnchor(Vector2(staticNode.attribute("anchorX").as_float(), staticNode.attribute("anchorY").as_float()));
   boxObject->setTouchChildrenEnabled(false);

   boxObject->attachTo(parent);

   b2BodyDef bodyDef;
   bodyDef.type = b2_staticBody;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(staticNode.attribute("posX").as_float(), staticNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   b2Body* body = world->CreateBody(&bodyDef);

   b2PolygonShape boxShape;
   boxShape.SetAsBox(staticNode.attribute("width").as_float() / 2.0f, staticNode.attribute("height").as_float() / 2.0f);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &boxShape;
   fixtureDef.density = 5.0f;
   fixtureDef.friction = 1.3f;

   body->CreateFixture(&fixtureDef);

   boxObject->setUserData(body);
   body->SetUserData(boxObject.get());

   m_boxedSprites.push_back(boxObject);
}


void CompoundObject::defineBoxedObject(oxygine::Resources& gameResources, Actor* parent, b2World* world, const Vector2& pos, xml_node& boxObjectNode)
{
   spSprite boxObject = new Sprite();
   boxObject->setName(boxObjectNode.attribute("name").as_string());
   boxObject->setResAnim(gameResources.getResAnim(boxObjectNode.attribute("texture").as_string()));

   boxObject->setSize(boxObjectNode.attribute("width").as_float(), boxObjectNode.attribute("height").as_float());
   boxObject->setAnchor(Vector2(boxObjectNode.attribute("anchorX").as_float(), boxObjectNode.attribute("anchorY").as_float()));
   boxObject->setTouchChildrenEnabled(false);

   boxObject->attachTo(parent);

   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(boxObjectNode.attribute("posX").as_float(), boxObjectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   b2Body* body = world->CreateBody(&bodyDef);

   b2PolygonShape boxShape;
   boxShape.SetAsBox(boxObjectNode.attribute("width").as_float() / 2.0f, boxObjectNode.attribute("height").as_float() / 2.0f);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &boxShape;
   fixtureDef.density = 5.0f;
   fixtureDef.friction = 1.3f;

   body->CreateFixture(&fixtureDef);

   boxObject->setUserData(body);
   body->SetUserData(boxObject.get());

   m_boxedSprites.push_back(boxObject);
}

void CompoundObject::definePolygonObject(oxygine::Resources& gameResources, Actor* parent, b2World* world, const Vector2& pos, xml_node& polygonObjectNode)
{
   vector<Vector2> vertices;
   vector<VectorT3<int> > triangles;

   spPolygon polygonObject = new Polygon();
   polygonObject->setName(polygonObjectNode.attribute("name").as_string());
   polygonObject->setResAnim(gameResources.getResAnim(polygonObjectNode.attribute("texture").as_string()));

   Vector2 mSize = Vector2(polygonObjectNode.attribute("width").as_float(), polygonObjectNode.attribute("height").as_float());

   //polygonObject->setSize(polygonObjectNode.attribute("width").as_float(), polygonObjectNode.attribute("height").as_float());
   //polygonObject->setAnchor(Vector2(polygonObjectNode.attribute("anchorX").as_float(), polygonObjectNode.attribute("anchorY").as_float()));
   polygonObject->setTouchChildrenEnabled(false);

   for (auto it = polygonObjectNode.child("vertices").children("vertex").begin();
      it != polygonObjectNode.child("vertices").children("vertex").end();
      ++it)
   {
      vertices.push_back(Vector2(it->attribute("x").as_float(), it->attribute("y").as_float()));
   }

   for (auto it = polygonObjectNode.child("triangles").children("triangle").begin();
      it != polygonObjectNode.child("triangles").children("triangle").end();
      ++it)
   {
      triangles.push_back(VectorT3<int>(
         it->attribute("v1").as_int(),
         it->attribute("v2").as_int(),
         it->attribute("v3").as_int()));
   }

   vertexPCT2* vs = PolygonVertices::createTriangleVertices(vertices, triangles, mSize, Vector2(0.0f, 0.0f));
   polygonObject->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

   polygonObject->attachTo(parent);

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
   bodyDef.type = b2_dynamicBody;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(polygonObjectNode.attribute("posX").as_float(), polygonObjectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   b2Body* body = world->CreateBody(&bodyDef);

   b2PolygonShape polyShape;
   polyShape.Set(b2vertices, num - 1);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &polyShape;
   fixtureDef.density = 5.0f;
   fixtureDef.friction = 1.3f;

   body->CreateFixture(&fixtureDef);

   polygonObject->setUserData(body);

   body->SetUserData(polygonObject.get());

   m_polygonSprites.push_back(polygonObject);
}

