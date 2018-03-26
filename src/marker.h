
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "Box2DDebugDraw.h"

DECLARE_SMART(Marker, spMarker);

class Marker : public oxygine::Box9Sprite
{
public:
	// rc is in stage coordinates, i.e. meters
	Marker(oxygine::Resources& gameResources, const RectF& rc);
};

