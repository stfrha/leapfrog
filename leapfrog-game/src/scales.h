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

   // Length of one scene update in seconds (1/60 s). Each scene's clock runs
   // at this fixed step, so the scene and everything in it (Box2D, thrusters,
   // emitters, guns, Lua scene timers) is updated exactly 60 times per second
   // of real time, whatever the display's frame rate: 0, 1 or more updates
   // per frame. Box2D steps this much per update.
   static const float c_updateStep /* = 1.0f / 60.0f*/;
};
