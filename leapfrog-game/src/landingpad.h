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
- leapfrog landed (LpLL), leapfrog took off (LpTO): only while the pad
  is active

Properties (for Lua: c_getObjectProperty / c_setObjectProperty):
- 0 state: 0 unstable, 1 landed, 2 in the air (read only)
- 1 lfLanded: 1 while the Leapfrog stands on the pad (read only)
- 2 active: 1 = active, 0 = inactive. The Leapfrog can land on an
  inactive pad (it is held and zoomed in on as usual), but nothing
  happens: no events and no refuelling. Set from the behaviour's
  active attribute (default true). Activating the pad while the
  Leapfrog stands on it sends the landed event at once.
*/

class LandingPad : public CompoundObject
{
public:
	enum propertyId
	{
		state = 0,
		lfLanded = 1,
		active = 2
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
   bool m_active;
   bool m_announceLanded;     // activated while landed: send the landed event in doUpdate

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

   static const oxygine::timeMS c_touchWindowMs = 400;   // both feet down within this
   static const oxygine::timeMS c_landedZoomMs = 5000;   // zoom-in duration after landing
   static const oxygine::timeMS c_takeOffZoomMs = 3000;  // zoom-out duration at take-off

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

   bool isActive(void) const { return m_active; }

protected:
   void doUpdate(const oxygine::UpdateState &us) override;
   void setPropertyImpl(int propId, float value) override;

};

