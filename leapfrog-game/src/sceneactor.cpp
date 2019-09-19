#include "sceneactor.h"
#include "landingactor.h"
#include "freespaceactor.h"
#include "orbitspacescene.h"
#include "bodyuserdata.h"
#include "mainactor.h"

using namespace oxygine;
using namespace std;
using namespace pugi;


SceneActor* SceneActor::defineScene(
   Resources& gameResources,
   CompoundObject* parentObject,
   b2World* world,
   pugi::xml_node& root,
   const string& initialState,
   int groupIndex)
{
   string sceneTypeStr = root.child("behaviour").child("sceneProperties").attribute("sceneType").as_string();

   SceneActor* baseScene = NULL;

   if (sceneTypeStr == "ground")
   {
      LandingActor* landingScene = new LandingActor(gameResources, world, root, initialState, groupIndex);

      baseScene = static_cast<SceneActor*>(landingScene);

   }
   else if (sceneTypeStr == "space")
   {
      FreeSpaceActor* spaceScene = new FreeSpaceActor(gameResources, world, root, initialState, groupIndex);

      baseScene = static_cast<SceneActor*>(spaceScene);
   }
   else if (sceneTypeStr == "orbitSpaceScene")
   {
      OrbitSpaceScene* orbitScene = new OrbitSpaceScene(
         gameResources,
         world, 
         root, 
         initialState,
         groupIndex);

      baseScene = static_cast<SceneActor*>(orbitScene);
   }
   else
   {
      return NULL;
   }

   baseScene->m_sceneWidth = root.child("behaviour").child("sceneProperties").attribute("width").as_float(1000.0f);
   baseScene->m_sceneHeight = root.child("behaviour").child("sceneProperties").attribute("height").as_float(500.0f);

   spSoftBoundary sb = new SoftBoundary(
      gameResources, 
      baseScene->m_sceneWidth, 
      baseScene->m_sceneHeight, 
      SoftBoundary::up);
   baseScene->addChild(sb);
   baseScene->m_boundaries.push_back(sb);

   sb = new SoftBoundary(
      gameResources,
      baseScene->m_sceneWidth,
      baseScene->m_sceneHeight,
      SoftBoundary::down);
   baseScene->addChild(sb);
   baseScene->m_boundaries.push_back(sb);

   sb = new SoftBoundary(
      gameResources,
      baseScene->m_sceneWidth,
      baseScene->m_sceneHeight,
      SoftBoundary::right);
   baseScene->addChild(sb);
   baseScene->m_boundaries.push_back(sb);

   sb = new SoftBoundary(
      gameResources,
      baseScene->m_sceneWidth,
      baseScene->m_sceneHeight,
      SoftBoundary::left);
   baseScene->addChild(sb);
   baseScene->m_boundaries.push_back(sb);

   return baseScene;
}

SceneActor::SceneActor(
   Resources& gameResources, 
   b2World* world, 
   float zoomScale, 
   const std::string& initialState) :
   CompoundObject(this, NULL),
   m_initialState(initialState),
   m_gameResources(&gameResources),
   m_world(world),
   m_zoomScale(zoomScale),
   m_stageToViewPortScale(m_zoomScale * Scales::c_stageToViewPortScale),
   m_physToStageScale(1.0f),
   m_panorateMode(center),
   m_externalControl(false),
   m_sceneType(landing),
   m_turnLeftPressed(false),
   m_turnRightPressed(false),
   m_boosterPressed(false),
   m_firePressed(false),
   m_zoomInPressed(false),
   m_zoomOutPressed(false),
   m_sceneWidth(1000.0f),
   m_sceneHeight(500.0f),
   m_panObject(NULL)
{
	Point size = Point(m_sceneWidth, m_sceneHeight);
	setSize(size);

	//m_world = new b2World(b2Vec2(0.0f, 0.0f));

	setScale(m_stageToViewPortScale);

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

}

SceneActor::~SceneActor()
{
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

bool SceneActor::isInsideOrbitField(b2Body* body)
{
   SoftBoundary* lower = NULL;
   SoftBoundary* right = NULL;

   for (auto bit = m_boundaries.begin(); bit != m_boundaries.end(); ++bit)
   {
      if (bit->get()->getDirection() == SoftBoundary::RepelDirectionEnum::down)
      {
         lower = bit->get();
      }

      if (bit->get()->getDirection() == SoftBoundary::RepelDirectionEnum::right)
      {
         right = bit->get();
      }
   }

   if ((lower != NULL) && (right != NULL))
   {
      return (lower->isInside(body) && right->isInside(body));
   }

   return false;
}



SceneActor::SceneTypeEnum SceneActor::getSceneType(void)
{
   return m_sceneType;
}

b2World* SceneActor::getWorld(void)
{
   return m_world;
}

Resources* SceneActor::getResources(void)
{
   return m_gameResources;
}

string* SceneActor::getInitialState(void)
{
   return &m_initialState;
}

void SceneActor::setPanorateMode(PanorateModeEnum mode)
{
   m_panorateMode = mode;
}

void SceneActor::setPanorateObject(CompoundObject* co)
{
   m_panObject = co;
}

void SceneActor::setZoom(float zoom)
{
   m_zoomScale = zoom;
   m_stageToViewPortScale = m_zoomScale * Scales::c_stageToViewPortScale;
   setScale(m_stageToViewPortScale);
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

void SceneActor::registerObjectsToMap(void)
{
   for (auto it = m_children.begin(); it != m_children.end(); ++it)
   {
      (*it)->registerToMap();
   }
}

void SceneActor::takeControlOfLeapfrog(bool control)
{
   m_externalControl = control;
}

void SceneActor::doUpdate(const UpdateState& us)
{
   // Find stagePos in viewPortCoord that makes frog at center
   Point vpSize = core::getDisplaySize(); // TODO: Replace with g_layout.getViewPortBounds

	//in real project you should make steps with fixed dt, check box2d documentation
	m_world->Step(us.dt / 1000.0f, 6, 2);

   // Kill all actors registrated for death
   sweepKillList();

   // Spawn all actors registred for birth
   sweepSpawnList();

	const Uint8* data = SDL_GetKeyboardState(0);
	
	if (data[SDL_SCANCODE_KP_PLUS] || m_zoomInPressed)
	{
		m_zoomScale *= 1.1f;
	}
	else if (data[SDL_SCANCODE_KP_MINUS] || m_zoomOutPressed)
	{
		m_zoomScale *= 0.9f;
	}


   // We dont want to be able to zoom out more than so that the whole
   // stage is visible, we dont want to see outside stage.
   
   if (m_zoomScale * Scales::c_stageToViewPortScale < vpSize.x / m_sceneWidth)
   {
      m_zoomScale = vpSize.x / m_sceneWidth / Scales::c_stageToViewPortScale;
   }

   if (m_zoomScale * Scales::c_stageToViewPortScale < vpSize.y / m_sceneHeight)
   {
      m_zoomScale = vpSize.y / m_sceneHeight / Scales::c_stageToViewPortScale;
   }

   m_stageToViewPortScale = m_zoomScale * Scales::c_stageToViewPortScale;

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

   if (data[SDL_SCANCODE_P])
   {
      m_leapfrog->dumpParts();
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

         // The sheild direction should not be set here, it
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

   if (m_panObject)
   {
      panPos = m_panObject->getCompoundObjectPosition();
   }
   else
   {
      panPos = m_leapfrog->getMainActor()->getPosition();
   }

   // wantedVpPos is the position in main actor coordinates where the center 
   // of the view port is
   Vector2 wantedVpPos = Vector2(0.0f, 0.0f);
   
   if (m_panorateMode == center)
   {
      wantedVpPos = Vector2(vpSize.x / 2.0f, vpSize.y / 2.0f);
   }
   else if (m_panorateMode == top)
   { 
      wantedVpPos = Vector2(vpSize.x / 2.0f, vpSize.y * 0.1f);
   }
   else if (m_panorateMode == bottom)
   {
      wantedVpPos = Vector2(vpSize.x / 2.0f, vpSize.y * 0.9f);
   }
   else if (m_panorateMode == topLeft)
   {
      wantedVpPos = Vector2(vpSize.x * 0.1f, vpSize.y * 0.1f);
   }
   
   if (m_panorateMode != fix)
   {
      // Now stagePos is the position that makes the position of the 
      // panorated object being at the wanted position.
      // But we want to restrict panorating outside of the stage outer bounds.
      // For example, stagePos.x must not be smaller than 
      if ((m_sceneWidth - panPos.x) * m_stageToViewPortScale < vpSize.x - wantedVpPos.x)
      {
         panPos.x = m_sceneWidth - (vpSize.x - wantedVpPos.x) / m_stageToViewPortScale;
      }

      if (panPos.x * m_stageToViewPortScale < wantedVpPos.x)
      {
         panPos.x = wantedVpPos.x / m_stageToViewPortScale;
      }

      if ((m_sceneHeight - panPos.y) * m_stageToViewPortScale < vpSize.y - wantedVpPos.y)
      {
         panPos.y = m_sceneHeight - (vpSize.y - wantedVpPos.y) / m_stageToViewPortScale;
      }

      if (panPos.y * m_stageToViewPortScale < wantedVpPos.y)
      {
         panPos.y = wantedVpPos.y / m_stageToViewPortScale;
      }


      // stagePos is where I must position the pos (this object)
      // in the main actor coordinate system, to achieve the correct
      // panoration
      Vector2 stagePos = wantedVpPos - panPos * m_stageToViewPortScale;


      




      setPosition(stagePos);
   }
}

void SceneActor::sweepKillList(void)
{
   for (auto it = m_deathList.begin(); it != m_deathList.end(); ++it)
   {
      spActor actor = *it;

      b2Body* myBody = ActorUserData::getBody(actor->getUserData());

      if (myBody)
      {
         myBody->GetWorld()->DestroyBody(myBody);

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
            *m_gameResources,
            this,
            this,
            m_world,
            Vector2(posX, posY),
            *spawnNode,
            "");

      }
   }

   m_spawnInstructions.clear();
}
