#include "planetactor.h"


using namespace oxygine;

PlanetActor::PlanetActor(Resources& gameResources) :
   SceneActor(gameResources)
{
   // I should probably load resources that are uniuqe here
   setPanorateMode(PME_TOP_LEFT);

   m_maskedSprite = new MaskedSprite();
   m_maskedSprite->attachTo(this);
   m_maskedSprite->setAnchor(0.5f, 0.5f);
   m_maskedSprite->setPosition(Vector2(500.0f, 4500.0f));

   m_planetSprite = new Box9Sprite();
   m_planetSprite->setResAnim(gameResources.getResAnim("planet_rock"));
   m_planetSprite->setSize(7000.0f, 7000.0f);
   m_planetSprite->setAnchor(0.5f, 0.5f);
   m_planetSprite->setGuides(0, 0, 0, 0);
   m_planetSprite->setHorizontalMode(Box9Sprite::TILING_FULL);
   m_planetSprite->setVerticalMode(Box9Sprite::TILING_FULL);
   m_planetSprite->attachTo(m_maskedSprite);

   spColorRectSprite crs = new ColorRectSprite();
   crs->setColor(Color::Pink);
   crs->setSize(100.0f, 100.0f);
   crs->setAnchor(0.5f, 0.5f);

   float alpha = -100.0 / 180.0f * MATH_PI;
   crs->setPosition(Vector2(3500.0f * cos(alpha), 3500.0f * sin(alpha)));
   crs->setRotation(alpha);
   crs->attachTo(m_maskedSprite);

   m_mask = new Sprite();
   m_mask->setResAnim(gameResources.getResAnim("planet_mask"));
   m_mask->setSize(7000.0f, 7000.0f);
   m_mask->setAnchor(0.5f, 0.5f);
   m_mask->attachTo(m_maskedSprite);
   m_mask->setVisible(false);
   // m_mask->setPriority(1);

   m_maskedSprite->setMask(m_mask, true);
   m_maskedSprite->attachTo(this);








   createLeapFrog(gameResources);

   m_leapfrog->goToMode(LFM_ORBIT);
   m_leapfrog->goToEnvironment(ENV_ORBIT);

   ((b2Body*)m_leapfrog->getUserData())->SetTransform(b2Vec2(500.0f, 10.0f), 0.0f);

   spTween rotTween = m_maskedSprite->addTween(Actor::TweenRotation(2.0f * MATH_PI), 90000);
}
