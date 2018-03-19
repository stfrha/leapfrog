#include "SoftBoundary.h"
#include "physdispconvert.h"
#include "scales.h"

using namespace oxygine;

// rc is in stage coordinates, i.e. meters
SoftBoundary::SoftBoundary(
   oxygine::Resources& gameResources,
   b2World* world,
   const RectF& rc,
   RepelDirectionEnum repelDir) :
   m_repelDir(repelDir),
   m_threshold(determineThreshold(rc))
{
	setHorizontalMode(Box9Sprite::TILING_FULL);
	//setVerticalMode(Box9Sprite::TILING_FULL);
	setResAnim(gameResources.getResAnim("deep_space_boundary"));

   //setAlpha(0.75f);

	// size and position here is in stage coordinate (same as rc)
	setSize(rc.getSize());
	setPosition(rc.getLeftTop());
	setAnchor(Vector2(0.5f, 0.5f));

   // Rotate sprite acc to position
   switch (m_repelDir)
   {
   case RDE_UP:
      setRotation(0.0f);
      break;
   case RDE_DOWN:
      setRotation(MATH_PI);
      break;
   case RDE_LEFT:
      setRotation(-MATH_PI / 2.0f);
      break;
   case RDE_RIGHT:
      setRotation(MATH_PI / 2.0f);
      break;
   }


	//b2BodyDef groundBodyDef;
 //  groundBodyDef.type = b2_kinematicBody;
	//groundBodyDef.position = PhysDispConvert::convert(getPosition(), 1.0);

   //// Rotate body acc to position
   //switch (m_repelDir)
   //{
   //case RDE_UP:
   //   groundBodyDef.angle = 0.0f;
   //   break;
   //case RDE_DOWN:
   //   groundBodyDef.angle = MATH_PI;
   //   break;
   //case RDE_LEFT:
   //   groundBodyDef.angle = -MATH_PI / 2.0f;
   //   break;
   //case RDE_RIGHT:
   //   groundBodyDef.angle = MATH_PI / 2.0f;
   //   break;
   //}


	//b2Body* boundaryBody = world->CreateBody(&groundBodyDef);

 //  setUserData(boundaryBody);

	//b2PolygonShape boundaryBox;
	//b2Vec2 sz = PhysDispConvert::convert(getSize() / Scales::c_physToStageScale / 2, 1);
	//boundaryBox.SetAsBox(sz.x, sz.y);
	//boundaryBody->CreateFixture(&boundaryBox, 0.0f);
 //  
 //  // The soft boundary only one fixture, so it is ok to use it
 //  b2Fixture* f = boundaryBody->GetFixtureList();

 //  b2Filter filter = f->GetFilterData();

 //  filter.groupIndex = -3;

 //  f->SetFilterData(filter);

 //  boundaryBody->SetUserData(this);


}

float SoftBoundary::determineThreshold(const RectF& rc)
{
   float threshold = 0.0f;

   if (m_repelDir == RDE_UP)
   {
      threshold = rc.getY() - rc.getHeight() / 2;
   }

   if (m_repelDir == RDE_DOWN)
   {
      threshold = rc.getY() + rc.getHeight() / 2;
   }

   if (m_repelDir == RDE_RIGHT)
   {
      // Note that we rotate the boundaries (to get the correct
      // orientation of the bitmap) so what would be width here
      // needs to be height
      threshold = rc.getX() + rc.getHeight() / 2;
   }

   if (m_repelDir == RDE_LEFT)
   {
      // Note that we rotate the boundaries (to get the correct
      // orientation of the bitmap) so what would be width here
      // needs to be height
      threshold = rc.getX() - rc.getHeight() / 2;
   }

   return threshold;
}

void SoftBoundary::testForRepel(b2Body* body)
{
   float penetrationLenght = 0.0f;
   float repelForceMagnitude = 0.0f;
   b2Vec2 forceVector;

   // Test if body should be handled:
   if (m_repelDir == RDE_UP)
   {
      if (body->GetPosition().y > m_threshold)
      {
         penetrationLenght = body->GetPosition().y - m_threshold;

         repelForceMagnitude = penetrationLenght * penetrationLenght * 50.0f;

         forceVector = b2Vec2(0.0f, -repelForceMagnitude);

         body->ApplyForceToCenter(forceVector, true);
      }
   }

   if (m_repelDir == RDE_DOWN)
   {
      if (body->GetPosition().y < m_threshold)
      {
         penetrationLenght = m_threshold - body->GetPosition().y;

         repelForceMagnitude = penetrationLenght * penetrationLenght * 50.0f;

         forceVector = b2Vec2(0.0f, repelForceMagnitude);

         body->ApplyForceToCenter(forceVector, true);
      }
   }

   if (m_repelDir == RDE_RIGHT)
   {
      if (body->GetPosition().x < m_threshold)
      {
         penetrationLenght = m_threshold - body->GetPosition().x;

         repelForceMagnitude = penetrationLenght * penetrationLenght * 50.0f;

         forceVector = b2Vec2(repelForceMagnitude, 0.0f);

         body->ApplyForceToCenter(forceVector, true);
      }
   }

   if (m_repelDir == RDE_LEFT)
   {
      if (body->GetPosition().x > m_threshold)
      {
         penetrationLenght = body->GetPosition().x - m_threshold;

         repelForceMagnitude = penetrationLenght * penetrationLenght * 50.0f;

         forceVector = b2Vec2(-repelForceMagnitude, 0.0f);

         body->ApplyForceToCenter(forceVector, true);
      }
   }

}