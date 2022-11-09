#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "collisionentity.h"
#include "compoundobject.h"
#include "leapfrog.h"

class SceneActor;

DECLARE_SMART(LandingPad, spLandingPad);

/*
Available events:
- leapfrog landed  
*/

class LandingPad : public CompoundObject
{
public:
	enum propertyId
	{
		state = 0,
		lfLanded = 1
	};

   enum LandingPadState
   {
      unstable,
      stableLand,
      stableAir
   };

private:
   bool m_leftFootContact;
   bool m_rightFootContact;
   int m_stableTicks;
   LandingPadState m_state;

   LeapFrog* m_latestLeapfrog;

public:
	LandingPad(
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);

   void leapfrogFootTouch(b2Contact* contact, bool leftFoot, LeapFrog* leapfrog);
   void leapfrogFootLift(b2Contact* contact, bool leftFoot, LeapFrog* leapfrog);

protected:
   void doUpdate(const oxygine::UpdateState &us);

};

