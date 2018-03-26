#include "Shield.h"

using namespace oxygine;

Shield::Shield(
   oxygine::Resources* gameResources,
   const Vector2& pos,
   float angle)
{
   setResAnim(gameResources->getResAnim("shield_exciter"));

//   ResAnim* resAnim = gameResources->getResAnim("shield_exciter");
   setAnchor(Vector2(0.0f, 0.0f));
   //setSize(512.0, 512.0);
   //setScale(12.0f / 512.0f);
   setSize(9.0, 4.5);
   setPosition(0.0f, 0.0f);
   setRotation(MATH_PI / 2.0f);
   setAlpha(100);

   //spTween posTween = addTween(TweenAnim(resAnim), 500);

   //posTween->setDoneCallback(CLOSURE(this, &Shield::atShieldComplete));
}

void Shield::doUpdate(const oxygine::UpdateState& us)
{
}

void Shield::atShieldComplete(oxygine::Event* event)
{
   this->detach();
}