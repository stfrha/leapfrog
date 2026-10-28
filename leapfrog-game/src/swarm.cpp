#include "swarm.h"

#include <algorithm>
#include <cstdlib>
#include <sstream>

#include "sceneactor.h"
#include "stripper.h"
#include "mainactor.h"

using namespace std;
using namespace oxygine;
using namespace pugi;

static float randomUnit(void)
{
   return static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
}

Swarm::Swarm(
   SceneActor* sceneActor,
   CompoundObject* parentObject,
   b2World* world,
   const oxygine::Vector2& pos,
   pugi::xml_node& root,
   int groupIndex) :
   CompoundObject(sceneActor, parentObject),
   m_world(world),
   m_active(true),
   m_ramping(false),
   m_initialDone(false),
   m_rampClock(0.0f),
   m_untilNext(0.0f),
   m_rate(0.5f),
   m_maxAlive(5.0f),
   m_startRate(0.5f),
   m_endRate(0.5f),
   m_startMaxAlive(5.0f),
   m_endMaxAlive(5.0f),
   m_rampTime(0.0f),
   m_initialSpawn(0),
   m_spawned(0),
   m_spawnInset(30.0f),
   m_minDistance(200.0f)
{
   initCompoundObjectParts(sceneActor, sceneActor, parentObject, world, pos, root, groupIndex);

   attachTo(m_sceneActor);

   m_spawnObjects = new SpawnObjectList();
   m_tally = new SpawnTally();

   readSwarmNode(root.child("behaviour").child("swarmProperties"));

   m_properties.push_back(ObjectProperty(this, NULL, propActive, m_active ? 1.0f : 0.0f));
   m_properties.push_back(ObjectProperty(this, NULL, propRate, m_rate));
   m_properties.push_back(ObjectProperty(this, NULL, propMaxAlive, m_maxAlive));
   m_properties.push_back(ObjectProperty(this, NULL, propAlive, 0.0f, true));
   m_properties.push_back(ObjectProperty(this, NULL, propSpawned, 0.0f, true));
   m_properties.push_back(ObjectProperty(this, NULL, propRecall, 0.0f));

   scheduleNext();
}

void Swarm::readSwarmNode(const xml_node& node)
{
   // Spawn files: <spawnObject><childObject file="..." /></spawnObject> each
   string files = node.attribute("spawn").as_string("stripper.xml");
   istringstream words(files);
   string file;
   xml_document doc;
   xml_node holder = doc.append_child("swarmSpawns");

   while (words >> file)
   {
      xml_node so = holder.append_child("spawnObject");
      so.append_attribute("probabilityFactor") = 1;
      xml_node cho = so.append_child("childObject");
      cho.append_attribute("name") = file.substr(0, file.find('.')).c_str();
      cho.append_attribute("file") = file.c_str();
      cho.append_attribute("posX") = 0;
      cho.append_attribute("posY") = 0;
   }

   m_spawnObjects->readSpawnObjectsNode(holder);

   m_active = node.attribute("active").as_bool(true);
   m_endRate = node.attribute("rate").as_float(0.5f);
   m_startRate = node.attribute("startRate").as_float(m_endRate);
   m_endMaxAlive = node.attribute("maxAlive").as_float(5.0f);
   m_startMaxAlive = node.attribute("startMaxAlive").as_float(m_endMaxAlive);
   m_rampTime = node.attribute("rampTime").as_float(0.0f);
   m_initialSpawn = node.attribute("initialSpawn").as_int(0);
   m_spawnInset = node.attribute("spawnInset").as_float(30.0f);
   m_minDistance = node.attribute("minDistance").as_float(200.0f);

   m_ramping = (m_rampTime > 0.0f);
   m_rate = m_ramping ? m_startRate : m_endRate;
   m_maxAlive = m_ramping ? m_startMaxAlive : m_endMaxAlive;

   readSides(node.attribute("sides").as_string(""));
}

void Swarm::readSides(const string& sides)
{
   m_sides.clear();

   if (sides != "")
   {
      istringstream words(sides);
      string w;

      while (words >> w)
      {
         if (w == "up") m_sides.push_back(sideUp);
         else if (w == "down") m_sides.push_back(sideDown);
         else if (w == "left") m_sides.push_back(sideLeft);
         else if (w == "right") m_sides.push_back(sideRight);
         else logs::messageln("Swarm: unknown side '%s'", w.c_str());
      }

      return;
   }

   // The sides that have a soft boundary. (A boundary is named by the way
   // it pushes: the top one pushes down, etc.)
   const vector<spSoftBoundary>& boundaries = m_sceneActor->getBoundaries();

   for (auto it = boundaries.begin(); it != boundaries.end(); ++it)
   {
      switch ((*it)->getDirection())
      {
      case SoftBoundary::down:  m_sides.push_back(sideUp); break;
      case SoftBoundary::up:    m_sides.push_back(sideDown); break;
      case SoftBoundary::right: m_sides.push_back(sideLeft); break;
      case SoftBoundary::left:  m_sides.push_back(sideRight); break;
      }
   }

   if (m_sides.empty())
   {
      m_sides = { sideUp, sideDown, sideLeft, sideRight };
   }
}

void Swarm::updateRamp(float dt)
{
   if (!m_ramping)
   {
      return;
   }

   m_rampClock += dt;

   float f = min(1.0f, m_rampClock / m_rampTime);

   m_rate = m_startRate + (m_endRate - m_startRate) * f;
   m_maxAlive = m_startMaxAlive + (m_endMaxAlive - m_startMaxAlive) * f;

   m_properties[propRate].setPropertyNoEvent(m_rate);
   m_properties[propMaxAlive].setPropertyNoEvent(m_maxAlive);

   if (f >= 1.0f)
   {
      m_ramping = false;
   }
}

// Next spawn in 1 / rate s, give or take 40 %, so they don't come in step
void Swarm::scheduleNext(void)
{
   if (m_rate <= 0.0f)
   {
      m_untilNext = 1.0f;     // look again in a second
      return;
   }

   m_untilNext = (0.6f + 0.8f * randomUnit()) / m_rate;
}

// A random spot just outside a random side's edge, in the boundary haze
bool Swarm::pickSpawnPoint(Vector2& pos)
{
   if (m_sides.empty())
   {
      return false;
   }

   Vector2 size = m_sceneActor->getSize();
   b2Vec2 lfPos(0.0f, 0.0f);
   bool haveLf = false;

   if (m_sceneActor->m_leapfrog)
   {
      b2Body* b = m_sceneActor->m_leapfrog->getBody("lfMainBody");

      if (b)
      {
         lfPos = b->GetPosition();
         haveLf = true;
      }
   }

   for (int attempt = 0; attempt < 12; attempt++)
   {
      int side = m_sides[rand() % m_sides.size()];

      switch (side)
      {
      case sideUp:    pos = Vector2(randomUnit() * size.x, -m_spawnInset); break;
      case sideDown:  pos = Vector2(randomUnit() * size.x, size.y + m_spawnInset); break;
      case sideLeft:  pos = Vector2(-m_spawnInset, randomUnit() * size.y); break;
      case sideRight: pos = Vector2(size.x + m_spawnInset, randomUnit() * size.y); break;
      }

      if (!haveLf || ((b2Vec2(pos.x, pos.y) - lfPos).Length() >= m_minDistance))
      {
         return true;
      }
   }

   // The Leapfrog is near every spot we tried (small scene): skip this one
   return false;
}

bool Swarm::spawnOne(void)
{
   Vector2 pos;

   if (!pickSpawnPoint(pos))
   {
      return false;
   }

   // Spawned at exactly pos (an area of 0 x 0 around it)
   m_sceneActor->addObjectToSpawnList(1, pos, Vector2(0.001f, 0.001f), m_spawnObjects, m_tally);
   m_spawned++;
   m_properties[propSpawned].setProperty((float)m_spawned);

   return true;
}

void Swarm::recall(void)
{
   vector<CompoundObject*> alive = m_tally->aliveObjects();

   for (auto it = alive.begin(); it != alive.end(); ++it)
   {
      Stripper* st = dynamic_cast<Stripper*>(*it);

      if (st)
      {
         st->leaveNow();
      }
   }
}

void Swarm::setPropertyImpl(int propId, float value)
{
   switch (propId)
   {
   case propActive:
      m_active = (value != 0.0f);
      break;
   case propRate:
      m_rate = max(0.0f, value);
      m_ramping = false;
      scheduleNext();
      break;
   case propMaxAlive:
      m_maxAlive = max(0.0f, value);
      m_ramping = false;
      break;
   case propRecall:
      if (value != 0.0f)
      {
         recall();
      }
      m_properties[propRecall].setPropertyNoEvent(0.0f);
      break;
   default:
      break;
   }
}

void Swarm::doUpdate(const oxygine::UpdateState& us)
{
   if (g_MainActor->isInPause())
   {
      return;
   }

   int alive = m_tally->countAlive();

   if ((int)m_properties[propAlive].getProperty() != alive)
   {
      m_properties[propAlive].setProperty((float)alive);
   }

   if (!m_active)
   {
      return;
   }

   if (!m_initialDone)
   {
      m_initialDone = true;

      for (int i = 0; i < m_initialSpawn; i++)
      {
         spawnOne();
      }
   }

   updateRamp(us.dtf);

   m_untilNext -= us.dtf;

   if (m_untilNext <= 0.0f)
   {
      // Count those queued this frame too (the tally includes them)
      if (m_tally->countAlive() < (int)(m_maxAlive + 0.5f))
      {
         spawnOne();
      }

      scheduleNext();
   }
}
