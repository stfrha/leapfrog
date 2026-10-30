#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "leapfrog.h"
#include "launchsite.h"
#include "scales.h"
#include "physdispconvert.h"
#include "spawnobject.h"
#include "softboundary.h"
#include "parallaxbackground.h"

#include <deque>
#include <set>

class ManualPan
{
public:
   ManualPan();

   oxygine::Vector2  m_manualPanPos;
   bool m_manPanEnable;
};

class SceneTimer
{
public:
   int m_id;
   int m_timeOutNumOfTicks;
   int m_ticks;

public:
   SceneTimer(int id, int timeOutNumOfTicks);
   bool tickTimer(void);
};



DECLARE_SMART(SceneActor, spSceneActor);

class SceneActor : public CompoundObject
{
public:
   enum PanorateModeEnum
   {
      top,
      midTop,
      center,
      bottom,
      midBottom,
      topLeft,
      fix
   };

   enum SceneTypeEnum
   {
      landing,
      deepSpace,
      orbit,
      hyperSpace
   };

   enum LaunchStateEnum
   {
      fadeIn,
      sceneIsOn,
      fadeOut
   };

private:
   spCompoundObject m_panObject;
   bool m_armManPanEnableChange;
   bool m_armPauseChange;
   std::vector<spSoftBoundary> m_boundaries;
   std::vector<b2Body*> m_boundedBodies;
   ManualPan* m_manualPan;
   std::vector<SceneTimer> m_timers;
   int m_timerIdCounter;

   std::vector<ParallaxBackground> m_parallaxBackgrounds;

   // When true, each parallax layer gets one uniform scale (the larger of
   // the x and y scales) so sprites on it keep their aspect ratio. The
   // layer's bottom edge stays where the per-axis scaling put it and the
   // extra size is split evenly left and right. Set from the scene's
   // sceneProperties attribute parallaxKeepAspect (default true).
   bool m_parallaxKeepAspect = true;

   // wantedVpPos is the position in main actor coordinates where we
   // want to keep the panorating body. The whole scene is moved to
   // make the body at the specified position.
   // This can be animated for smooth transitions.
   oxygine::Vector2 m_wantedVpPos;

   // This can be animated for smooth transitions.
   float m_zoomPos;

protected:
   float	m_zoomScale;
   float	m_defaultZoomScale;   // where the zoom-out at scene start ends
   float	m_startZoomScale;     // where every scene starts (zoomed in)
   float	m_stageToViewPortScale;
   float	m_physToStageScale;
   b2World * m_world;
   std::vector<oxygine::spActor> m_deathList;
   std::vector<spSpawnInstruction> m_spawnInstructions;
   PanorateModeEnum m_panorateMode;
   bool m_panorateLimitEnabled;
   bool m_externalControl;
   SceneTypeEnum m_sceneType;

   // Yes/No questions asked by the scripts (c_askQuestion), see
   // askQuestion below
   struct QueuedQuestion
   {
      int id = -1;
      std::string text;         // the question, said when its turn comes
      bool textGiven = true;
      std::string flightText;   // said if still in flight when the dialog
      std::string sender;       // has caught up ("" = nothing)
      bool flightTextGiven = true;
   };
   std::deque<QueuedQuestion> m_questionQueue;  // asked, not yet open
   QueuedQuestion m_openQuestion;  // the open one's record, back to the
                                   // queue as it is if it isn't answered
   int m_questionId;          // the open question, -1: none
   bool m_questionShown;      // the open question has its Yes/No buttons
   int m_questionAnswer;      // answer waiting to be sent to Lua (-2: none)
   int m_answeredQuestionId;
   bool m_questionKeysArmed;  // the answer keys have been up since it was shown
   bool m_flightControlsHeld; // just answered: booster, fire and steering
                              // wait until every flight button and key is up
   std::set<CompoundObject*> m_leapfrogStandsOn;  // pads and launch sites
   bool m_creditCheatArmed;   // C has been up since the last cheat
   void updateQuestion(const Uint8* keys);

public:
   const Vector2& getWantedVpPos() const; 
   void setWantedVpPos(const Vector2& pos);
   void setManualPan(ManualPan* mp);

   const float& getZoom() const;
   void setZoom(const float& pos);

   // Zoom smoothly so that `width` game units are visible across the
   // screen, e.g. when the Leapfrog has landed on a pad. Only zooms in:
   // if the view is already closer than that, it is left alone.
   void zoomInToVisibleWidth(float width, oxygine::timeMS duration);

   // Zoom smoothly back out to the scene's default zoom, the one the
   // zoom-out at scene start ends at. Only zooms out.
   void zoomOutToDefault(oxygine::timeMS duration);

   // Zoom smoothly in to the zoom every scene starts at, so a scene
   // that ends this way matches the start of the next one.
   void zoomInToStartZoom(oxygine::timeMS duration);

	SceneActor(
      b2World* world, 
      float initialZoomScale,
      float zoomScale);
   ~SceneActor();

   static SceneActor* defineScene(
      CompoundObject* parentObject,
      b2World* world,
      pugi::xml_node& root,
      int groupIndex);

   //float m_sceneWidth;
   //float m_sceneHeight;

   bool m_turnLeftPressed;
   bool m_turnRightPressed;
   bool m_boosterPressed;
   bool m_firePressed;
   bool m_pausePressed;
   bool m_zoomInPressed;
   bool m_zoomOutPressed;
   bool m_panButtonPressed;
   bool m_manPanEnablePressed;
   bool m_fuelCheatHeld = false;     // F (fuel cheat) held since the last frame
   oxygine::Vector2 m_panVector;
   LeapFrog* m_leapfrog;

   b2World* getWorld(void);

   // The soft boundaries of the scene (read only)
   const std::vector<spSoftBoundary>& getBoundaries(void) { return m_boundaries; }

   void addParallaxBackground(ParallaxBackground* background);
   void addParallaxBackgroundSprite(oxygine::spSprite sp, float parallaxAmount);

   void addBoundingBody(b2Body* body);
   void removeBoundingBody(b2Body* body);

   // Destroys a body and takes it off the bounded-bodies list first. Use
   // this instead of b2World::DestroyBody for bodies that may be bounded:
   // a destroyed body left on the list kept getting the boundary force
   // through the stale pointer, which Box2D reuses for the next body it
   // creates (often a flame particle, shot back into the scene).
   void destroyBody(b2Body* body);
   void testForBoundaryRepel(void);
   int isInsideTransitField(b2Body* body);

   // Where on the screen the followed object is kept; the view glides
   // there over `duration`
   void setPanorateMode(PanorateModeEnum mode, oxygine::timeMS duration = 4000);
   void setPanorateObject(CompoundObject* co);
   void enablePanorateLimit(bool enable);

   void addMeToDeathList(spActor actor);
   void addObjectToSpawnList(
      int numOfSpawns,
      oxygine::Vector2 leftTop,
      oxygine::Vector2 widthHeight,
      spSpawnObjectList spawnSource,
      spSpawnTally tally = NULL);
   
   int createSceneTimer(int numOfTicks);

   // Yes/No questions. Flight controls are never taken over in flight, so
   // a question is only answered with the Leapfrog standing on a landing
   // pad or a launch site (active or not). Questions wait in a queue in
   // the order asked (a question already queued or open isn't added
   // again) and open one at a time while the Leapfrog stands; the dialog
   // can say so meanwhile ("touch down and let's talk"). An open question
   // is shown once the dialog has shown its queued messages (the question
   // is normally the last one) and the booster and fire buttons are up;
   // then the booster button is Yes and the fire button No (keys Y or W,
   // N or Return), and the Leapfrog's controls do nothing until it is
   // answered, and after that until they have all been let go. Pushed
   // off the pad before that, the question goes back to the front of the
   // queue. The answer reaches Lua as event "QsYs" or "QsNo" with the
   // question id as parameter. Questions belong to the scene: those not
   // answered are dropped when it ends.
   // text (optional, by sender, on the left): the question itself, added
   // to the dialog when its turn comes: no other question open, none
   // before it in the queue, and the dialog has shown everything before
   // it (e.g. the replies to the previous answer). So a question asked
   // while another is open is not said until that one has been answered.
   // flightText (optional, by sender): added after the text if the
   // Leapfrog still isn't standing anywhere then, e.g. "Touch down and
   // let's talk". Decided then, not when the question is asked, since a
   // long dialog can outlast the landing.
   void askQuestion(int questionId, const std::string& text = "", const std::string& sender = "",
      const std::string& flightText = "");
   void cancelQuestion(int questionId);   // -1: all, open and queued
   void answerQuestion(bool yes);
   bool isQuestionShown(void);
   bool hasPendingQuestion(void);         // open or queued

   // Landing pads and launch sites report whether the Leapfrog stands on
   // them (a pad: both feet down and still; a launch site: until its launch
   // sequence takes the Leapfrog)
   void setLeapfrogStandsOn(CompoundObject* place, bool stands);
   bool doesLeapfrogStand(void);

   // The name of the pad or launch site the Leapfrog stands on, "" in
   // flight (c_leapfrogStandsOn). Set before the landed event (LpLL, LsLL)
   // is sent, so handlers of it see where the Leapfrog is.
   std::string leapfrogStandsOn(void);

   // Cheats: key C or a double tap on the HUD's credits (+100), a double
   // tap on the fuel bar (full tank)
   void cheatCredits(void);
   void cheatFuel(void);

   virtual void startLeapfrogInScene(std::string name);
   virtual void startLaunchSiteInScene(std::string name);

   void takeControlOfLeapfrog(bool control);

   SceneActor::SceneTypeEnum getSceneType(void);

   typedef Property2Args<float, oxygine::Vector2, const oxygine::Vector2&, SceneActor, &SceneActor::getWantedVpPos, &SceneActor::setWantedVpPos>  TweenWantedVpPos;
   typedef Property2Args<float, float, const float&, SceneActor, &SceneActor::getZoom, &SceneActor::setZoom>  TweenZoom;



protected:
	void doUpdate(const UpdateState& us) override;

   void sweepKillList(void);
   void sweepSpawnList(void);
   void removeTimer(int id);

};
