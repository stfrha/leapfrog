#include <algorithm>

// All objects that are derived from CompoundObject
// and thus can be instansiated as children to one
// CompoundObject
#include "launchsite.h"
#include "leapfrog.h"
#include "landingpad.h"
#include "asteroidfield.h"
#include "planetactor.h"
#include "orbitwindow.h"

#include "polygonvertices.h"

#include "compoundobject.h"

using namespace oxygine;
using namespace std;
using namespace pugi;

PropertyEventTrigger::PropertyEventTrigger() :
   m_eventId(0),
	m_triggerType(ignore),
	m_upperLimit(0.0f),
	m_lowerLimit(0.0f)
{
}

PropertyEventTrigger::PropertyEventTrigger(int eventId, TriggerType triggerType, float upperLimit, float lowerLimit) :
   m_eventId(eventId),
   m_triggerType(triggerType),
	m_upperLimit(upperLimit),
	m_lowerLimit(lowerLimit)
{
}

bool PropertyEventTrigger::evaluateTrigger(float value)
{
	if (m_triggerType == insideRange)
	{
		return ((value >= m_lowerLimit) && (value <= m_upperLimit));
	}
	else if (m_triggerType == outsideRange)
	{
		return ((value < m_lowerLimit) || (value > m_upperLimit));
	}

	return false;
}

int PropertyEventTrigger::getEventId(void)
{
   return m_eventId;
}


ObjectProperty::ObjectProperty() :
   m_object(NULL),
   m_extSetPropEvent(NULL),
   m_extReadOnly(false),
	m_value(0.0f),
	m_id(0)
{
}

ObjectProperty::ObjectProperty(
   CompoundObject* obj, 
   Event* extSetPropEvent,
   int id,
   float value, 
   bool readOnly) :
   m_object(obj),
   m_extSetPropEvent(extSetPropEvent),
   m_extReadOnly(readOnly),
   m_value(value),
	m_id(id)
{
}

ObjectProperty::~ObjectProperty()
{
	m_eventTriggers.clear();
	m_dualEventTriggers.clear();
}

void ObjectProperty::setProperty(float value)
{
	m_value = value;

   // Handeling an even may cause it to be removed as a trigger from this 
   // list which would invalidate the iterator. Therefore we build a list of
   // all events to be sent and send them after the loop
   vector<ObjectPropertyTriggeredEvent> m_eventList;

	for (auto it = m_eventTriggers.begin(); it != m_eventTriggers.end(); ++it)
	{
		if (it->evaluateTrigger(value))
		{
         // Trigger event
         m_eventList.push_back(ObjectPropertyTriggeredEvent(it->getEventId(), m_id, value));
		}		
	}

   for (auto it = m_eventList.begin(); it != m_eventList.end(); ++it)
   {
      m_object->dispatchEvent(&(*it));
   }

	for (auto it = m_dualEventTriggers.begin(); it != m_dualEventTriggers.end(); ++it)
	{
		(*it)->updateProperty(m_id);
	}
}

// This is used to set properties from outside of the object that 
// owns the property. 
void ObjectProperty::extSetProperty(float value)
{
   if (!m_extReadOnly)
   {
      setProperty(value);

      //test if event is set, and only dispatch event in that case
      if (m_extSetPropEvent)
      {
         m_object->dispatchEvent(m_extSetPropEvent);
      }
   }
}

float ObjectProperty::getProperty(void)
{
	return m_value;
}

void ObjectProperty::registerPropertyEventTrigger(
   int eventId,
	PropertyEventTrigger::TriggerType triggerType, 
	float lower, 
	float upper)
{
	m_eventTriggers.push_back(PropertyEventTrigger(eventId, triggerType, upper, lower));
}

void ObjectProperty::unregisterPropertyEventTrigger(int eventId)
{
   // iterate all triggers and remove those the has eventId
   for (auto it = m_eventTriggers.begin(); it != m_eventTriggers.end();)
   {
      if (it->getEventId() == eventId)
      {
         it = m_eventTriggers.erase(it);
      }
      else
      {
         ++it;
      }
   }
}


void ObjectProperty::registerDualPropEventTrigger( DualPropEventTrigger* trigger)
{
	m_dualEventTriggers.push_back(trigger);
}

void ObjectProperty::unregisterDualPropEventTrigger(DualPropEventTrigger* trigger)
{
	m_dualEventTriggers.erase(
		remove(
			m_dualEventTriggers.begin(), 
			m_dualEventTriggers.end(), trigger), 
		m_dualEventTriggers.end());
}

DualPropEventTrigger::DualPropEventTrigger() :
   m_eventId(0),
	m_prop1Id(0),
	m_prop2Id(0),
	m_prop1Trigger(0, PropertyEventTrigger::ignore, 0.0f, 0.0f),
	m_prop2Trigger(0, PropertyEventTrigger::ignore, 0.0f, 0.0f)
{
}

DualPropEventTrigger::DualPropEventTrigger(
   int eventId,
   int prop1Id, 
   int prop2Id, 
   PropertyEventTrigger& prop1Trigger, 
   PropertyEventTrigger& prop2Trigger) :
   m_eventId(eventId),
	m_prop1Id(prop1Id),
	m_prop2Id(prop2Id),
	m_prop1Trigger(prop1Trigger),
	m_prop2Trigger(prop2Trigger)
{

}

void DualPropEventTrigger::updateProperty(int propId)
{
	//if (propId == m_prop1Id)
	//{
	//	if (m_prop1Trigger.evaluateTrigger(m_))
	//	{
	//		// use pointer to CompoundObject to read value of the 
	//		// prop2Id into p2Val
	//		float p2Val /* = compoundObject->getValue(m_prop2Id)*/;
	//		if (m_prop2Trigger.evaluateTrigger(p2Val))
	//		{
	//			// Trigger m_eventId

	//		}
	//	}
	//}
	//else if (propId == m_prop2Id)
	//{
	//	if (m_prop2Trigger.evaluateTrigger(propId))
	//	{
	//		// use pointer to CompoundObject to read value of the 
	//		// prop1Id into p1Val
	//		float p1Val /* = compoundObject->getValue(m_prop1Id)*/;
	//		if (m_prop1Trigger.evaluateTrigger(p1Val))
	//		{
 //           // Trigger m_eventId
	//		}
	//	}
	//}
}

CompoundObject::CompoundObject() :
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
   Actor* sceneParent, 
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   string& defXmlFileName,
   string& initialState)
{
   readDefinitionXmlFile(gameResources, sceneParent, parentObject, world, pos, defXmlFileName, initialState);
}

CollisionEntityTypeEnum CompoundObject::getEntityType(void)
{
   return m_collisionType;
}

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
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   string& fileName,
   string& initialState)
{
   m_parentObject = parentObject;

   xml_document doc;

   xml_parse_result result = doc.load_file(fileName.c_str());

   xml_node root = doc.child("compoundObject");

   return initCompoundObject(gameResources, sceneParent, parentObject, world, pos, root, initialState);
}

CompoundObject* CompoundObject::initCompoundObject(
   oxygine::Resources& gameResources,
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   pugi::xml_node& root,
   string& initialState)
{
   // Decode the type string to create the correct type of object
   // but only store the pointer to the CompoundObject
   // Perhaps with some special cases
   string type = root.attribute("type").as_string();

   if (type == "leapfrog")
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
   else if (type == "launchSite")
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
   else if (type == "landingPad")
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
   //else if (type == "asteroidField")
   //{
   //   AsteroidField* af = new AsteroidField(
   //      gameResources,
   //      sceneParent,
   //      parentObject,
   //      world,
   //      root);
   //
   //   return static_cast<CompoundObject*>(af);
   //}
   //else if (type == "planetActor")
   //{
   //   PlanetActor* pa = new PlanetActor(
   //      gameResources,
   //      sceneParent,
   //      parentObject,
   //      root);

   //   return static_cast<CompoundObject*>(pa);
   //}
   //else if (type == "clippedWindow")
   //{
   //   OrbitWindow* ow = new OrbitWindow(
   //      gameResources,
   //      sceneParent,
   //      parentObject,
   //      root,
   //      initialState);

   //   return static_cast<CompoundObject*>(ow);
   //}
   else
   {
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
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   pugi::xml_node& root,
   string& initialState)
{

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

   for (auto it = root.children("ChildCompoundObjectRef").begin();
      it != root.children("ChildCompoundObjectRef").end();
      ++it)
   {
      defineChildObject(gameResources, sceneParent, this, world, pos, *it, initialState);
   }

   for (auto it = root.children("CompoundObjectSystemRef").begin();
      it != root.children("CompoundObjectSystemRef").end();
      ++it)
   {
      defineObjectSystem(gameResources, sceneParent, this, world, pos, *it, initialState);
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

   return true;
}

void CompoundObject::defineSpriteBox(
   oxygine::Resources& gameResources,
   oxygine::Actor* sceneParent,
   CompoundObject* parentObject,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject();

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
   oxygine::Actor* sceneParent,
   CompoundObject* parentObject,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject();

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
   oxygine::Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const oxygine::Vector2& pos,
   pugi::xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject();

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
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject();

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
   Actor* sceneParent, 
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject();

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
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject();

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   // Define sprite, which is a polygon, in this case
   vector<Vector2> vertices;
   vector<VectorT3<int> > triangles;

   spSprite object = new Sprite();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));

   object->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   //   object->setAnchor(Vector2(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float()));
   object->setAnchor(0.5f, 0.5f);
   object->setTouchChildrenEnabled(false);

   object->attachTo(newCo);

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
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject();

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

   body->SetUserData(newCo);

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineDynamicBox(
   Resources& gameResources, 
   Actor* sceneParent, 
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject();

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

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineDynamicPolygon(
   Resources& gameResources, 
   Actor* sceneParent, 
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject();

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


   //object->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   //object->setAnchor(Vector2(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float()));
   object->setTouchChildrenEnabled(false);

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

   body->SetUserData(newCo);

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineDynamicBoxedSpritePolygon(
   Resources& gameResources,
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode)
{
   CompoundObject* newCo = new CompoundObject();

   newCo->setName(objectNode.attribute("name").as_string());
   newCo->setPriority(objectNode.attribute("zLevel").as_int());
   newCo->m_parentObject = parentObject;

   // Define sprite, which is a polygon, in this case
   vector<Vector2> vertices;
   vector<VectorT3<int> > triangles;

   spSprite object = new Sprite();
   object->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));

   object->setSize(objectNode.attribute("width").as_float(), objectNode.attribute("height").as_float());
   //   object->setAnchor(Vector2(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float()));
   object->setAnchor(0.5f, 0.5f);
   object->setTouchChildrenEnabled(false);

   object->attachTo(newCo);

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

   body->SetUserData(newCo);

   newCo->setUserData(body);
   newCo->m_collisionType = CollisionEntity::convert(objectNode.attribute("collisionEntity").as_string());
   newCo->attachTo(sceneParent);

   m_children.push_back(newCo);
}

void CompoundObject::defineChildObject(
   Resources& gameResources, 
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos, 
   xml_node& objectNode,
   string& initialState)
{
   // Iterate the stateProperties of the node, looking for state attributes
   // that matches the supplied initialState. If initialState is empty,
   // the first statePropertý is used
   xml_node* stateNode = NULL;

   for (auto it = objectNode.children("stateProperties").begin(); it != objectNode.children("stateProperties").end(); ++it)
   {
      if ((initialState == "") || (it->attribute("state").as_string() == initialState))
      {
         stateNode = &(*it);
         break;
      }
   }

   if (!stateNode)
   {
      return;
   }

   Vector2 objPos = Vector2(
      stateNode->attribute("posX").as_float(),
      stateNode->attribute("posY").as_float());

   objPos += pos;

   string fileName = stateNode->attribute("file").as_string();

   // Read the child XML file
   CompoundObject* co = readDefinitionXmlFile(
      gameResources, 
      sceneParent, 
      parentObject, 
      world, 
      objPos, 
      fileName, 
      initialState);

   m_children.push_back(static_cast<CompoundObject*>(co));

   co->setName(objectNode.attribute("name").as_string());
}

void CompoundObject::defineObjectSystem(
   Resources& gameResources,
   Actor* sceneParent,
   CompoundObject* parentObject,
   b2World* world,
   const Vector2& pos,
   xml_node& objectNode,
   string& initialState)
{
   // But there is a type of objects that are not to be handeled like this. They
   // are, for instance, particle systems or object factories. They are not compound 
   // objects really but could define shapes or CO as children (for instance can a 
   // object factory define a CO from a file as its spawning object). They are also 
   // defined by a set of object specific parameters (xml attributes). How to handle
   // these? Are they CO-children? Are they special behaviour elements (which require 
   // their own collections in CO or scenes.
   // They shall be defined as other CO with state properties since different states 
   // may require differnt settings. But they may not require a PosX, PosY and file
   // but other parameters
   // Let there be Compound Object Systems!!!

   // Iterate the stateProperties of the node, looking for state attributes
   // that matches the supplied initialState. If initialState is empty,
   // the first statePropertý is used
   xml_node* stateNode = NULL;

   for (auto it = objectNode.children("stateProperties").begin(); it != objectNode.children("stateProperties").end(); ++it)
   {
      if ((initialState == "") || (it->attribute("state").as_string() == initialState))
      {
         stateNode = &(*it);
         break;
      }
   }

   if (!stateNode)
   {
      return;
   }

   // Decode the type string to create the correct type of object
   // but only store the pointer to the CompoundObject
   // Perhaps with some special cases
   string type = objectNode.attribute("type").as_string();

   if (type == "asteroidField")
   {
      AsteroidField* af = new AsteroidField(
         gameResources,
         sceneParent,
         parentObject,
         world,
         *stateNode);

      m_children.push_back(static_cast<CompoundObject*>(af));

      af->setName(objectNode.attribute("name").as_string());
   }
   else if (type == "planetActor")
   {
      PlanetActor* pa = new PlanetActor(
         gameResources,
         sceneParent,
         parentObject,
         *stateNode);

      m_children.push_back(static_cast<CompoundObject*>(pa));

      pa->setName(objectNode.attribute("name").as_string());
   }
   else if (type == "clippedWindow")
   {
      OrbitWindow* ow = new OrbitWindow(
         gameResources,
         sceneParent,
         parentObject,
         *stateNode,
         initialState);

      m_children.push_back(static_cast<CompoundObject*>(ow));

      ow->setName(objectNode.attribute("name").as_string());
   }
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

   joint->EnableMotor(false);
   joint->SetLimits(joint->GetJointAngle(), joint->GetJointAngle());
   joint->EnableLimit(true);

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

   joint->EnableMotor(false);
   joint->EnableLimit(true);

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



