#pragma once

#include "oxygine-framework.h"
#include "Box2d/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"

class SceneActor;
class CompoundObject;

DECLARE_SMART(Gun, spGun);

class Gun : public System
{
private:
   // Parameters from XML
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

   bool m_bouncy;

   // Working member variable
   bool m_fire;
   int m_timeSinceLast;
   int m_groupIndex;


   void readGunNode(const pugi::xml_node& objectNode);


public:
	Gun(
      oxygine::Resources* gameResources,
      SceneActor* sceneActor,
      CompoundObject* parentObject,
      b2World* world,
      const pugi::xml_node& objectNode,
      int groupIndex);

//   void setBoundedWallsActor(FreeSpaceActor* actor);

	void startGun(void);
	void stopGun(void);


protected:
	void doUpdate(const oxygine::UpdateState& us);

};
