#include <algorithm>

#include "Box2D/Box2D.h"

#include "compoundobject.h"
#include "compoundobjectevents.h"

#include "sceneactor.h"
#include "freespaceactor.h"

// All behaviours of CompoundObjects
// and thus can be instansiated as children to one
// CompoundObject
#include "launchsite.h"
#include "leapfrog.h"

#include "breakableobject.h"
#include "explosiveobject.h"
#include "magneticmine.h"
#include "steerableobject.h"
#include "pickupobject.h"
#include "landingpad.h"
#include "parallaxbackground.h"
#include "planetactor.h"
#include "orbitscene.h"
#include "system.h"

#include "polygonvertices.h"
#include "bodyuserdata.h"
#include "actoruserdata.h"

#include "groupindexsource.h"
using namespace oxygine;
using namespace std;
using namespace pugi;


CompoundObject::CompoundObject(SceneActor* sceneActor, CompoundObject* parentObject) :
   m_sceneActor(sceneActor),
   m_parentObject(parentObject),
   m_collisionType(CollisionEntity::notApplicable),
   m_behaviourType(notApplicable),
   m_isDead(false)
{
   //CompoundObjectCreatedEvent event;
   //dispatchEvent(&event);
}


CompoundObject::~CompoundObject()
{
	//m_boxedSprites.clear();
	//m_polygonSprites.clear();
	//m_staticPolygons.clear();
	m_namedJoints.clear();
	m_children.clear();
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
   b2Body* myBody = ActorUserData::getBody(getUserData());

   spActor child;

   child = getFirstChild();
   
   // Kill all Actor::children
   do
   {
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

void CompoundObject::removeShape(Actor* removeMe)
{
   m_shapes.erase(std::remove(m_shapes.begin(), m_shapes.end(), removeMe), m_shapes.end());
}

 Vector2 CompoundObject::getCompoundObjectPosition()
// Position of the CO is the position of any of its bodies
// For multi-body object it is not known what body to use
// so we use the first one we encounter
{
    Vector2 posSum = Vector2(0.0f, 0.0f);
    int numberOfPos = 0;

   for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it)
   {
      posSum += (*it)->getPosition();
      numberOfPos++;
   }

   if (numberOfPos == 0)
   {
      return Vector2(0.0f, 0.0f);
   }

   Vector2 avgPos = posSum / numberOfPos;

   return avgPos;
}


 CompoundObject* CompoundObject::initCompoundObjectTop(
    oxygine::Resources& gameResources,
    SceneActor* sceneParent,
    CompoundObject* parentObject,
    b2World* world,
    const string& defXmlFileName,
    const string& initialState)
 {
    return readDefinitionXmlFile(gameResources, sceneParent, parentObject, world, Vector2(0.0f, 0.0f), defXmlFileName, initialState);
 }

 CompoundObject* CompoundObject::readDefinitionXmlFile(
   // TODO: This method should not be executed on an existing CO
   // because if the CO has a behaviour one of the behaviour classes
   // that inherits from CO should be created. Therefore this method
   // should return the created object.
   // It reads the file into a buffer and then calls initCompoundObject
   // which does the rest of the job. 
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   const string& fileName,
   const string& initialState)
{
   ox::file::buffer bf;
   ox::file::read(fileName.c_str(), bf);

   xml_document doc;

//   xml_parse_result result = doc.load_file(fileName.c_str());
   xml_parse_result result = doc.load_buffer(&bf.data[0], bf.size());

   xml_node root = doc.child("compoundObject");

   return initCompoundObject(gameResources, sceneParent, parentObject, world, pos, root, initialState);
}

 void CompoundObject::addObjectsFromXmlFile(
    oxygine::Resources& gameResources,
    SceneActor* sceneParent,
    CompoundObject* parentObject,
    b2World* world,
    const std::string& fileName,
    const std::string& initialState)
 {
    ox::file::buffer bf;
    ox::file::read(fileName.c_str(), bf);

    xml_document doc;

    //   xml_parse_result result = doc.load_file(fileName.c_str());
    xml_parse_result result = doc.load_buffer(&bf.data[0], bf.size());

    xml_node root = doc.child("compoundObject");

    initCompoundObjectParts(
       gameResources,
       sceneParent,
       sceneParent,
       parentObject,
       world,
       Vector2(0.0f, 0.0f),
       root,
       string(""),
       0);
 }


CompoundObject* CompoundObject::initCompoundObject(
   // This method creates a new CO and fills it according to the supplied
   // XML node. The created CO is returned.
   // If the CO has a behaviour a behaviour class (which inherits from Compound Object)
   // is created and returned (cast as a CO)
   // Since this method is not called on an existing object it must be static.
   Resources& gameResources,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& root,
   const string& initialState)
{
   // Check if this object needs a groupIndex for collisions
   bool needsGroupIndex = root.attribute("needsGroupIndex").as_bool(false);
   int groupIndex = 0;

   if (needsGroupIndex)
   {
      groupIndex = g_GroupIndexSource.getNewGroupIndex();
   }

   // Look if there is a behaviour attached to the compound object
   string behavStr = root.child("behaviour").attribute("type").as_string();

   // Decode the type string to create the correct type of object
   // but only store the pointer to the CompoundObject
   // Perhaps with some special cases


   if (behavStr == "scene")
   {
      SceneActor* sa = SceneActor::defineScene(
         gameResources,
         parentObject,
         world,
         root,
         initialState,
         groupIndex);

      sa->m_behaviourType = BehaviourEnum::scene;

      return static_cast<CompoundObject*>(sa);
   }
   //else if (behavStr == "parallaxBackground")
   //{
   //   ParallaxBackground* sa = new ParallaxBackground(
   //      gameResources,
   //      sceneParent,
   //      parentObject,
   //      world,
   //      root,
   //      groupIndex);

   //   sa->m_behaviourType = BehaviourEnum::parallaxBackground;

   //   return static_cast<CompoundObject*>(sa);
   //}
   else if (behavStr == "orbitScene")
   {
      OrbitScene* sa = new OrbitScene(
         gameResources,
         parentObject,
         world,
         root,
         initialState,
         groupIndex);

      sa->m_behaviourType = BehaviourEnum::orbitScene;

      return static_cast<CompoundObject*>(sa);
   }

   else if (behavStr == "leapfrog")
   {
      LeapFrog* lf = new LeapFrog(
         gameResources,
         sceneParent,
         parentObject,
         world,
         pos,
         root,
         groupIndex);

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
         root,
         groupIndex);

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
         root,
         groupIndex);

      lp->m_behaviourType = BehaviourEnum::landingPad;

      return static_cast<CompoundObject*>(lp);
   }
   else if (behavStr == "breakableObject")
   {
      BreakableObject* bo = new BreakableObject(
         gameResources, 
         sceneParent, 
         parentObject,
         world, 
         pos, 
         root,
         groupIndex);

      bo->m_behaviourType = BehaviourEnum::breakableObject;
      bo->initGameStatus(new GameStatus());

      return static_cast<CompoundObject*>(bo);
   }
   else if (behavStr == "explosiveObject")
   {
      ExplosiveObject* bo = new ExplosiveObject(
         gameResources,
         sceneParent,
         parentObject,
         world,
         pos,
         root,
         root.child("behaviour").child("explosiveObjectProperties"),
         groupIndex);

      bo->m_behaviourType = BehaviourEnum::explosiveObject;

      return static_cast<CompoundObject*>(bo);
   }
   else if (behavStr == "magneticMine")
   {
      MagneticMine* bo = new MagneticMine(
         gameResources,
         sceneParent,
         parentObject,
         world,
         pos,
         root,
         root.child("behaviour").child("magneticMineProperties"),
         groupIndex);

      bo->m_behaviourType = BehaviourEnum::magneticMine;

      return static_cast<CompoundObject*>(bo);
   }
   else if (behavStr == "steerableObject")
   {
      SteerableObject* so = new SteerableObject(
         gameResources,
         sceneParent,
         parentObject,
         world,
         pos,
         root,
         groupIndex);

      so->m_behaviourType = BehaviourEnum::steerableObject;
      so->initGameStatus(new GameStatus());

      return static_cast<CompoundObject*>(so);
   }
   else if (behavStr == "pickupObject")
   {
      PickupObject* pu = new PickupObject(
         gameResources,
         sceneParent,
         parentObject,
         world,
         pos,
         root,
         groupIndex);

      pu->m_behaviourType = BehaviourEnum::pickup;

      return static_cast<CompoundObject*>(pu);
   }
   else
   {
      // If no behaviour was recognised, this CO is without behaviour.
      // Lets create it and then fill in its parts.
      CompoundObject* co = new CompoundObject(sceneParent, parentObject);
      co->initCompoundObjectParts(
         gameResources,
         sceneParent,
         sceneParent,
         parentObject,
         world,
         pos,
         root,
         string(""),
         groupIndex);

      return co;
   }

   return NULL;
}

bool CompoundObject::initCompoundObjectParts(
   oxygine::Resources& gameResources,
   oxygine::Actor* parentActor,
   SceneActor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   const pugi::xml_node& root,
   const string& initialState,
   int groupIndex,
   bool parentIsScene)
{
   // When defining childObjects that read from other XML-document
   // we somehow loose the pointer in this file,
   // Therefore we must do the trick of save the root-node
   // to a new xml-document in memory and go back to it later.

   xml_document* nodeDocHolder = new xml_document();

   nodeDocHolder->append_copy(root);
   xml_node savedNode = nodeDocHolder->child("compoundObject");
   
   for (auto it = savedNode.children("childObject").begin();
      it != savedNode.children("childObject").end();
      ++it)
   {
      defineChildObject(gameResources, sceneParent, this, world, pos, *it, initialState);
   }

   for (auto it = savedNode.children("spriteBox").begin();
      it != savedNode.children("spriteBox").end();
      ++it)
   {
      // define a object
      defineSpriteBox(gameResources, parentActor, this, pos, *it, parentIsScene);
   }

   for (auto it = savedNode.children("spritePolygon").begin();
      it != savedNode.children("spritePolygon").end();
      ++it)
   {
      // define a object
      defineSpritePolygon(gameResources, parentActor, this, pos, *it, parentIsScene);
   }

   for (auto it = savedNode.children("staticCircle").begin();
      it != savedNode.children("staticCircle").end();
      ++it)
   {
      // define a object
      defineStaticCircle(gameResources, parentActor, this, world, pos, *it, groupIndex);
   }

   for (auto it = savedNode.children("staticBox").begin();
      it != savedNode.children("staticBox").end();
      ++it)
   {
      // define a object
      defineStaticBox(gameResources, parentActor, this, world, pos, *it, groupIndex);
   }

   for (auto it = savedNode.children("staticPolygon").begin();
      it != savedNode.children("staticPolygon").end();
      ++it)
   {
      // define a object
      defineStaticPolygon(gameResources, parentActor, this, world, pos, *it, groupIndex);
   }

   for (auto it = savedNode.children("staticBoxedSpritePolygonBody").begin();
      it != savedNode.children("staticBoxedSpritePolygon").end();
      ++it)
   {
      defineStaticBoxedSpritePolygon(gameResources, parentActor, this, world, pos, *it, groupIndex);
   }

   for (auto it = savedNode.children("dynamicCircle").begin();
      it != savedNode.children("dynamicCircle").end();
      ++it)
   {
      // define a box object
      defineDynamicCircle(gameResources, parentActor, this, world, pos, *it, groupIndex);
   }

   for (auto it = savedNode.children("dynamicBox").begin();
      it != savedNode.children("dynamicBox").end();
      ++it)
   {
      // define a box object
      defineDynamicBox(gameResources, parentActor, this, world, pos, *it, groupIndex);
   }

   for (auto it = savedNode.children("dynamicPolygon").begin();
      it != savedNode.children("dynamicPolygon").end();
      ++it)
   {
      // define a polygon object
      string a = (*it).attribute("name").as_string();
      defineDynamicPolygon(gameResources, parentActor, this, world, pos, *it, groupIndex);
   }

   for (auto it = savedNode.children("dynamicBoxedSpritePolygonBody").begin();
      it != savedNode.children("dynamicBoxedSpritePolygonBody").end();
      ++it)
   {
      defineDynamicBoxedSpritePolygon(gameResources, parentActor, this, world, pos, *it, groupIndex);
   }

   for (auto it = savedNode.children("rope").begin();
      it != savedNode.children("rope").end();
      ++it)
   {
      defineRope(gameResources, parentActor, sceneParent, this, world, pos, *it);
   }

   for (auto it = savedNode.children("weldJoint").begin();
      it != savedNode.children("weldJoint").end();
      ++it)
   {
      defineWeldJoint(world, *it);
   }

   for (auto it = savedNode.children("revoluteJoint").begin();
      it != savedNode.children("revoluteJoint").end();
      ++it)
   {
      defineRevoluteJoint(world, *it);
   }

   for (auto it = savedNode.children("prismaticJoint").begin();
      it != savedNode.children("prismaticJoint").end();
      ++it)
   {
      definePrismaticJoint(world, *it);
   }

   // It is important that systems are iterated after all 
   // bodies and joints since, during the initialisation of the system,
   // references to such objects are searchd for. 
   for (auto it = savedNode.children("system").begin();
      it != savedNode.children("system").end();
      ++it)
   {
      string systemType = it->attribute("type").as_string();
      string systemName = it->attribute("name").as_string();

      System* sys = System::initialiseSystemNode(&gameResources, m_sceneActor, world, this, systemType, systemName, *it, groupIndex);

      // Is it really important to remember the systems here?
      m_systems.push_back(sys);
   }

   return true;
}

void CompoundObject::doCommonShapeDefinitions(
   Resources& gameResources,
   Sprite* sprite,
   xml_node& objectNode)
{
   sprite->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   sprite->setName(objectNode.attribute("name").as_string());
   sprite->setPriority(objectNode.attribute("zLevel").as_int());
   sprite->setTouchChildrenEnabled(false);
}

void CompoundObject::doCollisionDefinitions(
   b2Fixture*& fixture,
   xml_node& objectNode,
   int groupIndex)
{
   b2Filter filter;
   filter.categoryBits = objectNode.attribute("collisionCategory").as_int();
   filter.maskBits = objectNode.attribute("collisionMask").as_int();
   bool collideWithinGroup = objectNode.attribute("collideWithGroup").as_bool(false);
   if (collideWithinGroup)
   {
      filter.groupIndex = groupIndex;
   }
   else
   {
      filter.groupIndex = -groupIndex;
   }
   fixture->SetFilterData(filter);
}

void CompoundObject::doPhysicalDefinitions(
   b2Fixture*& fixture,
   xml_node& objectNode)
{
   fixture->SetDensity(objectNode.attribute("density").as_float(1.0f));
   fixture->SetFriction(objectNode.attribute("friction").as_float(1.0f));
}



// A SpriteBox can be part of a parallax background. This is the case if 
// the parallaxAmount is higher than 0 and the parent object is a CO with
// behaviour set to scene. 
void CompoundObject::defineSpriteBox(
   oxygine::Resources& gameResources,
   Actor* parentActor,
   CompoundObject* parentObject,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode,
   bool parentIsScene)
{
   // Define sprite
   spSprite sprite = new Sprite();
   doCommonShapeDefinitions(gameResources, sprite.get(), objectNode);
   Vector2 newPos(pos.x + objectNode.attribute("posX").as_float(), pos.y + objectNode.attribute("posY").as_float());
   sprite->setPosition(newPos);
   sprite->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   sprite->setRotation(objectNode.attribute("angle").as_float() * MATH_PI / 180.0f);
   sprite->setAnchor(0.5f, 0.5f);
   m_shapes.push_back(sprite.get());


   if (parentIsScene)
   {
      float parallaxAmount = objectNode.attribute("parallaxAmount").as_float(0.0f);

      if (parallaxAmount > 0.0f)
      {
         // We now know that parentObject is a scene and can thus cast it to this
         SceneActor* sa = static_cast<SceneActor*>(parentObject);
         sa->addParallaxBackgroundSprite(sprite, parallaxAmount);
         return;
      }
   }

   // If we get here, the parent wasn't a scene or the parallax amount was zero.
   // In this case, we attach the sprite to the parent actor in the normal way.
   sprite->attachTo(parentActor);
}

void CompoundObject::defineSpritePolygon(
   oxygine::Resources& gameResources,
   Actor* parentActor,
   CompoundObject* parentObject,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode,
   bool parentIsScene)
{
   // Define sprite, which is a polygon, in this case
   spPolygon sprite = new oxygine::Polygon();

   doCommonShapeDefinitions(gameResources, sprite.get(), objectNode);

   // TODO: What about rotation?

   // TODO: And what about position?
   Vector2 newPos(pos.x + objectNode.attribute("posX").as_float(), pos.y + objectNode.attribute("posY").as_float());
   sprite->setPosition(newPos);

   vector<Vector2> vertices(distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));

   PolygonVertices::createSpritePolygon(sprite.get(), vertices, objectNode);

   m_shapes.push_back(sprite.get());

   if (parentIsScene)
   {
      float parallaxAmount = objectNode.attribute("parallaxAmount").as_float(0.0f);

      if (parallaxAmount > 0.0f)
      {
         // We now know that parentObject is a scene and can thus cast it to this
         SceneActor* sa = static_cast<SceneActor*>(parentObject);
         sa->addParallaxBackgroundSprite(sprite, parallaxAmount);
         return;
      }
   }

   // If we get here, the parent wasn't a scene or the parallax amount was zero.
   // In this case, we attach the sprite to the parent actor in the normal way.
   sprite->attachTo(parentActor);

}

void CompoundObject::defineCircle(
   oxygine::Resources& gameResources,
   Actor* parentActor,
   CompoundObject* parentObject,
   b2World* world,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode,
   int groupIndex,
   bool staticBody)
{
   // Define sprite
   spSprite sprite = new Sprite();
   doCommonShapeDefinitions(gameResources, sprite.get(), objectNode);
   sprite->setSize(objectNode.attribute("radius").as_float() * 2.0f, objectNode.attribute("radius").as_float() * 2.0f);
   sprite->setAnchor(0.5f, 0.5f);
   sprite->attachTo(parentActor);
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
   bodyDef.angle = objectNode.attribute("angle").as_float() * MATH_PI / 180.0f;
   b2Body* body = world->CreateBody(&bodyDef);

   b2CircleShape circleShape;
   circleShape.m_radius = objectNode.attribute("radius").as_float();

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &circleShape;

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = sprite.get();
   bud->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());

   fixtureDef.userData = bud;

   b2Fixture* fixture = body->CreateFixture(&fixtureDef);

   doCollisionDefinitions(fixture, objectNode, groupIndex);
   
   if (!staticBody)
   {
      doPhysicalDefinitions(fixture, objectNode);
   }

   body->SetUserData(bud);

   body->ResetMassData();

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = this;

   sprite->setUserData(aud);
}

void CompoundObject::defineStaticCircle(
   oxygine::Resources& gameResources,
   Actor* parentActor,
   CompoundObject* parentObject,
   b2World* world,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode,
   int groupIndex)
{
   defineCircle(gameResources, parentActor, parentObject, world, pos, objectNode, groupIndex, true);
}

void CompoundObject::defineBox(
   Resources& gameResources,
   Actor* parentActor,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode,
   int groupIndex,
   bool staticBody)
{

   // Define sprite
   spSprite sprite = new Sprite();
   doCommonShapeDefinitions(gameResources, sprite.get(), objectNode);

   sprite->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   sprite->setAnchor(0.5f, 0.5f);
   sprite->attachTo(parentActor);
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
   bodyDef.angle = objectNode.attribute("angle").as_float() * MATH_PI / 180.0f;
   b2Body* body = world->CreateBody(&bodyDef);

   b2PolygonShape boxShape;
   boxShape.SetAsBox(objectNode.attribute("width").as_float() / 2.0f, objectNode.attribute("height").as_float() / 2.0f);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &boxShape;

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = sprite.get();
   bud->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());

   fixtureDef.userData = bud;

   b2Fixture* fixture = body->CreateFixture(&fixtureDef);

   doCollisionDefinitions(fixture, objectNode, groupIndex);

   if (!staticBody)
   {
      doPhysicalDefinitions(fixture, objectNode);
   }

   body->SetUserData(bud);

   body->ResetMassData();

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = this;

   sprite->setUserData(aud);
}

void CompoundObject::defineStaticBox(
   Resources& gameResources,
   Actor* parentActor,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode,
   int groupIndex)
{
   defineBox(gameResources, parentActor, parentObject, world, pos, objectNode, groupIndex, true);
}


void CompoundObject::defineStaticPolygon(
   Resources& gameResources, 
   Actor* parentActor,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode,
   int groupIndex)
{
   spPolygon sprite = new oxygine::Polygon();

   doCommonShapeDefinitions(gameResources, sprite.get(), objectNode);

   b2Body* body = NULL;
   b2Fixture* fixture = NULL;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   
   vector<Vector2> vertices(distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));

   PolygonVertices::createSpritePolygon(sprite.get(), vertices, objectNode);
   PolygonVertices::createChainBodyPolygon(
      vertices, world, bPos, body, fixture, objectNode);

   PolygonVertices::createPolygonBorders(sprite.get(), gameResources, vertices, objectNode);
   
   sprite->attachTo(parentActor);
   m_shapes.push_back(sprite.get());

   doCollisionDefinitions(fixture, objectNode, groupIndex);

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = sprite.get();
   bud->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());

   fixture->SetUserData(bud);
   body->SetUserData(bud);

   body->ResetMassData();

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = this;

   sprite->setUserData(aud);
}

void CompoundObject::defineBoxedSpritePolygon(
   Resources& gameResources,
   Actor* parentActor,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode,
   int groupIndex,
   bool staticBody)
{
   // Define sprite, which is a polygon, in this case

   spSprite sprite = new Sprite();
   doCommonShapeDefinitions(gameResources, sprite.get(), objectNode);

   sprite->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   sprite->setAnchor(0.5f, 0.5f);
   sprite->attachTo(parentActor);
   m_shapes.push_back(sprite.get());

   vector<Vector2> vertices(distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));
   b2Body* body = NULL;
   b2Fixture* fixture = NULL;
   b2Vec2 delta = b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + delta;

   PolygonVertices::readVertices(vertices, objectNode);
   PolygonVertices::createBodyPolygon(vertices, world, bPos, &body, &fixture, objectNode, staticBody, groupIndex);

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = sprite.get();
   bud->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());

   doCollisionDefinitions(fixture, objectNode, groupIndex);

   if (!staticBody)
   {
      doPhysicalDefinitions(fixture, objectNode);
   }

   fixture->SetUserData(bud);
   body->SetUserData(bud);

   body->ResetMassData();

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = this;

   sprite->setUserData(aud);
}

void CompoundObject::defineStaticBoxedSpritePolygon(
   Resources& gameResources,
   Actor* parentActor,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode,
   int groupIndex)
{
   defineBoxedSpritePolygon(gameResources, parentActor, parentObject, world, pos, objectNode, groupIndex, true);
}

void CompoundObject::defineDynamicCircle(
   Resources& gameResources,
   Actor* parentActor,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode,
   int groupIndex)
{
   defineCircle(gameResources, parentActor, parentObject, world, pos, objectNode, groupIndex, false);
}

void CompoundObject::defineDynamicBox(
   Resources& gameResources, 
   Actor* parentActor,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode,
   int groupIndex)
{
   defineBox(gameResources, parentActor, parentObject, world, pos, objectNode, groupIndex, false);
}

void CompoundObject::defineDynamicPolygon(
   Resources& gameResources, 
   Actor* parentActor,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode,
   int groupIndex)
{
   // Define sprite, which is a polygon, in this case

   spPolygon sprite = new oxygine::Polygon();
   doCommonShapeDefinitions(gameResources, sprite.get(), objectNode);

   vector<Vector2> vertices(distance(objectNode.child("vertices").children("vertex").begin(), objectNode.child("vertices").children("vertex").end()));
   b2Body* body = NULL;
   b2Fixture* fixture = NULL;
   b2Vec2 bPos = PhysDispConvert::convert(pos, 1.0f) + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());

   PolygonVertices::createSpritePolygon(sprite.get(), vertices, objectNode);
   PolygonVertices::createBodyPolygon(vertices, world, bPos, &body, &fixture, objectNode, false, groupIndex);

   // Here I could probably generate borders to the sprite by calling PolygonVertices::createPolygonBorder

   sprite->attachTo(parentActor);
   m_shapes.push_back(sprite.get());

   BodyUserData* bud = new BodyUserData();
   bud->m_actor = sprite.get();
   bud->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());

   doCollisionDefinitions(fixture, objectNode, groupIndex);

   doPhysicalDefinitions(fixture, objectNode);

   fixture->SetUserData(bud);
   body->SetUserData(bud);

   body->ResetMassData();

   ActorUserData* aud = new ActorUserData();
   aud->m_body = body;
   aud->m_parentCo = this;

   sprite->setUserData(aud);

}

void CompoundObject::defineDynamicBoxedSpritePolygon(
   Resources& gameResources,
   Actor* parentActor,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode,
   int groupIndex)
{
   defineBoxedSpritePolygon(gameResources, parentActor, parentObject, world, pos, objectNode, groupIndex, false);
}

void CompoundObject::defineRope(
   Resources& gameResources,
   Actor* parentActor,
   SceneActor* sceneActor,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& jointNode)
{
   // The whole rope is a CO, all segments attach to it.
   // Otherwise all segments would hava a name.
   // This means there is now a way to create a CompoundObject
   // without a childObject element. But I think it is fine.
   CompoundObject* newCo = new CompoundObject(sceneActor, parentObject);

   newCo->setName(jointNode.attribute("name").as_string());
   int zLevel = jointNode.attribute("zLevel").as_int();
   //newCo->setPriority(jointNode.attribute("zLevel").as_int());
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
   object->setPriority(zLevel);
   object->setTouchChildrenEnabled(false);
   b2Vec2 segmentPos = startPos + segmentLength * 0.5f * ropeDirection;
   object->setPosition(PhysDispConvert::convert(segmentPos, 1.0f));
   object->setRotation(ropeAngle);
   object->attachTo(parentActor);

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
   //fixtureDef.filter.categoryBits = jointNode.attribute("collisionCategory").as_int();
   //fixtureDef.filter.maskBits = jointNode.attribute("collisionMask").as_int();
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
      object->setPriority(zLevel);
      object->setTouchChildrenEnabled(false);
      b2Vec2 segmentPos = startPos + segmentLength * (0.5f + i) * ropeDirection;
      object->setPosition(PhysDispConvert::convert(segmentPos, 1.0f));
      object->setRotation(ropeAngle);
      object->attachTo(parentActor);

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
      //fixtureDef.filter.categoryBits = jointNode.attribute("collisionCategory").as_int();
      //fixtureDef.filter.maskBits = jointNode.attribute("collisionMask").as_int();
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

   newCo->attachTo(parentActor);

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

      // Create and define child by spreading the node tree 
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
      // Create and define the child by reading the file
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

      // CO is already attached to scene parent in the behaviour file
//      co->attachTo(sceneParent);

      co->setName(objectNode.attribute("name").as_string());
   }

   return co;
}

bool CompoundObject::getStateNode(xml_node& objectNode, const string& initialState, xml_node& stateNode)
{
   // Iterate the stateProperties of the node, looking for state attributes
   // that matches the supplied initialState. If initialState is empty,
   // the first stateProperty is used. If no match, look again but this 
   // time, return if a stateProperty with state: "default" exists

   for (auto it = objectNode.children("stateProperties").begin(); it != objectNode.children("stateProperties").end(); ++it)
   {
      if ((initialState == "") || (it->attribute("state").as_string() == initialState))
      {
         stateNode = *it;
         return true;
      }
   }

   for (auto it = objectNode.children("stateProperties").begin(); it != objectNode.children("stateProperties").end(); ++it)
   {
      string s = it->attribute("state").as_string();
      if (s == "default")
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

oxygine::spActor CompoundObject::getActor(const std::string& name)
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

oxygine::spActor CompoundObject::getActorImpl(const std::string& name)
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

   logs::messageln("Did not find the system %s that was searched for", name.c_str());

   return NULL;
}

//Sprite* CompoundObject::getSprite(void)
//{
//   return static_cast<Sprite*>(getFirstChild().get());
//}



// This method allows a object that needs to refere to 
// a foreign object (i.e. a shape, body or system that 
// is defined in another CompoundObject and may not 
// have been read at the creation time
// This is called after all objects are defined
// The base implementation simply calls the 
// same function to all its child objects.
// Any CO that need foreign accesses must override
// this to find the object or body
void CompoundObject::connectToForeignObjects(void)
{
   for (auto it = m_children.begin(); it != m_children.end(); ++it)
   {
      (*it)->connectToForeignObjects();
   }
}

// This method gets a link to a game status object
// This is the base version. Override this to 
// link individual resources to the associated 
// systems. So that the game status gets a link
// to the Shield system object.
void CompoundObject::initGameStatus(spGameStatus status)
{
   m_gameStatus = status;
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

ObjectProperty* CompoundObject::getObjectProperty(int propId)
{
   return getProp(propId);
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
   PropertyEventTrigger::TriggerType triggerType, 
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

void CompoundObject::addMeToDeathList(void)
{
   for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it)
   {
      m_sceneActor->addMeToDeathList(*it);
   }

   for (auto it = m_systems.begin(); it != m_systems.end(); ++it)
   {
      m_sceneActor->addMeToDeathList(*it);
   }

   m_sceneActor->addMeToDeathList(this);
}


void CompoundObject::hitByBullet(b2Contact* contact)
{
   m_isDead = true;
   m_sceneActor->addMeToDeathList(this);
}

// Iterate all bodies (and bodies of children recursively)
// And set all bodies to bounding bodies.

void CompoundObject::setAllBodiesToBounding(FreeSpaceActor* actor)
{
   for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it)
   {
      b2Body* b = ActorUserData::getBody((*it)->getUserData());

      if ( b != NULL)
      {
         actor->addBoundingBody(b);
      }
   }

   for (auto it = m_children.begin(); it != m_children.end(); ++it)
   {
      (*it)->setAllBodiesToBounding(actor);
   }
}

// Base implementation of this does nothing. Overloaded 
// can optionally implement a specilised version of this
void CompoundObject::registerToMap(void) 
{
   return;
}
