#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"

class FreeSpaceActor;
class SceneActor;

DECLARE_SMART(Gun, spGun);

class Gun : public oxygine::Actor
{
private:
//   FreeSpaceActor * m_freeSpaceActor;
   oxygine::Resources* m_gameResources;

   bool m_fire;

	b2Body * m_emitterBody;

   // The focus origo of the emitter and the center of the emitter line.
   // Expressed in local coordinates of the body (i.e. related to its focus point)
   b2Vec2 m_emitterOrigin;
	
	// The angle in relation to the emitter body defines direction for particle to flow
	float m_emittAngle;		

	// Time between bullets
	float m_interval;

	int m_lifetime; // [ms}

   float m_impulseMagnitude;

   int m_timeSinceLast;

   SceneActor* m_sceneActor;

public:
	Gun(
      oxygine::Resources& gameResources, 
      SceneActor* sceneActor,
      b2Body* body,
      b2Vec2 emitterOrigin, 
      float angle, 
      float fireRate,
      int lifetime,
      float impulseMagnitude,
      bool bouncy
   );

//   void setBoundedWallsActor(FreeSpaceActor* actor);

	void startGun(void);
	void stopGun(void);


protected:
	void doUpdate(const oxygine::UpdateState& us);

};
