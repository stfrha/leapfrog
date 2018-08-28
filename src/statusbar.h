#pragma once

#include "oxygine-framework.h"
#include "layout.h"
#include "gamestatusevents.h"

DECLARE_SMART(StatusBar, spStatusBar);

class SceneActor;

class StatusBar : public oxygine::Actor
{
private:
   SceneActor* m_sceneActor;
   oxygine::ProgressBar* m_progressBar;
   std::string m_headline;
   float m_maxProgess;
   StatusChangedEvent::GameStatusTypeEnum m_statusType;

public:
   StatusBar(
      oxygine::Resources& gameResources,
      SceneActor* sceneActor,
      const oxygine::Vector2 & pos,
      const oxygine::Vector2 & size,
      const float maxProgress,
      const float initialProgress,
      std::string headline,
      StatusChangedEvent::GameStatusTypeEnum statusType);

   void statusChangedListner(oxygine::Event *ev);

protected:
	void doUpdate(const oxygine::UpdateState& us);
};
