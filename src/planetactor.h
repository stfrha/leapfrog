#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "sceneactor.h"

DECLARE_SMART(PlanetActor, spPlanetActor);

class PlanetActor : public SceneActor
{
private:
   oxygine::spBox9Sprite   m_planetSprite;
   oxygine::spSprite   m_mask;
   oxygine::spMaskedSprite   m_maskedSprite;

public:
	PlanetActor(oxygine::Resources& gameResources);

};
