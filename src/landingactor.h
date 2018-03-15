#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "leapfrog.h"
#include "scales.h"
#include "physdispconvert.h"

DECLARE_SMART(LandingActor, spLandingActor);

class LandingActor : public oxygine::Actor
{
private:
	float	m_zoomScale;
	float	m_stageToViewPortScale;
	float	m_physToStageScale;
	b2World * m_world;
	spLeapFrog  m_leapfrog;
	spBox2DDraw m_debugDraw;

public:
	LandingActor(oxygine::Resources& gameResources);

protected:
	void doUpdate(const UpdateState& us);

};
