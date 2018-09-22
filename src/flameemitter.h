#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "sceneactor.h"

DECLARE_SMART(FlameEmitter, spFlameEmitter);

class SceneActor;

class FlameEmitter /*: public oxygine::Actor*/
{
private:
   oxygine::Resources* m_gameResources;

   bool m_emit;

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

   float m_scale;

   SceneActor* m_sceneActor;



public:
	FlameEmitter(
      oxygine::Resources& gameResources,
      SceneActor* sceneActor,
      b2Body* body, 
      b2Vec2 emitterOrigin, 
      float angle, 
      float emitterWidth,
      int lifetime,
      float impulseMagnitude,
      float radius);
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
