
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "Box2DDebugDraw.h"

DECLARE_SMART(Static, spStatic);

class Static : public oxygine::Box9Sprite
{
public:
	// rc is in stage coordinates, i.e. meters
	Static(oxygine::Resources& gameResources, b2World* world, const RectF& rc);
};

