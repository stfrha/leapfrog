#include "softboundary.h"
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
   setPriority(216);

   // Rotate sprite acc to position
   switch (m_repelDir)
   {
   case up:
      setRotation(0.0f);
      break;
   case down:
      setRotation(MATH_PI);
      break;
   case left:
      setRotation(-MATH_PI / 2.0f);
      break;
   case right:
      setRotation(MATH_PI / 2.0f);
      break;
   }
}

float SoftBoundary::determineThreshold(const RectF& rc)
{
   float threshold = 0.0f;

   if (m_repelDir == up)
   {
      threshold = rc.getY() - rc.getHeight() / 2;
   }

   if (m_repelDir == down)
   {
      threshold = rc.getY() + rc.getHeight() / 2;
   }

   if (m_repelDir == right)
   {
      // Note that we rotate the boundaries (to get the correct
      // orientation of the bitmap) so what would be width here
      // needs to be height
      threshold = rc.getX() + rc.getHeight() / 2;
   }

   if (m_repelDir == left)
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
   if (m_repelDir == up)
   {
      if (body->GetPosition().y > m_threshold)
      {
         penetrationLenght = body->GetPosition().y - m_threshold;

         repelForceMagnitude = penetrationLenght * penetrationLenght * 50.0f;

         forceVector = b2Vec2(0.0f, -repelForceMagnitude);

         body->ApplyForceToCenter(forceVector, true);
      }
   }

   if (m_repelDir == down)
   {
      if (body->GetPosition().y < m_threshold)
      {
         penetrationLenght = m_threshold - body->GetPosition().y;

         repelForceMagnitude = penetrationLenght * penetrationLenght * 50.0f;

         forceVector = b2Vec2(0.0f, repelForceMagnitude);

         body->ApplyForceToCenter(forceVector, true);
      }
   }

   if (m_repelDir == right)
   {
      if (body->GetPosition().x < m_threshold)
      {
         penetrationLenght = m_threshold - body->GetPosition().x;

         repelForceMagnitude = penetrationLenght * penetrationLenght * 50.0f;

         forceVector = b2Vec2(repelForceMagnitude, 0.0f);

         body->ApplyForceToCenter(forceVector, true);
      }
   }

   if (m_repelDir == left)
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

bool SoftBoundary::isInside(b2Body* body)
{
   // Test if body should be handled:
   if (m_repelDir == up)
   {
      if (body->GetPosition().y > m_threshold)
      {
         return true;
      }
   }

   if (m_repelDir == down)
   {
      if (body->GetPosition().y < m_threshold)
      {
         return true;
      }
   }

   if (m_repelDir == right)
   {
      if (body->GetPosition().x < m_threshold)
      {
         return true;
      }
   }

   if (m_repelDir == left)
   {
      if (body->GetPosition().x > m_threshold)
      {
         return true;
      }
   }

   return false;
}