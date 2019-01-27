#pragma once

#include "system.h"
#include "scales.h"
#include "physdispconvert.h"

DECLARE_SMART(FlameEmitter, spFlameEmitter);

class SceneActor;
class CompoundObject;

class FlameEmitter : public System
{
private:
   // Parameters from XML
   b2Body * m_emitterBody;

   // The focus origo of the emitter and the center of the emitter line.
   // Expressed in local coordinates of the body (i.e. related to its focus point)
   b2Vec2 m_emitterOrigin;
	
	// The angle in relation to the emitter body defines direction for particle to flow
	float m_emittAngle;		

	// Particles are generate from a line with this width perpendicular to the emittAngle
	float m_emitterWidth;

	int m_lifetime; // [ms}

   float m_impulseMagnitude;

	float m_noiseConeAngle;	// Defines a cone angle of noise added to direction

   float m_radius;

   // Working member variable
   float m_scale;

   bool m_emit;
   int m_groupIndex;


   void readFlameEmitterNode(const pugi::xml_node& objectNode);

public:
	FlameEmitter(
      oxygine::Resources* gameResources,
      SceneActor* sceneActor,
      CompoundObject* parentObject,
      b2World* world,
      const pugi::xml_node& objectNode,
      int groupIndex);
	void startEmitter(void);
	void stopEmitter(void);
   void setFlameScale(float scale);
   void setParameters(
      int lifetime,
      float impulseMagnitude,
      float radius);

protected:
	void doUpdate(const oxygine::UpdateState& us);

};
