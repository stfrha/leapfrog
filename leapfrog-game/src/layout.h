#pragma once

#include "oxygine-framework.h"



class Layout
{
private:
   oxygine::Vector2  m_viewPortBounds;
   float m_buttonWidth;
   float m_defaultFontSize;

public:
	Layout();

   void initLayout(void);

   float getButtonWidth(void);
   float getDefaultFontSize(void);
   float getXFromLeft(int column);
   float getXFromRight(int column);
   float getYFromTop(int row);
   float getYFromBottom(int row);
   oxygine::Vector2 getViewPortBounds(void);
};

extern Layout g_Layout;