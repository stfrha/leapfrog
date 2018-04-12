#pragma once
 
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundinterface.h"
#include "staticpolygon.h"

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
	TriggerType	m_triggerType;	// outside range trigger, inside range trigger
	float m_upperLimit;
	float m_lowerLimit;

public:
	PropertyEventTrigger();
	PropertyEventTrigger(TriggerType triggerType, float upperLimit, float lowerLimit);

	bool evaluateTrigger(float value);
};

class DualPropEventTrigger
{
private:
	int m_prop1Id;
	int m_prop2Id;
	PropertyEventTrigger m_prop1Trigger;
	PropertyEventTrigger m_prop2Trigger;

public:
	DualPropEventTrigger();
	DualPropEventTrigger(int prop1Id, int prop2Id, PropertyEventTrigger& prop1Trigger, PropertyEventTrigger& prop2Trigger);

	void updateProperty(int propId);
};

class ObjectProperty
{
private:
	int m_id;
	float m_value;
	std::vector<PropertyEventTrigger> m_eventTriggers;
	std::vector<DualPropEventTrigger*> m_dualEventTriggers;

public:
	ObjectProperty();
	ObjectProperty(int id, float value);
	~ObjectProperty();

	void setProperty(float value);
	float getProperty(void);
	void registerPropertyEventTrigger(PropertyEventTrigger::TriggerType triggerType, float lower, float upper);
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

class CompoundObject : public oxygine::Actor, public CompoundInterface
{
private:
   std::vector<oxygine::spSprite> m_boxedSprites;
   std::vector<oxygine::spSprite> m_polygonSprites;
   std::vector<oxygine::spSprite> m_staticPolygons;
   std::vector<oxygine::spSprite> m_staticBoxes;

   std::vector<NamedJoint*> m_namedJoints;

   std::vector<CompoundObject*> m_children;

   bool readDefinitionXmlFile(oxygine::Resources& gameResources, oxygine::Actor* parent, b2World* world, const oxygine::Vector2& pos, std::string& fileName);
   void defineStaticBox(oxygine::Resources& gameResources, oxygine::Actor* parent, b2World* world, const oxygine::Vector2& pos, pugi::xml_node& staticNode);
   void defineStaticPolygon(oxygine::Resources& gameResources, oxygine::Actor* parent, b2World* world, const oxygine::Vector2& pos, pugi::xml_node& staticNode);
   void defineBoxedObject(oxygine::Resources& gameResources, oxygine::Actor* parent, b2World* world, const oxygine::Vector2& pos, pugi::xml_node& boxObjectNode);
   void definePolygonObject(oxygine::Resources& gameResources, oxygine::Actor* parent, b2World* world, const oxygine::Vector2& pos, pugi::xml_node& polygonObjectNode);
   
   void defineWeldJoint(b2World* world, pugi::xml_node& weldJointNode);
   void defineRevoluteJoint(b2World* world, pugi::xml_node& revoluteJointNode);

public:
   CompoundObject(
      oxygine::Resources& gameResources,
      oxygine::Actor* parent,
      b2World* world, 
      const oxygine::Vector2& pos, 
      std::string& defXmlFileName);

   ~CompoundObject();

   virtual CompoundObject* getObjectImpl(const std::string& name);
   virtual b2Body* getBodyImpl(const std::string& name);
   virtual b2Joint* getJointImpl(const std::string& name);
};

