#include "reentryflameparticle.h"

#include "reentryflameemitter.h"

using namespace oxygine;

ReentryFlameEmitter::ReentryFlameEmitter(
   Resources& gameResources, 
   b2Body* body, 
   b2Vec2 emitterLineStart,
   b2Vec2 emitterLineEnd,
   float angle,
   float intensity,
   int lifetime,
   b2Vec2 maxSize) :
   m_emit(false),
   m_emitterBody(body),
   m_emitterLineStart(emitterLineStart),
   m_emitterLineEnd(emitterLineEnd),
   m_emittAngle(angle),
   m_intensity(intensity),
   m_lifetime(lifetime),
   m_maxSize(maxSize),
   m_gameResources(&gameResources),
   m_firstUpdate(true),
   m_timeOfNextParticle(0)
{

}

void ReentryFlameEmitter::startEmitter(void)
{
   m_emit = true;
}

void ReentryFlameEmitter::stopEmitter(void)
{
   m_emit = false;
}

void ReentryFlameEmitter::setParameters(float intensity,
   int lifetime,
   b2Vec2 maxSize)
{
   m_intensity = intensity;
   m_lifetime = lifetime;
   m_maxSize = maxSize;
}



void ReentryFlameEmitter::doUpdate(const oxygine::UpdateState& us)
{
   // If m_firstUpdate is true, this is the first update
   // so lets do some initialisation for data that uses
   // the us time.

   if (m_emit)
   {
      if (m_firstUpdate)
      {
         int pri = (int)(1.0f / m_intensity * 1000.0f);
         m_timeOfNextParticle = us.time + pri;

         m_firstUpdate = false;
      }

      if (us.time >= m_timeOfNextParticle)
      {
         // Create particle

         b2Vec2 startPos = m_emitterBody->GetWorldPoint(m_emitterLineStart);
         b2Vec2 endPos = m_emitterBody->GetWorldPoint(m_emitterLineEnd);

         b2Vec2 emittVector = endPos - startPos;
         float maxDist = emittVector.Normalize();

         // emittVector is now normalised and has the direction
         // from start to end pos

         // Randomise value between 0 and maxDist
         float dist = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / maxDist));

         emittVector *= dist;

         b2Vec2 emitPos = startPos + emittVector;

         float bodyAngle = m_emitterBody->GetAngle();
         float emitAngle = bodyAngle + m_emittAngle;

         // Create one test flame particle
         spReentryFlameParticle flameParticle = new ReentryFlameParticle(
            m_gameResources,
            PhysDispConvert::convert(emitPos, 1.0f),
            m_lifetime,
            PhysDispConvert::convert(m_maxSize, 1.0f));

         flameParticle->setAnchor(0.5f, 1.0f);
         flameParticle->setRotation(emitAngle - MATH_PI / 2.0f);

         // Attach to parent's parent which is the view actor
         flameParticle->attachTo(getParent()->getParent());

         int pri = (int)(1.0f / m_intensity * 1000.0f);
         m_timeOfNextParticle = us.time + pri;
      }
   }
   else
   {
      m_firstUpdate = false;
   }
}
