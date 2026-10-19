#include "sceneactor.h"
#include "compoundobject.h"
#include "flameparticle.h"
#include "flameemitter.h"

using namespace oxygine;
using namespace pugi;

namespace
{
   // Same rule as Box2D's default contact filter
   bool filtersCollide(const b2Filter& a, const b2Filter& b)
   {
      if ((a.groupIndex == b.groupIndex) && (a.groupIndex != 0))
      {
         return a.groupIndex > 0;
      }

      return ((a.maskBits & b.categoryBits) != 0) && ((a.categoryBits & b.maskBits) != 0);
   }

   // Finds out if a flame particle created at a point would start inside
   // something it collides with (an asteroid the leapfrog is pressed
   // against, for instance). Box2D pushes such a body out at high speed,
   // which throws the particle forward or sideways.
   class SpawnBlockedQuery : public b2QueryCallback
   {
   public:
      b2CircleShape m_shape;
      b2Transform m_xf;
      b2Filter m_filter;
      bool m_blocked;

      SpawnBlockedQuery(const b2Vec2& pos, float radius, const b2Filter& filter) :
         m_filter(filter),
         m_blocked(false)
      {
         m_shape.m_radius = radius;
         m_xf.Set(pos, 0.0f);
      }

      bool ReportFixture(b2Fixture* fixture) override
      {
         if (fixture->IsSensor() || !filtersCollide(m_filter, fixture->GetFilterData()))
         {
            return true;   // continue
         }

         for (int32 child = 0; child < fixture->GetShape()->GetChildCount(); child++)
         {
            if (b2TestOverlap(&m_shape, 0, fixture->GetShape(), child, m_xf, fixture->GetBody()->GetTransform()))
            {
               m_blocked = true;
               return false;   // stop
            }
         }

         return true;
      }
   };
}

FlameEmitter::FlameEmitter(
   SceneActor* sceneActor,
   CompoundObject* parent,
   b2World* world,
   const xml_node& objectNode,
   int groupIndex)  :
   System(
      sceneActor,
      world, 
      parent),
   m_groupIndex(groupIndex),
   m_emit(false),
   m_scale(1.0f)
{
   readFlameEmitterNode(objectNode);
   attachTo(sceneActor);
}

void FlameEmitter::readFlameEmitterNode(const xml_node& objectNode)
{
   m_emitterBody = m_parent->getBody(objectNode.attribute("body").as_string());

   m_emitterOrigin.x = objectNode.attribute("systemOriginX").as_float();
   m_emitterOrigin.y = objectNode.attribute("systemOriginY").as_float();
   m_emittAngle = objectNode.attribute("angle").as_float() * MATH_PI / 180.0f;
   m_emitterWidth = objectNode.attribute("emitterWidth").as_float();
   m_radius = objectNode.attribute("radius").as_float();
   m_lifetime = objectNode.attribute("lifeTime").as_int(); // [ms}
   m_impulseMagnitude = objectNode.attribute("impulseMagnitude").as_float();
}


void FlameEmitter::startEmitter(void)
{
   m_emit = true;
}

void FlameEmitter::stopEmitter(void)
{
   m_emit = false;
}

void FlameEmitter::setFlameScale(float scale)
{
   m_scale = scale;
}

void FlameEmitter::setParameters(
   int lifetime,
   float impulseMagnitude,
   float radius)
{
   m_lifetime = lifetime;
   m_impulseMagnitude = impulseMagnitude;
   m_radius = radius;
}



void FlameEmitter::doUpdate(const oxygine::UpdateState& us)
{
   // For now, emit one particle every update if m_emit is true
   if (m_emit)
   {
//      b2Vec2 originPos = m_emitterBody->GetPosition() + m_emitterBody->GetWorldPoint(m_emitterOrigin);
      b2Vec2 originPos = m_emitterBody->GetWorldPoint(m_emitterOrigin);

      // Randomise value between -m_emitterWidth / 2 and m_emitterWidth / 2
      float dist = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / m_emitterWidth)) - (m_emitterWidth / 2.0f);

      // Randomise angle to get spread
      float ra = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (30.0f / 180.0f * MATH_PI))) - (15.0f / 180.0f * MATH_PI);

      float bodyAngle = m_emitterBody->GetAngle() + ra;
      float emitAngle = bodyAngle + m_emittAngle;

      b2Vec2 emitPos = b2Vec2(dist * cos(bodyAngle), dist * sin(bodyAngle)) + originPos;

      // Randomise impulse magnitude to get spread: +/- 40 % of the
      // magnitude. (It was +/- 125 whatever the magnitude, which with the
      // magnitudes used now, 35-90, made some particles get a negative
      // impulse and fly forward instead of out of the engine.)
      float ri = (static_cast <float> (rand()) / static_cast <float> (RAND_MAX) - 0.5f) * 0.8f * m_impulseMagnitude;

      b2Vec2 impulse = b2Vec2((ri + m_impulseMagnitude) * cos(emitAngle), (ri + m_impulseMagnitude) * sin(emitAngle));

      // Don't create a particle inside something it would collide with
      // (radius and filter as in FlameParticle). Against a rock the flame
      // just disappears into it.
      b2Filter particleFilter;
      particleFilter.categoryBits = 8;
      particleFilter.maskBits = 65525;
      particleFilter.groupIndex = (int16)-m_groupIndex;

      SpawnBlockedQuery query(emitPos, 0.5f, particleFilter);
      b2AABB aabb;
      aabb.lowerBound = emitPos - b2Vec2(0.5f, 0.5f);
      aabb.upperBound = emitPos + b2Vec2(0.5f, 0.5f);
      m_emitterBody->GetWorld()->QueryAABB(&query, aabb);

      if (query.m_blocked)
      {
         return;
      }

      // Create one test flame particle
      // The particle starts with the velocity of the engine nozzle, not of
      // the body's centre: when the leapfrog turns, the nozzle (far from the
      // centre) moves sideways, and the centre's velocity would throw the
      // particles to the side.
      spFlameParticle flameParticle = new FlameParticle(
         m_emitterBody->GetWorld(), 
         emitPos, 
         m_emitterBody->GetLinearVelocityFromWorldPoint(emitPos),
         (int)((float)m_lifetime * m_scale), 
         m_scale * impulse,
         m_radius * m_scale,
         m_groupIndex);

      // Attach to parent's parent which is the view actor
      flameParticle->attachTo(m_sceneActor);
   }
}
