#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"

DECLARE_SMART(Shield, spShield);

class Shield : public oxygine::Sprite
{
private:

public:
	Shield(
      oxygine::Resources* gameResources,
      const oxygine::Vector2& pos, 
      float angle);

protected:
	void doUpdate(const oxygine::UpdateState& us);
   void atShieldComplete(oxygine::Event* event);
};
