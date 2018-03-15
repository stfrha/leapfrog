#pragma once

// A bitmap of a object may have its own scale to physical coordinates (meters).
// All Box2D bodies are specified in meters
// Stage scale is also in meters, thus physToStageScale = 1.0
// Final viewport scale is depening on the zoomScale value (of the stage actor)
// but there need to be a reference (for zoomScale = 1.0). The reference scale
// is ten pixels per meter. Thus stageToViewPortScale = 10.0.


class Scales
{
public:
	static const float c_physToStageScale /* = 1.0f*/;
	static const float c_stageToViewPortScale /* = 10.0f*/;
};
