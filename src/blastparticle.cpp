#include "blastparticle.h"

using namespace oxygine;

BlastParticle::BlastParticle(
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
   setPriority(162);

   // Randomise value between 0 and 255
   float color = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 255.0f));

   setColor((unsigned int)color, (unsigned int)color, (unsigned int)color, 255);


   // Generate goal position, lets sparkle 20 meters in random direction

   // Randomise value between 0 and 2pi
   float angle = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 2.0f / MATH_PI));

   Vector2 v = Vector2(distance * cos(angle), distance * sin(angle));
   
   spTween posTween = addTween(Actor::TweenPosition(pos + v), lifetime);
   spTween scaleTween = addTween(Actor::TweenScale(0.0f), lifetime);

   posTween->setDoneCallback(CLOSURE(this, &BlastParticle::atParticleDeath));
}

void BlastParticle::doUpdate(const oxygine::UpdateState& us)
{
}

void BlastParticle::atParticleDeath(oxygine::Event* event)
{
   this->detach();
}