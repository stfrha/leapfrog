#pragma once

enum CollisionEntityTypeEnum
{
   CET_LEAPFROG,
   CET_BULLET,
   CET_ASTEROID
};

class CollisionEntity
{
public:
   virtual CollisionEntityTypeEnum getEntityType(void) = 0;
};
