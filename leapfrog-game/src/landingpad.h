#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "collisionentity.h"
#include "compoundobject.h"
#include "leapfrog.h"
#include <vector>

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

   // Holding the Leapfrog on the pad. When both feet touch down within
   // c_touchWindowMs of each other, at safe landing speeds (LandingLimits),
   // the feet and the main body are welded to the pad so it doesn't bounce.
   // Boosting releases it; it can only be held again after both feet have
   // left the pad.
   b2World* m_world;
   b2Body* m_padBody;
   oxygine::timeMS m_leftTouchTime;
   oxygine::timeMS m_rightTouchTime;
   b2Vec2 m_leftTouchVelocity;
   b2Vec2 m_rightTouchVelocity;
   bool m_armHold;
   std::vector<b2Joint*> m_holdJoints;

   static const oxygine::timeMS c_touchWindowMs = 400;

   static bool isSafeLandingVelocity(const b2Vec2& v);
   bool jointExists(b2Joint* joint);
   void holdLeapfrog(void);
   void releaseLeapfrog(void);

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
   void doUpdate(const oxygine::UpdateState &us) override;

};

