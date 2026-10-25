#include "sceneactor.h"
#include "sceneactorevents.h"

#include "landingactor.h"
#include "freespaceactor.h"
#include "orbitspacescene.h"
#include "bodyuserdata.h"
#include "mainactor.h"
#include "layout.h"
#include "polygonvertices.h"
#include "headdowndisplay.h"
#include "messagedisplay.h"
#include "gamestatus.h"
#include "objectresources.h"

using namespace oxygine;
using namespace std;
using namespace pugi;

ManualPan::ManualPan() :
   m_manPanEnable(false),
   m_manualPanPos(Vector2(0.0f, 0.0f))
{

}

SceneTimer::SceneTimer(int id, int timeOutNumOfTicks) :
   m_id(id),
   m_timeOutNumOfTicks(timeOutNumOfTicks),
   m_ticks(0)
{
}

bool SceneTimer::tickTimer(void)
// Returns true if the timer timed out
{
   m_ticks++;
   return (m_ticks > m_timeOutNumOfTicks);
}


SceneActor* SceneActor::defineScene(
   CompoundObject* parentObject,
   b2World* world,
   pugi::xml_node& root,
   int groupIndex)
{

   string sceneTypeStr = root.child("behaviour").child("sceneProperties").attribute("sceneType").as_string();

   float width = root.child("behaviour").child("sceneProperties").attribute("width").as_float(1000.0f);
   float height = root.child("behaviour").child("sceneProperties").attribute("height").as_float(500.0f);

   g_Layout.initStageSize(Vector2(width, height), 150.0f);

   SceneActor* baseScene = NULL;

   // Calc zoom scale: Formula empirically found to be nice
   float initalZoomScale = g_Layout.getViewPortBounds().x / 384.0f;
   float zoomScale = g_Layout.getViewPortBounds().x / 3840.0f;

   if (sceneTypeStr == "ground")
   {
      LandingActor* landingScene = new LandingActor(
         world, 
         initalZoomScale, 
         zoomScale, 
         root, 
         groupIndex);

      baseScene = static_cast<SceneActor*>(landingScene);

   }
   else if (sceneTypeStr == "space")
   {
      FreeSpaceActor* spaceScene = new FreeSpaceActor(
         world, 
         initalZoomScale, 
         zoomScale, 
         root, 
         groupIndex);

      baseScene = static_cast<SceneActor*>(spaceScene);
   }
   else if (sceneTypeStr == "orbitSpaceScene")
   {
      OrbitSpaceScene* orbitScene = new OrbitSpaceScene(
         world, 
         root, 
         groupIndex);

      baseScene = static_cast<SceneActor*>(orbitScene);
   }
   else
   {
      return NULL;
   }

   Vector2 size = Vector2(width, height);

   baseScene->setSize(size);

   baseScene->m_parallaxKeepAspect = root.child("behaviour").child("sceneProperties").attribute("parallaxKeepAspect").as_bool(true);

   spSoftBoundary sb;

   bool boundary = root.child("behaviour").child("sceneProperties").attribute("hasUpperBoundary").as_bool(false);
   if (boundary)
   {
      sb = new SoftBoundary(
         SoftBoundary::down);
      baseScene->addChild(sb);
      baseScene->m_boundaries.push_back(sb);
   }

   boundary = root.child("behaviour").child("sceneProperties").attribute("hasLowerBoundary").as_bool(false);
   if (boundary)
   {
      sb = new SoftBoundary(
         SoftBoundary::up);
      baseScene->addChild(sb);
      baseScene->m_boundaries.push_back(sb);
   }

   boundary = root.child("behaviour").child("sceneProperties").attribute("hasLeftBoundary").as_bool(false);
   if (boundary)
   {
      sb = new SoftBoundary(
         SoftBoundary::right);
      baseScene->addChild(sb);
      baseScene->m_boundaries.push_back(sb);
   }

   boundary = root.child("behaviour").child("sceneProperties").attribute("hasRightBoundary").as_bool(false);
   if (boundary)
   {
      sb = new SoftBoundary(
         SoftBoundary::left);
      baseScene->addChild(sb);
      baseScene->m_boundaries.push_back(sb);
   }

   return baseScene;
}

SceneActor::SceneActor(
   b2World* world,
   float initialZoomScale,
   float zoomScale) :
   CompoundObject(this, NULL),
   m_world(world),
   m_zoomScale(initialZoomScale),
   m_defaultZoomScale(zoomScale),
   m_startZoomScale(initialZoomScale),
   m_stageToViewPortScale(m_zoomScale* Scales::c_stageToViewPortScale),
   m_physToStageScale(1.0f),
   m_panorateMode(center),
   m_panorateLimitEnabled(true),
   m_externalControl(false),
   m_sceneType(landing),
   m_turnLeftPressed(false),
   m_turnRightPressed(false),
   m_boosterPressed(false),
   m_firePressed(false),
   m_pausePressed(false),
   m_zoomInPressed(false),
   m_zoomOutPressed(false),
   m_panButtonPressed(false),
   m_manPanEnablePressed(false),
   m_armManPanEnableChange(true),
   m_panObject(NULL),
   m_manualPan(NULL),
   m_timerIdCounter(0),
   m_armPauseChange(true),
   m_leapfrog(NULL)
{
	setScale(m_stageToViewPortScale);

   // Set wanted viewport position to center (since panorate mode defaults to center)
   m_wantedVpPos = Vector2(g_Layout.getViewPortBounds().x / 2.0f, g_Layout.getViewPortBounds().y / 2.0f);


   // The sceneActor is also an actor whos has two parents
   // One is the actor.parent which is the NULL.
   // The other is the m_parentObject which is the CompoundObject
   // parent, which also is NULL.
   // It is assumed that all actor's user data holds the actor.parent
   // and thus, below we create a ActorUserData wich points to NULL.
   // actor.parent (sceneParent)
   ActorUserData* aud = new ActorUserData();
   aud->m_body = NULL;
   aud->m_parentCo = NULL;

   setUserData(aud);

   spTween zoomTween = addTween(SceneActor::TweenZoom(zoomScale), 4000, 1, false, 0, Tween::ease_inOutQuad);
   zoomTween->setName("sceneZoom");
}

SceneActor::~SceneActor()
{
}

void SceneActor::addParallaxBackgroundSprite(spSprite sp, float parallaxAmount)
{
   // Check if there already is a background with the same parallax amount
   for (auto it = m_parallaxBackgrounds.begin(); it != m_parallaxBackgrounds.end(); ++it)
   {
      if (fabs(it->m_parallaxAmount - parallaxAmount) < 0.000001)
      {
         // A background for this amount is already created, add the sprite to this
         sp->attachTo(it->m_sprite);
         return;
      }
   }

   // If we get here (without returning earlier) there was no background with the
   // same amount. Create a new one.
   m_parallaxBackgrounds.emplace_back(this, m_world, parallaxAmount);
   ParallaxBackground& sa = m_parallaxBackgrounds.back();
   addChild(sa.m_sprite);
   sp->attachTo(sa.m_sprite);
}

void SceneActor::addParallaxBackground(ParallaxBackground* background)
{
   m_parallaxBackgrounds.push_back(*background);
   addChild(background->m_sprite);
}

void SceneActor::addBoundingBody(b2Body* body)
{
   m_boundedBodies.push_back(body);
}

void SceneActor::removeBoundingBody(b2Body* body)
{
   m_boundedBodies.erase(std::remove(
      m_boundedBodies.begin(),
      m_boundedBodies.end(),
      body),
      m_boundedBodies.end());
}

void SceneActor::destroyBody(b2Body* body)
{
   if (body == NULL)
   {
      return;
   }

   removeBoundingBody(body);
   body->GetWorld()->DestroyBody(body);
}

void SceneActor::testForBoundaryRepel(void)
{
   for (auto it = m_boundedBodies.begin(); it != m_boundedBodies.end(); ++it)
   {
      for (auto bit = m_boundaries.begin(); bit != m_boundaries.end(); ++bit)
      {
         bit->get()->testForRepel(*it);
      }
   }
}

// Returns index of the transit fields
int SceneActor::isInsideTransitField(b2Body* body)
{
   SoftBoundary* lower = NULL;
   SoftBoundary* right = NULL;
   SoftBoundary* upper = NULL;
   SoftBoundary* left = NULL;

   for (auto bit = m_boundaries.begin(); bit != m_boundaries.end(); ++bit)
   {
      if (bit->get()->getDirection() == SoftBoundary::RepelDirectionEnum::up)
      {
         lower = bit->get();
      } 
      else if (bit->get()->getDirection() == SoftBoundary::RepelDirectionEnum::left)
      {
         right = bit->get();
      }
      else if (bit->get()->getDirection() == SoftBoundary::RepelDirectionEnum::right)
      {
         left = bit->get();
      }
      else if (bit->get()->getDirection() == SoftBoundary::RepelDirectionEnum::down)
      {
         upper = bit->get();
      }
   }

   if ((upper != NULL) && (right != NULL))
   {
      if (upper->isInside(body) && right->isInside(body))
      {
         return 1;
      }
   }

   if ((upper != NULL) && (left != NULL))
   {
      if (upper->isInside(body) && left->isInside(body))
      {
         return 2;
      }
   }

   if ((lower != NULL) && (right != NULL))
   {
      if (lower->isInside(body) && right->isInside(body))
      {
         return 3;
      }
   }

   if ((lower != NULL) && (left != NULL))
   {
      if (lower->isInside(body) && left->isInside(body))
      {
         return 4;
      }
   }

   return 0;
}


SceneActor::SceneTypeEnum SceneActor::getSceneType(void)
{
   return m_sceneType;
}

b2World* SceneActor::getWorld(void)
{
   return m_world;
}

void SceneActor::setPanorateMode(PanorateModeEnum mode, timeMS duration)
{
   m_panorateMode = mode;

   Vector2 newWantedPos = Vector2(0.0f, 0.0f);

   if (m_panorateMode == center)
   {
      newWantedPos = Vector2(g_Layout.getViewPortBounds().x / 2.0f, g_Layout.getViewPortBounds().y / 2.0f);
   }
   else if (m_panorateMode == top)
   {
      newWantedPos = Vector2(g_Layout.getViewPortBounds().x / 2.0f, g_Layout.getViewPortBounds().y * 0.1f);
   }
   else if (m_panorateMode == midTop)
   {
      newWantedPos = Vector2(g_Layout.getViewPortBounds().x / 2.0f, g_Layout.getViewPortBounds().y * 0.3f);
   }
   else if (m_panorateMode == bottom)
   {
      newWantedPos = Vector2(g_Layout.getViewPortBounds().x / 2.0f, g_Layout.getViewPortBounds().y * 0.9f);
   }
   else if (m_panorateMode == midBottom)
   {
      newWantedPos = Vector2(g_Layout.getViewPortBounds().x / 2.0f, g_Layout.getViewPortBounds().y * 0.7f);
   }
   else if (m_panorateMode == topLeft)
   {
      newWantedPos = Vector2(g_Layout.getViewPortBounds().x * 0.1f, g_Layout.getViewPortBounds().y * 0.1f);
   }
      
   // Replace a glide still running, so the newest mode wins
   removeTweensByName("scenePanorate");
   spTween panTween = addTween(SceneActor::TweenWantedVpPos(newWantedPos), duration, 1, false, 0, Tween::ease_inOutQuad);
   panTween->setName("scenePanorate");
}

const Vector2& SceneActor::getWantedVpPos() const
{
   return m_wantedVpPos;
}

void SceneActor::setWantedVpPos(const Vector2& pos)
{
   m_wantedVpPos = pos;
}

const float& SceneActor::getZoom() const
{
   return m_zoomScale;
}

void SceneActor::setZoom(const float& zoom)
{
   m_zoomScale = zoom;
   m_stageToViewPortScale = m_zoomScale * Scales::c_stageToViewPortScale;
   setScale(m_stageToViewPortScale);
}

void SceneActor::zoomOutToDefault(timeMS duration)
{
   // The default may be further out than the stage allows; doUpdate limits
   // the zoom so the whole stage just fits, so aim for that limit instead.
   float zoom = m_defaultZoomScale;
   float fitX = g_Layout.getViewPortBounds().x / g_Layout.getStageSize().x / Scales::c_stageToViewPortScale;
   float fitY = g_Layout.getViewPortBounds().y / g_Layout.getStageSize().y / Scales::c_stageToViewPortScale;
   if (zoom < fitX)
   {
      zoom = fitX;
   }
   if (zoom < fitY)
   {
      zoom = fitY;
   }

   if (zoom >= m_zoomScale)
   {
      // Already at least this far out
      return;
   }

   removeTweensByName("sceneZoom");
   spTween zoomTween = addTween(SceneActor::TweenZoom(zoom), duration, 1, false, 0, Tween::ease_inOutQuad);
   zoomTween->setName("sceneZoom");
}

void SceneActor::zoomInToVisibleWidth(float width, timeMS duration)
{
   // Visible width in stage units = view port width / (zoom * stage-to-view-port scale)
   float zoom = g_Layout.getViewPortBounds().x / (width * Scales::c_stageToViewPortScale);

   if (zoom <= m_zoomScale)
   {
      // Already at least this close
      return;
   }

   // Replace any zoom tween still running (e.g. the zoom-out at scene start)
   removeTweensByName("sceneZoom");
   spTween zoomTween = addTween(SceneActor::TweenZoom(zoom), duration, 1, false, 0, Tween::ease_inOutQuad);
   zoomTween->setName("sceneZoom");
}

void SceneActor::zoomInToStartZoom(timeMS duration)
{
   removeTweensByName("sceneZoom");
   spTween zoomTween = addTween(SceneActor::TweenZoom(m_startZoomScale), duration, 1, false, 0, Tween::ease_inOutQuad);
   zoomTween->setName("sceneZoom");
}

void SceneActor::setPanorateObject(CompoundObject* co)
{
   if (co)
   {
      m_panObject = co;
   }
}

void SceneActor::setManualPan(ManualPan* mp)
{
   m_manualPan = mp;
}

void SceneActor::enablePanorateLimit(bool enable)
{
   m_panorateLimitEnabled = enable;
}

void SceneActor::addMeToDeathList(spActor actor)
{
   if (std::find(m_deathList.begin(), m_deathList.end(), actor) != m_deathList.end())
   {
      // Attempted to add same object twice
   }
   else
   {
      m_deathList.push_back(actor);
   }
}

void SceneActor::addObjectToSpawnList(
   int numOfSpawns,
   oxygine::Vector2 topLeft,
   oxygine::Vector2 widthHeight,
   spSpawnObjectList spawnSource)
{
   spSpawnInstruction spi = new SpawnInstruction(numOfSpawns, topLeft, widthHeight, spawnSource);
   m_spawnInstructions.push_back(spi);
}

int SceneActor::createSceneTimer(int numOfTicks)
{
   int myId = m_timerIdCounter;
   SceneTimer timer(m_timerIdCounter++, numOfTicks);

   if (m_timerIdCounter > 32000)
   {
      m_timerIdCounter = 0;
   }


   m_timers.push_back(timer);

   return myId;
}

void SceneActor::takeControlOfLeapfrog(bool control)
{
   m_externalControl = control;
}

void SceneActor::doUpdate(const UpdateState& us)
{
   const Uint8* data = SDL_GetKeyboardState(0);

   // Handle timers
   vector<int> removeIds;

   for (auto it = m_timers.begin(); it != m_timers.end(); ++it)
   {
      if (it->tickTimer())
      {
         removeIds.push_back(it->m_id);

         SceneTimeoutEvent ev(it->m_id);
         m_sceneActor->dispatchEvent(&ev);

      }
   }

   // Remove all timers that timed out
   for (auto it = removeIds.begin(); it != removeIds.end(); ++it)
   {
      removeTimer(*it);
   }

   testForBoundaryRepel();

   // The scene's clock calls doUpdate at a fixed step (see
   // MainActor::createSceneClock), so this keeps the physics in real time
   float spu = Scales::c_updateStep;
   if (g_MainActor->isInPause())
   {
      spu = 0.0f;
   }
   m_world->Step(spu, 6, 2);

   // Kill all actors registrated for death
   sweepKillList();

   // Spawn all actors registred for birth
   sweepSpawnList();

   if (m_leapfrog != NULL)
   {
      if (!m_externalControl)
      {
         if (data[SDL_SCANCODE_W] || m_boosterPressed)
         {
            m_leapfrog->fireMainBooster(true);
         }
         else
         {
            m_leapfrog->fireMainBooster(false);
         }

         if (data[SDL_SCANCODE_A] || m_turnLeftPressed)
         {
            m_leapfrog->fireSteeringBooster(-1);
         }
         else if (data[SDL_SCANCODE_D] || m_turnRightPressed)
         {
            m_leapfrog->fireSteeringBooster(1);
         }
         else
         {
            m_leapfrog->fireSteeringBooster(0);
         }
      }

      if (data[SDL_SCANCODE_RETURN] || m_firePressed)
      {
         m_leapfrog->fireGun(true);
      }
      else
      {
         m_leapfrog->fireGun(false);
      }

      if (data[SDL_SCANCODE_0])
      {
         m_leapfrog->goToMode(LFM_RESET);
      }
      else if (data[SDL_SCANCODE_1])
      {
         m_leapfrog->goToMode(LFM_LANDING);
      }
      else if (data[SDL_SCANCODE_2])
      {
         m_leapfrog->goToMode(LFM_DEEP_SPACE);
      }
      else if (data[SDL_SCANCODE_3])
      {
         m_leapfrog->goToMode(LFM_REENTRY);
      }


      if (data[SDL_SCANCODE_KP_9])
      {
         m_leapfrog->fireReentryFlames(true);
      }
      else if (data[SDL_SCANCODE_KP_8])
      {
         m_leapfrog->fireReentryFlames(false);
      }

      // Cheat (keyboard only, so Windows and macOS): F adds 25 % of a full
      // tank (100) of fuel, once per key press
      if (data[SDL_SCANCODE_F])
      {
         if (!m_fuelCheatHeld && (g_HeadDownDisplay != NULL))
         {
            ObjectResources* res = g_GameStatus.getResources();
            float before = res->getFuel();
            res->deltaFuel(25.0f);
            g_HeadDownDisplay->logMessage("Cheat: +" + std::to_string(static_cast<int>(res->getFuel() - before + 0.5f)) + " fuel");
         }
         m_fuelCheatHeld = true;
      }
      else
      {
         m_fuelCheatHeld = false;
      }

      if (data[SDL_SCANCODE_P])
      {
         m_leapfrog->dumpParts();
      }
   }
	
	if (data[SDL_SCANCODE_KP_PLUS] || m_zoomInPressed)
	{
		m_zoomScale *= 1.1f;
      logs::messageln("Zoom: %f, view width: %f, hor dpi: %f, view height: %f, vert dpi: %f", m_zoomScale, g_Layout.getViewPortBounds().x, g_Layout.getHorizontalDpi(), g_Layout.getViewPortBounds().y, g_Layout.getVerticalDpi());

	}
	else if (data[SDL_SCANCODE_KP_MINUS] || m_zoomOutPressed)
	{
		m_zoomScale *= 0.9f;
      logs::messageln("Zoom: %f, view width: %f, hor dpi: %f, view height: %f, vert dpi: %f", m_zoomScale, g_Layout.getViewPortBounds().x, g_Layout.getHorizontalDpi(), g_Layout.getViewPortBounds().y, g_Layout.getVerticalDpi());
   }


   // We dont want to be able to zoom out more than so that the whole
   // stage is visible, we dont want to see outside stage.
   
   if (m_zoomScale * Scales::c_stageToViewPortScale < g_Layout.getViewPortBounds().x / g_Layout.getStageSize().x)
   {
      m_zoomScale = g_Layout.getViewPortBounds().x / g_Layout.getStageSize().x / Scales::c_stageToViewPortScale;
   }

   if (m_zoomScale * Scales::c_stageToViewPortScale < g_Layout.getViewPortBounds().y / g_Layout.getStageSize().y)
   {
      m_zoomScale = g_Layout.getViewPortBounds().y / g_Layout.getStageSize().y / Scales::c_stageToViewPortScale;
   }

   m_stageToViewPortScale = m_zoomScale * Scales::c_stageToViewPortScale;


   if (m_pausePressed && m_armPauseChange)
   {
      g_MainActor->goToPause();
      m_armPauseChange = false;
   }

   if (!m_pausePressed && !m_armPauseChange)
   {
      m_armPauseChange = true;
   }

   if (m_manPanEnablePressed && m_armManPanEnableChange)
   {
      if (m_manualPan->m_manPanEnable)
      {
         m_manualPan->m_manPanEnable = false;
      }
      else
      {
         m_manualPan->m_manPanEnable = true;
      }

      m_armManPanEnableChange = false;
   }

   if (!m_manPanEnablePressed && !m_armManPanEnableChange)
   {
      m_armManPanEnableChange = true;
   }



	//update each body position on display
	b2Body* body = m_world->GetBodyList();
	while (body)
	{
      // GetUserData gives const, cant use static_cast here
		Actor* actor = BodyUserData::getActor<Actor*>(body->GetUserData());
		b2Body* next = body->GetNext();
		if (actor)
		{
			const b2Vec2& pos = body->GetPosition();
			actor->setPosition(PhysDispConvert::convert(pos, Scales::c_physToStageScale));

         // The shield direction should not be set here, it
         // will change angle at collisions

         CollisionEntityTypeEnum ce = BodyUserData::getCollisionType(body->GetUserData());

         if (ce == CollisionEntityTypeEnum::shield)
         {
            actor->setRotation(static_cast<Shield*>(actor)->getAngle());
         }
         else
         {
            actor->setRotation(body->GetAngle());
         }
		}

		body = next;
	}

	setScale(m_stageToViewPortScale);

	// Pan object position in stage coordinates
   Vector2 panPos(0.0f, 0.0f);

   m_manualPan->m_manualPanPos += m_panVector;


   if (m_manualPan->m_manPanEnable)
   {
      panPos = m_manualPan->m_manualPanPos;
   }
   else
   { 
      if (m_panObject)
      {
         panPos = m_panObject->getCompoundObjectPosition();
      }
      else if (m_leapfrog != NULL)
      {
         panPos = m_leapfrog->getMainActor()->getPosition();
      }

   }
      
   if (m_panorateMode != fix)
   {
      // Now stagePos is the position that makes the position of the 
      // panorated object being at the wanted position.
      // But we want to restrict panorating outside of the stage outer bounds.
      // However, the limit can be enabled or disabled
      if (m_panorateLimitEnabled)
      {
         if ((g_Layout.getStageSize().x - panPos.x) * m_stageToViewPortScale < g_Layout.getViewPortBounds().x - m_wantedVpPos.x)
         {
            panPos.x = g_Layout.getStageSize().x - (g_Layout.getViewPortBounds().x - m_wantedVpPos.x) / m_stageToViewPortScale;
         }

         if ((panPos.x) * m_stageToViewPortScale < m_wantedVpPos.x)
         {
            panPos.x = m_wantedVpPos.x / m_stageToViewPortScale;
         }

         if ((g_Layout.getStageSize().y - panPos.y) * m_stageToViewPortScale < g_Layout.getViewPortBounds().y - m_wantedVpPos.y)
         {
            panPos.y = g_Layout.getStageSize().y - (g_Layout.getViewPortBounds().y - m_wantedVpPos.y) / m_stageToViewPortScale;
         }

         if ((panPos.y) * m_stageToViewPortScale < m_wantedVpPos.y)
         {
            panPos.y = m_wantedVpPos.y / m_stageToViewPortScale;
         }

         //if ((g_Layout.getStageBounds().getRight() - panPos.x) * m_stageToViewPortScale < g_Layout.getViewPortBounds().x - m_wantedVpPos.x)
         //{
         //   panPos.x = g_Layout.getStageBounds().getRight() - (g_Layout.getViewPortBounds().x - m_wantedVpPos.x) / m_stageToViewPortScale;
         //}

         //if ((panPos.x - g_Layout.getStageBounds().getLeft()) * m_stageToViewPortScale < m_wantedVpPos.x)
         //{
         //   panPos.x = m_wantedVpPos.x / m_stageToViewPortScale + g_Layout.getStageBounds().getLeft();
         //}

         //if ((g_Layout.getStageBounds().getBottom() - panPos.y) * m_stageToViewPortScale < g_Layout.getViewPortBounds().y - m_wantedVpPos.y)
         //{
         //   panPos.y = g_Layout.getStageBounds().getBottom() - (g_Layout.getViewPortBounds().y - m_wantedVpPos.y) / m_stageToViewPortScale;
         //}

         //if ((panPos.y - g_Layout.getStageBounds().getTop()) * m_stageToViewPortScale < m_wantedVpPos.y)
         //{
         //   panPos.y = m_wantedVpPos.y / m_stageToViewPortScale + g_Layout.getStageBounds().getTop();
         //}
      }

      // If pan position got out of bounds, the panPos was changed. We don't
      // want the manualPanPos to wonder beyond that.
      if (m_manualPan->m_manPanEnable)
      {
         m_manualPan->m_manualPanPos = panPos;
      }

      // stagePos is where I must position the pos (this object)
      // in the main actor coordinate system, to achieve the correct
      // panoration
      Vector2 stagePos = m_wantedVpPos - panPos * m_stageToViewPortScale;

      setPosition(stagePos);


      // Now scale and position the parallax backgrounds
      for (auto it = m_parallaxBackgrounds.begin(); it != m_parallaxBackgrounds.end(); ++it)
      {
         // We now seek the size that makes the background image be the size of the parallax boundary
         // (i.e. vpBounds for amount = 0, and stageBounds for amount = 1) but expressed in stage coordinates
         // We want to scale the background to this and it will then be scaled back by the scale of stage to vp.
         // I.e. we want the size in stage coordinates, now it is in vp coordinates.
         
         Vector2 vpBoundsInStage = g_Layout.getViewPortBounds() / m_stageToViewPortScale ;

         Vector2 size = (vpBoundsInStage +
            (g_Layout.getStageSize() - vpBoundsInStage) * (1.0f - it->m_parallaxAmount));
         //Vector2 size = (vpBoundsInStage +
         //   (g_Layout.getStageBounds().getSize() - vpBoundsInStage) * (1.0f - it->m_parallaxAmount));


         // Should not set size, should set scale
         // Original size is defined from xml, for example 1200,500. This should be scaled to xSize and ySize
         // So we get xScale and yScale from xScale = xSize / 1200

         float xScale = size.x / g_Layout.getStageSize().x;
         float yScale = size.y / g_Layout.getStageSize().y;

         Vector2 layerPos = -stagePos / m_stageToViewPortScale * it->m_parallaxAmount;

         if (m_parallaxKeepAspect)
         {
            // xScale and yScale differ when the screen's aspect ratio differs
            // from the stage's, which stretches everything on the layer. Use
            // the larger of them for both axes so the layer still covers the
            // area the per-axis scaling covered.
            float scale = (xScale > yScale) ? xScale : yScale;
            Vector2 stageSize = g_Layout.getStageSize();

            // Keep the bottom edge in place (ground lines stay put) and
            // split the extra width evenly between left and right.
            layerPos.x += (xScale - scale) * stageSize.x * 0.5f;
            layerPos.y += (yScale - scale) * stageSize.y;

            xScale = scale;
            yScale = scale;
         }

         it->m_sprite->setScale(xScale, yScale);
         it->m_sprite->setPosition(layerPos);

      }
   }

   setScale(m_stageToViewPortScale);
}

void SceneActor::sweepKillList(void)
{
   for (auto it = m_deathList.begin(); it != m_deathList.end(); ++it)
   {
      spActor actor = *it;

      if (m_panObject == actor)
      {
         m_panObject = NULL;
      }

      b2Body* myBody = ActorUserData::getBody(actor->getUserData());

      if (myBody)
      {
         destroyBody(myBody);
      }

      actor->detach();
   }

   m_deathList.clear();
}

void SceneActor::sweepSpawnList(void)
{
   for (auto it = m_spawnInstructions.begin(); it != m_spawnInstructions.end(); ++it)
   {
      for (int i = 0; i < (*it)->m_numOfSpawns; i++)
      {
         pugi::xml_node* spawnNode = (*it)->m_spawnSource->getSpawnObjectNode();

         if (spawnNode == NULL)
         {
            return;
         }

         // TODO: randomise a position within topLeft and widthHeight
         float posX = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (*it)->m_widthHeight.x)) + (*it)->m_leftTop.x;
         float posY = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (*it)->m_widthHeight.y)) + (*it)->m_leftTop.y;


         defineChildObject(
            this,
            this,
            m_world,
            Vector2(posX, posY),
            *spawnNode);

      }
   }

   m_spawnInstructions.clear();
}

void SceneActor::removeTimer(int id)
{
   for (auto it = m_timers.begin(); it != m_timers.end(); ++it)
   {
      if (it->m_id == id)
      {
         m_timers.erase(it);
         break;
      }
   }
}

void SceneActor::startLeapfrogInScene(string name)
// This is the base implementation of starting the leapfrog in 
// the scene. Scenes can be loaded without leapfrog and it can 
// be added at a later time in a mission or a cut-scene. 
{
   if (m_leapfrog == NULL)
   {
      m_leapfrog = static_cast<LeapFrog*>(getObject(name));
   }
}

void SceneActor::startLaunchSiteInScene(string name)
// This is the base implementation of starting a launchsite in 
// the scene. Scenes can be loaded without launchsites and it can 
// be added at a later time in a mission or a cut-scene. 
// So far, the base method does nothing. All is in the overidden
// methods.
{

}