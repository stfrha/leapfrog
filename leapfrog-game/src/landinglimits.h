#pragma once

// Safe landing speeds, shared by the HUD's speed bars (tick marks) and the
// landing pads (which only hold a Leapfrog that touched down within them).
// [m/s]; the game's y axis points down, so a descent is a positive y speed.
namespace LandingLimits
{
   const float c_maxHorizontalSpeed = 2.0f;
   const float c_maxDescentSpeed = 4.0f;
}
