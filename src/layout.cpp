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

