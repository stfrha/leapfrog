
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "Box2DDebugDraw.h"

DECLARE_SMART(SoftBoundary, spSoftBoundary);

// The SoftBoundary, though imposing forces on other bodies are not
// bodies themselfs. 

class SoftBoundary : public oxygine::Box9Sprite
{
public:
   enum RepelDirectionEnum
   {
      up,
      down,
      right,
      left
   };

private:
   RepelDirectionEnum m_repelDir;
   
   // The x or y value that is the threshold for when force
   // will be applied to the body. For repel direction up this
   // is SoftBoundary.position.y - height / 2
   float m_threshold;

   float determineThreshold(const RectF& rc);

public:
	// rc is in stage coordinates, i.e. meters
	SoftBoundary(
      oxygine::Resources& gameResources, 
      b2World* world, 
      const RectF& rc, 
      RepelDirectionEnum repelDir);

   void testForRepel(b2Body* body);
   bool isInside(b2Body* body);
};


