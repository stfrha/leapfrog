#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "compoundobject.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
//#include "actortodie.h"

class SceneActor;

enum AsteroidStateEnum
{
   ASE_SMALL,     // Has Four edges, is about 2 meters in diameter
   ASE_MIDDLE,    // Has five edges, is about 5 meters in diameter
   ASE_LARGE,     // Has five edges, is about 10 meters in diameter
   ASE_AUTO       // Size is selected randomly at creation
};

class AsteroidSpawnInstruction
{
public:
   int m_num;
   AsteroidStateEnum m_state;
   pugi::xml_node* m_childNode;
   b2Vec2 m_leftTop;
   b2Vec2 m_rightBottom;

   AsteroidSpawnInstruction() :
      m_num(0),
      m_state(ASE_SMALL),
      m_childNode(NULL),
      m_leftTop(b2Vec2(0.0f, 0.0f)),
      m_rightBottom(b2Vec2(0.0f, 0.0f))
   {
   }

   AsteroidSpawnInstruction(
      int num, 
      AsteroidStateEnum state, 
      pugi::xml_node* childNode, 
      b2Vec2 leftTop, 
      b2Vec2 rightBottom) :
      m_num(num),
      m_state(state),
      m_childNode(childNode),
      m_leftTop(leftTop),
      m_rightBottom(rightBottom)
   {
   }
};


DECLARE_SMART(Asteroid, spAsteroid);


// The Asteroid class is an actor that is used to channel
// updates through (if needed). Therefore it should be attached
// to the instansiating Actor. 
// The Polygon created by it will be an child of the Asteroid 
// Actor.

class Asteroid : public CompoundObject
{
private:
   oxygine::Resources* m_gameResource;
   b2World* m_world;
   oxygine::spPolygon m_poly;
   AsteroidStateEnum m_state;
   int m_num;
   float m_radius;

   pugi::xml_document* m_smallCoNodeHolder;
   pugi::xml_node m_smallCoNode;

   pugi::xml_document* m_mediumCoNodeHolder;
   pugi::xml_node m_mediumCoNode;

   pugi::xml_document* m_bigCoNodeHolder;
   pugi::xml_node m_bigCoNode;

   
   // Width or height of bitmap, wichever is smallest
   int m_bitmapPxSize;  
   
   // Max radius will be mapped to half of m_bitmapPxSize number of pixels
   float m_asteroideMaxRadius; 
   
   float m_bitmapScale;

   int   m_damage;

   SceneActor* m_sceneActor;

   std::vector<AsteroidSpawnInstruction>  m_asteroidSpawnList;

   void addAsteroidSpawnInstruction(const AsteroidSpawnInstruction& inst);
   void spawnAsteroids(void);

public:
	Asteroid(
      oxygine::Resources& gameResources, 
      SceneActor* sceneActor,
      b2World* world,
      const oxygine::Vector2& pos, 
      AsteroidStateEnum state);

   virtual CollisionEntityTypeEnum getEntityType(void);

   float getRadius();

   void hitByBullet(b2Contact* contact); 
   void hitShield(b2Contact* contact);     // Returns true if asteroid was shattered
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
