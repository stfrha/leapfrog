#pragma once
 
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundinterface.h"
#include "staticpolygon.h"
#include "collisionentity.h"

class ObjectPropertyTriggeredEvent : public oxygine::Event
{
public:
   enum { EVENT = eventID('O', 'p', 'T', 'r') };
   int m_triggedEventId;
   int m_propertyId;
   float m_propertyValue;
   ObjectPropertyTriggeredEvent(int eventId, int propertyId, float value) :
      Event(EVENT), 
      m_triggedEventId(eventId),
      m_propertyId(propertyId),
      m_propertyValue(value)
   {}
};

class PropertyEventTrigger
{
public:
	enum TriggerType
	{
		ignore,
		insideRange,
		outsideRange
	};

private:
   int m_eventId;
   TriggerType	m_triggerType;	// outside range trigger, inside range trigger
	float m_upperLimit;
	float m_lowerLimit;

public:
	PropertyEventTrigger();
	PropertyEventTrigger(int eventId, TriggerType triggerType, float upperLimit, float lowerLimit);

	bool evaluateTrigger(float value);
   int getEventId(void);

};

class DualPropEventTrigger
{
private:
   int m_eventId;
   int m_prop1Id;
	int m_prop2Id;
	PropertyEventTrigger m_prop1Trigger;
	PropertyEventTrigger m_prop2Trigger;

public:
	DualPropEventTrigger();
	DualPropEventTrigger(int eventId, int prop1Id, int prop2Id, PropertyEventTrigger& prop1Trigger, PropertyEventTrigger& prop2Trigger);

	void updateProperty(int propId);
};

class CompoundObject;

class ObjectProperty
{
   typedef void(*ExtSetProperyCallback)();

private:
   bool m_extReadOnly;
	int m_id;
	float m_value;
   CompoundObject* m_object;
   oxygine::Event* m_extSetPropEvent;

	std::vector<PropertyEventTrigger> m_eventTriggers;
	std::vector<DualPropEventTrigger*> m_dualEventTriggers;

public:
	ObjectProperty();
	ObjectProperty(
      CompoundObject* obj, 
      oxygine::Event* extSetPropEvent, 
      int id, 
      float value, 
      bool readOnly = false);
	~ObjectProperty();

	void setProperty(float value);
   void extSetProperty(float value);
	float getProperty(void);
	void registerPropertyEventTrigger(
      int eventId, 
      PropertyEventTrigger::TriggerType triggerType, 
      float lower, 
      float upper);
   void unregisterPropertyEventTrigger(int eventId);
   void registerDualPropEventTrigger(DualPropEventTrigger* trigger);
	void unregisterDualPropEventTrigger(DualPropEventTrigger* trigger);
};



class NamedJoint
{
public:
   NamedJoint()
   {
      m_joint = NULL;
      m_name = "";
   }

   NamedJoint(b2Joint* joint, std::string name) :
      m_joint(joint),
      m_name(name)
   {   }

   b2Joint* m_joint;
   std::string m_name;
};

DECLARE_SMART(CompoundObject, spCompoundObject);

class CompoundObject : public oxygine::Actor, public CompoundInterface, public CollisionEntity
{
protected:

private:
   //std::vector<oxygine::spSprite> m_boxedSprites;
   //std::vector<oxygine::spSprite> m_polygonSprites;
   //std::vector<oxygine::spSprite> m_staticPolygons;
   //std::vector<oxygine::spSprite> m_staticBoxes;

   std::vector<NamedJoint*> m_namedJoints;

   CompoundObject* m_parentObject;

   CollisionEntityTypeEnum m_collisionType;

   void defineSpriteBox(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineSpritePolygon(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineStaticCircle(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineStaticBox(
      oxygine::Resources& gameResources, 
      oxygine::Actor* sceneParent, 
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode);

   void defineStaticPolygon(
      oxygine::Resources& gameResources, 
      oxygine::Actor* sceneParent, 
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode);

   void defineStaticBoxedSpritePolygon(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineDynamicCircle(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineDynamicBox(
      oxygine::Resources& gameResources, 
      oxygine::Actor* sceneParent, 
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode);

   void defineDynamicPolygon(
      oxygine::Resources& gameResources, 
      oxygine::Actor* sceneParent, 
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode);

   void defineDynamicBoxedSpritePolygon(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineChildObject(
      oxygine::Resources& gameResources, 
      oxygine::Actor* sceneParent, 
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode,
      std::string& initialState);

   void defineObjectSystem(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      std::string& initialState);

   void defineWeldJoint(
      b2World* world, 
      pugi::xml_node& weldJointNode);

   void defineRevoluteJoint(
      b2World* world, 
      pugi::xml_node& revoluteJointNode);

   void definePrismaticJoint(
      b2World* world,
      pugi::xml_node& jointNode);

   ObjectProperty* getProp(int propId);

   virtual void setPropertyImpl(int propId, float value);

   bool CompoundObject::getStateNode(pugi::xml_node& objectNode, std::string& initialState, pugi::xml_node& stateNode);

public:
   // Property values are protected within the ObjectProperty type
   // so it is safe to expose the list of properties here.
   std::vector<ObjectProperty> m_properties;
   std::vector<CompoundObject*> m_children;

	CompoundObject();

   ~CompoundObject();

   void initCompoundObjectTop(
	   oxygine::Resources& gameResources,
	   oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
	   b2World* world,
	   const oxygine::Vector2& pos,
	   std::string& defXmlFileName,
      std::string& initialState);

   CompoundObject* readDefinitionXmlFile(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      std::string& fileName,
      std::string& initialState);

   CompoundObject* initCompoundObject(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      std::string& initialState);

   bool initCompoundObjectParts(
      oxygine::Resources& gameResources,
      oxygine::Actor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      std::string& initialState);

   virtual CollisionEntityTypeEnum getEntityType(void);

   oxygine::Sprite* getSprite(void);
   CompoundObject* getParentObject(void);

   // Should these three be protected since they are called by the base class?
   virtual CompoundObject* getObjectImpl(const std::string& name);
   virtual b2Body* getBodyImpl(const std::string& name);
   virtual b2Joint* getJointImpl(const std::string& name);

  
   void extSetProperty(int propId, float value);
   void setProperty(int propId, float value);
   float getProperty(int propId);
   void registerPropertyEventTrigger(int eventId, int propId, PropertyEventTrigger::TriggerType triggerType, float lower, float upper);
   void registerDualPropEventTrigger(int propId, DualPropEventTrigger* trigger);
   void unregisterDualPropEventTrigger(int propId, DualPropEventTrigger* trigger);

};

