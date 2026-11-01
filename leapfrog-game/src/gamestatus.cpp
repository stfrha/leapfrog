#include "gamestatus.h"

using namespace oxygine;
using namespace std;
using namespace pugi;

// Instansiate global class
GameStatus g_GameStatus;

const string GameStatus::c_gameStatusFileName = "leapfrog_game_progress.xml";
const string GameStatus::c_keyItemsFileName = "key_items.xml";

const char* const GameStatus::c_legHealthAttributes[GameStatus::c_numLegJoints] =
{
   "leftHipHealth", "leftKneeHealth", "leftFootHealth",
   "rightHipHealth", "rightKneeHealth", "rightFootHealth"
};

GameStatus::GameStatus() :
   m_leapfrogResources(),
   m_currentSceneFile(""),
   m_exitSceneFile(""),
   m_currentSceneType(0),
   m_currentMission(0),
   m_currentState(0),
   m_exitSceneType(0),
   m_exitParameter(0),
   m_gunLevel(0),
   m_gunFireRateLevel(0),
   m_gunBulletSpeedLevel(0),
   m_gunBulletDamageLevel(0),
   m_shieldMaxLevel(0),
   m_damageMaxLevel(0),
   m_speedLevel(0),
   m_turnSpeedLevel(0),
   m_fuelMaxLevel(0),
   m_leftHipJointLevel(0),
   m_leftKneeJointLevel(0),
   m_leftFootJointLevel(0),
   m_rightHipJointLevel(0),
   m_rightKneeJointLevel(0),
   m_rightFootJointLevel(0),
   m_keyItemDefsRead(false)
{
   for (int i = 0; i < c_numLegJoints; i++)
   {
      m_legHealth[i] = 1.0f;
   }
}

float GameStatus::getLegHealth(int index)
{
   if ((index < 0) || (index >= c_numLegJoints))
   {
      return 1.0f;
   }

   return m_legHealth[index];
}

void GameStatus::setLegHealth(int index, float health)
{
   if ((index >= 0) && (index < c_numLegJoints))
   {
      m_legHealth[index] = health;
   }
}

void GameStatus::writeLegHealth(xml_node& root)
{
   // Saves from before leg health lack these attributes; add them
   for (int i = 0; i < c_numLegJoints; i++)
   {
      xml_attribute attr = root.attribute(c_legHealthAttributes[i]);

      if (!attr)
      {
         attr = root.append_attribute(c_legHealthAttributes[i]);
      }

      attr.set_value(m_legHealth[i]);
   }
}


// ------------------------------------------------------------------------
// Key items

void GameStatus::readKeyItemDefinitions(void)
{
   if (m_keyItemDefsRead)
   {
      return;
   }

   m_keyItemDefsRead = true;
   m_keyItemDefs.clear();

   if (!ox::file::exists(c_keyItemsFileName))
   {
      logs::messageln("GameStatus: no %s, no key items", c_keyItemsFileName.c_str());
      return;
   }

   ox::file::buffer bf;
   ox::file::read(c_keyItemsFileName.c_str(), bf);

   xml_document doc;
   if ((bf.size() == 0) || !doc.load_buffer(&bf.data[0], bf.size()))
   {
      logs::warning("GameStatus: could not parse %s", c_keyItemsFileName.c_str());
      return;
   }

   for (xml_node n = doc.child("keyItems").child("keyItem"); n; n = n.next_sibling("keyItem"))
   {
      KeyItem item;
      item.m_id = n.attribute("id").as_string();
      item.m_kind = n.attribute("kind").as_string();
      item.m_name = n.attribute("name").as_string(item.m_id.c_str());
      item.m_issuer = n.attribute("issuer").as_string();
      item.m_tier = n.attribute("tier").as_int(0);
      item.m_initial = n.attribute("initial").as_bool(false);

      if (item.m_id.empty())
      {
         logs::warning("GameStatus: a key item in %s has no id", c_keyItemsFileName.c_str());
         continue;
      }

      if (getKeyItem(item.m_id) != NULL)
      {
         logs::warning("GameStatus: key item %s is declared twice", item.m_id.c_str());
         continue;
      }

      m_keyItemDefs.push_back(item);
   }
}

const KeyItem* GameStatus::getKeyItem(const std::string& id)
{
   readKeyItemDefinitions();

   for (size_t i = 0; i < m_keyItemDefs.size(); i++)
   {
      if (m_keyItemDefs[i].m_id == id)
      {
         return &m_keyItemDefs[i];
      }
   }

   return NULL;
}

KeyItemState GameStatus::getKeyItemState(const std::string& id)
{
   std::map<std::string, KeyItemState>::const_iterator it = m_keyItemStates.find(id);
   if (it != m_keyItemStates.end())
   {
      return it->second;
   }

   const KeyItem* item = getKeyItem(id);
   return ((item != NULL) && item->m_initial) ? KIS_HELD : KIS_UNKNOWN;
}

bool GameStatus::setKeyItemState(const std::string& id, KeyItemState state, KeyItemState* previous)
{
   const KeyItem* item = getKeyItem(id);
   if (item == NULL)
   {
      return false;
   }

   if (previous != NULL)
   {
      *previous = getKeyItemState(id);
   }

   KeyItemState def = item->m_initial ? KIS_HELD : KIS_UNKNOWN;
   if (state == def)
   {
      m_keyItemStates.erase(id);
   }
   else
   {
      m_keyItemStates[id] = state;
   }

   return true;
}

std::vector<std::string> GameStatus::getHeldKeyItems(const std::string& kind)
{
   readKeyItemDefinitions();

   std::vector<std::string> held;
   for (size_t i = 0; i < m_keyItemDefs.size(); i++)
   {
      const KeyItem& item = m_keyItemDefs[i];
      if ((kind.empty() || (item.m_kind == kind)) && (getKeyItemState(item.m_id) == KIS_HELD))
      {
         held.push_back(item.m_id);
      }
   }

   return held;
}

const char* GameStatus::keyItemStateName(KeyItemState state)
{
   switch (state)
   {
   case KIS_HELD:    return "held";
   case KIS_REVOKED: return "revoked";
   default:          return "unknown";
   }
}

bool GameStatus::keyItemStateFromName(const std::string& name, KeyItemState& state)
{
   if (name == "held")    { state = KIS_HELD; return true; }
   if (name == "revoked") { state = KIS_REVOKED; return true; }
   if (name == "unknown") { state = KIS_UNKNOWN; return true; }
   return false;
}

// <keyItems><item id="..." state="held|revoked|unknown"/>...</keyItems>:
// only the items whose state differs from their default, so items added
// to key_items.xml later get their default in old saves
void GameStatus::writeKeyItems(xml_node& root)
{
   root.remove_child("keyItems");
   xml_node items = root.append_child("keyItems");

   for (std::map<std::string, KeyItemState>::const_iterator it = m_keyItemStates.begin(); it != m_keyItemStates.end(); ++it)
   {
      xml_node n = items.append_child("item");
      n.append_attribute("id").set_value(it->first.c_str());
      n.append_attribute("state").set_value(keyItemStateName(it->second));
   }
}

void GameStatus::readKeyItems(xml_node& root)
{
   m_keyItemStates.clear();

   for (xml_node n = root.child("keyItems").child("item"); n; n = n.next_sibling("item"))
   {
      std::string id = n.attribute("id").as_string();
      KeyItemState state;
      if (!id.empty() && keyItemStateFromName(n.attribute("state").as_string(), state))
      {
         // Kept even if key_items.xml no longer declares it
         m_keyItemStates[id] = state;
      }
   }
}

void GameStatus::gameStatusNewGameInit(void)
{
   // Resources cannot be set yet. For initialization they are 
   // written 
   m_leapfrogResources.setResources(70, 100.0f, 25.0f, 20, 0.0f);
   m_currentSceneFile = "not_defined";
   m_exitSceneFile = "not_defined";
   m_currentSceneType = 0;
   m_currentMission = 0;
   m_currentState = 0;
   m_exitSceneType = -1;   // This will force loading the initial leapfrog for the mission.
   m_exitParameter = -1;
   m_gunLevel = 1;
   m_gunFireRateLevel = 1;
   m_gunBulletSpeedLevel = 1;
   m_gunBulletDamageLevel = 1;
   m_shieldMaxLevel = 1;
   m_damageMaxLevel = 1;
   m_speedLevel = 1;
   m_turnSpeedLevel = 1;
   m_fuelMaxLevel = 1;
   m_leftHipJointLevel = 1;
   m_leftKneeJointLevel = 1;
   m_leftFootJointLevel = 1;
   m_rightHipJointLevel = 1;
   m_rightKneeJointLevel = 1;
   m_rightFootJointLevel = 1;

   xml_node root = m_gameStatusDocument.child("leapfrogGameStatus");

   root.attribute("ammo").set_value(20);
   root.attribute("shield").set_value(100);
   root.attribute("fuel").set_value(30);
   root.attribute("credits").set_value(70);
   root.attribute("damage").set_value(0);
   root.attribute("currentSceneFile").set_value(m_currentSceneFile.c_str());
   root.attribute("exitSceneFile").set_value(m_exitSceneFile.c_str());
   root.attribute("currentSceneType").set_value(m_currentSceneType);
   root.attribute("currentMission").set_value(m_currentMission);
   root.attribute("currentState").set_value(m_currentState);
   root.attribute("exitSceneType").set_value(m_exitSceneType);
   root.attribute("exitParameter").set_value(m_exitParameter);
   root.attribute("gunLevel").set_value(m_gunLevel);
   root.attribute("gunFireRateLevel").set_value(m_gunFireRateLevel);
   root.attribute("gunBulletSpeedLevel").set_value(m_gunBulletSpeedLevel);
   root.attribute("gunBulletDamageLevel").set_value(m_gunBulletDamageLevel);
   root.attribute("shieldMaxLevel").set_value(m_shieldMaxLevel);
   root.attribute("damageMaxLevel").set_value(m_damageMaxLevel);
   root.attribute("speedLevel").set_value(m_speedLevel);
   root.attribute("turnSpeedLevel").set_value(m_turnSpeedLevel);
   root.attribute("fuelMaxLevel").set_value(m_fuelMaxLevel);
   root.attribute("leftHipJointLevel").set_value(m_leftHipJointLevel);
   root.attribute("leftKneeJointLevel").set_value(m_leftKneeJointLevel);
   root.attribute("leftFootJointLevel").set_value(m_leftFootJointLevel);
   root.attribute("rightHipJointLevel").set_value(m_rightHipJointLevel);
   root.attribute("rightKneeJointLevel").set_value(m_rightKneeJointLevel);
   root.attribute("rightFootJointLevel").set_value(m_rightFootJointLevel);

   for (int i = 0; i < c_numLegJoints; i++)
   {
      m_legHealth[i] = 1.0f;
   }

   writeLegHealth(root);

   // Every key item at its default (the initial ones held)
   m_keyItemStates.clear();
   writeKeyItems(root);

   // m_gameStatusDocument.save_file(c_gameStatusFileName.c_str());
   
   oxygine_xml_writer my_writer;
   m_gameStatusDocument.save(my_writer);
   ox::file::write(c_gameStatusFileName, my_writer.m_buf);

}

void GameStatus::initializeGameStatusXmlDocument(void)
{
   m_gameStatusDocument.remove_child("leapfrogGameStatus");

   xml_node root = m_gameStatusDocument.append_child("leapfrogGameStatus");

   root.append_attribute("ammo");
   root.append_attribute("shield");
   root.append_attribute("fuel");
   root.append_attribute("credits");
   root.append_attribute("damage");
   root.append_attribute("currentSceneFile");
   root.append_attribute("exitSceneFile");
   root.append_attribute("currentSceneType");
   root.append_attribute("currentMission");
   root.append_attribute("currentState");
   root.append_attribute("exitSceneType");
   root.append_attribute("exitParameter");
   root.append_attribute("gunLevel");
   root.append_attribute("gunFireRateLevel");
   root.append_attribute("gunBulletSpeedLevel");
   root.append_attribute("gunBulletDamageLevel");
   root.append_attribute("shieldMaxLevel");
   root.append_attribute("damageMaxLevel");
   root.append_attribute("speedLevel");
   root.append_attribute("turnSpeedLevel");
   root.append_attribute("fuelMaxLevel");
   root.append_attribute("leftHipJointLevel");
   root.append_attribute("leftKneeJointLevel");
   root.append_attribute("leftFootJointLevel");
   root.append_attribute("rightHipJointLevel");
   root.append_attribute("rightKneeJointLevel");
   root.append_attribute("rightFootJointLevel");

   for (int i = 0; i < c_numLegJoints; i++)
   {
      root.append_attribute(c_legHealthAttributes[i]);
   }
}

void GameStatus::setSceneMissionState(
   const std::string& scene, 
   int mission, 
   int state, 
   int type, 
   const std::string& exitScene, 
   int exitSceneType, 
   int exitParameter )
{
   m_currentSceneFile = scene;
   m_currentMission = mission;
   m_currentState = state;
   m_currentSceneType = type;
   m_exitSceneFile = exitScene;
   m_exitSceneType = exitSceneType;
   m_exitParameter = exitParameter;

   // TODO: Is this used? If so, should exit data be added?
}

void GameStatus::saveGameStatus(void)
{
   xml_node root = m_gameStatusDocument.child("leapfrogGameStatus");

   root.attribute("ammo").set_value(m_leapfrogResources.getShots());
   root.attribute("shield").set_value(m_leapfrogResources.getShield());
   root.attribute("fuel").set_value(m_leapfrogResources.getFuel());
   root.attribute("credits").set_value(m_leapfrogResources.getCredits());
   root.attribute("damage").set_value(m_leapfrogResources.getDamage());
   root.attribute("currentSceneFile").set_value(m_currentSceneFile.c_str());
   root.attribute("exitSceneFile").set_value(m_exitSceneFile.c_str());
   root.attribute("currentSceneType").set_value(m_currentSceneType);
   root.attribute("currentMission").set_value(m_currentMission);
   root.attribute("currentState").set_value(m_currentState);
   root.attribute("exitSceneType").set_value(m_exitSceneType);
   root.attribute("exitParameter").set_value(m_exitParameter);
   root.attribute("gunLevel").set_value(m_gunLevel);
   root.attribute("gunFireRateLevel").set_value(m_gunFireRateLevel);
   root.attribute("gunBulletSpeedLevel").set_value(m_gunBulletSpeedLevel);
   root.attribute("gunBulletDamageLevel").set_value(m_gunBulletDamageLevel);
   root.attribute("shieldMaxLevel").set_value(m_shieldMaxLevel);
   root.attribute("damageMaxLevel").set_value(m_damageMaxLevel);
   root.attribute("speedLevel").set_value(m_speedLevel);
   root.attribute("turnSpeedLevel").set_value(m_turnSpeedLevel);
   root.attribute("fuelMaxLevel").set_value(m_fuelMaxLevel);
   root.attribute("leftHipJointLevel").set_value(m_leftHipJointLevel);
   root.attribute("leftKneeJointLevel").set_value(m_leftKneeJointLevel);
   root.attribute("leftFootJointLevel").set_value(m_leftFootJointLevel);
   root.attribute("rightHipJointLevel").set_value(m_rightHipJointLevel);
   root.attribute("rightKneeJointLevel").set_value(m_rightKneeJointLevel);
   root.attribute("rightFootJointLevel").set_value(m_rightFootJointLevel);
   writeLegHealth(root);
   writeKeyItems(root);

   // m_gameStatusDocument.save_file(c_gameStatusFileName.c_str());

   oxygine_xml_writer my_writer;
   m_gameStatusDocument.save(my_writer);
   ox::file::write(c_gameStatusFileName, my_writer.m_buf);
}

void GameStatus::saveGameSceneStatus(void)
{
   // We only want to change some data, not all. We assume that the 
   // m_gameStatusDocument is up to date.

   oxygine_xml_writer my_writer;
   m_gameStatusDocument.save(my_writer);
   ox::file::write(c_gameStatusFileName, my_writer.m_buf);
}

void GameStatus::readGameStatus(void)
{
   xml_node root = m_gameStatusDocument.child("leapfrogGameStatus");

   m_currentSceneFile = root.attribute("currentSceneFile").as_string("");
   m_exitSceneFile = root.attribute("exitSceneFile").as_string("");
   m_currentSceneType = root.attribute("currentSceneType").as_int(0);
   m_currentMission = root.attribute("currentMission").as_int(0);
   m_currentState = root.attribute("currentState").as_int(0);
   m_exitSceneType = root.attribute("exitSceneType").as_int(0);
   m_exitParameter = root.attribute("exitParameter").as_int(0);
   m_gunLevel = root.attribute("gunLevel").as_int(1);
   m_gunFireRateLevel = root.attribute("gunFireRateLevel").as_int(1);
   m_gunBulletSpeedLevel = root.attribute("gunBulletSpeedLevel").as_int(1);
   m_gunBulletDamageLevel = root.attribute("gunBulletDamageLevel").as_int(1);
   m_shieldMaxLevel = root.attribute("shieldMaxLevel").as_int(1);
   m_damageMaxLevel = root.attribute("damageMaxLevel").as_int(1);
   m_speedLevel = root.attribute("speedLevel").as_int(1);
   m_turnSpeedLevel = root.attribute("turnSpeedLevel").as_int(1);
   m_fuelMaxLevel = root.attribute("fuelMaxLevel").as_int(1);
   m_leftHipJointLevel = root.attribute("leftHipJointLevel").as_int(1);
   m_leftKneeJointLevel = root.attribute("leftKneeJointLevel").as_int(1);
   m_leftFootJointLevel = root.attribute("leftFootJointLevel").as_int(1);
   m_rightHipJointLevel = root.attribute("rightHipJointLevel").as_int(1);
   m_rightKneeJointLevel = root.attribute("rightKneeJointLevel").as_int(1);
   m_rightFootJointLevel = root.attribute("rightFootJointLevel").as_int(1);

   for (int i = 0; i < c_numLegJoints; i++)
   {
      m_legHealth[i] = root.attribute(c_legHealthAttributes[i]).as_float(1.0f);
   }
}

void GameStatus::restoreLeapfrogResources(void)
{
   xml_node root = m_gameStatusDocument.child("leapfrogGameStatus");

   m_leapfrogResources.setResources(
      root.attribute("ammo").as_int(1),
      root.attribute("shield").as_float(1.0),
      root.attribute("fuel").as_float(1.0),
      root.attribute("credits").as_int(1),
      root.attribute("damage").as_float(1.0)
   );
}

void GameStatus::restoreGameStatus(void)
{
   ox::file::buffer bf;

   if (ox::file::exists(c_gameStatusFileName))
   {
      ox::file::read(c_gameStatusFileName.c_str(), bf);

      xml_parse_result result = m_gameStatusDocument.load_buffer(&bf.data[0], bf.size());

      readGameStatus();

      // Not in readGameStatus: c_saveGameStatus calls that at a mission's
      // first scene, which must not undo items given since the last save
      xml_node root = m_gameStatusDocument.child("leapfrogGameStatus");
      readKeyItems(root);
   }
   else
   {
      // File did not exists, we create a fresh one with default
      // settings
      initializeGameStatusXmlDocument();
      gameStatusNewGameInit();
   }
}


ObjectResources* GameStatus::getResources(void)
{
   return &m_leapfrogResources;
}

std::string GameStatus::getCurrentScene(void)
{
   return m_currentSceneFile;
}

int GameStatus::getCurrentMission(void)
{
   return m_currentMission;
}

int GameStatus::getCurrentState(void)
{
   return m_currentState;
}

int GameStatus::getCurrentSceneType(void)
{
   return m_currentSceneType;
}

std::string GameStatus::getExitScene(void)
{
   return m_exitSceneFile;
}

int GameStatus::getExitSceneType(void)
{
   return m_exitSceneType;
}

int GameStatus::getExitParameter(void)
{
   return m_exitParameter;
}



