#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "collisionentity.h"
#include "compoundobject.h"
#include "flameemitter.h"

class LeapFrog;
class SceneActor;

DECLARE_SMART(LaunchSite, spLaunchSite);

/*
Available events:
- leapfrog landed on leg rest 
- isPrepared
- launchSequenceComplete
- hasLaunched

Available states:
- idle, boosters and tank attached, Leapfrog has not landed
- extendGrabber, grabber is extended to just below Leapfrog (if there 
                 is a prompt for not launching, this is the place)
- grabLeapfrog, Leapfrog is grabbed, i.e. welded to the extended grabber
- lowerFootRests, Foot rests are rotated down
- attachLeapfrog, grabber is retacted pulling Leapfrog with it, Leapfrog goes to Deep Space Mode
- countdown, count down from five on the HUD log; the engines are
             ignited half a second after "4 seconds to launch"
- ignite, engines running, count down to zero
- tZero, apply force to main booster, stay for 0.5 s
- release, release joints holding main tank booster

- prepare, leg rests rotate down, leapfrog is attached to spring which is draged into main tank, (leapfrog goes to deep space mode)
- launch sequence - Stoppable countdown, smoke and small engine fire, 
- launch execute - Unstoppable countdown, booster fire, is released and all goes up

Properties (for Lua: c_getObjectProperty / c_setObjectProperty):
- 0 state: LaunchStateEnum (read only)
- 1 trigger launch: setting it starts the launch sequence (works whether
  the site is active or not: the script decides)
- 2 active: 1 = active, 0 = inactive. The Leapfrog can land on an
  inactive launch site, but it sends no landed event (LsLL), so nothing
  starts. Set from the behaviour's active attribute (default true).
  Activating it while the Leapfrog stands on it sends the event at once.
- 3 abort launch: setting it (to any value) takes back a triggered launch
  while the site still waits for clearance (state leapfrogLanded), back
  to idle. Later the sequence has the Leapfrog in its grabber and runs to
  the end; the setting is then ignored (logged). Reads 0.

Questions (SceneActor::askQuestion): the Leapfrog counts as standing on
the site while it is idle with both feet on it, and while a triggered
launch waits in leapfrogLanded. A triggered launch waits there for as long
as a question is pending (open or queued), so a question can decide
whether to go on or abort (property 3). Leaving the site before the
grabber moves cancels the launch (back to idle; the next landing sends
LsLL again).
*/




class LaunchSite : public CompoundObject
{
public:
	enum propertyId
	{
		state = 0,
		lfLanded = 1,     // property 1 is the launch trigger
		active = 2,
		abortLaunch = 3
	};

   enum LaunchStateEnum
   {
      idle,
      leapfrogLanded,
      extendGrabber,
      lowerFootRests,
      attachLeapfrog,
      countdown,
      ignite,
      tZero,
      release,
      supportBoosterBurnout,
      dropSupportBooster,
      mainBoosterBurnout,
      dropMainBooster,
      finishSequence
   };

private:
   b2World * m_world;
   SceneActor* m_sceneParent;

   bool m_leftFootContact;
   bool m_rightFootContact;

   LaunchStateEnum m_state;
   oxygine::timeMS m_stateStartTime;
   int m_countdownCounter;
   bool m_enginesIgnited;
   bool m_armInitialRegistration;
   bool m_active;
   bool m_announceLanded;     // activated while landed: send the landed event in doUpdate
   bool m_launchOnHold;       // waiting in leapfrogLanded for a question

   b2RevoluteJoint* m_leftFootRestJoint;
   b2RevoluteJoint* m_rightFootRestJoint;
   b2Joint * m_leftHolderJoint;
   b2Joint* m_rightHolderJoint;
   b2RevoluteJoint* m_leftHolderTrolleyJoint;
   b2RevoluteJoint* m_rightHolderTrolleyJoint;
   b2PrismaticJoint* m_grabberJoint;
   b2WeldJoint* m_leftSupportBoosterJoint;
   b2WeldJoint* m_rightSupportBoosterJoint;

   LeapFrog* m_leapFrog;
   b2Body*  m_grabberBody;
   b2RevoluteJointDef*	m_grabLeapfrogJointDef1;
   b2RevoluteJointDef*	m_grabLeapfrogJointDef2;
   b2RevoluteJoint* m_grabLeapfrogJoint1;
   b2RevoluteJoint* m_grabLeapfrogJoint2;

   oxygine::spActor m_tankActor;

   b2Body*  m_mainTankBody;
   b2Body*  m_leftBoosterBody;
   b2Body*  m_rightBoosterBody;

   spFlameEmitter m_boosterFlame;
   spFlameEmitter m_leftFlame;
   spFlameEmitter m_rightFlame;

   oxygine::timeMS m_angleHoldStartTime;
   oxygine::timeMS m_angleHoldDuration;



   void logCountdown(int secondsLeft);
   void igniteEngines(void);
   void setLaunchTriggerProperty(oxygine::Event *ev);
   void updateHoldAngle(oxygine::timeMS now);

public:
	LaunchSite(
      SceneActor* sceneParent,
      CompoundObject* parentObject,
      b2World* world,
      const oxygine::Vector2& pos,
      pugi::xml_node& root,
      int groupIndex);

   void leapfrogFootTouch(b2Contact* contact, bool leftFoot, LeapFrog* leapfrog);
   void leapfrogFootLift(b2Contact* contact, bool leftFoot, LeapFrog* leapfrog);

   void startLaunchSequence(void);

   bool isActive(void) const { return m_active; }

protected:
   void doUpdate(const oxygine::UpdateState &us) override;
   void setPropertyImpl(int propId, float value) override;

};

