#include <sstream>

#include "reentryflameemitter.h"
#include "reentryflameparticle.h"
#include "sceneactor.h"
#include "compoundobject.h"

using namespace std;
using namespace oxygine;
using namespace pugi;

ReentryFlameEmitter::ReentryFlameEmitter(
   Resources* gameResources,
   SceneActor* sceneActor,
   CompoundObject* parent,
   b2World* world,
   const xml_node& objectNode) :
   System(
      gameResources,
      sceneActor,
      world,
      parent),
   m_emit(false),
   m_firstUpdate(true)
{
   readReentryFlameEmitterNode(objectNode);
   attachTo(sceneActor);

   m_particleCounter = 0;
}

void ReentryFlameEmitter::readReentryFlameEmitterNode(const xml_node& objectNode)
{
   m_emitterBody = m_parent->getBody(objectNode.attribute("body").as_string());

   m_emitterLineStart.x = objectNode.attribute("emitterLineStartX").as_float();
   m_emitterLineStart.y = objectNode.attribute("emitterLineStartY").as_float();
   m_emitterLineEnd.x = objectNode.attribute("emitterLineEndX").as_float();
   m_emitterLineEnd.y = objectNode.attribute("emitterLineEndY").as_float();
   m_emittAngle = objectNode.attribute("angle").as_float();

   m_intensity = objectNode.attribute("intensity").as_float();
   m_lifetime = objectNode.attribute("lifeTime").as_int(); // [ms}

   m_maxSize.x = objectNode.attribute("maxWidth").as_float();
   m_maxSize.y = objectNode.attribute("maxHeight").as_float();
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
         flameParticle->attachTo(m_sceneActor);

         m_particleCounter++;

         int pri = (int)(1.0f / m_intensity * 1000.0f);
         m_timeOfNextParticle = us.time + pri;
      }
   }
   else
   {
      m_firstUpdate = true;
   }
}

string ReentryFlameEmitter::dumpData(void)
{
   b2Vec2 startPos = m_emitterBody->GetWorldPoint(m_emitterLineStart);
   b2Vec2 endPos = m_emitterBody->GetWorldPoint(m_emitterLineEnd);

   ostringstream  message;
   
   message << "(World coordinates) Start.x: " << startPos.x << ", Start.y: " << startPos.y;
   message << " End.x: " << endPos.x << ", End.y: " << endPos.y;
   message << " particles generated: " << m_particleCounter << endl;

   return message.str();
}


