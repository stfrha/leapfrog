#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"



DECLARE_SMART(StaticPolygon, spStaticPolygon);

// The StaticPolygon class is an actor that is used to channel
// updates through (if needed). Therefore it should be attached
// to the instansiating Actor. 
// The Polygon created by it will be an child of the StaticPolygon 
// Actor.

class StaticPolygon : public oxygine::Actor
{
private:
   oxygine::Resources* m_gameResource;
   oxygine::spPolygon m_poly;
   std::string m_textureName;

  
   // Width or height of bitmap, wichever is smallest
   int m_bitmapPxSize;  
   float m_bitmapScale;


   void loadPolygonFile(oxygine::Resources& gameResources, oxygine::Actor* parent, b2World* world, const oxygine::Vector2& pos, std::string fileName);

   void createPolygon(void);

public:
	StaticPolygon(
      oxygine::Resources& gameResources, 
      std::string fileName,
      b2World* world);

protected:

};
