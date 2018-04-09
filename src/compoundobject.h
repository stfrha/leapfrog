#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "staticpolygon.h"

DECLARE_SMART(CompoundObject, spCompoundObject);

class CompoundObject : public oxygine::Actor
{
private:
   std::vector<oxygine::spSprite> m_boxedSprites;
   std::vector<oxygine::spSprite> m_polygonSprites;
   std::vector<oxygine::spSprite> m_staticPolygons;
   std::vector<oxygine::spSprite> m_staticBoxes;

   std::vector<b2WeldJoint> m_weldJoints;
   std::vector<b2RevoluteJoint> m_revoluteJoints;
   std::vector<b2PrismaticJoint> m_prismaticJoints;

   bool readDefinitionXmlFile(oxygine::Resources& gameResources, oxygine::Actor* parent, b2World* world, const oxygine::Vector2& pos, std::string& fileName);
   void defineStaticBox(oxygine::Resources& gameResources, oxygine::Actor* parent, b2World* world, const oxygine::Vector2& pos, pugi::xml_node& staticNode);
   void defineStaticPolygon(oxygine::Resources& gameResources, oxygine::Actor* parent, b2World* world, const oxygine::Vector2& pos, pugi::xml_node& staticNode);
   void defineBoxedObject(oxygine::Resources& gameResources, oxygine::Actor* parent, b2World* world, const oxygine::Vector2& pos, pugi::xml_node& boxObjectNode);
   void definePolygonObject(oxygine::Resources& gameResources, oxygine::Actor* parent, b2World* world, const oxygine::Vector2& pos, pugi::xml_node& polygonObjectNode);

public:
   CompoundObject(
      oxygine::Resources& gameResources,
      oxygine::Actor* parent,
      b2World* world, 
      const oxygine::Vector2& pos, 
      std::string& defXmlFileName);

   ~CompoundObject();

   oxygine::Sprite* getBoxedObject(std::string name);
   oxygine::Polygon* getPolygonObject(std::string name);
   StaticPolygon* getStaticPolygonObject(std::string name);
};

