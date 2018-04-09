
#include "oxygine-framework.h"
#include "Polygon.h"
#include "Box2D/Box2D.h"
#include "Box2DDebugDraw.h"

DECLARE_SMART(Marker, spMarker);

class Marker : public oxygine::Polygon
{
public:
	// rc is in stage coordinates, i.e. meters
	Marker(oxygine::Resources& gameResources, const RectF& rc, b2World* world);
};

