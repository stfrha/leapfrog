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

   std::string getCurrentScene(void);
   int getCurrentSceneType(void);
   int getCurrentMission(void);
   int getCurrentState(void);
   std::string getExitScene(void);
   int getExitSceneType(void);
   int getExitParameter(void);

};


extern GameStatus g_GameStatus;


