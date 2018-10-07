#include <algorithm>

#include "Box2D/Box2D.h"

#include "compoundobject.h"

#include "sceneactor.h"
#include "freespaceactor.h"

// All behaviours of CompoundObjects
// and thus can be instansiated as children to one
// CompoundObject
#include "launchsite.h"
#include "leapfrog.h"
#include "breakableobject.h"
#include "landingpad.h"
#include "planetactor.h"
#include "orbitwindow.h"
#include "blastemitter.h"
#include "system.h"

#include "polygonvertices.h"
#include "bodyuserdata.h"
#include "actoruserdata.h"

using namespace oxygine;
using namespace std;
using namespace pugi;


CompoundObject::CompoundObject(SceneActor* sceneActor, CompoundObject* parentObject) :
   m_sceneActor(sceneActor),
   m_parentObject(parentObject),
   m_collisionType(CET_NOT_APPLICABLE),
   m_behaviourType(notApplicable)
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
   const string& defXmlFileName,
   const string& initialState)
{
   readDefinitionXmlFile(gameResources, m_sceneActor, parentObject, world, Vector2(0.0f, 0.0f), defXmlFileName, initialState);
}

CollisionEntityTypeEnum CompoundObject::getEntityType(void)
{
   return m_collisionType;
}


CompoundObject* CompoundObject::getParentObject()
{
   return m_parentObject;
}

CompoundObject* CompoundObject::getParentWithBehaviour(BehaviourEnum behav)
{
   if (m_behaviourType == behav)
   {
      return this;
   }
   else
   {
      if (m_parentObject)
      {
         return m_parentObject->getParentWithBehaviour(behav);
      }
   }

   return NULL;
}

void CompoundObject::killAllChildBodies(void)
{
   b2Body* myBody = (b2Body*)getUserData();

   spActor child;

   child = getFirstChild();
   
   // Kill all Actor::children
   do
   {
      // getUserData gives a const pointer and static_cast cant handle that
      // so we use a c-type cast
      b2Body* b = ActorUserData::getBody(child->getUserData());

      if (b)
      {
         b->GetWorld()->DestroyBody(b);
      }

      child = child->getNextSibling();
   } while (child);

   // Kill children of all child COs. 
   for (auto it = m_children.begin(); it != m_children.end(); ++it)
   {
      (*it)->killAllChildBodies();
   }
}


b2Vec2 CompoundObject::getCompoundObjectPosition()
// Position of the CO is the position of any of its bodies
// For multi-body object it is not known what body to use
// so we use the first one we encounter
{
   for (auto it = m_children.begin(); it != m_children.end(); ++it)
   {
      b2Body* b = ActorUserData::getBody((*it)->getUserData());

      if (b != NULL)
      {
         return b->GetPosition();
      }
   }

   return b2Vec2(0.0f, 0.0f);
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

      lf->m_behaviourType = BehaviourEnum::leapfrog;

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

      ls->m_behaviourType = BehaviourEnum::launchSite;

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

      lp->m_behaviourType = BehaviourEnum::landingPad;

      return static_cast<CompoundObject*>(lp);
   }
   else if (behavStr == "breakableObject")
   {
      BreakableObject* bo = new BreakableObject(
         gameResources, 
         sceneParent, 
         this, 
         world, 
         pos, 
         root);

      bo->m_behaviourType = BehaviourEnum::breakableObject;

      return static_cast<CompoundObject*>(bo);
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
   // bodies and joints since, during the initialisation of the system,
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
   // Define sprite
   spSprite sprite = new Sprite();
   sprite->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   sprite->setName(objectNode.attribute("name").as_string());
   sprite->setPriority(objectNode.attribute("zLevel").as_int());
   sprite->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   sprite->setAnchor(0.5f, 0.5f);
   sprite->setTouchChildrenEnabled(false);
   Vector2 newPos(pos.x + objectNode.attribute("posX").as_float(), pos.y + objectNode.attribute("posY").as_float());
   sprite->setPosition(newPos);
   sprite->setRotation(objectNode.attribute("angle").as_float());
   sprite->attachTo(sceneParent);
   m_shapes.push_back(sprite.get());
}

void CompoundObject::defineSpritePolygon(
   oxygine::Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode)
{
   // Define sprite, which is a polygon, in this case
   spPolygon sprite = new oxygine::Polygon();
   sprite->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   sprite->setName(objectNode.attribute("name").as_string());
   sprite->setPriority(objectNode.attribute("zLevel").as_int());
   sprite->setTouchChildrenEnabled(false);

   // TODO: What about rotation?
   
   vector<Vector2> vertices(distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));

   PolygonVertices::createSpritePolygon(sprite.get(), vertices, objectNode);

   sprite->attachTo(sceneParent);
   m_shapes.push_back(sprite.get());
}

void CompoundObject::defineCircle(
   oxygine::Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode,
   bool staticBody)
{
   // Define sprite
   spSprite sprite = new Sprite();
   sprite->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   sprite->setName(objectNode.attribute("name").as_string());
   sprite->setPriority(objectNode.attribute("zLevel").as_int());
   sprite->setSize(objectNode.attribute("radius").as_float() * 2.0f, objectNode.attribute("radius").as_float() * 2.0f);
   sprite->setAnchor(0.5f, 0.5f);
   sprite->setTouchChildrenEnabled(false);
   sprite->attachTo(sceneParent);
   m_shapes.push_back(sprite.get());

   b2BodyDef bodyDef;
   if (staticBody)
   {
      bodyDef.type = b2_staticBody;
   }
   else
   {
      bodyDef.type = b2_dynamicBody;
   }
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

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = sprite.get();
   bud->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());

   fixtureDef.userData = bud;

   body->CreateFixture(&fixtureDef);

   body->SetUserData(bud);

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = this;

   sprite->setUserData(aud);
}

void CompoundObject::defineStaticCircle(
   oxygine::Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode)
{
   defineCircle(gameResources, sceneParent, parentObject, world, pos, objectNode, true);
}

void CompoundObject::defineBox(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode,
   bool staticBody)
{

   // Define sprite
   spSprite sprite = new Sprite();
   sprite->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   sprite->setName(objectNode.attribute("name").as_string());
   sprite->setPriority(objectNode.attribute("zLevel").as_int());
   sprite->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   sprite->setAnchor(0.5f, 0.5f);
   sprite->setTouchChildrenEnabled(false);
   sprite->attachTo(sceneParent);
   m_shapes.push_back(sprite.get());

   b2BodyDef bodyDef;
   if (staticBody)
   {
      bodyDef.type = b2_staticBody;
   }
   else
   {
      bodyDef.type = b2_dynamicBody;
   }
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

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = sprite.get();
   bud->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());

   fixtureDef.userData = bud;

   body->CreateFixture(&fixtureDef);

   body->SetUserData(bud);

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = this;

   sprite->setUserData(aud);
}

void CompoundObject::defineStaticBox(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   defineBox(gameResources, sceneParent, parentObject, world, pos, objectNode, true);
}


void CompoundObject::defineStaticPolygon(
   Resources& gameResources, 
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode)
{
   spPolygon sprite = new oxygine::Polygon();
   sprite->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   sprite->setName(objectNode.attribute("name").as_string());
   sprite->setPriority(objectNode.attribute("zLevel").as_int());

   b2Body* body = NULL;
   b2Fixture* fixture = NULL;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   
   vector<Vector2> vertices(distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));

   PolygonVertices::createSpritePolygon(sprite.get(), vertices, objectNode);
   PolygonVertices::createChainBodyPolygon(
      vertices, world, bPos, body, fixture, objectNode);
   PolygonVertices::createPolygonBorders(sprite.get(), gameResources, vertices, objectNode);
   
   sprite->attachTo(sceneParent);
   m_shapes.push_back(sprite.get());

   b2Filter filter;
   filter.categoryBits = objectNode.attribute("collisionCategory").as_int();
   filter.maskBits = objectNode.attribute("collisionMask").as_int();
   fixture->SetFilterData(filter);

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = sprite.get();
   bud->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());

   fixture->SetUserData(bud);
   body->SetUserData(bud);

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = this;

   sprite->setUserData(aud);
}

void CompoundObject::defineBoxedSpritePolygon(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode,
   bool staticBody)
{
   // Define sprite, which is a polygon, in this case

   spSprite sprite = new Sprite();
   sprite->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   sprite->setName(objectNode.attribute("name").as_string());
   sprite->setPriority(objectNode.attribute("zLevel").as_int());
   sprite->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   sprite->setAnchor(0.5f, 0.5f);
   sprite->setTouchChildrenEnabled(false);
   sprite->attachTo(sceneParent);
   m_shapes.push_back(sprite.get());

   vector<Vector2> vertices(distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));
   b2Body* body = NULL;
   b2Fixture* fixture = NULL;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());

   PolygonVertices::readVertices(vertices, objectNode);
   PolygonVertices::createBodyPolygon(vertices, world, bPos, &body, &fixture, objectNode, staticBody);

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = sprite.get();
   bud->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());

   fixture->SetUserData(bud);

   body->SetUserData(bud);

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = this;

   sprite->setUserData(aud);
}

void CompoundObject::defineStaticBoxedSpritePolygon(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   defineBoxedSpritePolygon(gameResources, sceneParent, parentObject, world, pos, objectNode, true);
}

void CompoundObject::defineDynamicCircle(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   defineCircle(gameResources, sceneParent, parentObject, world, pos, objectNode, false);
}

void CompoundObject::defineDynamicBox(
   Resources& gameResources, 
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode)
{
   defineBox(gameResources, sceneParent, parentObject, world, pos, objectNode, false);
}

void CompoundObject::defineDynamicPolygon(
   Resources& gameResources, 
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode)
{
   // Define sprite, which is a polygon, in this case

   spPolygon sprite = new oxygine::Polygon();
   sprite->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   sprite->setName(objectNode.attribute("name").as_string());
   sprite->setPriority(objectNode.attribute("zLevel").as_int());
   sprite->setTouchChildrenEnabled(false);

   vector<Vector2> vertices(distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));
   b2Body* body = NULL;
   b2Fixture* fixture = NULL;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());

   PolygonVertices::createSpritePolygon(sprite.get(), vertices, objectNode);
   PolygonVertices::createBodyPolygon(vertices, world, bPos, &body, &fixture, objectNode, false);

   // Here I could probably generate borders to the sprite by calling PolygonVertices::createPolygonBorder

   sprite->attachTo(sceneParent);
   m_shapes.push_back(sprite.get());

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = sprite.get();
   bud->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());

   fixture->SetUserData(bud);
   body->SetUserData(bud);

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = this;

   sprite->setUserData(aud);

}

void CompoundObject::defineDynamicBoxedSpritePolygon(
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   defineBoxedSpritePolygon(gameResources, sceneParent, parentObject, world, pos, objectNode, false);
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
   // Otherwise all segments would hava a name.
   // This means there is now a way to create a CompoundObject
   // without a childObject element. But I think it is fine.
   CompoundObject* newCo = new CompoundObject(sceneParent, parentObject);

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

   CollisionEntityTypeEnum collType = CollisionEntity::convert(jointNode.attribute("collisionEntity").as_string());

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
   object->attachTo(sceneParent);

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

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = object.get();
   bud->m_collisionType = collType;

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &boxShape;
   fixtureDef.density = jointNode.attribute("density").as_float(1.0f);
   fixtureDef.friction = 1.0f;
   fixtureDef.filter.categoryBits = jointNode.attribute("collisionCategory").as_int();
   fixtureDef.filter.maskBits = jointNode.attribute("collisionMask").as_int();
   fixtureDef.userData = bud;

   firstSegBody->CreateFixture(&fixtureDef);
   
   firstSegBody->SetUserData(bud);

   ActorUserData* aud = new ActorUserData();
   aud->m_body = firstSegBody;
   aud->m_parentCo = this;

   object->setUserData(aud);

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
      object->attachTo(sceneParent);

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

      BodyUserData* bud = new BodyUserData();
      bud->m_actor = object.get();
      bud->m_collisionType = collType;

      b2FixtureDef fixtureDef;
      fixtureDef.shape = &boxShape;
      fixtureDef.density = jointNode.attribute("density").as_float(1.0f);
      fixtureDef.friction = 1.0f;
      fixtureDef.filter.categoryBits = jointNode.attribute("collisionCategory").as_int();
      fixtureDef.filter.maskBits = jointNode.attribute("collisionMask").as_int();
      fixtureDef.userData = bud;

      segBody->CreateFixture(&fixtureDef);
      segBody->SetUserData(bud);

      ActorUserData* aud = new ActorUserData();
      aud->m_body = segBody;
      aud->m_parentCo = this;

      object->setUserData(aud);

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

CompoundObject* CompoundObject::defineChildObject(
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
      return NULL;
   }

   xml_node propNode = stateNode.child("properties");

   if (propNode.empty())
   {
      return NULL;
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
         return NULL;
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

      // CO is alreadu attached to scene parent in the behaviour file
//      co->attachTo(sceneParent);

      co->setName(objectNode.attribute("name").as_string());
   }

   return co;
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

void CompoundObject::splitString(const string& source, string& thisLevel, string& lowerLevels)
{
   thisLevel = source.substr(0, source.find("."));
   if (thisLevel.size() < source.size())
   {
      lowerLevels = source.substr(thisLevel.size() + 1);
   }
}

/*
Used to find pointers to CompoundObjects as children of this compound object.
name string can be hierarcical like so: nameGrandPareent.nameParent.nameChild
If the name string is hierarchical, all but the last name in the string
are CompoundObjects
*/
CompoundObject* CompoundObject::getObject(const std::string& name)
{
   string thisLevel;
   string lowerLevels;

   splitString(name, thisLevel, lowerLevels);

   if (lowerLevels.empty())
   {
      return getObjectImpl(thisLevel);
   }
   else
   {
      CompoundObject* co = getObjectImpl(thisLevel);

      if (co)
      {
         return co->getObject(lowerLevels);
      }
   }

   return NULL;
}

oxygine::Actor* CompoundObject::getActor(const std::string& name)
{
   string thisLevel;
   string lowerLevels;

   splitString(name, thisLevel, lowerLevels);

   if (lowerLevels.empty())
   {
      return getActorImpl(thisLevel);
   }
   else
   {
      CompoundObject* co = getObjectImpl(thisLevel);

      if (co)
      {
         return co->getActor(lowerLevels);
      }
   }

   return NULL;
}

/*
Used to find pointers to b2Bodies as Actor::children of this compound object.
name string can be hierarcical like so: nameGrandPareent.nameParent.nameChild
*/
b2Body* CompoundObject::getBody(const string& name)
{
   string thisLevel;
   string lowerLevels;

   splitString(name, thisLevel, lowerLevels);

   if (lowerLevels.empty())
   {
      return getBodyImpl(thisLevel);
   }
   else
   {
      CompoundObject* co = getObjectImpl(thisLevel);

      if (co)
      {
         return co->getBody(lowerLevels);
      }
   }

   return NULL;
}

b2Joint* CompoundObject::getJoint(const std::string& name)
{
   string thisLevel;
   string lowerLevels;

   splitString(name, thisLevel, lowerLevels);

   if (lowerLevels.empty())
   {
      return getJointImpl(thisLevel);
   }
   else
   {
      CompoundObject* co = getObjectImpl(thisLevel);

      if (co)
      {
         return co->getJoint(lowerLevels);
      }
   }

   return NULL;
}

System* CompoundObject::getSystem(const std::string& name)
{
   string thisLevel;
   string lowerLevels;

   splitString(name, thisLevel, lowerLevels);

   if (lowerLevels.empty())
   {
      return getSystemImpl(thisLevel);
   }
   else
   {
      CompoundObject* co = getObjectImpl(thisLevel);

      if (co)
      {
         return co->getSystem(lowerLevels);
      }
   }

   return NULL;
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
     
   return NULL;
}

oxygine::Actor* CompoundObject::getActorImpl(const std::string& name)
{
   for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it)
   {
      if ((*it)->getName() == name)
      {
         return (*it);
      }
   }

   logs::messageln("Did not find the actor %s that was searched for", name.c_str());

   return NULL;
}

b2Body* CompoundObject::getBodyImpl(const std::string& name)
{
   spActor child = getActorImpl(name);

   if (child)
   {
      // getUserData gives a const pointer and static_cast cant handle that
      // so we use a c-type cast
      return (b2Body*)ActorUserData::getBody(child->getUserData());
   }

   logs::messageln("Did not find the body %s that was searched for", name.c_str());

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

   logs::messageln("Did not find the joint %s that was searched for", name.c_str());

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

   logs::messageln("Did not find the system %s that was searched for", name);

   return NULL;
}

//Sprite* CompoundObject::getSprite(void)
//{
//   return static_cast<Sprite*>(getFirstChild().get());
//}

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

   m_sceneActor->addMeToDeathList((KillableInterface*)this);
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

// Iterate all bodies (and bodies of children recursively)
// And set all bodies to bounding bodies.

void CompoundObject::setAllBodiesToBounding(FreeSpaceActor* actor)
{
   for (auto it = m_children.begin(); it != m_children.end(); ++it)
   {
      b2Body* b = ActorUserData::getBody((*it)->getUserData());

      if ( b != NULL)
      {
         actor->addBoundingBody(b);
      }

      (*it)->setAllBodiesToBounding(actor);
   }
}
