#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "collisionentity.h"
#include "compoundobject.h"
#include "leapfrog.h"

class MenuActor;

DECLARE_SMART(MenuButton, spMenuButton);

class MenuButton : public oxygine::Box9Sprite
{
private:
   oxygine::spTextField m_text;

public:
   MenuButton(
      MenuActor* menuParent,
      const oxygine::Vector2& pos,
      const oxygine::Vector2& size,
      std::string caption /*  ,
      event eventToFireAtButtonCLick*/
      );

   void setCaption(const std::string& caption);

protected:
   void doUpdate(const oxygine::UpdateState& us) override;


};


DECLARE_SMART(MenuActor, spMenuActor);

class MenuActor : public oxygine::Actor
{
private:

   void b1Clicked(oxygine::Event* event);
   void b2Clicked(oxygine::Event* event);
   void b3Clicked(oxygine::Event* event);
   void b7Clicked(oxygine::Event* event);

   // The test row: < label > (lua/test_starts.lua)
   int m_testCount;
   int m_testIndex;              // 1..m_testCount
   spMenuButton m_testLabel;

   void showTestStart(void);
   void testPrevClicked(oxygine::Event* event);
   void testNextClicked(oxygine::Event* event);
   void testStartClicked(oxygine::Event* event);

public:
   MenuActor();

protected:
   void doUpdate(const oxygine::UpdateState &us) override;

};

