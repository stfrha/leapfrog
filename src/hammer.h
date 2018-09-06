#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "steeringmanager.h"

class SceneActor;
class AsteroidField;

DECLARE_SMART(Hammer, spHammer);

class Hammer : public CompoundObject
{
public:
   enum launchState
   {
      start,
      goRight,
      goUp,
      goLeft,
      goDown
   };


private:
   oxygine::Resources* m_gameResource;
   b2World* m_world;
   b2Body* m_body;

   launchState m_state;
   oxygine::timeMS m_stateStartTime;
   SteeringManager* m_steeringManager;

   int   m_damage;

   SceneActor* m_sceneActor;
   
public:
	Hammer(
      oxygine::Resources& gameResources, 
      SceneActor* sceneActor,
      b2World* world,
      const oxygine::Vector2& pos);

   virtual CollisionEntityTypeEnum getEntityType(void);

   void hitByBullet(b2Contact* contact); 
   void hitShield(b2Contact* contact);     // Returns true if hammer was shattered
   void hitByLepfrog(b2Contact* contact);

protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atDeathOfAsteroid(void);

};
