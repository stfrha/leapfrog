#include "smokeparticle.h"

using namespace oxygine;

SmokeParticle::SmokeParticle(
   oxygine::Resources* gameResources,
   const Vector2& pos,
   int lifetime,
   float distance,
   float particleSize)
{
   setResAnim(gameResources->getResAnim("white_circle"));
   setSize(particleSize, particleSize);
   setPosition(pos);
   setAnchor(Vector2(0.5f, 0.5f));
   setAlpha(64);

   setColor(Color::DarkGray);

   // Randomise value between 0 and 1
   float scale = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1.0f));
   setScale(scale);

   spTween alphaTween = addTween(Actor::TweenAlpha(0), lifetime);

   alphaTween->setDoneCallback(CLOSURE(this, &SmokeParticle::atParticleDeath));
}

void SmokeParticle::doUpdate(const oxygine::UpdateState& us)
{
}

void SmokeParticle::atParticleDeath(oxygine::Event* event)
{
   this->detach();
}