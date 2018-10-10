#include "sceneactor.h"
#include "compoundobject.h"
#include "flameparticle.h"
#include "flameemitter.h"

using namespace oxygine;
using namespace pugi;

FlameEmitter::FlameEmitter(
   Resources* gameResources,
   SceneActor* sceneActor,
   CompoundObject* parent,
   b2World* world,
   const xml_node& objectNode,
   int groupIndex)  :
   System(
      gameResources,
      sceneActor,
      world, 
      parent),
   m_groupIndex(groupIndex),
   m_emit(false),
   m_scale(1.0f)
{
   readFlameEmitterNode(objectNode);
   attachTo(sceneActor);
}

void FlameEmitter::readFlameEmitterNode(const xml_node& objectNode)
{
   m_emitterBody = m_parent->getBody(objectNode.attribute("body").as_string());

   m_emitterOrigin.x = objectNode.attribute("emitterOriginX").as_float();
   m_emitterOrigin.y = objectNode.attribute("emitterOriginY").as_float();
   m_emittAngle = objectNode.attribute("angle").as_float();
   m_emitterWidth = objectNode.attribute("emitterWidth").as_float();
   m_radius = objectNode.attribute("radius").as_float();
   m_lifetime = objectNode.attribute("lifeTime").as_int(); // [ms}
   m_impulseMagnitude = objectNode.attribute("impulseMagnitude").as_float();
}


void FlameEmitter::startEmitter(void)
{
   m_emit = true;
}

void FlameEmitter::stopEmitter(void)
{
   m_emit = false;
}

void FlameEmitter::setFlameScale(float scale)
{
   m_scale = scale;
}

void FlameEmitter::setParameters(
   int lifetime,
   float impulseMagnitude,
   float radius)
{
   m_lifetime = lifetime;
   m_impulseMagnitude = impulseMagnitude;
   m_radius = radius;
}



void FlameEmitter::doUpdate(const oxygine::UpdateState& us)
{
   // For now, emit one particle every update if m_emit is true
   if (m_emit)
   {
//      b2Vec2 originPos = m_emitterBody->GetPosition() + m_emitterBody->GetWorldPoint(m_emitterOrigin);
      b2Vec2 originPos = m_emitterBody->GetWorldPoint(m_emitterOrigin);

      // Randomise value between -m_emitterWidth / 2 and m_emitterWidth / 2
      float dist = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / m_emitterWidth)) - (m_emitterWidth / 2.0f);

      // Randomise angle to get spread
      float ra = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (30.0f / 180.0f * MATH_PI))) - (15.0f / 180.0f * MATH_PI);

      float bodyAngle = m_emitterBody->GetAngle() + ra;
      float emitAngle = bodyAngle + m_emittAngle;

      b2Vec2 emitPos = b2Vec2(dist * cos(bodyAngle), dist * sin(bodyAngle)) + originPos;

      // Randomise impulse magnistude to get spread
      float ri = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 250.0f)) - 125.0f;

      b2Vec2 impulse = b2Vec2((ri + m_impulseMagnitude) * cos(emitAngle), (ri + m_impulseMagnitude) * sin(emitAngle));

      // Create one test flame particle
      spFlameParticle flameParticle = new FlameParticle(
         *m_gameResources, 
         m_emitterBody->GetWorld(), 
         emitPos, 
         m_emitterBody->GetLinearVelocity(),
         (int)((float)m_lifetime * m_scale), 
         m_scale * impulse,
         m_radius * m_scale,
         m_groupIndex);

      // Attach to parent's parent which is the view actor
      flameParticle->attachTo(m_sceneActor);
   }
}
