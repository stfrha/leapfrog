#pragma once
 
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "killableinterface.h"
#include "collisionentity.h"
#include "objectproperty.h"
#include "system.h"

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

class SceneActor;
class FreeSpaceActor;

class CompoundObject : public oxygine::Actor, public KillableInterface, public CollisionEntity
{
public:
   enum BehaviourEnum
   {
      notApplicable,
      leapfrog,
      launchSite,
      landingPad,
      breakableObject,
      steerableObject
   };


private:

   SceneActor * m_sceneActor;

   std::vector<NamedJoint*> m_namedJoints;

   CollisionEntityTypeEnum m_collisionType;
   BehaviourEnum m_behaviourType;

   void defineSpriteBox(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineSpritePolygon(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineCircle(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      bool staticBody);

   void defineStaticCircle(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineBox(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      bool staticBody);

   void defineStaticBox(
      oxygine::Resources& gameResources, 
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode);

   void defineStaticPolygon(
      oxygine::Resources& gameResources, 
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode);

   void defineBoxedSpritePolygon(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      bool staticBody);

   void defineStaticBoxedSpritePolygon(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineDynamicCircle(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineDynamicBox(
      oxygine::Resources& gameResources, 
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode);

   void defineDynamicPolygon(
      oxygine::Resources& gameResources, 
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode);

   void defineDynamicBoxedSpritePolygon(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineRope(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode);

   void defineWeldJoint(
      b2World* world, 
      pugi::xml_node& weldJointNode);

   void defineRevoluteJoint(
      b2World* world, 
      pugi::xml_node& revoluteJointNode);

   void definePrismaticJoint(
      b2World* world,
      pugi::xml_node& jointNode);

   void splitString(const std::string& source, std::string& thisLevel, std::string& lowerLevels);
   b2Body* getBodyImpl(const std::string& name);
   oxygine::spActor getActorImpl(const std::string& name);
   CompoundObject* getObjectImpl(const std::string& name);
   b2Joint* getJointImpl(const std::string& name);
   System* getSystemImpl(const std::string& name);

   ObjectProperty* getProp(int propId);

   virtual void setPropertyImpl(int propId, float value);

   bool getStateNode(pugi::xml_node& objectNode, const std::string& initialState, pugi::xml_node& stateNode);

protected:
   CompoundObject * m_parentObject;

   void setAllBodiesToBounding(FreeSpaceActor* actor);


public:
   // Property values are protected within the ObjectProperty type
   // so it is safe to expose the list of properties here.
   std::vector<ObjectProperty> m_properties;
   std::vector<CompoundObject*> m_children;
   std::vector<System*> m_systems;

   CompoundObject(SceneActor* sceneActor, CompoundObject* parentObject);

   ~CompoundObject();

   void initCompoundObjectTop(
	   oxygine::Resources& gameResources,
      CompoundObject* parentObject,
	   b2World* world,
	   const std::string& defXmlFileName,
	   const std::string& initialState);

   CompoundObject* readDefinitionXmlFile(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
	  const std::string& fileName,
	  const std::string& initialState);

   CompoundObject* initCompoundObject(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
	  const std::string& initialState);

   bool initCompoundObjectParts(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      const std::string& initialState);

   CompoundObject* defineChildObject(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      const std::string& initialState);

   virtual CollisionEntityTypeEnum getEntityType(void);

   CompoundObject* getParentObject();
   CompoundObject* getParentWithBehaviour(BehaviourEnum behav);
   virtual void killAllChildBodies(void);

   b2Vec2 getCompoundObjectPosition();

   void hitByBullet(b2Contact* contact);

   // oxygine::Sprite* getSprite(void);

   // Should these three be protected since they are called by the base class?
   CompoundObject* getObject(const std::string& name);
   oxygine::spActor getActor(const std::string& name);
   b2Body* getBody(const std::string& name);
   b2Joint* getJoint(const std::string& name);
   System* getSystem(const std::string& name);
  
   void extSetProperty(int propId, float value);
   void setProperty(int propId, float value);
   float getProperty(int propId);
   void registerPropertyEventTrigger(int eventId, int propId, PropertyEventTrigger::TriggerType triggerType, float lower, float upper);
   void registerDualPropEventTrigger(int propId, DualPropEventTrigger* trigger);
   void unregisterDualPropEventTrigger(int propId, DualPropEventTrigger* trigger);

};

