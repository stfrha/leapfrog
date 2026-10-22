#include <algorithm>
#include <cstring>
#include "oxygine-framework.h"
#include "polygonvertices.h"
#include "physdispconvert.h"
#include "scales.h"
#include "graphicresources.h"

using namespace oxygine;
using namespace std;
using namespace pugi;

vertexPCT2 PolygonVertices::initVertex(
   const Vector2& pos, 
   const unsigned int color, 
   const Vector2& textureMeterSize,
   const Vector2& alignment, 
   const Vector2& textureMeterOffset)
{
   vertexPCT2 v;
   v.color = color;
   v.x = pos.x;
   v.y = pos.y;
   v.z = 0;

   // I don't really know why there should be "-(minus) textureMeterOffsett, but it works
   v.u = v.x / textureMeterSize.x - textureMeterOffset.x / textureMeterSize.x /*- alignment.x / textureMeterSize.x*/;
   v.v = v.y / textureMeterSize.y - textureMeterOffset.y / textureMeterSize.y /*- alignment.y / textureMeterSize.y */;

   return v;
}

vertexPCT2* PolygonVertices::createTriangleVertices(
   const vector<Vector2>& vertices, 
   const vector<VectorT3<int> >& triangles, 
   const Vector2& textureMeterSize, 
   const Vector2& textureMeterOffset,
   const Color& color)
{
   // oxygine::Polygon ignores Sprite::setColor and draws with the vertex
   // colours, so a polygon's tint is put in its vertices (premultiplied,
   // like the textures).
   const unsigned int vertexColor = color.premultiplied().rgba();

   // find alignment so bitmap "starts" at the left-most point of the polygon
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

      *p = initVertex(pos, vertexColor, textureMeterSize, alignment, textureMeterOffset);
      ++p;

      pos = Vector2(
         vertices[(size_t)triangles[t].y - 1].x,
         vertices[(size_t)triangles[t].y - 1].y);

      *p = initVertex(pos, vertexColor, textureMeterSize, alignment, textureMeterOffset);
      ++p;

      pos = Vector2(
         vertices[(size_t)triangles[t].z - 1].x,
         vertices[(size_t)triangles[t].z - 1].y);

      *p = initVertex(pos, vertexColor, textureMeterSize, alignment, textureMeterOffset);
      ++p;

      //Oxygine uses "triangles strip" rendering mode
      //dublicate last vertex (degenerate triangles)
      *p = initVertex(pos, vertexColor, textureMeterSize, alignment, textureMeterOffset);
      ++p;
   }

   return loclVerts;
}

void PolygonVertices::createLocalSpritePolygon(
   oxygine::Polygon* polygon,
   std::vector<oxygine::Vector2>& vertices,
   pugi::xml_node objectNode)
{
   vector<VectorT3<int> > triangles;

   Vector2 textureMeterSize = Vector2(objectNode.attribute("textureMeterWidth").as_float(),
      objectNode.attribute("textureMeterHeight").as_float());
   Vector2 textureMeterOffset = Vector2(objectNode.attribute("textureOffsetMeterX").as_float(),
      objectNode.attribute("textureOffsetMeterY").as_float());

   for (auto it = objectNode.child("vertices").children("vertex").begin();
      it != objectNode.child("vertices").children("vertex").end();
      ++it)
   {
      vertices.push_back(Vector2(it->attribute("x").as_float(), it->attribute("y").as_float()));
   }

   for (auto it = objectNode.child("triangles").children("triangle").begin();
      it != objectNode.child("triangles").children("triangle").end();
      ++it)
   {
      triangles.push_back(VectorT3<int>(
         it->attribute("v1").as_int(),
         it->attribute("v2").as_int(),
         it->attribute("v3").as_int()));
   }

   vertexPCT2* vs = createTriangleVertices(
      vertices,
      triangles,
      textureMeterSize,
      textureMeterOffset,
      polygon->getColor());

   polygon->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

}

void PolygonVertices::readVertices(
   std::vector<oxygine::Vector2>& vertices,
   pugi::xml_node objectNode)
{
   for (auto it = objectNode.child("vertices").children("vertex").begin();
      it != objectNode.child("vertices").children("vertex").end();
      ++it)
   {
      vertices[it->attribute("id").as_int() - 1] = Vector2(it->attribute("x").as_float(), it->attribute("y").as_float());
   }
}

void PolygonVertices::readTriangles(
   std::vector<oxygine::VectorT3<int> >& triangles,
   pugi::xml_node objectNode)
{
   for (auto it = objectNode.child("triangles").children("triangle").begin();
      it != objectNode.child("triangles").children("triangle").end();
      ++it)
   {
      triangles[it->attribute("id").as_int() - 1] = VectorT3<int>(
         it->attribute("v1").as_int(),
         it->attribute("v2").as_int(),
         it->attribute("v3").as_int());
   }
}

void PolygonVertices::createSpritePolygon(
   oxygine::Polygon* polygon, 
   vector<Vector2>& vertices,
   pugi::xml_node objectNode)
{
   readVertices(vertices, objectNode);

   vector<VectorT3<int> > triangles(distance(objectNode.child("triangles").children("triangle").begin(), objectNode.child("triangles").children("triangle").end()));

   readTriangles(triangles, objectNode);

   Vector2 textureMeterSize = Vector2(objectNode.attribute("textureMeterWidth").as_float(),
      objectNode.attribute("textureMeterHeight").as_float());
   Vector2 textureMeterOffset = Vector2(objectNode.attribute("textureOffsetMeterX").as_float(),
      objectNode.attribute("textureOffsetMeterY").as_float());

   // Tint from the shape's color attribute, and the fade if it has one
   PolygonFade fade = readFade(objectNode, vertices);
   setFadedVertices(polygon, vertices, triangles, textureMeterSize, textureMeterOffset,
      polygon->getColor(), fade, Vector2(0.0f, 0.0f), 0.0f);
}

// ---------------------------------------------------------------------------
// Fade
// ---------------------------------------------------------------------------

float PolygonFade::strength(float s) const
{
   float g;
   float span = m_s1 - m_s0;

   if (fabs(span) < 1e-6f)
   {
      g = (s >= m_s0) ? 1.0f : 0.0f;
   }
   else
   {
      g = (s - m_s0) / span;
   }

   if (g < 0.0f) g = 0.0f;
   if (g > 1.0f) g = 1.0f;

   return g * m_amount;
}

static float distanceToSegment(const Vector2& p, const Vector2& a, const Vector2& b)
{
   Vector2 ab = b - a;
   Vector2 ap = p - a;
   float len2 = ab.x * ab.x + ab.y * ab.y;
   float t = (len2 > 1e-12f) ? (ap.x * ab.x + ap.y * ab.y) / len2 : 0.0f;
   if (t < 0.0f) t = 0.0f;
   if (t > 1.0f) t = 1.0f;
   Vector2 d = ap - ab * t;
   return sqrt(d.x * d.x + d.y * d.y);
}

float PolygonFade::edgeDistance(const Vector2& p) const
{
   float best = FLT_MAX;
   for (size_t i = 0; i < m_edgeA.size(); i++)
   {
      float d = distanceToSegment(p, m_edgeA[i], m_edgeB[i]);
      if (d < best) best = d;
   }
   return best;
}

float PolygonFade::edgeOpacity(const Vector2& p) const
{
   if (!m_edgeEnabled)
   {
      return 1.0f;
   }
   float t = edgeDistance(p) / m_edgeWidth;
   if (t >= 1.0f)
   {
      return 1.0f;
   }
   if (t < 0.0f) t = 0.0f;
   float s = t * t * (3.0f - 2.0f * t);          // smooth start and end
   return 1.0f - m_edgeAmount * (1.0f - s);
}

static bool parseFadeColor(const char* str, Color& color)
{
   if ((str == NULL) || (str[0] == 0))
   {
      return false;
   }
   if (str[0] == '#')
   {
      str++;
   }
   size_t len = strlen(str);
   if ((len != 6) && (len != 8))
   {
      return false;
   }
   unsigned int v = 0;
   for (size_t i = 0; i < 6; i++)
   {
      char c = str[i];
      int d;
      if ((c >= '0') && (c <= '9')) d = c - '0';
      else if ((c >= 'a') && (c <= 'f')) d = c - 'a' + 10;
      else if ((c >= 'A') && (c <= 'F')) d = c - 'A' + 10;
      else return false;
      v = (v << 4) | (unsigned int)d;
   }
   // Alpha 0: the overlay's add colour must not change its alpha
   color = Color((v >> 16) & 0xff, (v >> 8) & 0xff, v & 0xff, 0);
   return true;
}

// Reads <edgeFade width amount sides> into fade (see PolygonFade)
static void readEdgeFade(
   pugi::xml_node objectNode,
   const vector<Vector2>& vertices,
   PolygonFade& fade)
{
   xml_node edgeNode = objectNode.child("edgeFade");
   if (edgeNode.empty() || (vertices.size() < 3))
   {
      return;
   }

   float width = edgeNode.attribute("width").as_float(0.0f);
   float amount = edgeNode.attribute("amount").as_float(1.0f);
   if (amount > 1.0f) amount = 1.0f;
   if ((width <= 0.0f) || (amount <= 0.0f))
   {
      return;
   }

   string sides = edgeNode.attribute("sides").as_string("all");
   bool all = (sides.find("all") != string::npos) || (sides.find_first_not_of(" \t,") == string::npos);
   bool top = all || (sides.find("top") != string::npos);
   bool bottom = all || (sides.find("bottom") != string::npos);
   bool left = all || (sides.find("left") != string::npos);
   bool right = all || (sides.find("right") != string::npos);

   // Which way the outline goes round decides which side of an edge is out
   float doubleArea = 0.0f;
   size_t n = vertices.size();
   for (size_t i = 0; i < n; i++)
   {
      const Vector2& a = vertices[i];
      const Vector2& b = vertices[(i + 1) % n];
      doubleArea += a.x * b.y - b.x * a.y;
   }

   for (size_t i = 0; i < n; i++)
   {
      const Vector2& a = vertices[i];
      const Vector2& b = vertices[(i + 1) % n];
      Vector2 d = b - a;
      if ((d.x == 0.0f) && (d.y == 0.0f))
      {
         continue;
      }
      // Outward normal (y axis down)
      Vector2 out = (doubleArea > 0.0f) ? Vector2(d.y, -d.x) : Vector2(-d.y, d.x);

      bool use;
      if (-out.y >= fabs(out.x)) use = top;
      else if (out.y >= fabs(out.x)) use = bottom;
      else if (out.x < 0.0f) use = left;
      else use = right;

      if (use)
      {
         fade.m_edgeA.push_back(a);
         fade.m_edgeB.push_back(b);
      }
   }

   if (fade.m_edgeA.empty())
   {
      return;
   }

   fade.m_edgeWidth = width;
   fade.m_edgeAmount = amount;
   fade.m_edgeEnabled = true;
}

PolygonFade PolygonVertices::readFade(
   pugi::xml_node objectNode,
   const vector<Vector2>& polygonVertices)
{
   PolygonFade fade;

   if (polygonVertices.empty())
   {
      return fade;
   }

   readEdgeFade(objectNode, polygonVertices, fade);

   xml_node fadeNode = objectNode.child("fade");
   if (fadeNode.empty())
   {
      return fade;
   }

   fade.m_amount = fadeNode.attribute("amount").as_float(0.0f);
   if (fade.m_amount > 1.0f)
   {
      fade.m_amount = 1.0f;
   }
   if (fade.m_amount <= 0.0f)
   {
      return fade;
   }

   float angle = fadeNode.attribute("angle").as_float(0.0f) * MATH_PI / 180.0f;
   fade.m_dir = Vector2(sin(angle), cos(angle));      // 0 = down (y axis down), 90 = right

   fade.m_toColor = parseFadeColor(fadeNode.attribute("color").as_string(""), fade.m_color);

   // Start and end as fractions of the polygon's extent along the direction
   float pmin = FLT_MAX;
   float pmax = -FLT_MAX;
   for (size_t i = 0; i < polygonVertices.size(); i++)
   {
      float p = polygonVertices[i].x * fade.m_dir.x + polygonVertices[i].y * fade.m_dir.y;
      if (p < pmin) pmin = p;
      if (p > pmax) pmax = p;
   }
   float start = fadeNode.attribute("start").as_float(0.0f);
   float end = fadeNode.attribute("end").as_float(1.0f);
   fade.m_s0 = pmin + start * (pmax - pmin);
   fade.m_s1 = pmin + end * (pmax - pmin);

   fade.m_enabled = true;
   return fade;
}

namespace
{
   struct FadePoint
   {
      Vector2 p;     // local position
      float s;       // position along the fade direction, polygon coordinates
   };

   // axis 0: x, 1: y (of the local position), 2: s
   inline float fadeAxis(const FadePoint& f, int axis)
   {
      return (axis == 0) ? f.p.x : ((axis == 1) ? f.p.y : f.s);
   }

   // Sutherland-Hodgman: the part of a convex polygon with value <= cut
   // (keepLess) or value >= cut, along one axis
   void clipFade(const vector<FadePoint>& in, int axis, float cut, bool keepLess, vector<FadePoint>& out)
   {
      out.clear();
      size_t n = in.size();
      for (size_t i = 0; i < n; i++)
      {
         const FadePoint& a = in[i];
         const FadePoint& b = in[(i + 1) % n];
         float va = fadeAxis(a, axis);
         float vb = fadeAxis(b, axis);
         bool inA = keepLess ? (va <= cut) : (va >= cut);
         bool inB = keepLess ? (vb <= cut) : (vb >= cut);

         if (inA)
         {
            out.push_back(a);
         }
         if (inA != inB)
         {
            float t = (cut - va) / (vb - va);
            FadePoint m;
            m.p = a.p + (b.p - a.p) * t;
            m.s = a.s + (b.s - a.s) * t;
            if (axis == 0) m.p.x = cut;
            else if (axis == 1) m.p.y = cut;
            else m.s = cut;
            out.push_back(m);
         }
      }
   }

   float cross2(const Vector2& a, const Vector2& b)
   {
      return a.x * b.y - a.y * b.x;
   }

   bool segmentsCross(const Vector2& a, const Vector2& b, const Vector2& c, const Vector2& d)
   {
      float d1 = cross2(b - a, c - a);
      float d2 = cross2(b - a, d - a);
      float d3 = cross2(d - c, a - c);
      float d4 = cross2(d - c, b - c);
      return (((d1 > 0) != (d2 > 0)) && ((d3 > 0) != (d4 > 0)));
   }

   bool insideConvex(const vector<FadePoint>& poly, const Vector2& p)
   {
      bool pos = false;
      bool neg = false;
      size_t n = poly.size();
      for (size_t i = 0; i < n; i++)
      {
         float c = cross2(poly[(i + 1) % n].p - poly[i].p, p - poly[i].p);
         if (c > 0) pos = true;
         if (c < 0) neg = true;
      }
      return !(pos && neg);
   }

   // True if every point of the convex piece is at least the edge fade's
   // width from every fading edge (so it needs no cutting)
   bool farFromFadingEdges(const vector<FadePoint>& piece, const PolygonFade& fade)
   {
      float w = fade.m_edgeWidth;
      size_t n = piece.size();
      for (size_t e = 0; e < fade.m_edgeA.size(); e++)
      {
         const Vector2& a = fade.m_edgeA[e];
         const Vector2& b = fade.m_edgeB[e];
         if (insideConvex(piece, a) || insideConvex(piece, b))
         {
            return false;
         }
         for (size_t i = 0; i < n; i++)
         {
            const Vector2& c = piece[i].p;
            const Vector2& d = piece[(i + 1) % n].p;
            if (segmentsCross(a, b, c, d))
            {
               return false;
            }
            if ((distanceToSegment(c, a, b) < w) || (distanceToSegment(a, c, d) < w) ||
               (distanceToSegment(b, c, d) < w))
            {
               return false;
            }
         }
      }
      return true;
   }

   // Cuts a convex piece into the cells of a grid of `step` along x and y
   // (lines at whole multiples of step, so neighbouring pieces are cut at
   // the same places) and appends the cells to out.
   void cutOnGrid(const vector<FadePoint>& piece, float step, vector<vector<FadePoint> >& out)
   {
      float minX = FLT_MAX, maxX = -FLT_MAX, minY = FLT_MAX, maxY = -FLT_MAX;
      for (size_t i = 0; i < piece.size(); i++)
      {
         minX = std::min(minX, piece[i].p.x);
         maxX = std::max(maxX, piece[i].p.x);
         minY = std::min(minY, piece[i].p.y);
         maxY = std::max(maxY, piece[i].p.y);
      }
      int x0 = (int)floor(minX / step);
      int x1 = (int)floor(maxX / step);
      int y0 = (int)floor(minY / step);
      int y1 = (int)floor(maxY / step);

      vector<FadePoint> a, column, b, cell;
      for (int ix = x0; ix <= x1; ix++)
      {
         clipFade(piece, 0, (float)ix * step, false, a);
         if (a.size() < 3) continue;
         clipFade(a, 0, (float)(ix + 1) * step, true, column);
         if (column.size() < 3) continue;
         for (int iy = y0; iy <= y1; iy++)
         {
            clipFade(column, 1, (float)iy * step, false, b);
            if (b.size() < 3) continue;
            clipFade(b, 1, (float)(iy + 1) * step, true, cell);
            if (cell.size() < 3) continue;
            out.push_back(cell);
         }
      }
   }

   // Splits a convex cell of a grid of `size` into quarters, again and again,
   // where it lies within the edge fade's width of a fading edge, down to
   // cells of `step` (size must be step times a power of two). A cell that
   // is far from the fading edges stays whole: its opacity is 1 all over,
   // so its edges match whatever the neighbouring cells are cut into.
   void subdivideNearEdges(const vector<FadePoint>& cell, float size, float step, const PolygonFade& fade,
      vector<vector<FadePoint> >& out)
   {
      if ((size <= step * 1.001f) || farFromFadingEdges(cell, fade))
      {
         out.push_back(cell);
         return;
      }

      float half = size * 0.5f;
      vector<vector<FadePoint> > quarters;
      cutOnGrid(cell, half, quarters);
      for (size_t q = 0; q < quarters.size(); q++)
      {
         subdivideNearEdges(quarters[q], half, step, fade, out);
      }
   }
}

vertexPCT2* PolygonVertices::createFadedTriangleVertices(
   const vector<Vector2>& vertices,
   const vector<VectorT3<int> >& triangles,
   const Vector2& textureMeterSize,
   const Vector2& textureMeterOffset,
   const Color& color,
   const PolygonFade& fade,
   const Vector2& origin,
   float angle,
   bool overlay,
   int& numTriangles)
{
   float ca = cos(angle);
   float sa = sin(angle);
   float lo = (fade.m_s0 < fade.m_s1) ? fade.m_s0 : fade.m_s1;
   float hi = (fade.m_s0 < fade.m_s1) ? fade.m_s1 : fade.m_s0;
   Vector2 unused(0.0f, 0.0f);

   // The fill's own alpha follows the linear fade only when that fades to
   // transparent; a fade to a colour is drawn by the overlay
   bool linearAlpha = fade.m_enabled && !fade.m_toColor;

   // Edge fade: the polygon is cut on a coarse grid, and only the cells
   // within the fade's width of a fading edge are split further (in
   // quarters) down to cells as wide as the fade (the opacity then ramps
   // almost linearly over the width rather than along the smooth curve,
   // which can't be told apart at these sizes). Every vertex is transformed
   // by oxygine on the CPU each frame, so the count matters: a grid of a
   // third of the width over every coarse cell near an edge made 16 700
   // triangles of 11 for a 110 x 23 polygon with width 1 (8-10 fps for that
   // one polygon in a Debug build); this makes about 5 500.
   float step = fade.m_edgeWidth;
   float coarse = step * 16.0f;

   vector<vertexPCT2> out;
   out.reserve(triangles.size() * 4 * 3);

   vector<FadePoint> tri(3);
   vector<FadePoint> piece;
   vector<FadePoint> tmp;
   vector<vector<FadePoint> > coarseCells;
   vector<vector<FadePoint> > cells;

   for (size_t t = 0; t < triangles.size(); t++)
   {
      int idx[3] = { triangles[t].x - 1, triangles[t].y - 1, triangles[t].z - 1 };
      for (int k = 0; k < 3; k++)
      {
         const Vector2& p = vertices[(size_t)idx[k]];
         Vector2 q(origin.x + ca * p.x - sa * p.y, origin.y + sa * p.x + ca * p.y);
         tri[k].p = p;
         tri[k].s = q.x * fade.m_dir.x + q.y * fade.m_dir.y;
      }

      // Three bands: before the linear fade, the gradient, after it (or
      // the whole triangle when there is no linear fade)
      for (int band = 0; band < 3; band++)
      {
         if (!fade.m_enabled)
         {
            if (band > 0) break;
            piece = tri;
         }
         else if (band == 0)
         {
            clipFade(tri, 2, lo, true, piece);
         }
         else if (band == 1)
         {
            clipFade(tri, 2, lo, false, tmp);
            clipFade(tmp, 2, hi, true, piece);
         }
         else
         {
            clipFade(tri, 2, hi, false, piece);
         }

         if (piece.size() < 3)
         {
            continue;
         }

         cells.clear();
         if (!fade.m_edgeEnabled)
         {
            cells.push_back(piece);
         }
         else
         {
            coarseCells.clear();
            cutOnGrid(piece, coarse, coarseCells);
            for (size_t c = 0; c < coarseCells.size(); c++)
            {
               subdivideNearEdges(coarseCells[c], coarse, step, fade, cells);
            }
         }

         for (size_t c = 0; c < cells.size(); c++)
         {
            const vector<FadePoint>& cell = cells[c];
            for (size_t k = 1; k + 1 < cell.size(); k++)
            {
               const FadePoint* corner[3] = { &cell[0], &cell[k], &cell[k + 1] };
               vertexPCT2 v[3];
               for (int i = 0; i < 3; i++)
               {
                  float f = fade.m_enabled ? fade.strength(corner[i]->s) : 0.0f;
                  float e = fade.edgeOpacity(corner[i]->p);
                  Color vc;
                  if (overlay)
                  {
                     vc = Color(0, 0, 0, (unsigned char)(color.a * f * e + 0.5f));
                  }
                  else
                  {
                     float keep = linearAlpha ? (1.0f - f) : 1.0f;
                     vc = Color(color.r, color.g, color.b, (unsigned char)(color.a * keep * e + 0.5f));
                  }
                  v[i] = initVertex(corner[i]->p, vc.premultiplied().rgba(), textureMeterSize, unused, textureMeterOffset);
               }
               out.push_back(v[0]);
               out.push_back(v[1]);
               out.push_back(v[2]);
               out.push_back(v[2]);      // degenerate, as createTriangleVertices
            }
         }
      }
   }

   numTriangles = (int)(out.size() / 4);
   vertexPCT2* vs = new vertexPCT2[out.size() > 0 ? out.size() : 1];
   for (size_t i = 0; i < out.size(); i++)
   {
      vs[i] = out[i];
   }
   return vs;
}

void PolygonVertices::setFadedVertices(
   oxygine::Polygon* polygon,
   const vector<Vector2>& vertices,
   const vector<VectorT3<int> >& triangles,
   const Vector2& textureMeterSize,
   const Vector2& textureMeterOffset,
   const Color& color,
   const PolygonFade& fade,
   const Vector2& origin,
   float angle)
{
   if ((fade.m_enabled && !fade.m_toColor) || fade.m_edgeEnabled)
   {
      int n = 0;
      vertexPCT2* vs = createFadedTriangleVertices(vertices, triangles, textureMeterSize, textureMeterOffset,
         color, fade, origin, angle, false, n);
      polygon->setVertices(vs, sizeof(vertexPCT2) * n * 4, vertexPCT2::FORMAT, true);
   }
   else
   {
      vertexPCT2* vs = createTriangleVertices(vertices, triangles, textureMeterSize, textureMeterOffset, color);
      polygon->setVertices(vs, sizeof(vertexPCT2) * triangles.size() * 4, vertexPCT2::FORMAT, true);
   }

   if (fade.m_enabled && fade.m_toColor)
   {
      // Haze overlay: the same triangles and texture, black vertex colour
      // with the fade's strength as alpha, and the fade colour as add colour.
      // oxygine's shader gives base * vertex colour + add colour * alpha, so
      // the overlay is the fade colour wherever the texture is, as strong as
      // the fade there (and edge-faded like the fill).
      int n = 0;
      vertexPCT2* ovs = createFadedTriangleVertices(vertices, triangles, textureMeterSize, textureMeterOffset,
         color, fade, origin, angle, true, n);
      spPolygon haze = new oxygine::Polygon();
      haze->setResAnim(polygon->getResAnim());
      haze->setAddColor(fade.m_color);
      haze->setVertices(ovs, sizeof(vertexPCT2) * n * 4, vertexPCT2::FORMAT, true);
      haze->setTouchEnabled(false);
      haze->attachTo(polygon);
   }
}

void PolygonVertices::createBodyPolygon(
   std::vector<oxygine::Vector2>& vertices,
   b2World* world,
   b2Vec2& pos,
   b2Body** body,
   b2Fixture** fixture,
   pugi::xml_node objectNode,
   bool staticBody,
   int groupIndex)
{
   int num = vertices.size() + 1;

   b2Vec2* b2vertices = new b2Vec2[num];

   // Polygon of a body shape is physical coordinates, i.e. in meters
   Vector2 tv;

   for (int i = 0; i < num - 1; i++)
   {
      tv = vertices[i];
      b2vertices[i] = PhysDispConvert::convert(tv, 1.0f);
   }

   tv = vertices[0];
   b2vertices[num - 1] = PhysDispConvert::convert(tv, 1.0f);

   b2BodyDef bodyDef;
   if (staticBody)
   {
      bodyDef.type = b2_staticBody;
   }
   else
   {
      bodyDef.type = b2_dynamicBody;
   }
   b2Vec2 delta = b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   b2Vec2 bPos = pos /*+ delta*/;
   bodyDef.position = bPos;
   bodyDef.angle = objectNode.attribute("angle").as_float() * MATH_PI / 180.0f;
   *body = world->CreateBody(&bodyDef);

   b2PolygonShape polyShape;
   polyShape.Set(b2vertices, num - 1);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &polyShape;

   *fixture = (*body)->CreateFixture(&fixtureDef);

}

void PolygonVertices::createChainBodyPolygon(
   std::vector<oxygine::Vector2>& vertices,
   b2World* world,
   b2Vec2& pos,
   b2Body*& body,
   b2Fixture*& fixture, 
   pugi::xml_node objectNode)
{
   int num = vertices.size() + 1;

   b2Vec2* b2vertices = new b2Vec2[num];

   // Polygon of a body shape is physical coordinates, i.e. in meters
   Vector2 tv;

   for (int i = 0; i < num - 1; i++)
   {
      tv = vertices[i];
      b2vertices[i] = PhysDispConvert::convert(tv, 1.0f);
   }

   tv = vertices[0];
   b2vertices[num - 1] = PhysDispConvert::convert(tv, 1.0f);

   b2BodyDef bodyDef;
   bodyDef.type = b2_staticBody;
   b2Vec2 bPos = pos;
   bodyDef.position = bPos;
   body = world->CreateBody(&bodyDef);

   b2ChainShape chain;
   chain.CreateChain(b2vertices, num);

   fixture = body->CreateFixture(&chain, 1);
}

void PolygonVertices::createPolygonBorders(
   oxygine::Polygon* polygon,
   vector<Vector2>& polygonVertices,
   pugi::xml_node objectNode)
{
   // Which border an edge gets (ground, ceiling, walls) depends on the
   // edge's direction, so it depends on which way round the polygon was
   // drawn. All polygons with borders so far go round with a negative
   // signed area (y axis down); a polygon drawn the other way round is
   // walked backwards here, so its ground border still ends up on the ground.
   float doubleArea = 0.0f;
   for (size_t i = 0; i < polygonVertices.size(); i++)
   {
      const Vector2& a = polygonVertices[i];
      const Vector2& b = polygonVertices[(i + 1) % polygonVertices.size()];
      doubleArea += a.x * b.y - b.x * a.y;
   }

   vector<Vector2> vertices(polygonVertices);
   if (doubleArea > 0.0f)
   {
      std::reverse(vertices.begin(), vertices.end());
   }

   // Here we should attach border to all edges of the polygon
   int edgeNum = vertices.size();

   xml_node borderNode = objectNode.child("border");

   if (!borderNode.empty())
   {
      // Prepare all xml data in advance to speed up processing
      float groundLeftAngle = borderNode.attribute("leftGroundAngle").as_float() * MATH_PI / 180.0f;
      float groundRightAngle = borderNode.attribute("rightGroundAngle").as_float() * MATH_PI / 180.0f;
      float ceilingLeftAngle = borderNode.attribute("leftCeilingAngle").as_float() * MATH_PI / 180.0f;
      float ceilingRightAngle = borderNode.attribute("rightCeilingAngle").as_float() * MATH_PI / 180.0f;

      bool groundBorder = false;
      bool ceilingBorder = false;
      bool leftWallBorder = false;
      bool rightWallBorder = false;

      string groundTexture = borderNode.child("groundBorder").attribute("texture").as_string("notDefined");
      string ceilingTexture = borderNode.child("ceilingBorder").attribute("texture").as_string("notDefined");
      string leftWallTexture = borderNode.child("leftWallBorder").attribute("texture").as_string("notDefined");
      string rightWallTexture = borderNode.child("rightWallBorder").attribute("texture").as_string("notDefined");

      float groundHorOffset;
      float groundTextMeterWidth;
      float groundTextMeterHeight;

      float ceilingHorOffset;
      float ceilingTextMeterWidth;
      float ceilingTextMeterHeight;

      float leftWallHorOffset;
      float leftWallTextMeterWidth;
      float leftWallTextMeterHeight;

      float rightWallHorOffset;
      float rightWallTextMeterWidth;
      float rightWallTextMeterHeight;

      if (groundTexture != "notDefined")
      {
         groundBorder = true;
         groundHorOffset = borderNode.child("groundBorder").attribute("horisontalMeterOffset").as_float();
         groundTextMeterWidth = borderNode.child("groundBorder").attribute("textureMeterWidth").as_float();
         groundTextMeterHeight = borderNode.child("groundBorder").attribute("textureMeterHeight").as_float();
      }

      if (ceilingTexture != "notDefined")
      {
         ceilingBorder = true;
         ceilingHorOffset = borderNode.child("ceilingBorder").attribute("horisontalMeterOffset").as_float();
         ceilingTextMeterWidth = borderNode.child("ceilingBorder").attribute("textureMeterWidth").as_float();
         ceilingTextMeterHeight = borderNode.child("ceilingBorder").attribute("textureMeterHeight").as_float();
      }

      if (leftWallTexture != "notDefined")
      {
         leftWallBorder = true;
         leftWallHorOffset = borderNode.child("leftWallBorder").attribute("horisontalMeterOffset").as_float();
         leftWallTextMeterWidth = borderNode.child("leftWallBorder").attribute("textureMeterWidth").as_float();
         leftWallTextMeterHeight = borderNode.child("leftWallBorder").attribute("textureMeterHeight").as_float();
      }

      if (rightWallTexture != "notDefined")
      {
         rightWallBorder = true;
         rightWallHorOffset = borderNode.child("rightWallBorder").attribute("horisontalMeterOffset").as_float();
         rightWallTextMeterWidth = borderNode.child("rightWallBorder").attribute("textureMeterWidth").as_float();
         rightWallTextMeterHeight = borderNode.child("rightWallBorder").attribute("textureMeterHeight").as_float();
      }

      if (groundBorder || ceilingBorder || leftWallBorder || rightWallBorder)
      {
         PolygonFade fade = readFade(objectNode, polygonVertices);
         fade.m_edgeEnabled = false;      // borders are not edge-faded

         // One border sprite per edge, from each vertex back to the one
         // before it (the editor draws them the same way). All edges get
         // one, including the closing edge from the last vertex to the first.
         for (int i = 0; i < edgeNum; i++)
         {
            Vector2 prev = vertices[(i + edgeNum - 1) % edgeNum];
            Vector2 current = vertices[i];
            Vector2 diffV = current - prev;
            float compAngle = atan2(diffV.y, diffV.x);
            float angle = compAngle + MATH_PI;

            if (angle > MATH_PI)
            {
               angle -= 2.0f * MATH_PI;
            }

            compAngle = angle;

            float distance = diffV.length();
            Vector2 rotatedNormal = Vector2(-sin(angle), cos(angle));

            bool doThisBorder = false;
            float horOffset;
            float textMeterWidth;
            float textMeterHeight;
            string texture;

            // Select border data by the angle and the angle limits
            if ((compAngle <= groundRightAngle) && (compAngle > groundLeftAngle))
            {
               // This is a ground border
               doThisBorder = groundBorder;
               if (doThisBorder)
               {
                  horOffset = groundHorOffset;
                  textMeterWidth = groundTextMeterWidth;
                  textMeterHeight = groundTextMeterHeight;
                  texture = groundTexture;
               }
            }
            else if ((compAngle > groundRightAngle) && (compAngle < ceilingRightAngle))
            {
               // This is a right wall border
               doThisBorder = rightWallBorder;
               if (doThisBorder)
               {
                  horOffset = rightWallHorOffset;
                  textMeterWidth = rightWallTextMeterWidth;
                  textMeterHeight = rightWallTextMeterHeight;
                  texture = rightWallTexture;
               }
            }
            else if ((compAngle < groundLeftAngle) && (compAngle > ceilingLeftAngle))
            {
               // This is a left wall border
               doThisBorder = leftWallBorder;
               if (doThisBorder)
               {
                  horOffset = leftWallHorOffset;
                  textMeterWidth = leftWallTextMeterWidth;
                  textMeterHeight = leftWallTextMeterHeight;
                  texture = leftWallTexture;
               }
            }
            else
            {
               // This is a ceiling border
               doThisBorder = ceilingBorder;
               if (doThisBorder)
               {
                  horOffset = ceilingHorOffset;
                  textMeterWidth = ceilingTextMeterWidth;
                  textMeterHeight = ceilingTextMeterHeight;
                  texture = ceilingTexture;
               }
            }

            if (doThisBorder)
            {
               spPolygon border = new oxygine::Polygon();
               border->setResAnim(g_GraphRes.getResAnim(texture));
               border->setAnchor(0.0f, 0.5f);
               border->setAddColor(polygon->getAddColor());      // same tint as the polygon

               vector<Vector2> vertices(4);
               vertices[0] = Vector2(0.0f, 0.0f);
               vertices[1] = Vector2(0.0f, textMeterHeight);
               vertices[2] = Vector2(distance, textMeterHeight);
               vertices[3] = Vector2(distance, 0.0f);

               vector<VectorT3<int> > triangles(2);

               triangles[0] = VectorT3<int>(1, 2, 3);
               triangles[1] = VectorT3<int>(1, 3, 4);

               // The polygon's tint, and its fade (in the polygon's
               // coordinates, so it continues across the border)
               PolygonVertices::setFadedVertices(
                  border.get(),
                  vertices,
                  triangles,
                  Vector2(textMeterWidth, textMeterHeight),
                  Vector2(0.0f, 0.0f),
                  polygon->getColor(),
                  fade,
                  current - rotatedNormal * horOffset,
                  angle);

               border->setAnchor(0.0f, 0.0f);
               border->setPosition(current - rotatedNormal * horOffset);
               border->setRotation(angle);

               border->attachTo(polygon);
            }
         }
      }
   }
}

b2Vec2 PolygonVertices::globalToLocalConversion(
   b2Vec2 ownGlobalPos,
   float cosAngle,
   float sinAngle,
   b2Vec2 targetGlobalPos)
{
   // Lets use this link: https://gamedev.stackexchange.com/questions/79765/how-do-i-convert-from-the-global-coordinate-space-to-a-local-space

   b2Vec2 localPos;

   localPos.x = (targetGlobalPos.x - ownGlobalPos.x) * cosAngle + (targetGlobalPos.y - ownGlobalPos.y) * sinAngle;

   // Since we have negative y-axis this is negative compared to the links answer
   localPos.y = -(targetGlobalPos.x - ownGlobalPos.x) * sinAngle + (targetGlobalPos.y - ownGlobalPos.y) * cosAngle;

   return localPos;
}

b2Vec2 PolygonVertices::localToGlobalConversion(
   b2Vec2 ownGlobalPos,
   float cosAngle,
   float sinAngle,
   b2Vec2 targetLocalPos)
{
   // Lets use this link: https://gamedev.stackexchange.com/questions/79765/how-do-i-convert-from-the-global-coordinate-space-to-a-local-space

   b2Vec2 globalPos;

   globalPos.x = targetLocalPos.x * cosAngle - targetLocalPos.y * sinAngle + ownGlobalPos.x;

   // Since we have negative y-axis this is negative compared to the links answer
   globalPos.y = targetLocalPos.x * sinAngle + targetLocalPos.y * cosAngle + ownGlobalPos.y;

   return globalPos;
}

