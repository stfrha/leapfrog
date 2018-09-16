
#include "steeringbody.h"
#include "collisionentity.h"

SteeringBody::SteeringBody(
   b2Body* host
) :
   m_body(host)
{

}

b2Vec2 SteeringBody::GetPosition()
{
   return m_body->GetPosition();
}

b2Vec2 SteeringBody::GetLinearVelocity()
{
   return m_body->GetLinearVelocity();
}


float SteeringBody::GetMass()
{
   return m_body->GetMass();

}

b2Body* SteeringBody::GetBodyList()
{
   return m_body->GetWorld()->GetBodyList();
}


