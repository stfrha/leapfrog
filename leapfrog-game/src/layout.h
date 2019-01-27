#pragma once



class Layout
{
private:
   float m_buttonWidth;

public:
	Layout();

   void initLayout(void);

   float getButtonWidth(void);
   float getXFromLeft(int column);
   float getXFromRight(int column);
   float getYFromTop(int row);
   float getYFromBottom(int row);

};

extern Layout g_Layout;