#pragma once

#include "oxygine-framework.h"

#include <string>

// Who a character or map item belongs to. Shown as the colour of message
// bubble frames and of the squares on the HDD map. The colours are set in
// Layout::getAffiliationColor.
//   none      not categorised: phosphor green, like the rest of the HUD
//   leapfrog  the pilot, the silo and Edith
//   neutral, enemy, friendly
// An affiliation can change during the story (the pirates go from enemy
// to friendly in Act 3), see c_setSenderAffiliation and
// c_setObjectMapAffiliation.
enum class Affiliation
{
   none,
   leapfrog,
   neutral,
   enemy,
   friendly
};

// "none", "leapfrog", "neutral", "enemy" or "friendly". Returns false
// (and leaves result unchanged) for anything else.
bool parseAffiliation(const std::string& name, Affiliation& result);

// This class define geometrics of the display
// The following members exists:
// View Port Bounds     : This is, per definition, starting at 0,0 to the 
//                        extent of the display viewport, this is in viewport 
//                        coordinate system. This is accessed via the getViewPortBounds method
// m_stageSize          : This is the map size in stage coordinates (1.0f is 1 meter)
//                        it starts at 0,0 and goes to the width and height stated
//                        in the sceneProperties.
// Safe area         : The HUD column getters (getXFromLeft/Right) start at the
//                        safe area edges, so buttons and frames stay clear of
//                        an iPhone camera notch. The game view still uses the
//                        whole screen.
// m_stageBounds        : This is the stage framed by soft boundaries. This start at
//                        (-boundaryWidth , -boundaryWidth) and extends to 
//                        (m_stageSize + boundaryWidth). It is the maximum extent 
//                        that can be zoomed out.
class Layout
{
private:
   oxygine::Vector2  m_stageSize;
   oxygine::RectF  m_stageBounds;
   float m_boundaryWidth;
   float m_buttonWidth;
   float m_defaultFontSize;
   float m_ddpi;
   float m_hdpi;
   float m_vdpi;

   // Screen edges covered by a camera notch etc. [stage pixels]. Only
   // left/right are used: the game is landscape only and the bottom
   // home indicator doesn't hide anything.
   float m_safeInsetLeft;
   float m_safeInsetRight;

   void initSafeArea(void);


public:
	Layout();

   void initLayout(int sizeCode = 0);
   void initStageSize(oxygine::Vector2 stageSize, float boundaryWidth);

   float getButtonWidth(void);
   float getDefaultFontSize(void);
   float getXFromLeft(int column);
   float getXFromRight(int column);
   float getYFromTop(int row);
   float getYFromBottom(int row);
   oxygine::Vector2 getViewPortBounds(void);
   oxygine::Vector2 getStageSize(void);
   oxygine::RectF getStageBounds(void);
   float getBoundaryWidth();
   oxygine::Color getPhosphorColor(void);
   oxygine::Color getAffiliationColor(Affiliation affiliation);
   float getHorizontalDpi(void);
   float getVerticalDpi(void);
   float getMaxDpi(void);
   float getSafeInsetLeft(void);
   float getSafeInsetRight(void);
};

extern Layout g_Layout;
