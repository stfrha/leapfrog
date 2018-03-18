#include "flameparticle.h"

#include "flameemitter.h"

using namespace oxygine;

FlameEmitter::FlameEmitter(
   Resources& gameResources, 
   b2Body* body, 
   b2Vec2 emitterOrigin, 
   float angle, 
   float emitterWidth,
   float intensity,
   float lifetime,
   float impulseMagnitude,
   float radius) :
   m_emit(false),
   m_emitterBody(body),
   m_emitterOrigin(emitterOrigin),
   m_emittAngle(angle),
   m_emitterWidth(emitterWidth),
   m_intensity(intensity),
   m_lifetime(lifetime),
   m_impulseMagnitude(impulseMagnitude),
   m_gameResources(&gameResources),
   m_radius(radius)
{

}

void FlameEmitter::startEmitter(void)
{
   m_emit = true;
}

void FlameEmitter::stopEmitter(void)
{
   m_emit = false;
}


void FlameEmitter::doUpdate(const oxygine::UpdateState& us)
{
   // FOr now, emit one particle every update if m_emit is true
   if (m_emit)
   {
//      b2Vec2 originPos = m_emitterBody->GetPosition() + m_emitterBody->GetWorldPoint(m_emitterOrigin);
      b2Vec2 originPos = m_emitterBody->GetWorldPoint(m_emitterOrigin);

      // Randomise value between -m_emitterWidth / 2 and m_emitterWidth / 2
      float dist = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / m_emitterWidth)) - m_emitterWidth / 2;

      float bodyAngle = m_emitterBody->GetAngle();
      float emitAngle = bodyAngle + m_emittAngle;

      b2Vec2 emitPos = b2Vec2(dist * cos(bodyAngle), dist * sin(bodyAngle)) + originPos;

      b2Vec2 impulse = b2Vec2(m_impulseMagnitude * cos(emitAngle), m_impulseMagnitude * sin(emitAngle));

      // Create one test flame particle
      spFlameParticle flameParticle = new FlameParticle(*m_gameResources, m_emitterBody->GetWorld(), emitPos, m_lifetime, impulse, m_radius);

      // Attach to parent's parent which is the view actor
      flameParticle->attachTo(getParent()->getParent());
      printf("Emitted flame\n");

   }

}
