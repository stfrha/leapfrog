#include "softboundary.h"
#include "physdispconvert.h"
#include "scales.h"
#include "layout.h"
#include "graphicresources.h"

using namespace oxygine;

SoftBoundary::SoftBoundary(
   RepelDirectionEnum repelDir) :
   m_repelDir(repelDir)
{

   // rc is in stage coordinates, i.e. meters
   RectF rc;

   // Place the sprite at the correct position
   // according to the direction specifier
   switch (m_repelDir)
   {
   case up:
      rc = RectF(
         g_Layout.getStageBounds().getLeft() + g_Layout.getStageBounds().getWidth() / 2.0f,
         g_Layout.getStageBounds().getBottom() - g_Layout.getBoundaryWidth() / 2.0f,
         g_Layout.getStageBounds().getWidth(),
         g_Layout.getBoundaryWidth());
      break;
   case down:
      rc = RectF(
         g_Layout.getStageBounds().getLeft() + g_Layout.getStageBounds().getWidth() / 2.0f,
         g_Layout.getStageBounds().getTop() + g_Layout.getBoundaryWidth() / 2.0f,
         g_Layout.getStageBounds().getWidth(),
         g_Layout.getBoundaryWidth());
      break;
   case left:
      rc = RectF(
         g_Layout.getStageBounds().getRight() - g_Layout.getBoundaryWidth() / 2.0f,
         g_Layout.getStageBounds().getTop() + g_Layout.getStageBounds().getHeight() / 2.0f,
         g_Layout.getStageBounds().getHeight(),
         g_Layout.getBoundaryWidth());
      break;
   case right:
      rc = RectF(
         g_Layout.getStageBounds().getLeft() + g_Layout.getBoundaryWidth() / 2.0f,
         g_Layout.getStageBounds().getTop() + g_Layout.getStageBounds().getHeight() / 2.0f,
         g_Layout.getStageBounds().getHeight(),
         g_Layout.getBoundaryWidth());
      break;
   }

   m_threshold = determineThreshold(rc);

   // size and position here is in stage coordinate (same as rc)
   setSize(rc.getSize());
   setPosition(rc.getLeftTop());
   setAnchor(Vector2(0.5f, 0.5f));
   setPriority(216);

   Sprite* mask = new Sprite();
   mask->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::game).getResAnim("deep_space_boundary"));
   mask->setSize(rc.getSize().x, rc.getSize().y - 5);
   mask->attachTo(this);
   mask->setVisible(false);

   // Add a white sprite masked by a alpha-sprite:
   spMaskedSprite maskedSprite = new MaskedSprite();
   maskedSprite->setAnchor(Vector2(0.5f, 0.5f));
   maskedSprite->attachTo(this);

   oxygine::ColorRectSprite* maskBackground = new  oxygine::ColorRectSprite();
   maskBackground->setColor(oxygine::Color::White);
   maskBackground->setSize(rc.getSize());
   maskBackground->setPosition(0, 1);
   maskBackground->attachTo(maskedSprite);

   maskedSprite->setMask(mask, true);

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


SoftBoundary::RepelDirectionEnum SoftBoundary::getDirection(void)
{
   return m_repelDir;
}