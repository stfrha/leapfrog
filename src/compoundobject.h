#pragma once
 
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundinterface.h"
#include "staticpolygon.h"

DECLARE_SMART(CompoundObject, spCompoundObject);

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

