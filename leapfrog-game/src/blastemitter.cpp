
#include "blastemitter.h"
#include "blastparticle.h"

using namespace oxygine;

BlastEmitter::BlastEmitter(
   Vector2 emitterOrigin, 
   float intensity,
   int lifetime, 
   int particleLifeTime,
   float particleDistance,
   float particleSize) :
   m_emitterOrigin(emitterOrigin),
   m_intensity(intensity),
   m_timeToDie(0),
   m_timeOfNextParticle(0),
   m_particleLifeTime(particleLifeTime),
   m_lifetime(lifetime),
   m_particleDistance(particleDistance),
   m_particleSize(particleSize)
{
   m_stats.m_creationTime = getTimeMS();
}

void BlastEmitter::doUpdate(const oxygine::UpdateState& us)
{
   // If timeToDie is zero, this is the first update
   // so lets do some initialisation for data that uses
   // the us time.

   if (m_timeToDie == 0)
   {
      m_timeToDie = us.time + m_lifetime;

      int pri = (int)(1.0f / m_intensity * 1000.0f);

      m_timeOfNextParticle = us.time + pri;
   }
   //m_stats.m_updateTimeUS.push_back(us.time);
   //m_stats.m_updateTimeGT.push_back(getTimeMS());
   // m_stats.m_updateTimeClk.push_back(getClock()->getTime());
   
   if (us.time >= m_timeToDie)
   {
      m_stats.m_killTime = getTimeMS();

      // Kill this emitter and thus, stop emitting
      this->detach();
      return;
   }

   if (us.time >= m_timeOfNextParticle)
   {
      // Create particle
      spBlastParticle particle = new BlastParticle(m_emitterOrigin, m_particleLifeTime, m_particleDistance, m_particleSize);

      particle->attachTo(getParent());

      int pri = (int)(1.0f / m_intensity * 1000.0f);


      m_stats.m_pri.push_back(pri);

      //m_stats.m_timeOfParticleGeneration.push_back(now);

      m_timeOfNextParticle = us.time + pri;

      //m_stats.m_timeOfNextParticle.push_back(m_timeOfNextParticle);
   }
}
