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
   oxygine::Actor* m_eventActor;
   oxygine::ProgressBar* m_progressBar;
   std::string m_headline;
   int m_propertyId;
   float m_maxProgess;
   ObjectResourcesTypeEnum m_statusType;

public:
   StatusBar(
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
	void doUpdate(const oxygine::UpdateState& us) override;
};


DECLARE_SMART(StatusLiteral, spStatusLiteral);

class StatusLiteral : public oxygine::Actor
{
private:
   SceneActor * m_sceneActor;
   oxygine::Actor* m_eventActor;
   oxygine::TextField* m_tf;
   int m_propertyId;
   std::string m_headline;
   ObjectResourcesTypeEnum m_statusType;

public:
   StatusLiteral(
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
   void doUpdate(const oxygine::UpdateState& us) override;
};


