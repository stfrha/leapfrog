
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "Box2DDebugDraw.h"

DECLARE_SMART(SoftBoundary, spSoftBoundary);

// The SoftBoundary, though imposing forces on other bodies are not
// bodies themselfs. 

class SoftBoundary : public oxygine::Sprite
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
      RepelDirectionEnum repelDir);

   void testForRepel(b2Body* body);
   bool isInside(b2Body* body);
   RepelDirectionEnum getDirection(void);

};


