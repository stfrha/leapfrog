#include "oxygine-framework.h"
#include "polygonvertices.h"
#include "physdispconvert.h"
#include "scales.h"

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
   const Vector2& textureMeterOffset)
{
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

      *p = initVertex(pos, Color::White, textureMeterSize, alignment, textureMeterOffset);
      ++p;

      pos = Vector2(
         vertices[(size_t)triangles[t].y - 1].x,
         vertices[(size_t)triangles[t].y - 1].y);

      *p = initVertex(pos, Color::White, textureMeterSize, alignment, textureMeterOffset);
      ++p;

      pos = Vector2(
         vertices[(size_t)triangles[t].z - 1].x,
         vertices[(size_t)triangles[t].z - 1].y);

      *p = initVertex(pos, Color::White, textureMeterSize, alignment, textureMeterOffset);
      ++p;

      //Oxygine uses "triangles strip" rendering mode
      //dublicate last vertex (degenerate triangles)
      *p = initVertex(pos, Color::White, textureMeterSize, alignment, textureMeterOffset);
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
      textureMeterOffset);

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

   vertexPCT2* vs = createTriangleVertices(
      vertices,
      triangles,
      textureMeterSize,
      textureMeterOffset);

   polygon->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);
}

void PolygonVertices::createBodyPolygon(
   std::vector<oxygine::Vector2>& vertices,
   b2World* world,
   b2Vec2& pos,
   b2Body** body,
   b2Fixture** fixture,
   pugi::xml_node objectNode,
   bool staticBody)
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
   b2Vec2 bPos = pos + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   bodyDef.angle = objectNode.attribute("angle").as_float();
   *body = world->CreateBody(&bodyDef);

   b2PolygonShape polyShape;
   polyShape.Set(b2vertices, num - 1);

   b2FixtureDef fixtureDef;
   fixtureDef.shape = &polyShape;

   *fixture = (*body)->CreateFixture(&fixtureDef);

   b2Filter filter;
   filter.categoryBits = objectNode.attribute("collisionCategory").as_int();
   filter.maskBits = objectNode.attribute("collisionMask").as_int();
   (*fixture)->SetFilterData(filter);

   (*fixture)->SetDensity(objectNode.attribute("density").as_float(1.0f));
   (*fixture)->SetFriction(objectNode.attribute("friction").as_float(1.0f));

}

void PolygonVertices::createChainBodyPolygon(
   std::vector<oxygine::Vector2>& vertices,
   b2World* world,
   b2Vec2& pos,
   b2Body* body,
   b2Fixture* fixture, 
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
   b2Vec2 bPos = pos + b2Vec2(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   bodyDef.position = bPos;
   body = world->CreateBody(&bodyDef);

   b2ChainShape chain;
   chain.CreateChain(b2vertices, num);

   fixture = body->CreateFixture(&chain, 1);
}

void PolygonVertices::createPolygonBorders(
   oxygine::Polygon* polygon,
   Resources& gameResources,
   vector<Vector2>& vertices,
   pugi::xml_node objectNode)
{
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

         // Polygon of a body shape is physical coordinates, i.e. in meters
         for (int i = 1; i < edgeNum - 1; i++)
         {
            // We draw a border sprite from prev to current. Set anchor to 0,0.5
            // So position should be prev. Calculate angle between points and
            // set that to sprite. Then attach it to polygon.

            Vector2 prev = vertices[i - 1];
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

            // angle is the amount to rotate the texture. This should be between 
            // -pi/2 and pi/2 for ground and above 3*pi/2 or below -3*pi/2 for ceiling.
            // compAngle is -PI away, i.e. Ground should have compAngle from pi/2 to 3pi/2
            // But why does it not work with angle (non-comp)?
            // compAngle goes from -pi to pi which means that angle goes from
            // 0 to 2*pi. This is the problem. We would like to change so that
            // angle goes from -pi to pi. I.e. If angle > pi, angle = angle - 2 * pi

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
               border->setResAnim(gameResources.getResAnim(texture));
               border->setAnchor(0.0f, 0.5f);

               vector<Vector2> vertices(4);
               vertices[0] = Vector2(0.0f, 0.0f);
               vertices[1] = Vector2(0.0f, textMeterHeight);
               vertices[2] = Vector2(distance, textMeterHeight);
               vertices[3] = Vector2(distance, 0.0f);

               vector<VectorT3<int> > triangles(2);

               triangles[0] = VectorT3<int>(1, 2, 3);
               triangles[1] = VectorT3<int>(1, 3, 4);

               vertexPCT2* vs = PolygonVertices::createTriangleVertices(
                  vertices,
                  triangles,
                  Vector2(textMeterWidth, textMeterHeight),
                  Vector2(0.0f, 0.0f));

               border->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

               border->setAnchor(0.0f, 0.0f);
               border->setPosition(current - rotatedNormal * horOffset);
               border->setRotation(angle);

               border->attachTo(polygon);

            }
         }

         Vector2 prev = vertices[edgeNum - 1];
         Vector2 current = vertices[0];
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
            if (doThisBorder)
            {
               doThisBorder = groundBorder;
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
            border->setResAnim(gameResources.getResAnim(texture));
            border->setAnchor(0.0f, 0.5f);

            vector<Vector2> vertices(4);
            vertices[0] = Vector2(0.0f, 0.0f);
            vertices[1] = Vector2(0.0f, textMeterHeight);
            vertices[2] = Vector2(distance, textMeterHeight);
            vertices[3] = Vector2(distance, 0.0f);

            vector<VectorT3<int> > triangles(2);

            triangles[0] = VectorT3<int>(1, 2, 3);
            triangles[1] = VectorT3<int>(1, 3, 4);

            vertexPCT2* vs = PolygonVertices::createTriangleVertices(
               vertices,
               triangles,
               Vector2(textMeterWidth, textMeterHeight),
               Vector2(0.0f, 0.0f));

            border->setVertices(vs, sizeof(vertexPCT2) *  triangles.size() * 4, vertexPCT2::FORMAT, true);

            border->setAnchor(0.0f, 0.0f);
            border->setPosition(current - rotatedNormal * horOffset);
            border->setRotation(angle);

            border->attachTo(polygon);

         }
      }
   }
}
