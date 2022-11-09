#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "system.h"

#include "scales.h"
#include "physdispconvert.h"

DECLARE_SMART(ReentryFlameEmitter, spReentryFlameEmitter);

class ReentryFlameEmitter : public System
{
private:
   // Parameters from XML
   b2Body * m_emitterBody;

   // One of the points of the emitter line. 
   // Expressed in local coordinates of the body (i.e. related to its focus point)
   b2Vec2 m_emitterLineStart;

   // The other point of the emitter line
   // Expressed in local coordinates of the body (i.e. related to its focus point)
   b2Vec2 m_emitterLineEnd;

   // The angle in relation to the emitter body defines direction for particle to flow
   float m_emittAngle;

   // Number of particle per second
   float m_intensity;

   int m_lifetime; // [ms}

   b2Vec2 m_maxSize; // Defines the maximum size of the flames


   // Working member variable
   bool m_emit;

   // For debugging
   int m_particleCounter;

   // Intensity calculating members
   bool m_firstUpdate;
   int m_timeOfNextParticle; // [ms}


   void readReentryFlameEmitterNode(const pugi::xml_node& objectNode);


public:
	//ReentryFlameEmitter(
 //     oxygine::Resources& gameResources, 
 //     b2Body* body, 
 //     b2Vec2 emitterLineStart, 
 //     b2Vec2 emitterLineEnd,
 //     float angle,
 //     float intensity,
 //     int lifetime,
 //     b2Vec2 maxSize);

   ReentryFlameEmitter(
      SceneActor* sceneActor,
      CompoundObject* parentObject,
      b2World* world,
      const pugi::xml_node& objectNode);


   void startEmitter(void);
	void stopEmitter(void);
   void setParameters(float intensity,
      int lifetime,
      b2Vec2 maxSize);

   std::string dumpData(void);

protected:
	void doUpdate(const oxygine::UpdateState& us);

};
