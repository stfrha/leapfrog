#include "gamestatus.h"

using namespace oxygine;
using namespace std;
using namespace pugi;

// Instansiate global class
GameStatus g_GameStatus;

const string GameStatus::c_gameStatusFileName = "leapfrog_game_progress.xml";

GameStatus::GameStatus() :
   m_leapfrogResources(),
   m_currentSceneFile(""),
   m_currentMission(0),
   m_currentState(0),
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
   m_rightFootJointLevel(0)
{
}


void GameStatus::gameStatusNewGameInit(void)
{
   // Resources cannot be set yet. For initialization they are 
   // written 
   m_leapfrogResources.setResources(70, 100.0f, 25.0f, 20, 0.0f);
   m_currentSceneFile = "not_defined";
   m_currentMission = 0;
   m_currentState = 0;
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
   root.attribute("currentMission").set_value(m_currentMission);
   root.attribute("currentState").set_value(m_currentState);
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

   m_gameStatusDocument.save_file(c_gameStatusFileName.c_str());

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
   root.append_attribute("currentMission");
   root.append_attribute("currentState");
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
}

void GameStatus::setSceneMissionState(const std::string& scene, int mission, int state)
{
   m_currentSceneFile = scene;
   m_currentMission = mission;
   m_currentState = state;
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
   root.attribute("currentMission").set_value(m_currentMission);
   root.attribute("currentState").set_value(m_currentState);
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

   m_gameStatusDocument.save_file(c_gameStatusFileName.c_str());
}

void GameStatus::readGameStatus(void)
{
   xml_node root = m_gameStatusDocument.child("leapfrogGameStatus");

   m_currentSceneFile = root.attribute("currentSceneFile").as_string("");
   m_currentMission = root.attribute("currentMission").as_int(0);
   m_currentState = root.attribute("currentState").as_int(0);
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




