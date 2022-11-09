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
public:
   MenuButton(
      MenuActor* menuParent,
      const oxygine::Vector2& pos,
      const oxygine::Vector2& size,
      std::string caption /*  ,
      event eventToFireAtButtonCLick*/
      );

protected:
   void doUpdate(const oxygine::UpdateState& us);


};

class MainActor;


DECLARE_SMART(MenuActor, spMenuActor);

class MenuActor : public oxygine::Actor
{
public:
private:
   MainActor* m_mainActor;

   void b1Clicked(oxygine::Event* event);
   void b2Clicked(oxygine::Event* event);
   void b3Clicked(oxygine::Event* event);
   void b4Clicked(oxygine::Event* event);
   void b5Clicked(oxygine::Event* event);
   void b6Clicked(oxygine::Event* event);
   void b7Clicked(oxygine::Event* event);

public:
   MenuActor(MainActor* mainActor);

protected:
   void doUpdate(const oxygine::UpdateState &us);

};

