
#include "oxygine-framework.h"

class PolygonVertices
{
public:
   PolygonVertices();

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
};

