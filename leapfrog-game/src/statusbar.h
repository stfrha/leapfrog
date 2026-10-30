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




// A small picture of the Leapfrog's legs under the status bars: the body,
// the two legs and the six leg joints (hips, knees, feet). Each joint is a
// ring that fills with red as it loses health, all red when it has failed.
// Health is read from the game status, which the Leapfrog keeps current.

DECLARE_SMART(LegStatus, spLegStatus);

class LegStatus : public oxygine::Actor
{
public:
   static const int c_numJoints = 6;
   static const int c_ringSegments = 24;

private:
   std::vector<oxygine::spColorRectSprite> m_rings[c_numJoints];
   float m_shownHealth[c_numJoints];

   void addSegment(const oxygine::Vector2& a, const oxygine::Vector2& b, float thickness, const oxygine::Color& color);
   void addDisk(const oxygine::Vector2& centre, float radius, const oxygine::Color& color);
   void addRing(int joint, const oxygine::Vector2& centre, float radius, float thickness);
   void showHealth(int joint, float health);

public:
   // In a box at pos (top left) of size; the picture keeps its proportions
   // and is centred in the box
   LegStatus(oxygine::Actor* attachToMe, const oxygine::Vector2& pos, const oxygine::Vector2& size);

protected:
   void doUpdate(const oxygine::UpdateState& us) override;
};
