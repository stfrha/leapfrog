#pragma once

enum CollisionEntityTypeEnum
{
   CET_LEAPFROG,
   CET_BULLET,
   CET_ASTEROID,
   CET_FLAME_PARTICLE,
   CET_LF_BIG_LEG,
   CET_LF_SMALL_LEG,
   CET_LF_BOOSTER,
   CET_LF_FOOT,
   CET_LF_STEER_BOOSTER,
   CET_LF_SHIELD
};

class CollisionEntity
{
public:
   virtual CollisionEntityTypeEnum getEntityType(void) = 0;
};
