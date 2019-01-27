#pragma once

#include "oxygine-framework.h"


class BlastEmitterStatistics
{
public:
   std::vector<int>  m_timeOfParticleGeneration;
   std::vector<int>  m_updateTimeUS;
   std::vector<int>  m_updateTimeGT;
   std::vector<int>  m_updateTimeClk;
   std::vector<int>  m_timeOfNextParticle;
   std::vector<int>  m_pri;
   int  m_creationTime;
   int m_killTime;

   BlastEmitterStatistics()
   {
      m_timeOfParticleGeneration.clear();
      m_creationTime = 0;
      m_killTime = 0;
   }
};


DECLARE_SMART(BlastEmitter, spBlastEmitter);

class BlastEmitter : public oxygine::Actor
{
private:
   oxygine::Resources* m_gameResources;
   
   oxygine::Vector2 m_emitterOrigin;

   BlastEmitterStatistics m_stats;

	// Number of particle per second
	float m_intensity;

	int m_timeToDie; // [ms}
   int m_timeOfNextParticle;
   int m_particleLifeTime;
   float m_particleDistance;
   int m_lifetime;
   float m_particleSize;

public:
	BlastEmitter(
      oxygine::Resources* gameResources,
      oxygine::Vector2 emitterOrigin, 
      float intensity,
      int lifetime,
      int particleLifeTime,
      float particleDistance,
      float particleSize);

protected:
	void doUpdate(const oxygine::UpdateState& us);

};
