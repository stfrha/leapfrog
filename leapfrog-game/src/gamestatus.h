#pragma once

#include "oxygine-framework.h"

#include "objectresources.h"

#include <map>
#include <string>
#include <vector>


class oxygine_xml_writer : public pugi::xml_writer
{
public:
   ox::file::buffer m_buf;

   virtual void write(const void* data, size_t size)
   {
      for (size_t i = 0; i < size; i++)
      {
         m_buf.push_back(static_cast<const char*>(data)[i]);
      }
   }
};

// Key items: things the player holds that unlock progress, never used up
// (data/key_items.xml declares them):
//   pass       a faction's permission for services (fuel, repair,
//              upgrades, space launch) at its pads and stations
//   solution   a landing site's re-entry solution
//   route      a hyperspace route to another world
//   blueprint  needed for an upgrade
// Each is unknown, held or revoked. Passes are typically revoked (by
// their issuer); knowledge can be lost (back to unknown).
class KeyItem
{
public:
   std::string m_id;
   std::string m_kind;     // pass, solution, route, blueprint
   std::string m_name;     // shown to the player
   std::string m_issuer;   // who issues it (an affiliation name), may be ""
   int m_tier;             // for passes: higher tiers allow more; 0 if none
   bool m_initial;         // held from the start of a game

   KeyItem() : m_tier(0), m_initial(false) { }
};

enum KeyItemState
{
   KIS_UNKNOWN = 0,
   KIS_HELD = 1,
   KIS_REVOKED = 2
};

DECLARE_SMART(GameStatus, spGameStatus)


class GameStatus
{
private:
   static const std::string c_gameStatusFileName;


   pugi::xml_document m_gameStatusDocument;

   ObjectResources m_leapfrogResources;
   std::string m_currentSceneFile;
   std::string m_exitSceneFile;
   int m_currentSceneType;
   int m_currentMission;
   int m_currentState;
   int m_exitSceneType;
   int m_exitParameter;
   int m_gunLevel;
   int m_gunFireRateLevel;
   int m_gunBulletSpeedLevel;
   int m_gunBulletDamageLevel;
   int m_shieldMaxLevel;
   int m_damageMaxLevel;
   int m_speedLevel;
   int m_turnSpeedLevel;
   int m_fuelMaxLevel;
   int m_leftHipJointLevel;
   int m_leftKneeJointLevel;
   int m_leftFootJointLevel;
   int m_rightHipJointLevel;
   int m_rightKneeJointLevel;
   int m_rightFootJointLevel;

   // Leg joint health, 1 = sound, 0 = limp. Index: 0 left hip, 1 left knee,
   // 2 left foot, 3 right hip, 4 right knee, 5 right foot
   static const int c_numLegJoints = 6;
   static const char* const c_legHealthAttributes[c_numLegJoints];
   float m_legHealth[c_numLegJoints];

   void writeLegHealth(pugi::xml_node& root);

   // Key items: the definitions (data/key_items.xml, read once) and the
   // states that differ from an item's default (held if initial, else
   // unknown). The save stores the same as <keyItems><item id state/>.
   static const std::string c_keyItemsFileName;
   std::vector<KeyItem> m_keyItemDefs;
   bool m_keyItemDefsRead;
   std::map<std::string, KeyItemState> m_keyItemStates;

   void readKeyItemDefinitions(void);
   void writeKeyItems(pugi::xml_node& root);
   void readKeyItems(pugi::xml_node& root);

public:
   GameStatus();

   void gameStatusNewGameInit(void);
   void initializeGameStatusXmlDocument(void);
   void setSceneMissionState(
      const std::string& scene, int mission, int state, int type,
      const std::string& exitScene,
      int exitSceneType,
      int exitParameter);
   void saveGameStatus(void);
   void saveGameSceneStatus(void);
   void readGameStatus(void);
   void restoreGameStatus(void);
   void restoreLeapfrogResources(void);
   ObjectResources* getResources(void);

   float getLegHealth(int index);
   void setLegHealth(int index, float health);

   // Key items. getKeyItem: NULL if key_items.xml has no such id.
   const KeyItem* getKeyItem(const std::string& id);
   KeyItemState getKeyItemState(const std::string& id);
   // Sets the state (in memory; saved with the game). False for an unknown
   // id. previous: the state before.
   bool setKeyItemState(const std::string& id, KeyItemState state, KeyItemState* previous = NULL);
   // Ids of the held items, of one kind ("" = all), in file order
   std::vector<std::string> getHeldKeyItems(const std::string& kind);
   static const char* keyItemStateName(KeyItemState state);
   static bool keyItemStateFromName(const std::string& name, KeyItemState& state);

   std::string getCurrentScene(void);
   int getCurrentSceneType(void);
   int getCurrentMission(void);
   int getCurrentState(void);
   std::string getExitScene(void);
   int getExitSceneType(void);
   int getExitParameter(void);

};


extern GameStatus g_GameStatus;


