#include <algorithm>

#include "Box2D/Box2D.h"

#include "compoundobject.h"

#include "sceneactor.h"

// All behaviours of CompoundObjects
// and thus can be instansiated as children to one
// CompoundObject
#include "launchsite.h"
#include "leapfrog.h"
#include "asteroid.h"
#include "landingpad.h"
#include "planetactor.h"
#include "orbitwindow.h"
#include "blastemitter.h"
#include "system.h"

#include "polygonvertices.h"

using namespace oxygine;
using namespace std;
using namespace pugi;


CompoundObject::CompoundObject(SceneActor* sceneActor) :
   m_sceneActor(sceneActor),
   m_collisionType(CET_NOT_APPLICABLE)
{ }


CompoundObject::~CompoundObject()
{
	//m_boxedSprites.clear();
	//m_polygonSprites.clear();
	//m_staticPolygons.clear();
	m_namedJoints.clear();
	m_children.clear();
}

void CompoundObject::initCompoundObjectTop(
   oxygine::Resources& gameResources, 
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   const string& defXmlFileName,
   const string& initialState)
{
   readDefinitionXmlFile(gameResources, m_sceneActor, parentObject, world, pos, defXmlFileName, initialState);
}

CollisionEntityTypeEnum CompoundObject::getEntityType(void)
{
   return m_collisionType;
}

void CompoundObject::hitByBullet(b2Contact* contact)
{
   // Assume unshattered blast
   int emitterLifetime = 150;
   int particleLifetime = 500;
   float particleDistance = 30.0f;
   float particleSize = 0.75f;
   float blastIntensity = 200.0f;
   
   // Take damage
//   m_damage += 1;
   
   //if (m_damage >= 4)
   //{
      emitterLifetime = 250;
      particleLifetime = 500;
      particleDistance = 60.0f;
      particleSize = 0.9f;
      blastIntensity = 300.0f;
   
      m_sceneActor->addMeToDeathList(this);
   //}
   
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

//void CompoundObject::killActor(void)
//{
//   b2Body* myBody = (b2Body*)getUserData();
//
//   myBody->GetWorld()->DestroyBody(myBody);
//
//   this->detach();
//}
//
//

Sprite* CompoundObject::getSprite(void)
{
   return static_cast<Sprite*>(getFirstChild().get());
}

CompoundObject* CompoundObject::getParentObject(void)
{
   return m_parentObject;
}

CompoundObject* CompoundObject::readDefinitionXmlFile(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   const string& fileName,
   const string& initialState)
{
   m_parentObject = parentObject;

   ox::file::buffer bf;
   ox::file::read(fileName.c_str(), bf);

   xml_document doc;

//   xml_parse_result result = doc.load_file(fileName.c_str());
   xml_parse_result result = doc.load_buffer(&bf.data[0], bf.size());

   xml_node root = doc.child("compoundObject");

   return initCompoundObject(gameResources, sceneParent, parentObject, world, pos, root, initialState);
}

CompoundObject* CompoundObject::initCompoundObject(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& root,
   const string& initialState)
{
   // Look if there is a behaviour attached to the compound object
   string behavStr = root.child("behaviour").attribute("type").as_string();

   // Decode the type string to create the correct type of object
   // but only store the pointer to the CompoundObject
   // Perhaps with some special cases

   if (behavStr == "leapfrog")
   {
      LeapFrog* lf = new LeapFrog(
         gameResources,
         sceneParent,
         parentObject,
         world,
         pos,
         root);

      return static_cast<CompoundObject*>(lf);
   }
   else if (behavStr == "launchSite")
   {
      LaunchSite* ls = new LaunchSite(
         gameResources,
         sceneParent,
         parentObject,
         world,
         pos,
         root);

      return static_cast<CompoundObject*>(ls);
   }
   else if (behavStr == "landingPad")
   {
      LandingPad* lp = new LandingPad(
         gameResources,
         sceneParent,
         parentObject,
         world,
         pos,
         root);
   
      return static_cast<CompoundObject*>(lp);
   }
   else if (behavStr == "asteroid")
   {
      Asteroid* lp = new Asteroid(gameResources, (SceneActor*)sceneParent, world, pos, ASE_AUTO);

      return static_cast<CompoundObject*>(lp);
   }
   else
   {
      // If no behaviour was recognised, define the parts of the 
      // compound objects
      initCompoundObjectParts(
         gameResources,
         sceneParent,
         parentObject,
         world,
         pos,
         root,
         string(""));

      return this;
   }

   return NULL;
}


bool CompoundObject::initCompoundObjectParts(
   oxygine::Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   pugi::xml_node& root,
   const string& initialState)
{
   for (auto it = root.children("childObject").begin();
      it != root.children("childObject").end();
      ++it)
   {
      defineChildObject(gameResources, sceneParent, this, world, pos, *it, initialState);
   }


   //for (auto it = root.children("ChildCompoundObjectRef").begin();
   //   it != root.children("ChildCompoundObjectRef").end();
   //   ++it)
   //{
   //   defineChildObject(gameResources, sceneParent, this, world, pos, *it, initialState);
   //}

   //for (auto it = root.children("CompoundObject").begin();
   //   it != root.children("CompoundObject").end();
   //   ++it)
   //{
   //   CompoundObject* co = CompoundObject::initCompoundObject(gameResources, sceneParent, this, world, pos, *it, initialState);

   //   m_children.push_back(co);

   //   co->setName(it->attribute("name").as_string());
   //}

   for (auto it = root.children("spriteBox").begin();
      it != root.children("spriteBox").end();
      ++it)
   {
      // define a object
      defineSpriteBox(gameResources, sceneParent, this, pos, *it);
   }

   for (auto it = root.children("spritePolygon").begin();
      it != root.children("spritePolygon").end();
      ++it)
   {
      // define a object
      defineSpritePolygon(gameResources, sceneParent, this, pos, *it);
   }

   for (auto it = root.children("staticCircle").begin();
      it != root.children("staticCircle").end();
      ++it)
   {
      // define a object
      defineStaticCircle(gameResources, sceneParent, this, world, pos, *it);
   }

   for (auto it = root.children("staticBox").begin();
      it != root.children("staticBox").end();
      ++it)
   {
      // define a object
      defineStaticBox(gameResources, sceneParent, this, world, pos, *it);
   }

   for (auto it = root.children("staticPolygon").begin();
      it != root.children("staticPolygon").end();
      ++it)
   {
      // define a object
      defineStaticPolygon(gameResources, sceneParent, this, world, pos, *it);
   }

   for (auto it = root.children("staticBoxedSpritePolygonBody").begin();
      it != root.children("staticBoxedSpritePolygon").end();
      ++it)
   {
      defineStaticBoxedSpritePolygon(gameResources, sceneParent, this, world, pos, *it);
   }

   for (auto it = root.children("dynamicCircle").begin();
      it != root.children("dynamicCircle").end();
      ++it)
   {
      // define a box object
      defineDynamicCircle(gameResources, sceneParent, this, world, pos, *it);
   }

   for (auto it = root.children("dynamicBox").begin();
      it != root.children("dynamicBox").end();
      ++it)
   {
      // define a box object
      defineDynamicBox(gameResources, sceneParent, this, world, pos, *it);
   }

   for (auto it = root.children("dynamicPolygon").begin();
      it != root.children("dynamicPolygon").end();
      ++it)
   {
      // define a polygon object
      string a = (*it).attribute("name").as_string();
      defineDynamicPolygon(gameResources, sceneParent, this, world, pos, *it);
   }

   for (auto it = root.children("dynamicBoxedSpritePolygonBody").begin();
      it != root.children("dynamicBoxedSpritePolygonBody").end();
      ++it)
   {
      defineDynamicBoxedSpritePolygon(gameResources, sceneParent, this, world, pos, *it);
   }

   for (auto it = root.children("rope").begin();
      it != root.children("rope").end();
      ++it)
   {
      defineRope(gameResources, sceneParent, this, world, pos, *it);
   }

   for (auto it = root.children("weldJoint").begin();
      it != root.children("weldJoint").end();
      ++it)
   {
      defineWeldJoint(world, *it);
   }

   for (auto it = root.children("revoluteJoint").begin();
      it != root.children("revoluteJoint").end();
      ++it)
   {
      defineRevoluteJoint(world, *it);
   }

   for (auto it = root.children("prismaticJoint").begin();
      it != root.children("prismaticJoint").end();
      ++it)
   {
      definePrismaticJoint(world, *it);
   }

   // It is important that systems are iterated after all 
   // bodies and joints since, during the initialisation of the system
   // references to such objects are searchd for. 
   for (auto it = root.children("system").begin();
      it != root.children("system").end();
      ++it)
   {
      string systemType = it->attribute("type").as_string();
      string systemName = it->attribute("name").as_string();

      xml_node stateNode;

      if (getStateNode(*it, initialState, stateNode))
      {
         System* sys = System::initialiseSystemNode(&gameResources, m_sceneActor, world, this, systemType, systemName, stateNode);

         // Is it really important to remember the systems here?
         m_systems.push_back(sys);
      }
   }


   for (auto it = root.children("planetActor").begin();
      it != root.children("planetActor").end();
      ++it)
   {
      xml_node stateNode;
      
      if (getStateNode(*it, initialState, stateNode))
      {
         PlanetActor* pa = new PlanetActor(
            gameResources,
            sceneParent,
            parentObject,
            stateNode.child("properties"));

         m_children.push_back(static_cast<CompoundObject*>(pa));

         pa->setName(it->attribute("name").as_string());
      }
   }

   // Asteroid fields are replaced by Object Factories
   //for (auto it = root.children("asteroidField").begin();
   //   it != root.children("asteroidField").end();
   //   ++it)
   //{
   //   xml_node stateNode;

   //   if (getStateNode(*it, initialState, stateNode))
   //   {
   //      AsteroidField* af = new AsteroidField(
   //         gameResources,
   //         sceneParent,
   //         parentObject,
   //         world,
   //         stateNode.child("properties"));

   //      m_children.push_back(static_cast<CompoundObject*>(af));

   //      af->setName(it->attribute("name").as_string());
   //   }
   //}

   for (auto it = root.children("clippedWindow").begin();
      it != root.children("clippedWindow").end();
      ++it)
   {
      xml_node stateNode;

      if (getStateNode(*it, initialState, stateNode))
      {
         OrbitWindow* ow = new OrbitWindow(
            gameResources,
            sceneParent,
            parentObject,
            stateNode.child("properties"),
            initialState);

         m_children.push_back(static_cast<CompoundObject*>(ow));
         ow->setName(it->attribute("name").as_string());
      }
   }

   return true;
}

void CompoundObject::defineSpriteBox(
   oxygine::Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject(sceneParent);

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   // Define sprite
   spSprite object = new Sprite();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));

   object->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   //   object->setAnchor(Vector2(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float()));
   object->setAnchor(0.5f, 0.5f);
   object->setTouchChildrenEnabled(false);

   object->attachTo(newCo);

   Vector2 newPos(pos.x + objectNode.attribute("posX").as_float(), pos.y + objectNode.attribute("posY").as_float());
   newCo->setPosition(newPos);
   newCo->setRotation(objectNode.attribute("angle").as_float());

   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineSpritePolygon(
   oxygine::Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject(sceneParent);

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   // Define sprite, which is a polygon, in this case
   vector<Vector2> vertices;
   vector<VectorT3<int> > triangles;

   spPolygon object = new oxygine::Polygon();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));

   Vector2 textureMeterSize = Vector2(objectNode.attribute("textureMeterWidth").as_float(),
      objectNode.attribute("textureMeterHeight").as_float());

   Vector2 textureMeterOffset = Vector2(objectNode.attribute("textureOffsetMeterX").as_float(),
      objectNode.attribute("textureOffsetMeterY").as_float());

   for (auto it = objectNode.child("vertices").children("vertex").begin();
      it != objectNode.child("vertices").children("vertex").end();
      ++it)
   {
      vertices.push_back(Vector2(it->attribute("x").as_float(), it->attribute("y").as_float()));
   }

   for (auto it = objectNode.child("triangles").children("triangle").begin();
      it != objectNode.child("triangles").children("triangle").end();
      ++it)
   {
      triangles.push_back(VectorT3<int>(
         it->attribute("v1").as_int(),
         it->attribute("v2").as_int(),
         it->attribute("v3").as_int()));
   }

   vertexPCT2* vs = PolygonVertices::createTriangleVertices(
      vertices,
      triangles,
      textureMeterSize,
      textureMeterOffset);

   object->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

   object->attachTo(newCo);

   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineStaticCircle(
   oxygine::Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject(sceneParent);

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   // Define sprite
   spSprite object = new Sprite();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));

   object->setSize(objectNode.attribute("radius").as_float() * 2.0f, objectNode.attribute("radius").as_float() * 2.0f);
   //   object->setAnchor(Vector2(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float()));
   object->setAnchor(0.5f, 0.5f);
   object->setTouchChildrenEnabled(false);

   object->attachTo(newCo);

   b2BodyDef bodyDef;
   bodyDef.type = b2_staticBody;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   bodyDef.angle = objectNode.attribute("angle").as_float();
   b2Body* body = world->CreateBody(&bodyDef);

   b2CircleShape circleShape;
   circleShape.m_radius = objectNode.attribute("radius").as_float();

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &circleShape;
   fixtureDef.density = 5.0f;
   fixtureDef.friction = 1.3f;
   fixtureDef.filter.categoryBits = objectNode.attribute("collisionCategory").as_int();
   fixtureDef.filter.maskBits = objectNode.attribute("collisionMask").as_int();
   fixtureDef.userData = (CollisionEntity*)newCo;

   body->CreateFixture(&fixtureDef);

   body->SetUserData(newCo);

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineStaticBox(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject(sceneParent);

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   // Define sprite
   spSprite object = new Sprite();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));

   object->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   //   object->setAnchor(Vector2(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float()));
   object->setAnchor(0.5f, 0.5f);
   object->setTouchChildrenEnabled(false);

   object->attachTo(newCo);

   b2BodyDef bodyDef;
   bodyDef.type = b2_staticBody;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   bodyDef.angle = objectNode.attribute("angle").as_float();
   b2Body* body = world->CreateBody(&bodyDef);

   b2PolygonShape boxShape;
   boxShape.SetAsBox(objectNode.attribute("width").as_float() / 2.0f, objectNode.attribute("height").as_float() / 2.0f);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &boxShape;
   fixtureDef.density = 5.0f;
   fixtureDef.friction = 1.3f;
   fixtureDef.filter.categoryBits = objectNode.attribute("collisionCategory").as_int();
   fixtureDef.filter.maskBits = objectNode.attribute("collisionMask").as_int();
   fixtureDef.userData = (CollisionEntity*)newCo;

   body->CreateFixture(&fixtureDef);

   body->SetUserData(newCo);

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}


void CompoundObject::defineStaticPolygon(
   Resources& gameResources, 
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject(sceneParent);

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   spPolygon object = new oxygine::Polygon();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   
   Vector2 textureMeterSize = Vector2(objectNode.attribute("textureMeterWidth").as_float(), 
      objectNode.attribute("textureMeterHeight").as_float());
   
   Vector2 textureMeterOffset = Vector2(objectNode.attribute("textureOffsetMeterX").as_float(), 
      objectNode.attribute("textureOffsetMeterY").as_float());

   vector<Vector2> vertices(std::distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));

   for (auto it = objectNode.child("vertices").children("vertex").begin();
      it != objectNode.child("vertices").children("vertex").end();
      ++it)
   {
      vertices[it->attribute("id").as_int() - 1] = Vector2(it->attribute("x").as_float(), it->attribute("y").as_float());
   }

   vector<VectorT3<int> > triangles(std::distance(objectNode.child("triangles").children("triangle").begin(), objectNode.child("triangles").children("triangle").end()));

   for (auto it = objectNode.child("triangles").children("triangle").begin();
      it != objectNode.child("triangles").children("triangle").end();
      ++it)
   {
      triangles[it->attribute("id").as_int() - 1] = VectorT3<int>(
         it->attribute("v1").as_int(),
         it->attribute("v2").as_int(),
         it->attribute("v3").as_int());
   }

   vertexPCT2* vs = PolygonVertices::createTriangleVertices(
      vertices, 
      triangles, 
      textureMeterSize, 
      textureMeterOffset);

   object->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

   object->attachTo(newCo);


   // Here we should attach border to all edges of the polygon
   int edgeNum = vertices.size();

   xml_node borderNode = objectNode.child("border");

   if (!borderNode.empty())
   {
      // Prepare all xml data in advance to speed up processing
      float groundLeftAngle = borderNode.attribute("leftGroundAngle").as_float() * MATH_PI / 180.0f;
      float groundRightAngle = borderNode.attribute("rightGroundAngle").as_float() * MATH_PI / 180.0f;
      float ceilingLeftAngle = borderNode.attribute("leftCeilingAngle").as_float() * MATH_PI / 180.0f;
      float ceilingRightAngle = borderNode.attribute("rightCeilingAngle").as_float() * MATH_PI / 180.0f;

      bool groundBorder = false;
      bool ceilingBorder = false;
      bool leftWallBorder = false;
      bool rightWallBorder = false;

      string groundTexture = borderNode.child("groundBorder").attribute("texture").as_string("notDefined");
      string ceilingTexture = borderNode.child("ceilingBorder").attribute("texture").as_string("notDefined");
      string leftWallTexture = borderNode.child("leftWallBorder").attribute("texture").as_string("notDefined");
      string rightWallTexture = borderNode.child("rightWallBorder").attribute("texture").as_string("notDefined");

      float groundHorOffset;
      float groundTextMeterWidth;
      float groundTextMeterHeight;

      float ceilingHorOffset;
      float ceilingTextMeterWidth;
      float ceilingTextMeterHeight;

      float leftWallHorOffset;
      float leftWallTextMeterWidth;
      float leftWallTextMeterHeight;

      float rightWallHorOffset;
      float rightWallTextMeterWidth;
      float rightWallTextMeterHeight;

      //ResAnim groundRes = gameResources.getResAnim("grotto_border");
      //ResAnim ceilingRes = gameResources.getResAnim("grotto_ceiling_border");
      //ResAnim leftRes = gameResources.getResAnim("grotto_left_border");
      //ResAnim rightRes = gameResources.getResAnim("grotto_left_border");


      if (groundTexture != "notDefined")
      {
         groundBorder = true;
         groundHorOffset = borderNode.child("groundBorder").attribute("horisontalMeterOffset").as_float();
         groundTextMeterWidth = borderNode.child("groundBorder").attribute("textureMeterWidth").as_float();
         groundTextMeterHeight = borderNode.child("groundBorder").attribute("textureMeterHeight").as_float();
      }

      if (ceilingTexture != "notDefined")
      {
         ceilingBorder = true;
         ceilingHorOffset = borderNode.child("ceilingBorder").attribute("horisontalMeterOffset").as_float();
         ceilingTextMeterWidth = borderNode.child("ceilingBorder").attribute("textureMeterWidth").as_float();
         ceilingTextMeterHeight = borderNode.child("ceilingBorder").attribute("textureMeterHeight").as_float();
      }

      if (leftWallTexture != "notDefined")
      {
         leftWallBorder = true;
         leftWallHorOffset = borderNode.child("leftWallBorder").attribute("horisontalMeterOffset").as_float();
         leftWallTextMeterWidth = borderNode.child("leftWallBorder").attribute("textureMeterWidth").as_float();
         leftWallTextMeterHeight = borderNode.child("leftWallBorder").attribute("textureMeterHeight").as_float();
      }

      if (rightWallTexture != "notDefined")
      {
         rightWallBorder = true;
         rightWallHorOffset = borderNode.child("rightWallBorder").attribute("horisontalMeterOffset").as_float();
         rightWallTextMeterWidth = borderNode.child("rightWallBorder").attribute("textureMeterWidth").as_float();
         rightWallTextMeterHeight = borderNode.child("rightWallBorder").attribute("textureMeterHeight").as_float();
      }

      if (groundBorder || ceilingBorder || leftWallBorder || rightWallBorder)
      {

         // Polygon of a body shape is physical coordinates, i.e. in meters
         for (int i = 1; i < edgeNum - 1; i++)
         {
            // We draw a border sprite from prev to current. Set anchor to 0,0.5
            // So position should be prev. Calculate angle between points and
            // set that to sprite. Then attach it to polygon.

            Vector2 prev = vertices[i - 1];
            Vector2 current = vertices[i];
            Vector2 diffV = current - prev;
            float compAngle = atan2(diffV.y, diffV.x);
            float angle = compAngle + MATH_PI;
            
            if (angle > MATH_PI)
            {
               angle -= 2.0f * MATH_PI;
            }

            compAngle = angle;

            float distance = diffV.length();
            Vector2 rotatedNormal = Vector2(-sin(angle), cos(angle));

            bool doThisBorder = false;
            float horOffset;
            float textMeterWidth;
            float textMeterHeight;
            string texture;

            // angle is the amount to rotate the texture. This should be between 
            // -pi/2 and pi/2 for ground and above 3*pi/2 or below -3*pi/2 for ceiling.
            // compAngle is -PI away, i.e. Ground should have compAngle from pi/2 to 3pi/2
            // But why does it not work with angle (non-comp)?
            // compAngle goes from -pi to pi which means that angle goes from
            // 0 to 2*pi. This is the problem. We would like to change so that
            // angle goes from -pi to pi. I.e. If angle > pi, angle = angle - 2 * pi

            // Select border data by the angle and the angle limits
            if ((compAngle <= groundRightAngle) && (compAngle > groundLeftAngle))
            {
               // This is a ground border
               doThisBorder = groundBorder;
               if (doThisBorder)
               {
                  horOffset = groundHorOffset;
                  textMeterWidth = groundTextMeterWidth;
                  textMeterHeight = groundTextMeterHeight;
                  texture = groundTexture;
               }
            }
            else if ((compAngle > groundRightAngle) && (compAngle < ceilingRightAngle))
            {
               // This is a right wall border
               doThisBorder = rightWallBorder;
               if (doThisBorder)
               {
                  horOffset = rightWallHorOffset;
                  textMeterWidth = rightWallTextMeterWidth;
                  textMeterHeight = rightWallTextMeterHeight;
                  texture = rightWallTexture;
               }
            }
            else if ((compAngle < groundLeftAngle) && (compAngle > ceilingLeftAngle))
            {
               // This is a left wall border
               doThisBorder = leftWallBorder;
               if (doThisBorder)
               {
                  horOffset = leftWallHorOffset;
                  textMeterWidth = leftWallTextMeterWidth;
                  textMeterHeight = leftWallTextMeterHeight;
                  texture = leftWallTexture;
               }
            }
            else
            {
               // This is a ceiling border
               doThisBorder = ceilingBorder;
               if (doThisBorder)
               {
                  horOffset = ceilingHorOffset;
                  textMeterWidth = ceilingTextMeterWidth;
                  textMeterHeight = ceilingTextMeterHeight;
                  texture = ceilingTexture;
               }
            }

            if (doThisBorder)
            {
               spPolygon border = new oxygine::Polygon();
               border->setResAnim(gameResources.getResAnim(texture));
               border->setAnchor(0.0f, 0.5f);

               vector<Vector2> vertices(4);
               vertices[0] = Vector2(0.0f, 0.0f);
               vertices[1] = Vector2(0.0f, textMeterHeight);
               vertices[2] = Vector2(distance, textMeterHeight);
               vertices[3] = Vector2(distance, 0.0f);

               vector<VectorT3<int> > triangles(2);

               triangles[0] = VectorT3<int>(1, 2, 3);
               triangles[1] = VectorT3<int>(1, 3, 4);

               vertexPCT2* vs = PolygonVertices::createTriangleVertices(
                  vertices,
                  triangles,
                  Vector2(textMeterWidth, textMeterHeight),
                  Vector2(0.0f, 0.0f));

               border->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

               border->setAnchor(0.0f, 0.0f);
               border->setPosition(current - rotatedNormal * horOffset);
               border->setRotation(angle);

               border->attachTo(object);

            }
         }

         Vector2 prev = vertices[edgeNum - 1];
         Vector2 current = vertices[0];
         Vector2 diffV = current - prev;
         float compAngle = atan2(diffV.y, diffV.x);
         float angle = compAngle + MATH_PI;

         if (angle > MATH_PI)
         {
            angle -= 2.0f * MATH_PI;
         }

         compAngle = angle;

         float distance = diffV.length();
         Vector2 rotatedNormal = Vector2(-sin(angle), cos(angle));

         bool doThisBorder = false;
         float horOffset;
         float textMeterWidth;
         float textMeterHeight;
         string texture;

         // Select border data by the angle and the angle limits
         if ((compAngle <= groundRightAngle) && (compAngle > groundLeftAngle))
         {
            // This is a ground border
            if (doThisBorder)
            {
               doThisBorder = groundBorder;
               horOffset = groundHorOffset;
               textMeterWidth = groundTextMeterWidth;
               textMeterHeight = groundTextMeterHeight;
               texture = groundTexture;
            }
         }
         else if ((compAngle > groundRightAngle) && (compAngle < ceilingRightAngle))
         {
            // This is a right wall border
            doThisBorder = rightWallBorder;
            if (doThisBorder)
            {
               horOffset = rightWallHorOffset;
               textMeterWidth = rightWallTextMeterWidth;
               textMeterHeight = rightWallTextMeterHeight;
               texture = rightWallTexture;
            }
         }
         else if ((compAngle < groundLeftAngle) && (compAngle > ceilingLeftAngle))
         {
            // This is a left wall border
            doThisBorder = leftWallBorder;
            if (doThisBorder)
            {
               horOffset = leftWallHorOffset;
               textMeterWidth = leftWallTextMeterWidth;
               textMeterHeight = leftWallTextMeterHeight;
               texture = leftWallTexture;
            }
         }
         else
         {
            // This is a ceiling border
            doThisBorder = ceilingBorder;
            if (doThisBorder)
            {
               horOffset = ceilingHorOffset;
               textMeterWidth = ceilingTextMeterWidth;
               textMeterHeight = ceilingTextMeterHeight;
               texture = ceilingTexture;
            }
         }

         if (doThisBorder)
         {
            spPolygon border = new oxygine::Polygon();
            border->setResAnim(gameResources.getResAnim(texture));
            border->setAnchor(0.0f, 0.5f);

            vector<Vector2> vertices(4);
            vertices[0] = Vector2(0.0f, 0.0f);
            vertices[1] = Vector2(0.0f, textMeterHeight);
            vertices[2] = Vector2(distance, textMeterHeight);
            vertices[3] = Vector2(distance, 0.0f);

            vector<VectorT3<int> > triangles(2);

            triangles[0] = VectorT3<int>(1, 2, 3);
            triangles[1] = VectorT3<int>(1, 3, 4);

            vertexPCT2* vs = PolygonVertices::createTriangleVertices(
               vertices,
               triangles,
               Vector2(textMeterWidth, textMeterHeight),
               Vector2(0.0f, 0.0f));

            border->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

            border->setAnchor(0.0f, 0.0f);
            border->setPosition(current - rotatedNormal * horOffset);
            border->setRotation(angle);

            border->attachTo(object);

         }










         //spPolygon border = new oxygine::Polygon();
         //border->setResAnim(gameResources.getResAnim("grotto_border"));
         //border->setAnchor(0.0f, 0.5f);

         //vector<Vector2> lastVertices(4);
         //lastVertices[0] = Vector2(0.0f, 0.0f);
         //lastVertices[1] = Vector2(0.0f, 5.0f);
         //lastVertices[2] = Vector2(distance, 5.0);
         //lastVertices[3] = Vector2(distance, 0.0f);

         //vector<VectorT3<int> > lastTriangles(2);

         //lastTriangles[0] = VectorT3<int>(1, 2, 3);
         //lastTriangles[1] = VectorT3<int>(1, 3, 4);

         //vertexPCT2* lastVs = PolygonVertices::createTriangleVertices(
         //   lastVertices,
         //   lastTriangles,
         //   Vector2(128.0f, 10.0f),
         //   Vector2(0.0f, 0.0f));

         //border->setVertices(lastVs, sizeof(vertexPCT2) *  lastTriangles.size() * 4, vertexPCT2::FORMAT, true);

         //border->setAnchor(0.0f, 0.0f);
         //border->setPosition(current - rotatedNormal * 2.0);
         //border->setRotation(angle);

         //border->attachTo(object);

      }


   }


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
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   b2Body* body = world->CreateBody(&bodyDef);

   b2ChainShape chain;
   chain.CreateChain(b2vertices, num);

   b2Fixture* fixture = body->CreateFixture(&chain, 1);
   fixture->SetUserData((CollisionEntity*)newCo);

   b2Filter filter;
   filter.categoryBits = objectNode.attribute("collisionCategory").as_int();
   filter.maskBits = objectNode.attribute("collisionMask").as_int();
   fixture->SetFilterData(filter);

   body->SetUserData(newCo);

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineStaticBoxedSpritePolygon(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject(sceneParent);

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   // Define sprite, which is a polygon, in this case

   spSprite object = new Sprite();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));

   object->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   //   object->setAnchor(Vector2(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float()));
   object->setAnchor(0.5f, 0.5f);
   object->setTouchChildrenEnabled(false);

   object->attachTo(newCo);

   vector<Vector2> vertices(distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));

   for (auto it = objectNode.child("vertices").children("vertex").begin();
      it != objectNode.child("vertices").children("vertex").end();
      ++it)
   {
      vertices[it->attribute("id").as_int() - 1] = Vector2(it->attribute("x").as_float(), it->attribute("y").as_float());
   }

   vector<VectorT3<int> > triangles(distance(objectNode.child("triangles").children("triangle").begin(), objectNode.child("triangles").children("triangle").end()));

   for (auto it = objectNode.child("triangles").children("triangle").begin();
      it != objectNode.child("triangles").children("triangle").end();
      ++it)
   {
      triangles[it->attribute("id").as_int() - 1] = VectorT3<int>(
         it->attribute("v1").as_int(),
         it->attribute("v2").as_int(),
         it->attribute("v3").as_int());
   }

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
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   bodyDef.angle = objectNode.attribute("angle").as_float();
   b2Body* body = world->CreateBody(&bodyDef);

   b2PolygonShape polyShape;
   polyShape.Set(b2vertices, num - 1);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &polyShape;
   fixtureDef.density = 5.0f;
   fixtureDef.friction = 1.3f;
   fixtureDef.filter.categoryBits = objectNode.attribute("collisionCategory").as_int();
   fixtureDef.filter.maskBits = objectNode.attribute("collisionMask").as_int();
   fixtureDef.userData = (CollisionEntity*)newCo;

   body->CreateFixture(&fixtureDef);

   body->SetUserData(newCo);

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineDynamicCircle(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject(sceneParent);

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   // Define sprite
   spSprite object = new Sprite();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   object->setSize(objectNode.attribute("radius").as_float() * 2.0f, objectNode.attribute("radius").as_float() * 2.0f);
   //   object->setAnchor(Vector2(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float()));
   object->setAnchor(0.5f, 0.5f);
   object->setTouchChildrenEnabled(false);

   object->attachTo(newCo);

   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   bodyDef.angle = objectNode.attribute("angle").as_float();
   b2Body* body = world->CreateBody(&bodyDef);

   b2CircleShape circleShape;
   circleShape.m_radius = objectNode.attribute("radius").as_float();

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &circleShape;
   fixtureDef.density = objectNode.attribute("density").as_float(1.0f);
   fixtureDef.friction = objectNode.attribute("friction").as_float(1.0f);
   fixtureDef.filter.categoryBits = objectNode.attribute("collisionCategory").as_int();
   fixtureDef.filter.maskBits = objectNode.attribute("collisionMask").as_int();
   fixtureDef.userData = (CollisionEntity*)newCo;

   body->CreateFixture(&fixtureDef);

   body->ResetMassData();

   body->SetUserData(newCo);

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineDynamicBox(
   Resources& gameResources, 
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject(sceneParent);

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   // Define sprite
   spSprite object = new Sprite();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   object->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   //   object->setAnchor(Vector2(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float()));
   object->setAnchor(0.5f, 0.5f);
   object->setTouchChildrenEnabled(false);

   object->attachTo(newCo);

   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   bodyDef.angle = objectNode.attribute("angle").as_float();
   b2Body* body = world->CreateBody(&bodyDef);

   b2PolygonShape boxShape;
   boxShape.SetAsBox(objectNode.attribute("width").as_float() / 2.0f, objectNode.attribute("height").as_float() / 2.0f);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &boxShape;
   fixtureDef.density = objectNode.attribute("density").as_float(1.0f);
   fixtureDef.friction = objectNode.attribute("friction").as_float(1.0f);
   fixtureDef.filter.categoryBits = objectNode.attribute("collisionCategory").as_int();
   fixtureDef.filter.maskBits = objectNode.attribute("collisionMask").as_int();
   fixtureDef.userData = (CollisionEntity*)newCo;

   body->CreateFixture(&fixtureDef);

   body->SetUserData(newCo);
   body->ResetMassData();

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineDynamicPolygon(
   Resources& gameResources, 
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject(sceneParent);

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   // Define sprite, which is a polygon, in this case

   spPolygon object = new oxygine::Polygon();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));

   Vector2 textureMeterSize = Vector2(objectNode.attribute("textureMeterWidth").as_float(),
      objectNode.attribute("textureMeterHeight").as_float());

   Vector2 textureMeterOffset = Vector2(objectNode.attribute("textureOffsetMeterX").as_float(),
      objectNode.attribute("textureOffsetMeterY").as_float());


   //object->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   //object->setAnchor(Vector2(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float()));
   object->setTouchChildrenEnabled(false);

   vector<Vector2> vertices(distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));

   for (auto it = objectNode.child("vertices").children("vertex").begin();
      it != objectNode.child("vertices").children("vertex").end();
      ++it)
   {
      vertices[it->attribute("id").as_int() - 1] = Vector2(it->attribute("x").as_float(), it->attribute("y").as_float());
   }

   vector<VectorT3<int> > triangles(distance(objectNode.child("triangles").children("triangle").begin(), objectNode.child("triangles").children("triangle").end()));

   for (auto it = objectNode.child("triangles").children("triangle").begin();
      it != objectNode.child("triangles").children("triangle").end();
      ++it)
   {
      triangles[it->attribute("id").as_int() - 1] = VectorT3<int>(
         it->attribute("v1").as_int(),
         it->attribute("v2").as_int(),
         it->attribute("v3").as_int());
   }

   vertexPCT2* vs = PolygonVertices::createTriangleVertices(vertices, triangles, textureMeterSize, textureMeterOffset);
   object->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

   object->attachTo(newCo);

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
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   bodyDef.angle = objectNode.attribute("angle").as_float();
   b2Body* body = world->CreateBody(&bodyDef);

   b2PolygonShape polyShape;
   polyShape.Set(b2vertices, num - 1);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &polyShape;
   fixtureDef.density = objectNode.attribute("density").as_float(1.0f);
   fixtureDef.friction = objectNode.attribute("friction").as_float(1.0f);
   fixtureDef.filter.categoryBits = objectNode.attribute("collisionCategory").as_int();
   fixtureDef.filter.maskBits = objectNode.attribute("collisionMask").as_int();
   fixtureDef.userData = (CollisionEntity*)newCo;

   body->CreateFixture(&fixtureDef);

   body->ResetMassData();
   body->SetUserData(newCo);

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineDynamicBoxedSpritePolygon(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject(sceneParent);

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   // Define sprite, which is a polygon, in this case

   spSprite object = new Sprite();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));

   object->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   //   object->setAnchor(Vector2(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float()));
   object->setAnchor(0.5f, 0.5f);
   object->setTouchChildrenEnabled(false);

   object->attachTo(newCo);

   vector<Vector2> vertices(distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));

   for (auto it = objectNode.child("vertices").children("vertex").begin();
      it != objectNode.child("vertices").children("vertex").end();
      ++it)
   {
      vertices[it->attribute("id").as_int() - 1] = Vector2(it->attribute("x").as_float(), it->attribute("y").as_float());
   }

   vector<VectorT3<int> > triangles(distance(objectNode.child("triangles").children("triangle").begin(), objectNode.child("triangles").children("triangle").end()));

   for (auto it = objectNode.child("triangles").children("triangle").begin();
      it != objectNode.child("triangles").children("triangle").end();
      ++it)
   {
      triangles[it->attribute("id").as_int() - 1] = VectorT3<int>(
         it->attribute("v1").as_int(),
         it->attribute("v2").as_int(),
         it->attribute("v3").as_int());
   }

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
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   bodyDef.angle = objectNode.attribute("angle").as_float();
   b2Body* body = world->CreateBody(&bodyDef);

   b2PolygonShape polyShape;
   polyShape.Set(b2vertices, num - 1);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &polyShape;
   fixtureDef.density = objectNode.attribute("density").as_float(1.0f);
   fixtureDef.friction = objectNode.attribute("friction").as_float(1.0f);
   fixtureDef.filter.categoryBits = objectNode.attribute("collisionCategory").as_int();
   fixtureDef.filter.maskBits = objectNode.attribute("collisionMask").as_int();
   fixtureDef.userData = (CollisionEntity*)newCo;

   body->CreateFixture(&fixtureDef);

   body->ResetMassData();
   body->SetUserData(newCo);

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineRope(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& jointNode)
{
   // The whole rope is a CO, all segments attach to it.
   CompoundObject* newCo = new CompoundObject(sceneParent);

   newCo->setName(jointNode.attribute("name").as_string());
   newCo->setPriority(jointNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;
   
   int numOfSegments = jointNode.attribute("noOfSegments").as_int();
   float ropeLength = jointNode.attribute("length").as_float(10.0f);
   float segmentLength = ropeLength / numOfSegments;
   float thickness = jointNode.attribute("thickness").as_float();
   float density = jointNode.attribute("density").as_float(1.0f);
   string texture = jointNode.attribute("texture").as_string();
   float maxLength = 0.01f;

   vector<b2Body*> ropeBodies;

   // Find first body to attach the rope to.
   b2RopeJointDef	jointDef;

   b2Body* bodyA = getBody(jointNode.attribute("objectAName").as_string());

   if (!bodyA)
   {
      return;
   }

   // Find ending body to attach the rope to. If there is no defined a loose rope
   // will be created
   b2Body* bodyB = getBody(jointNode.attribute("objectBName").as_string());



   // To get the initial position and angle of each segment correct we need to 
   // get the start and end point of the rope. The start point is always 
   // the anchorpoint to body A.
   // The end point is anchor of body B if body B exists. If body B does not exist
   // the end point is the rope length directly below the start point 
   // (a straight haning rope)
   b2Vec2 bodyAAnchor = b2Vec2(jointNode.attribute("objectAAnchorX").as_float(), jointNode.attribute("objectAAnchorY").as_float());   
   b2Vec2 startPos = bodyA->GetWorldPoint(bodyAAnchor);
   b2Vec2 endPos;
   b2Vec2 bodyBAnchor;

   if (bodyB != NULL)
   {
      bodyBAnchor = b2Vec2(jointNode.attribute("objectBAnchorX").as_float(), jointNode.attribute("objectBAnchorY").as_float());
      endPos = bodyB->GetWorldPoint(bodyBAnchor);
   }
   else
   {
      endPos = startPos + b2Vec2(0, ropeLength);
   }

   b2Vec2 ropeVector = endPos - startPos;
   b2Vec2 ropeDirection = ropeVector;
   float distance = ropeDirection.Normalize();

   float ropeAngle = acos(ropeVector.x / distance);

   // Now create the first segment. This is done outside the loop
   // since the fastening body is special (not a rope segment) and
   // has its own anchor point.

   // Define sprite of first segment
   spSprite object = new Sprite();
   object->setResAnim(gameResources.getResAnim(texture));
   object->setSize(segmentLength, thickness);
   object->setAnchor(0.5f, 0.5f);
   object->setTouchChildrenEnabled(false);
   b2Vec2 segmentPos = startPos + segmentLength * 0.5f * ropeDirection;
   object->setPosition(PhysDispConvert::convert(segmentPos, 1.0f));
   object->setRotation(ropeAngle);
   object->attachTo(newCo);

   // Define body of first segment
   b2BodyDef bodyDef;
   bodyDef.type = b2_dynamicBody;
   bodyDef.position = segmentPos;
   bodyDef.angle = ropeAngle;

   //// TODO: Hardcoded, only works for one example, need to be changed
   //bodyDef.position = b2Vec2(200.0f, 210.0f);

   b2Body* firstSegBody = world->CreateBody(&bodyDef);

   b2PolygonShape boxShape;
   boxShape.SetAsBox(segmentLength, thickness);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &boxShape;
   fixtureDef.density = jointNode.attribute("density").as_float(1.0f);
   fixtureDef.friction = 1.0f;
   fixtureDef.filter.categoryBits = jointNode.attribute("collisionCategory").as_int();
   fixtureDef.filter.maskBits = jointNode.attribute("collisionMask").as_int();
   fixtureDef.userData = (CollisionEntity*)newCo;

   firstSegBody->CreateFixture(&fixtureDef);
   firstSegBody->SetUserData(object.get());
   object->setUserData(firstSegBody);

   // Define joint between beginning fastening body and first segment
   jointDef.bodyA = bodyA;
   jointDef.bodyB = firstSegBody;
   jointDef.localAnchorA.Set(jointNode.attribute("objectAAnchorX").as_float(), jointNode.attribute("objectAAnchorY").as_float());
   jointDef.localAnchorB.Set(-segmentLength / 2.0f, 0.0f);
   jointDef.collideConnected = false;
   jointDef.maxLength = maxLength;

   world->CreateJoint(&jointDef);

   ropeBodies.push_back(firstSegBody);

   // Now loop all segments until the last
   for (int i = 1; i < numOfSegments; i++)
   {
      // Create sprite of segment i
      spSprite object = new Sprite();
      object->setResAnim(gameResources.getResAnim(texture));
      object->setSize(segmentLength, thickness);
      object->setAnchor(0.5f, 0.5f);
      object->setTouchChildrenEnabled(false);
      b2Vec2 segmentPos = startPos + segmentLength * (0.5f + i) * ropeDirection;
      object->setPosition(PhysDispConvert::convert(segmentPos, 1.0f));
      object->setRotation(ropeAngle);
      object->attachTo(newCo);

      // Define body of segment i
      b2BodyDef bodyDef;
      bodyDef.type = b2_dynamicBody;
      bodyDef.position = segmentPos;
      bodyDef.angle = ropeAngle;

      //// TODO: Hardcoded, only works for one example, need to be changed
      //bodyDef.position = b2Vec2(200.0f, 210.0f + i * segmentLength);

      b2Body* segBody = world->CreateBody(&bodyDef);

      b2PolygonShape boxShape;
      boxShape.SetAsBox(segmentLength, thickness);

      b2FixtureDef fixtureDef;
      fixtureDef.shape = &boxShape;
      fixtureDef.density = jointNode.attribute("density").as_float(1.0f);
      fixtureDef.friction = 1.0f;
      fixtureDef.filter.categoryBits = jointNode.attribute("collisionCategory").as_int();
      fixtureDef.filter.maskBits = jointNode.attribute("collisionMask").as_int();
      fixtureDef.userData = (CollisionEntity*)newCo;

      segBody->CreateFixture(&fixtureDef);
      segBody->SetUserData(object.get());
      object->setUserData(segBody);

      // Define joint between beginning fastening body and first segment
      jointDef.bodyA = ropeBodies[i-1];
      jointDef.bodyB = segBody;
      jointDef.localAnchorA.Set(segmentLength / 2.0f, 0.0f);
      jointDef.localAnchorB.Set(-segmentLength / 2.0f, 0.0f);
      jointDef.collideConnected = false;
      jointDef.maxLength = maxLength;

      world->CreateJoint(&jointDef);

      ropeBodies.push_back(segBody);
   }

   // Only if BodyB is defined in XML will the last segment be
   // attached to something
   if (bodyB)
   {
      // Define joint between last segment and ending fastening body 
      jointDef.bodyA = ropeBodies[numOfSegments - 1];
      jointDef.bodyB = bodyB;
      jointDef.localAnchorA.Set(segmentLength / 2.0f, 0.0f);
      jointDef.localAnchorB.Set(jointNode.attribute("objectBAnchorX").as_float(), jointNode.attribute("objectBAnchorY").as_float());
      jointDef.collideConnected = false;
      jointDef.maxLength = maxLength;

      world->CreateJoint(&jointDef);
   }

   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineChildObject(
   Resources& gameResources, 
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode,
   const string& initialState)
{
   xml_node stateNode;

   if (!getStateNode(objectNode, initialState, stateNode))
   {
      return;
   }

   xml_node propNode = stateNode.child("properties");

   if (propNode.empty())
   {
      return;
   }

   Vector2 objPos = Vector2(
      propNode.attribute("posX").as_float(),
      propNode.attribute("posY").as_float());

   objPos += pos;

   string fileName = propNode.attribute("file").as_string("notApplicable");

   CompoundObject* co = NULL;

   if (fileName == "notApplicable")
   {
      xml_node coNode = propNode.child("compoundObject");

      if (coNode.empty())
      {
         return;
      }

      // Define child directly
      co = initCompoundObject(
         gameResources,
         sceneParent,
         parentObject,
         world,
         objPos,
         coNode,
         initialState);
   }
   else
   {
      // Define child file
      // Read the child XML file
      co = readDefinitionXmlFile(
         gameResources,
         sceneParent,
         parentObject,
         world,
         objPos,
         fileName,
         initialState);
   }

   if (co)
   {
      m_children.push_back(static_cast<CompoundObject*>(co));

      co->setName(objectNode.attribute("name").as_string());
   }
}

bool CompoundObject::getStateNode(xml_node& objectNode, const string& initialState, xml_node& stateNode)
{
   // Iterate the stateProperties of the node, looking for state attributes
   // that matches the supplied initialState. If initialState is empty,
   // the first stateProperty is used

   for (auto it = objectNode.children("stateProperties").begin(); it != objectNode.children("stateProperties").end(); ++it)
   {
      if ((initialState == "") || (it->attribute("state").as_string() == initialState))
      {
         stateNode = *it;
         return true;
      }
   }

   return false;
}

//void CompoundObject::defineObjectSystem(
//   Resources& gameResources,
//   Actor* sceneParent,
//   CompoundObject* parentObject,
//   b2World* world,
//   const Vector2& pos,
//   xml_node& objectNode,
//   string& initialState)
//{
//   // But there is a type of objects that are not to be handeled like this. They
//   // are, for instance, particle systems or object factories. They are not compound 
//   // objects really but could define shapes or CO as children (for instance can a 
//   // object factory define a CO from a file as its spawning object). They are also 
//   // defined by a set of object specific parameters (xml attributes). How to handle
//   // these? Are they CO-children? Are they special behaviour elements (which require 
//   // their own collections in CO or scenes.
//   // They shall be defined as other CO with state properties since different states 
//   // may require differnt settings. But they may not require a PosX, PosY and file
//   // but other parameters
//   // Let there be Compound Object Systems!!!
//
//   // Iterate the stateProperties of the node, looking for state attributes
//   // that matches the supplied initialState. If initialState is empty,
//   // the first statePropert� is used
//   xml_node* stateNode = NULL;
//
//   for (auto it = objectNode.children("stateProperties").begin(); it != objectNode.children("stateProperties").end(); ++it)
//   {
//      if ((initialState == "") || (it->attribute("state").as_string() == initialState))
//      {
//         stateNode = &(*it);
//         break;
//      }
//   }
//
//   if (!stateNode)
//   {
//      return;
//   }
//
//   // Decode the type string to create the correct type of object
//   // but only store the pointer to the CompoundObject
//   // Perhaps with some special cases
//   string type = objectNode.attribute("type").as_string();
//
//   if (type == "asteroidField")
//   {
//      AsteroidField* af = new AsteroidField(
//         gameResources,
//         sceneParent,
//         parentObject,
//         world,
//         *stateNode);
//
//      m_children.push_back(static_cast<CompoundObject*>(af));
//
//      af->setName(objectNode.attribute("name").as_string());
//   }
//   else if (type == "planetActor")
//   {
//      PlanetActor* pa = new PlanetActor(
//         gameResources,
//         sceneParent,
//         parentObject,
//         *stateNode);
//
//      m_children.push_back(static_cast<CompoundObject*>(pa));
//
//      pa->setName(objectNode.attribute("name").as_string());
//   }
//   else if (type == "clippedWindow")
//   {
//      OrbitWindow* ow = new OrbitWindow(
//         gameResources,
//         sceneParent,
//         parentObject,
//         *stateNode,
//         initialState);
//
//      m_children.push_back(static_cast<CompoundObject*>(ow));
//
//      ow->setName(objectNode.attribute("name").as_string());
//   }
//}

void CompoundObject::defineWeldJoint(b2World* world, pugi::xml_node& jointNode)
{
   b2WeldJointDef	jointDef;

   b2Body* bodyA = getBody(jointNode.attribute("objectAName").as_string());

   if (!bodyA)
   {
      return;
   }

   b2Body* bodyB = getBody(jointNode.attribute("objectBName").as_string());

   if (!bodyB)
   {
      return;
   }

   jointDef.bodyA = bodyA;
   jointDef.bodyB = bodyB;
   jointDef.localAnchorA.Set(jointNode.attribute("objectAAnchorX").as_float(), jointNode.attribute("objectAAnchorY").as_float());
   jointDef.localAnchorB.Set(jointNode.attribute("objectBAnchorX").as_float(), jointNode.attribute("objectBAnchorY").as_float());
   jointDef.collideConnected = false;

   b2WeldJoint* joint = (b2WeldJoint*)world->CreateJoint(&jointDef);

   NamedJoint* nj = new NamedJoint(joint, jointNode.attribute("name").as_string());
   m_namedJoints.push_back(nj);
}

void CompoundObject::defineRevoluteJoint(b2World* world, pugi::xml_node& jointNode)
{
   b2RevoluteJointDef	jointDef;

   b2Body* bodyA = getBody(jointNode.attribute("objectAName").as_string());

   if (!bodyA)
   {
      return;
   }

   b2Body* bodyB = getBody(jointNode.attribute("objectBName").as_string());

   if (!bodyB)
   {
      return;
   }

   jointDef.bodyA = bodyA;
   jointDef.bodyB = bodyB;
   jointDef.localAnchorA.Set(jointNode.attribute("objectAAnchorX").as_float(), jointNode.attribute("objectAAnchorY").as_float());
   jointDef.localAnchorB.Set(jointNode.attribute("objectBAnchorX").as_float(), jointNode.attribute("objectBAnchorY").as_float());
   jointDef.collideConnected = false;
   b2RevoluteJoint* joint = (b2RevoluteJoint*)world->CreateJoint(&jointDef);

   //joint->EnableMotor(false);
   //joint->SetLimits(joint->GetJointAngle(), joint->GetJointAngle());
   //joint->EnableLimit(true);

   if (jointNode.attribute("useMotor").as_bool())
   {
      joint->SetMotorSpeed(jointNode.attribute("motorSpeed").as_float());
      joint->SetMaxMotorTorque(jointNode.attribute("maxMotorTorque").as_float());
      joint->EnableMotor(true);
   }
   else
   {
      joint->EnableMotor(false);
   }

   if (jointNode.attribute("lockJoint").as_bool())
   {
      joint->SetLimits(joint->GetJointAngle(), joint->GetJointAngle());
      joint->EnableLimit(true);
   }
   else
   {
      joint->EnableLimit(false);
   }

   NamedJoint* nj = new NamedJoint(joint, jointNode.attribute("name").as_string());
   m_namedJoints.push_back(nj);

}

void CompoundObject::definePrismaticJoint(b2World* world, pugi::xml_node& jointNode)
{
   b2PrismaticJointDef  jointDef;

   b2Body* bodyA = getBody(jointNode.attribute("objectAName").as_string());

   if (!bodyA)
   {
      return;
   }

   b2Body* bodyB = getBody(jointNode.attribute("objectBName").as_string());

   if (!bodyB)
   {
      return;
   }

   jointDef.bodyA = bodyA;
   jointDef.bodyB = bodyB;
   jointDef.localAnchorA.Set(jointNode.attribute("objectAAnchorX").as_float(), jointNode.attribute("objectAAnchorY").as_float());
   jointDef.localAnchorB.Set(jointNode.attribute("objectBAnchorX").as_float(), jointNode.attribute("objectBAnchorY").as_float());
   jointDef.collideConnected = false;
   jointDef.localAxisA.Set(jointNode.attribute("objectALocalAxisX").as_float(), jointNode.attribute("objectALocalAxisY").as_float());
   jointDef.referenceAngle = jointNode.attribute("refAngle").as_float();
   jointDef.lowerTranslation = jointNode.attribute("lowerLimit").as_float();
   jointDef.upperTranslation = jointNode.attribute("upperLimit").as_float();
   b2PrismaticJoint* joint = (b2PrismaticJoint*)world->CreateJoint(&jointDef);

   //joint->EnableMotor(false);
   //joint->EnableLimit(true);

   if (jointNode.attribute("useMotor").as_bool())
   {
      joint->SetMotorSpeed(jointNode.attribute("motorSpeed").as_float());
      joint->SetMaxMotorForce(jointNode.attribute("maxMotorForce").as_float());
      joint->EnableMotor(true);
   }
   else
   {
      joint->EnableMotor(false);
   }

   if (jointNode.attribute("lockJoint").as_bool())
   {
      joint->EnableLimit(true);
   }
   else
   {
      joint->EnableLimit(false);
   }


   NamedJoint* nj = new NamedJoint(joint, jointNode.attribute("name").as_string());
   m_namedJoints.push_back(nj);

}

CompoundObject* CompoundObject::getObjectImpl(const std::string& name)
{
   for (auto it = m_children.begin(); it != m_children.end(); ++it)
   {
      if ((*it)->getName() == name)
      {
         return (CompoundObject*)(*it);
      }
   }
     
   //for (auto it = m_boxedSprites.begin(); it != m_boxedSprites.end(); ++it)
   //{
   //   if (it->get()->getName() == name)
   //   {
   //      return (CompoundObject*)(it->get());
   //   }
   //}

   //for (auto it = m_polygonSprites.begin(); it != m_polygonSprites.end(); ++it)
   //{
   //   if (it->get()->getName() == name)
   //   {
   //      return (CompoundObject*)(it->get());
   //   }
   //}

   //for (auto it = m_staticPolygons.begin(); it != m_staticPolygons.end(); ++it)
   //{
   //   if (it->get()->getName() == name)
   //   {
   //      return (CompoundObject*)(it->get());
   //   }
   //}

   //for (auto it = m_staticBoxes.begin(); it != m_staticBoxes.end(); ++it)
   //{
   //   if (it->get()->getName() == name)
   //   {
   //      return (CompoundObject*)(it->get());
   //   }
   //}

   return NULL;
}

b2Body* CompoundObject::getBodyImpl(const std::string& name)
{
   CompoundObject* co = getObjectImpl(name);

   if (co)
   {
      return (b2Body*)co->getUserData();
   }

   //for (auto it = m_boxedSprites.begin(); it != m_boxedSprites.end(); ++it)
   //{
   //   if (it->get()->getName() == name)
   //   {
   //      return (b2Body*)it->get()->getUserData();
   //   }
   //}

   //for (auto it = m_polygonSprites.begin(); it != m_polygonSprites.end(); ++it)
   //{
   //   if (it->get()->getName() == name)
   //   {
   //      return (b2Body*)it->get()->getUserData();
   //   }
   //}

   //for (auto it = m_staticPolygons.begin(); it != m_staticPolygons.end(); ++it)
   //{
   //   if (it->get()->getName() == name)
   //   {
   //      return (b2Body*)it->get()->getUserData();
   //   }
   //}

   //for (auto it = m_staticBoxes.begin(); it != m_staticBoxes.end(); ++it)
   //{
   //   if (it->get()->getName() == name)
   //   {
   //      return (b2Body*)it->get()->getUserData();
   //   }
   //}

   return NULL;
}

b2Joint* CompoundObject::getJointImpl(const std::string& name)
{
   for (auto it = m_namedJoints.begin(); it != m_namedJoints.end(); ++it)
   {
      if ((*it)->m_name == name)
      {
         return (*it)->m_joint;
      }
   }

   return NULL;
}

System* CompoundObject::getSystemImpl(const std::string& name)
{
   for (auto it = m_systems.begin(); it != m_systems.end(); ++it)
   {
      if ((*it)->getName() == name)
      {
         return (*it);
      }
   }

   return NULL;
}

ObjectProperty* CompoundObject::getProp(int propId)
{
   if ((propId > 0) && ((unsigned int)propId < m_properties.size()))
   {
      return &(m_properties[propId]);
   }

   return NULL;
}

void CompoundObject::setProperty(int propId, float value)
{
   ObjectProperty* p = getProp(propId);
   if (p)
   {
      p->setProperty(value);
   }
}

void CompoundObject::extSetProperty(int propId, float value)
{
   setProperty(propId, value);
   setPropertyImpl(propId, value);
}

// The setPropertyImpl here does nothing. It is ment to be 
// overridden by a inherited class to decode and do special
// handling when a property is changed.
void CompoundObject::setPropertyImpl(int propId, float value)
{
}

float CompoundObject::getProperty(int propId)
{
   ObjectProperty* p = getProp(propId);
   if (p)
   {
      return p->getProperty();
   }

   return 0.0f;
}

void CompoundObject::registerPropertyEventTrigger(
   int propId, 
   int eventId,
   PropertyEventTrigger::TriggerType 
   triggerType, 
   float lower, 
   float upper)
{
   ObjectProperty* p = getProp(propId);
   if (p)
   {
      return p->registerPropertyEventTrigger(eventId, triggerType, lower, upper);
   }
}

void CompoundObject::registerDualPropEventTrigger(int propId, DualPropEventTrigger* trigger)
{
   ObjectProperty* p = getProp(propId);
   if (p)
   {
      return p->registerDualPropEventTrigger(trigger);
   }
}

void CompoundObject::unregisterDualPropEventTrigger(int propId, DualPropEventTrigger* trigger)
{
   ObjectProperty* p = getProp(propId);
   if (p)
   {
      return p->unregisterDualPropEventTrigger(trigger);
   }
}


