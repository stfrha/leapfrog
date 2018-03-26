#include "freespaceactor.h"

#include "bullet.h"

#include "gun.h"

using namespace oxygine;

Gun::Gun(
   Resources& gameResources, 
   SceneActor* sceneActor,
   b2Body* body,
   b2Vec2 emitterOrigin,
   float angle,
   float fireRate,
   float lifetime,
   float bulletSpeed,
   bool bouncy) :
   m_fire(false),
   m_freeSpaceActor(NULL),
   m_emitterBody(body),
   m_emitterOrigin(emitterOrigin),
   m_emittAngle(angle),
   m_interval(1.0f / fireRate * 1000.0),
   m_lifetime(lifetime),
   m_bulletSpeed(bulletSpeed),
   m_gameResources(&gameResources),
   m_sceneActor(sceneActor)
{
}

void Gun::setBoundedWallsActor(FreeSpaceActor* actor)
{
   m_freeSpaceActor = actor;
}


void Gun::startGun(void)
{
   if (!m_fire)
   {
      // If this is the first time the gun is fired, 
      // we reset m_timeSinceLast
      // Setting this to above m_interval will cause an immediate shot
      // when fire is pressed
      m_timeSinceLast = m_interval + 1;
   }

   m_fire = true;
}

void Gun::stopGun(void)
{
   m_fire = false;
}


void Gun::doUpdate(const oxygine::UpdateState& us)
{
   if (m_fire)
   {
      m_timeSinceLast += us.dt;

      if (m_timeSinceLast >= m_interval)
      {
         b2Vec2 emitPos = m_emitterBody->GetWorldPoint(m_emitterOrigin);

         float bodyAngle = m_emitterBody->GetAngle();
         float emitAngle = bodyAngle + m_emittAngle;

         b2Vec2 craftSpeed = m_emitterBody->GetLinearVelocity();

         spBullet bullet = new Bullet(
            *m_gameResources,
            m_sceneActor,
            m_emitterBody->GetWorld(),
            emitPos,
            emitAngle,
            m_bulletSpeed,
            craftSpeed,
            m_lifetime,
            false);

         // Attach to parent's parent which is the view actor
         bullet->attachTo(getParent()->getParent());

         if (m_freeSpaceActor != NULL)
         {
            m_freeSpaceActor->addBoundingBody((Actor*)bullet.get());
         }


         m_timeSinceLast = 0;

      }
   }
}
