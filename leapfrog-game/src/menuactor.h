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
      oxygine::Resources& hudResources,
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

public:
	MenuActor(
      Resources& hudResources,
      MainActor* parent,
      const Vector2& pos,
      const Vector2& size);

protected:
   void doUpdate(const oxygine::UpdateState &us);

};

