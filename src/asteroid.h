#pragma once

#include "oxygine-framework.h"
#include "Polygon.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"


enum AsteroidStateEnum
{
   ASE_SMALL,     // Has Five edges, is about 10 meters in diameter
   ASE_MIDDLE,    // Has six edges, is about 20 meters in diameter
   ASE_LARGE      // Has six edges, is about 50 meters in diameter
};

DECLARE_SMART(Asteroid, spAsteroid);


// The Asteroid class is an actor that is used to channel
// updates through (if needed). Therefore it should be attached
// to the instansiating Actor. 
// The Polygon created by it will be an child of the Asteroid 
// Actor.

class Asteroid : public oxygine::Actor
{
private:
   oxygine::spPolygon m_poly;
   AsteroidStateEnum m_state;
   int m_num;
   float m_radius;
   
   // Width or height of bitmap, wichever is smallest
   int m_bitmapPxSize;  
   
   // Max radius will be mapped to half of m_bitmapPxSize number of pixels
   float m_asteroideMaxRadius; 
   
   float m_bitmapScale;

public:
	Asteroid(
      oxygine::Resources& gameResources, 
      b2World* world, 
      const b2Vec2& pos, 
      AsteroidStateEnum state);

protected:
	void doUpdate(const oxygine::UpdateState& us);
   void handleCollision(oxygine::Event* event);
   void destroyAsteroid(void);
   float generateVertex(oxygine::Vector2& v, int i, int num);
   oxygine::vertexPCT2 initVertex(const oxygine::Vector2& pos, unsigned int color);
   oxygine::vertexPCT2 getVertex(int i, int num);
   oxygine::vertexPCT2* createVertices(int num);
   void createPolygon(void);
   float generateRadius(void);
   float generateBitmapScale(void);
   int generateNum(void);

};
