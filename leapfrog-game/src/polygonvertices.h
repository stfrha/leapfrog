
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"

// A fade over a polygon (its fill and its border decorations), read from a
// <fade> child of the polygon's xml node:
//   amount  how strong the fade is where it is complete, 0..1 (0 = off)
//   angle   the direction the fade grows in, degrees: 0 = downwards,
//           90 = to the right, 180 = upwards, 270 = to the left
//   start   where the fade begins, as a fraction 0..1 of the polygon's
//           extent in that direction (default 0)
//   end     where the fade is complete (default 1); it grows linearly
//           from start to end
//   color   rrggbb: fade towards this colour (haze); empty or missing:
//           fade to transparent
// The fade is exact: triangles are split along the lines where it starts
// and ends, so the vertex colours (which are interpolated linearly) give
// the right gradient.
//
// An edge fade, read from an <edgeFade> child of the polygon's xml node,
// makes the polygon's fill transparent towards its outline:
//   width   how far in from the edge the fade reaches (game units)
//   amount  0..1, how transparent the fill is at the edge (default 1)
//   sides   which edges fade, by the direction their outside faces:
//           any of "top bottom left right" (each edge belongs to the
//           side within 45 degrees of its outward normal), or "all"
//           (default)
// The fill's opacity follows a smooth curve over the width, measured as
// the distance to the nearest fading edge. Triangles are cut on a grid
// near those edges (cells as wide as the fade, split from coarse cells
// only where they are near a fading edge) so the vertex colours can carry
// it. A narrow fade on a long outline still makes thousands of triangles
// (width 1 on a 370-unit outline: about 5 500), so keep the width
// reasonable. Border decorations are not edge-faded.
struct PolygonFade
{
   bool m_enabled;
   bool m_toColor;
   oxygine::Color m_color;
   oxygine::Vector2 m_dir;
   float m_s0;
   float m_s1;
   float m_amount;

   bool m_edgeEnabled;
   float m_edgeWidth;
   float m_edgeAmount;
   std::vector<oxygine::Vector2> m_edgeA;   // fading edges, polygon coordinates
   std::vector<oxygine::Vector2> m_edgeB;

   PolygonFade() :
      m_enabled(false), m_toColor(false), m_color(0, 0, 0, 0), m_dir(0.0f, 1.0f),
      m_s0(0.0f), m_s1(1.0f), m_amount(0.0f),
      m_edgeEnabled(false), m_edgeWidth(0.0f), m_edgeAmount(0.0f)
   {
   }

   // Strength 0..amount at position s along m_dir (polygon coordinates)
   float strength(float s) const;

   // Opacity factor 1 - edgeAmount .. 1 at p (polygon coordinates)
   float edgeOpacity(const oxygine::Vector2& p) const;

   // Distance from p to the nearest fading edge
   float edgeDistance(const oxygine::Vector2& p) const;
};


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
      const oxygine::Vector2& mOffset,
      const oxygine::Color& color = oxygine::Color(oxygine::Color::White));

   static PolygonFade readFade(
      pugi::xml_node objectNode,
      const std::vector<oxygine::Vector2>& polygonVertices);

   // Like createTriangleVertices, with the fade applied: triangles are split
   // where the fade starts and ends, and each vertex gets the fade's
   // strength there. With an edge fade the pieces are also cut on a grid
   // near the fading edges and each vertex gets its edge opacity. The vertices are local to an actor at `origin`, rotated
   // by `angle` (radians), in the polygon's coordinates (for a border
   // decoration; identity for the polygon itself), so the fade lines up.
   //   overlay = false: the colour's alpha is reduced (fade to transparent)
   //   overlay = true:  vertex colour (0, 0, 0, strength) for a haze overlay
   //                    drawn with the same texture and add colour = the
   //                    fade colour, so it covers what the texture covers
   // Returns new[] vertices (4 per triangle, oxygine's strip format) and
   // the number of triangles in numTriangles.
   static oxygine::vertexPCT2* createFadedTriangleVertices(
      const std::vector<oxygine::Vector2>& vertices,
      const std::vector<oxygine::VectorT3<int> >& triangles,
      const oxygine::Vector2& mSize,
      const oxygine::Vector2& mOffset,
      const oxygine::Color& color,
      const PolygonFade& fade,
      const oxygine::Vector2& origin,
      float angle,
      bool overlay,
      int& numTriangles);

   // Puts the polygon's vertices on `polygon`, faded if the fade is to
   // transparent, and for a fade to a colour adds the haze overlay as a
   // child (drawn before anything attached later, e.g. the borders).
   static void setFadedVertices(
      oxygine::Polygon* polygon,
      const std::vector<oxygine::Vector2>& vertices,
      const std::vector<oxygine::VectorT3<int> >& triangles,
      const oxygine::Vector2& mSize,
      const oxygine::Vector2& mOffset,
      const oxygine::Color& color,
      const PolygonFade& fade,
      const oxygine::Vector2& origin,
      float angle);

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
      b2Body*& body, 
      b2Fixture*& fixture, 
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
      std::vector<oxygine::Vector2>& vertices,
      pugi::xml_node objectNode);

   static b2Vec2 globalToLocalConversion(
      b2Vec2 ownGlobalPos, 
      float cosAngle, 
      float sinAngle, 
      b2Vec2 targetGlobalPos);
   
   static b2Vec2 localToGlobalConversion(
      b2Vec2 ownGlobalPos, 
      float cosAngle, 
      float sinAngle, 
      b2Vec2 targetLocalPos);

};

