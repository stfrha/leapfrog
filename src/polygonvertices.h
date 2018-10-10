
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"


class PolygonVertices
{
   static void createLocalSpritePolygon(
      oxygine::Polygon* polygon, 
      std::vector<oxygine::Vector2>& vertices,
      pugi::xml_node objectNode);

public:
   static oxygine::vertexPCT2 initVertex(
      const oxygine::Vector2& pos, 
      const unsigned int color, 
      const oxygine::Vector2& bmSize, 
      const oxygine::Vector2& alignement, 
      const oxygine::Vector2& bmOffset);

   /*
   vertices  : List of vertices
   triangles : List of triangles
   mSize     : The size (in meters) of the source bitmap
   mOffset   : The offset (in meters) of the bitmap mapping on the sprite
               If this is all zero, the leftmost point of the polygon sprite
               will get the left-most pixel of the bitmap
   */
   static oxygine::vertexPCT2* createTriangleVertices(
      const std::vector<oxygine::Vector2>& vertices,
      const std::vector<oxygine::VectorT3<int> >& triangles,
      const oxygine::Vector2& mSize,
      const oxygine::Vector2& mOffset);      

   static void readVertices(
      std::vector<oxygine::Vector2>& vertices,
      pugi::xml_node objectNode);

   static void readTriangles(
      std::vector<oxygine::VectorT3<int> >& triangles,
      pugi::xml_node objectNode);

   static void createSpritePolygon(
      oxygine::Polygon* polygon, 
      std::vector<oxygine::Vector2>& vertices,
      pugi::xml_node objectNode);
   
   static void createChainBodyPolygon(
      std::vector<oxygine::Vector2>& vertices,
      b2World* world,
      b2Vec2& pos,
      b2Body* body, 
      b2Fixture* fixture, 
      pugi::xml_node objectNode);

   static void createBodyPolygon(
      std::vector<oxygine::Vector2>& vertices,
      b2World* world,
      b2Vec2& pos,
      b2Body** body,
      b2Fixture** fixture,
      pugi::xml_node objectNode,
      bool staticBody,
      int groupIndex);


   static void createPolygonBorders(
      oxygine::Polygon* polygon,
      oxygine::Resources& gameResources,
      std::vector<oxygine::Vector2>& vertices,
      pugi::xml_node objectNode);
};

