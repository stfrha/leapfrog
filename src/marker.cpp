#include "Marker.h"
#include "physdispconvert.h"
#include "scales.h"

using namespace oxygine;

// rc is in stage coordinates, i.e. meters
Marker::Marker(Resources& gameResources, const RectF& rc)
{
	//setHorizontalMode(Box9Sprite::TILING_FULL);
	//setVerticalMode(Box9Sprite::TILING_FULL);
   
   //ResAnim* ra = gameResources.getResAnim("markers");

	setResAnim(gameResources.getResAnim("markers"));

	// size and position here is in stage coordinate (same as rc)
	setSize(rc.getSize());
	setPosition(rc.getLeftTop());
	setAnchor(Vector2(0.5f, 0.5f));
}
