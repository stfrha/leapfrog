#pragma once

#include "oxygine-framework.h"

// This class define geometrics of the display
// The following members exists:
// View Port Bounds     : This is, per definition, starting at 0,0 to the 
//                        extent of the display viewport, this is in viewport 
//                        coordinate system. This is accessed via the getViewPortBounds method
// m_stageSize          : This is the map size in stage coordinates (1.0f is 1 meter)
//                        it starts at 0,0 and goes to the width and height stated
//                        in the sceneProperties.
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


public:
	Layout();

   void initLayout(void);
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
   float getHorizontalDpi(void);
   float getVerticalDpi(void);
   float getMaxDpi(void);
};

extern Layout g_Layout;
