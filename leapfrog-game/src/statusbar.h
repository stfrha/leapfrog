#pragma once

#include "oxygine-framework.h"
#include "layout.h"
#include "objectresourcesevents.h"

DECLARE_SMART(StatusBar, spStatusBar);

class SceneActor;

class StatusBar : public oxygine::Actor
{
private:
   SceneActor* m_sceneActor;
   oxygine::spActor m_eventActor;
   oxygine::spProgressBar m_progressBar;
   std::string m_headline;
   int m_propertyId;
   float m_maxProgess;
   ObjectResourcesTypeEnum m_statusType;

public:
   StatusBar(
      oxygine::Resources& gameResources,
      oxygine::Actor* eventActor,
      SceneActor* sceneActor,
      oxygine::Actor* attachToMe,
      int propertyId,
      const oxygine::Vector2 & pos,
      const oxygine::Vector2 & size,
      const int fontSize,
      const float maxProgress,
      const float initialProgress,
      std::string headline,
      ObjectResourcesTypeEnum statusType);

   void disconnectListner(void);
   void statusChangedListner(oxygine::Event *ev);
   void dummyListner(oxygine::Event *ev);

protected:
	void doUpdate(const oxygine::UpdateState& us);
};


DECLARE_SMART(StatusLiteral, spStatusLiteral);

class StatusLiteral : public oxygine::Actor
{
private:
   SceneActor * m_sceneActor;
   oxygine::spActor m_eventActor;
   oxygine::spTextField m_tf;
   int m_propertyId;
   std::string m_headline;
   ObjectResourcesTypeEnum m_statusType;

public:
   StatusLiteral(
      oxygine::Resources& gameResources,
      oxygine::Actor* eventActor,
      SceneActor* sceneActor,
      Actor* attachToMe,
      int propertyId,
      const oxygine::Vector2 & pos,
      const oxygine::Vector2 & size,
      const int fontSize,
      const int initialLiteral,
      std::string headline,
      ObjectResourcesTypeEnum statusType);

   void disconnectListner(void);
   void statusChangedListner(oxygine::Event *ev);
   void dummyListner(oxygine::Event *ev);
   void setLiteral(int value);

protected:
   void doUpdate(const oxygine::UpdateState& us);
};


