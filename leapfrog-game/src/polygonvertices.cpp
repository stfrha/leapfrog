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

PolygonFade PolygonVertices::readFade(
   pugi::xml_node objectNode,
   const vector<Vector2>& polygonVertices)
{
   PolygonFade fade;

   xml_node fadeNode = objectNode.child("fade");
   if (fadeNode.empty() || polygonVertices.empty())
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

   // Sutherland-Hodgman: the part of a convex polygon with s <= cut
   // (keepLess) or s >= cut
   void clipFade(const vector<FadePoint>& in, float cut, bool keepLess, vector<FadePoint>& out)
   {
      out.clear();
      size_t n = in.size();
      for (size_t i = 0; i < n; i++)
      {
         const FadePoint& a = in[i];
         const FadePoint& b = in[(i + 1) % n];
         bool inA = keepLess ? (a.s <= cut) : (a.s >= cut);
         bool inB = keepLess ? (b.s <= cut) : (b.s >= cut);

         if (inA)
         {
            out.push_back(a);
         }
         if (inA != inB)
         {
            float t = (cut - a.s) / (b.s - a.s);
            FadePoint m;
            m.p = a.p + (b.p - a.p) * t;
            m.s = cut;
            out.push_back(m);
         }
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

   vector<vertexPCT2> out;
   out.reserve(triangles.size() * 4 * 3);

   vector<FadePoint> tri(3);
   vector<FadePoint> piece;
   vector<FadePoint> tmp;

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

      // Three bands: before the fade, the gradient, after it
      for (int band = 0; band < 3; band++)
      {
         if (band == 0)
         {
            clipFade(tri, lo, true, piece);
         }
         else if (band == 1)
         {
            clipFade(tri, lo, false, tmp);
            clipFade(tmp, hi, true, piece);
         }
         else
         {
            clipFade(tri, hi, false, piece);
         }

         if (piece.size() < 3)
         {
            continue;
         }

         for (size_t k = 1; k + 1 < piece.size(); k++)
         {
            const FadePoint* corner[3] = { &piece[0], &piece[k], &piece[k + 1] };
            vertexPCT2 v[3];
            for (int c = 0; c < 3; c++)
            {
               float f = fade.strength(corner[c]->s);
               Color vc;
               if (overlay)
               {
                  vc = Color(0, 0, 0, (unsigned char)(color.a * f + 0.5f));
               }
               else
               {
                  vc = Color(color.r, color.g, color.b, (unsigned char)(color.a * (1.0f - f) + 0.5f));
               }
               v[c] = initVertex(corner[c]->p, vc.premultiplied().rgba(), textureMeterSize, unused, textureMeterOffset);
            }
            out.push_back(v[0]);
            out.push_back(v[1]);
            out.push_back(v[2]);
            out.push_back(v[2]);      // degenerate, as createTriangleVertices
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
   if (fade.m_enabled && !fade.m_toColor)
   {
      int n = 0;
      vertexPCT2* vs = createFadedTriangleVertices(vertices, triangles, textureMeterSize, textureMeterOffset,
         color, fade, origin, angle, false, n);
      polygon->setVertices(vs, sizeof(vertexPCT2) * n * 4, vertexPCT2::FORMAT, true);
      return;
   }

   vertexPCT2* vs = createTriangleVertices(vertices, triangles, textureMeterSize, textureMeterOffset, color);
   polygon->setVertices(vs, sizeof(vertexPCT2) * triangles.size() * 4, vertexPCT2::FORMAT, true);

   if (fade.m_enabled && fade.m_toColor)
   {
      // Haze overlay: the same triangles and texture, black vertex colour
      // with the fade's strength as alpha, and the fade colour as add colour.
      // oxygine's shader gives base * vertex colour + add colour * alpha, so
      // the overlay is the fade colour wherever the texture is, as strong as
      // the fade there.
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

