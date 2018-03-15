#pragma once
#include "oxygine-framework.h"
#include "Box2D/Box2D.h"

DECLARE_SMART(ClipedWindow, spClipedWindow);

class ClipedWindow : public oxygine::ClipRectActor
{
private:
	oxygine::spActor	m_actor;

public:
	ClipedWindow();

protected:

};
