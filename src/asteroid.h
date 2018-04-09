#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "actortodie.h"

class SceneActor;
class FreeSpaceActor;

enum AsteroidStateEnum
{
   ASE_SMALL,     // Has Four edges, is about 2 meters in diameter
   ASE_MIDDLE,    // Has five edges, is about 5 meters in diameter
   ASE_LARGE      // Has five edges, is about 10 meters in diameter
};

class AsteroidSpawnInstruction
{
public:
   int m_num;
   AsteroidStateEnum m_state;
   b2Vec2 m_pos;

   AsteroidSpawnInstruction() :
      m_num(0),
      m_state(ASE_SMALL),
      m_pos(b2Vec2(0.0f, 0.0f))
   {
   }

   AsteroidSpawnInstruction(int num, AsteroidStateEnum state, b2Vec2 pos) :
      m_num(num),
      m_state(state),
      m_pos(pos)
   {
   }
};


DECLARE_SMART(Asteroid, spAsteroid);


// The Asteroid class is an actor that is used to channel
// updates through (if needed). Therefore it should be attached
// to the instansiating Actor. 
// The Polygon created by it will be an child of the Asteroid 
// Actor.

class Asteroid : public oxygine::Actor, CollisionEntity, ActorToDie
{
private:
   oxygine::Resources* m_gameResource;
   oxygine::spPolygon m_poly;
   AsteroidStateEnum m_state;
   int m_num;
   float m_radius;
   
   // Width or height of bitmap, wichever is smallest
   int m_bitmapPxSize;  
   
   // Max radius will be mapped to half of m_bitmapPxSize number of pixels
   float m_asteroideMaxRadius; 
   
   float m_bitmapScale;

   int   m_damage;

   SceneActor* m_sceneActor;
   FreeSpaceActor * m_freeSpaceActor;

public:
	Asteroid(
      oxygine::Resources& gameResources, 
      SceneActor* sceneActor,
      b2World* world,
      const b2Vec2& pos, 
      AsteroidStateEnum state,
      FreeSpaceActor* actor);

   virtual CollisionEntityTypeEnum getEntityType(void);

   virtual void killActor(void);

   bool hitByBullet(b2Contact* contact);     // Returns true if asteroid was shattered
   void hitByLepfrog(b2Contact* contact);

protected:
	void doUpdate(const oxygine::UpdateState& us);
   float generateVertex(oxygine::Vector2& v, int i, int num);
   oxygine::vertexPCT2 initVertex(const oxygine::Vector2& pos, unsigned int color);
   oxygine::vertexPCT2 getVertex(oxygine::Vector2* v2vertices, int i);
   oxygine::vertexPCT2* createVertices(oxygine::Vector2* v2vertices, int num);
   void generateVertices(oxygine::Vector2* vertices);
   void createPolygon(oxygine::Vector2* v2vertices, int num);
   float generateRadius(void);
   float generateBitmapScale(void);
   int generateNum(void);
   void atDeathOfAsteroid(void);

};
