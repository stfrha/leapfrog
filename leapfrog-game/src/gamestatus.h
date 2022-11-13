#pragma once

#include "oxygine-framework.h"

#include "objectresources.h"


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

DECLARE_SMART(GameStatus, spGameStatus)


class GameStatus
{
private:
   static const std::string c_gameStatusFileName;


   pugi::xml_document m_gameStatusDocument;

   ObjectResources m_leapfrogResources;
   std::string m_currentSceneFile;
   int m_currentSceneType;
   int m_currentMission;
   int m_currentState;
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

public:
   GameStatus();

   void gameStatusNewGameInit(void);
   void initializeGameStatusXmlDocument(void);
   void setSceneMissionState(const std::string& scene, int mission, int state, int type);
   void saveGameStatus(void);
   void readGameStatus(void);
   void restoreGameStatus(void);
   void restoreLeapfrogResources(void);
   ObjectResources* getResources(void);

   std::string getCurrentScene(void);
   int getCurrentSceneType(void);
   int getCurrentMission(void);
   int getCurrentState(void);

};


extern GameStatus g_GameStatus;


