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
   CET_LF_SHIELD,
   CET_LAUNCH_SITE_FOOT_REST,
   CET_NOT_APPLICABLE
};

class CollisionEntity
{
public:
   virtual CollisionEntityTypeEnum getEntityType(void) = 0;

   static CollisionEntityTypeEnum convert(const std::string& value)
   {
      if (value == "leapfrog") return CET_LEAPFROG;
      else if (value == "bullet") return CET_BULLET;
      else if (value == "asteroid") return CET_ASTEROID;
      else if (value == "flameParticle") return CET_FLAME_PARTICLE;
      else if (value == "lfBigLeg") return CET_LF_BIG_LEG;
      else if (value == "lfSmallLeg") return CET_LF_SMALL_LEG;
      else if (value == "lfBooster") return CET_LF_BOOSTER;
      else if (value == "lfFoot") return CET_LF_FOOT;
      else if (value == "lfSteerBooster") return CET_LF_STEER_BOOSTER;
      else if (value == "lfShield") return CET_LF_SHIELD;
      else if (value == "launchSiteFootRest") return CET_LAUNCH_SITE_FOOT_REST;
      return CET_NOT_APPLICABLE;
   }
};
