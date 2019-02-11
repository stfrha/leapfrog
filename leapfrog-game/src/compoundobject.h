#pragma once
 
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "collisionentity.h"
#include "objectproperty.h"
#include "system.h"
#include "gamestatus.h"
#include "headdowndisplay.h"

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

class CompoundObject : public oxygine::Actor, public CollisionEntity
{
public:
   enum BehaviourEnum
   {
      notApplicable,
      scene,
      orbitScene,
      leapfrog,
      launchSite,
      landingPad,
      breakableObject,
      explosiveObject,
      magneticMine,
      steerableObject
   };


private:
   // Lists for searching for objects defined for the CompoundObject
   // Note:
   // Shapes (sprites and bodies) are attached to scene parent but linked
   // to 
   std::vector<NamedJoint*> m_namedJoints;

   CollisionEntityTypeEnum m_collisionType;
   BehaviourEnum m_behaviourType;

   void doCommonShapeDefinitions(
      oxygine::Resources& gameResources,
      oxygine::Sprite* sprite,
      oxygine::Vector2 pos,
      pugi::xml_node& objectNode);

   void doCollisionDefinitions(
      b2Fixture*& fixture,
      pugi::xml_node& objectNode,
      int groupIndex);

   void doPhysicalDefinitions(
      b2Fixture*& fixture,
      pugi::xml_node& objectNode);

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
      int groupIndex,
      bool staticBody);

   void defineStaticCircle(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      int groupIndex);

   void defineBox(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      int groupIndex,
      bool staticBody);

   void defineStaticBox(
      oxygine::Resources& gameResources, 
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode,
      int groupIndex);

   void defineStaticPolygon(
      oxygine::Resources& gameResources, 
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode,
      int groupIndex);

   void defineBoxedSpritePolygon(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      int groupIndex,
      bool staticBody);

   void defineStaticBoxedSpritePolygon(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      int groupIndex);

   void defineDynamicCircle(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      int groupIndex);

   void defineDynamicBox(
      oxygine::Resources& gameResources, 
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode,
      int groupIndex);

   void defineDynamicPolygon(
      oxygine::Resources& gameResources, 
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos, 
      pugi::xml_node& objectNode,
      int groupIndex);

   void defineDynamicBoxedSpritePolygon(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      int groupIndex);

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
   SceneActor * m_sceneActor;
   CompoundObject * m_parentObject;
   std::vector<oxygine::spActor> m_shapes;

   void setAllBodiesToBounding(FreeSpaceActor* actor);


public:
   // Property values are protected within the ObjectProperty type
   // so it is safe to expose the list of properties here.
   std::vector<ObjectProperty> m_properties;
   std::vector<CompoundObject*> m_children;
   std::vector<System*> m_systems;
   spGameStatus m_gameStatus;
   bool m_isDead;

   CompoundObject(SceneActor* sceneActor, CompoundObject* parentObject);

   ~CompoundObject();



// Static, public methods used to create new CO (or behaviour decendant of CO)

   static CompoundObject* initCompoundObjectTop(
	   oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
	   b2World* world,
	   const std::string& defXmlFileName,
	   const std::string& initialState);

   static CompoundObject* readDefinitionXmlFile(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      const std::string& fileName,
      const std::string& initialState);

   static CompoundObject* initCompoundObject(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      const std::string& initialState);



   // Normal, non-static, public methods of an instance of a CO

   bool initCompoundObjectParts(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      const std::string& initialState,
      int groupIndex);

   CompoundObject* defineChildObject(
      oxygine::Resources& gameResources,
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& objectNode,
      const std::string& initialState);

   void initGameStatus(spGameStatus status);


   CompoundObject* getParentObject();
   CompoundObject* getParentWithBehaviour(BehaviourEnum behav);
   void killAllChildBodies(void);
   void removeShape(oxygine::Actor* removeMe);

   oxygine::Vector2 getCompoundObjectPosition();

   void hitByBullet(b2Contact* contact);
   void addMeToDeathList(void);

   virtual void registerToMap(void);

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

