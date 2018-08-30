#include "layout.h"
#include "oxygine-framework.h"

using namespace oxygine;

// Instansiate global class
Layout g_Layout;

Layout::Layout()
{
   m_buttonWidth = 0.0f;
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
}

float Layout::getButtonWidth(void)
{
   return m_buttonWidth;
}

float Layout::getXFromLeft(int column)
{
   return (float)column * m_buttonWidth;
}

float Layout::getXFromRight(int column)
{
   return (getStage()->getSize()).x - ((float) column + 1.0f) * m_buttonWidth;
}

float Layout::getYFromTop(int row)
{
   return (float)row * m_buttonWidth;
}

float Layout::getYFromBottom(int row)
{
   return (getStage()->getSize()).y - ((float)row + 1.0f) * m_buttonWidth;
}

