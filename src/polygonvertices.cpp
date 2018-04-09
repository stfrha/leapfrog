#include "polygonvertices.h"
#include "physdispconvert.h"
#include "scales.h"

using namespace oxygine;
using namespace std;

vertexPCT2 PolygonVertices::initVertex(
   const Vector2& pos, 
   const unsigned int color, 
   const Vector2& bmSize, 
   const Vector2& alignment, 
   const Vector2& bmOffset)
{
   vertexPCT2 v;
   v.color = color;
   v.x = pos.x;
   v.y = pos.y;
   v.z = 0;

   // TODO: Learn how this works and correct accordingly
   Vector2 scale = bmSize;
   v.u = v.x / scale.x + bmOffset.x / scale.x - alignment.x / scale.x;
   v.v = v.y / scale.y + bmOffset.y / scale.y - alignment.y / scale.y ;

   return v;
}

vertexPCT2* PolygonVertices::createTriangleVertices(
   const vector<Vector2>& vertices, 
   const vector<VectorT3<int> >& triangles, 
   const Vector2& mSize, 
   const Vector2& mOffset)
{
   // find alignment so bitmap "starts" at the left-most point of the polygone
   // This is done by finding the min of the x and y coordinates of all 
   // vertices
   Vector2 alignment = Vector2(FLT_MAX, FLT_MAX);

   for (auto it = vertices.begin(); it != vertices.end(); ++it)
   {
      if (it->x < alignment.x)
      {
         alignment.x = it->x;
      }

      if (it->y < alignment.y)
      {
         alignment.y = it->y;
      }
   }

   alignment.x = alignment.x;
   alignment.y = alignment.y;

   int num = triangles.size();
   int verticesCount = num * 4;

   vertexPCT2* loclVerts = new vertexPCT2[verticesCount];

   vertexPCT2* p = loclVerts;
   Vector2 pos;

   for (int t = 0; t < num; t++)
   {
      pos = Vector2(
         vertices[(size_t)triangles[t].x - 1].x,
         vertices[(size_t)triangles[t].x - 1].y);

      *p = initVertex(pos, Color::White, mSize, alignment, mOffset);
      ++p;

      pos = Vector2(
         vertices[(size_t)triangles[t].y - 1].x,
         vertices[(size_t)triangles[t].y - 1].y);

      *p = initVertex(pos, Color::White, mSize, alignment, mOffset);
      ++p;

      pos = Vector2(
         vertices[(size_t)triangles[t].z - 1].x,
         vertices[(size_t)triangles[t].z - 1].y);

      *p = initVertex(pos, Color::White, mSize, alignment, mOffset);
      ++p;

      //Oxygine uses "triangles strip" rendering mode
      //dublicate last vertex (degenerate triangles)
      *p = initVertex(pos, Color::White, mSize, alignment, mOffset);
      ++p;
   }

   return loclVerts;
}

