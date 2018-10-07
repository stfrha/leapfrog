#include "freespaceactor.h"
#include "bullet.h"
#include "gun.h"
#include "gamestatus.h"

using namespace oxygine;
using namespace pugi;

Gun::Gun(
   Resources* gameResources,
   SceneActor* sceneActor,
   CompoundObject* parentObject,
   b2World* world,
   const xml_node& objectNode) :
   System(gameResources, sceneActor, world, parentObject),
   m_fire(false)
   //m_emitterBody(body),
   //m_emitterOrigin(emitterOrigin),
   //m_emittAngle(angle),
   //m_interval(1.0f / fireRate * 1000.0f),
   //m_lifetime(lifetime),
   //m_impulseMagnitude(impulseMagnitude),
   //m_gameResources(&gameResources),
   //m_sceneActor(sceneActor)
{
   readGunNode(objectNode);
   attachTo(sceneActor);
}

void Gun::readGunNode(const xml_node& objectNode)
{
   m_emitterBody = m_parent->getBody(objectNode.attribute("body").as_string());

   m_emitterOrigin.x = objectNode.attribute("emitterOriginX").as_float();
   m_emitterOrigin.y = objectNode.attribute("emitterOriginY").as_float();
   m_emittAngle = objectNode.attribute("angle").as_float();
   m_lifetime = objectNode.attribute("lifeTime").as_int(); // [ms}
   m_impulseMagnitude = objectNode.attribute("impulseMagnitude").as_float();
   m_interval = 1.0f / objectNode.attribute("fireRate").as_float() * 1000.0f;
   m_bouncy = objectNode.attribute("impulseMagnitude").as_bool();
}

void Gun::startGun(void)
{
   if (!m_fire)
   {
      // If this is the first time the gun is fired, 
      // we reset m_timeSinceLast
      // Setting this to above m_interval will cause an immediate shot
      // when fire is pressed
      m_timeSinceLast = (int)m_interval + 1;
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
         if (g_GameStatus.getShots() > 0)
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
               m_impulseMagnitude,
               craftSpeed,
               m_lifetime,
               false);

            // Attach to parent's parent which is the view actor
            bullet->attachTo(m_sceneActor);

            m_timeSinceLast = 0;

            g_GameStatus.deltaShots(-1);
         }
      }
   }
}
