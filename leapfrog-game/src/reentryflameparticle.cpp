#include "reentryflameparticle.h"
#include "graphicresources.h"

using namespace oxygine;

ReentryFlameParticle::ReentryFlameParticle(
   const Vector2& pos,
   int lifetime,
   const Vector2& growToSize)
{
   setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::game).getResAnim("reentry_flames"));
   setSize(growToSize);
   setPosition(pos);
   setAnchor(Vector2(0.5f, 1.0f));
   setScale(0.1f);
   setAlpha(128);
   setPriority(138);


   //// Randomise value between 0 and 255
   //float color = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 255.0f));
   //setColor((unsigned int)color, (unsigned int)color, (unsigned int)color, 255);
 
   spTweenQueue scaleTweenQueue = new TweenQueue;
   scaleTweenQueue->add(Actor::TweenScale(1.0f), (int)((float)lifetime * 0.25f));
   scaleTweenQueue->add(Actor::TweenScale(0.0f), (int)((float)lifetime * 0.75f));

   spTween scaleTween = addTween(scaleTweenQueue);

   scaleTween->setDoneCallback(CLOSURE(this, &ReentryFlameParticle::atParticleDeath));
}

void ReentryFlameParticle::doUpdate(const oxygine::UpdateState& us)
{
}

void ReentryFlameParticle::atParticleDeath(oxygine::Event* event)
{
   this->detach();
}