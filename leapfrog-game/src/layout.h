#pragma once



class Layout
{
private:
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

};

extern Layout g_Layout;