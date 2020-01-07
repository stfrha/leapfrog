#include "layout.h"

using namespace oxygine;

// Instansiate global class
Layout g_Layout;

Layout::Layout() :
   m_defaultFontSize(0.0f),
   m_buttonWidth(0.0f),
   m_boundaryWidth(150.0f)
{
}

void Layout::initLayout(void)
{
   float division = 8.0f;
   m_buttonWidth = (getStage()->getSize()).y / division;

   // Find the button width in inces and make sure it is not smaller 
   // than 1 cm = 0.4 inch
   float ddpi, hdpi, vdpi;

   if (SDL_GetDisplayDPI(0, &ddpi, &hdpi, &vdpi) == 0)
   {
      float inchButtonWidth = m_buttonWidth / hdpi;

      if (inchButtonWidth < 0.4f)
      {
         m_buttonWidth = hdpi * 0.4f;
      }

      float inchButtonHeight = m_buttonWidth / vdpi;

      if (inchButtonHeight < 0.4f)
      {
         m_buttonWidth = vdpi * 0.4f;
      }
   }

   m_defaultFontSize = g_Layout.getButtonWidth() / 2.0f / 2.0f / 8.0f * 6.0f;

   initStageSize(Vector2(1000.0f, 500.0f), 150.0f);
}

void Layout::initStageSize(oxygine::Vector2 stageSize, float boundaryWidth)
{
   m_stageSize = stageSize;
   m_stageBounds.setX(-boundaryWidth / 2.0f);
   m_stageBounds.setY(-boundaryWidth / 2.0f);
   m_stageBounds.setWidth(stageSize.x + boundaryWidth);
   m_stageBounds.setHeight(stageSize.y + boundaryWidth);
}


float Layout::getButtonWidth(void)
{
   return m_buttonWidth;
}

float Layout::getDefaultFontSize(void)
{
   return m_defaultFontSize;
}

float Layout::getXFromLeft(int column)
{
   return (float)column * m_buttonWidth;
}

float Layout::getXFromRight(int column)
{
   float stageWidth = (getStage()->getSize()).x;
   float x = stageWidth - ((float)column + 1.0f) * m_buttonWidth;
   return x;
}

float Layout::getYFromTop(int row)
{
   return (float)row * m_buttonWidth;
}

float Layout::getYFromBottom(int row)
{
   return (getStage()->getSize()).y - ((float)row + 1.0f) * m_buttonWidth;
}


Vector2 Layout::getViewPortBounds(void)
{
   return getStage()->getSize();
}

Vector2 Layout::getStageSize(void)
{
   return m_stageSize;
}


RectF Layout::getStageBounds(void)
{
   return m_stageBounds;
}

float Layout::getBoundaryWidth()
{
   return m_boundaryWidth;
}
