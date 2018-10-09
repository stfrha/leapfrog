#pragma once

#include "oxygine-framework.h"


class CollisionEntity
{
public:
   enum CollisionEntityTypeEnum
   {
      notApplicable,
      leapfrog,
      bullet,
      flameParticle,
      lfBigLeg,
      lfSmallLeg,
      lfBooster,
      lfLeftFoot,
      lfRightFoot,
      lfSteerBooster,
      lfShield,
      lsLeftFootRest,
      lsRightFootRest,
      landingPad,
      breakableObject,
      destroyableObject,
      steerableObject
   };

   static CollisionEntityTypeEnum convert(const std::string& value)
   {
      if (value == "leapfrog") return CollisionEntity::leapfrog;
      else if (value == "bullet") return CollisionEntity::bullet;
      else if (value == "flameParticle") return CollisionEntity::flameParticle;
      else if (value == "lfBigLeg") return CollisionEntity::lfBigLeg;
      else if (value == "lfSmallLeg") return CollisionEntity::lfSmallLeg;
      else if (value == "lfBooster") return CollisionEntity::lfBooster;
      else if (value == "lfLeftFoot") return CollisionEntity::lfLeftFoot;
      else if (value == "lfRightFoot") return CollisionEntity::lfRightFoot;
      else if (value == "lfSteerBooster") return CollisionEntity::lfSteerBooster;
      else if (value == "lfShield") return CollisionEntity::lfShield;
      else if (value == "launchSiteLeftFootRest") return CollisionEntity::lsLeftFootRest;
      else if (value == "launchSiteRightFootRest") return CollisionEntity::lsRightFootRest;
      else if (value == "landingPad") return CollisionEntity::landingPad;
      else if (value == "breakableObject") return CollisionEntity::breakableObject;
      else if (value == "destroyableObject") return CollisionEntity::destroyableObject;
      else if (value == "steerableObject") return CollisionEntity::steerableObject;

      return CollisionEntity::notApplicable;
   }
};
