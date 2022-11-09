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
- countdown, count down from ten to three
- ignite, start engine flame emitters, countdown to zero
- tZero, apply force to main booster, stay for 0.5 s
- release, release joints holding main tank booster

- prepare, leg rests rotate down, leapfrog is attached to spring which is draged into main tank, (leapfrog goes to deep space mode)
- launch sequence - Stoppable countdown, smoke and small engine fire, 
- launch execute - Unstoppable countdown, booster fire, is released and all goes up
*/




class LaunchSite : public CompoundObject
{
public:
	enum propertyId
	{
		state = 0,
		lfLanded = 1
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
   bool m_armInitialRegistration;

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



   void showCountdownNumber(int n);
   
   void countDownFadeInComplete(oxygine::Event *event);
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

protected:
   void doUpdate(const oxygine::UpdateState &us);

};

