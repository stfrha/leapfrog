#include "layout.h"

#if defined(__APPLE__)
#include <TargetConditionals.h>
#endif

#if TARGET_OS_IPHONE
#include <objc/runtime.h>
#include <objc/message.h>
#include "SDL_syswm.h"
#endif

using namespace oxygine;

#if TARGET_OS_IPHONE
namespace
{
   // UIEdgeInsets (CGFloat is double on all 64-bit iOS devices)
   struct EdgeInsets
   {
      double top;
      double left;
      double bottom;
      double right;
   };

   // Reads [UIWindow safeAreaInsets] through the Objective-C runtime,
   // so this file can stay C++ and no build project needs changing.
   bool getIosSafeAreaInsets(SDL_Window* window, EdgeInsets& insets)
   {
      SDL_SysWMinfo info;
      SDL_VERSION(&info.version);
      if (!SDL_GetWindowWMInfo(window, &info) || (info.info.uikit.window == NULL))
      {
         return false;
      }

      id uiWindow = (id)(void*)info.info.uikit.window;

      // Make sure the window has been laid out, otherwise the insets
      // can still be zero this early in the start-up
      ((void (*)(id, SEL))objc_msgSend)(uiWindow, sel_registerName("layoutIfNeeded"));

      SEL sel = sel_registerName("safeAreaInsets");
#if defined(__arm64__)
      insets = ((EdgeInsets (*)(id, SEL))objc_msgSend)(uiWindow, sel);
#else
      // x86_64 simulator returns large structs in memory
      insets = ((EdgeInsets (*)(id, SEL))objc_msgSend_stret)(uiWindow, sel);
#endif
      return true;
   }
}
#endif

// Instansiate global class
Layout g_Layout;

Layout::Layout() :
   m_defaultFontSize(0.0f),
   m_buttonWidth(0.0f),
   m_boundaryWidth(150.0f),
   m_safeInsetLeft(0.0f),
   m_safeInsetRight(0.0f)
{
}

void Layout::initLayout(int sizeCode)
{
   if (sizeCode == 0)
   {
      float division = 8.0f;
      m_buttonWidth = (getStage()->getSize()).y / division;

      // Find the button width in inces and make sure it is not smaller 
      // than 1 cm = 0.4 inch
      if (SDL_GetDisplayDPI(0, &m_ddpi, &m_hdpi, &m_vdpi) == 0)
      {
         float inchButtonWidth = m_buttonWidth / m_hdpi;

         if (inchButtonWidth < 0.4f)
         {
            m_buttonWidth = m_hdpi * 0.4f;
         }

         float inchButtonHeight = m_buttonWidth / m_vdpi;

         if (inchButtonHeight < 0.4f)
         {
            m_buttonWidth = m_vdpi * 0.4f;
         }
      }
   }
   else if (sizeCode == 1)
   {
      m_buttonWidth = 177.0f;  // Same as on android
   }
   else if (sizeCode == 2)
   {
      m_buttonWidth = 88.5f;  // Same as on android
   }


   m_defaultFontSize = g_Layout.getButtonWidth() / 2.0f / 2.0f / 8.0f * 6.0f;

   initSafeArea();

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


void Layout::initSafeArea(void)
{
   m_safeInsetLeft = 0.0f;
   m_safeInsetRight = 0.0f;

#if TARGET_OS_IPHONE
   SDL_Window* window = core::getWindow();
   EdgeInsets insets;

   if ((window != NULL) && getIosSafeAreaInsets(window, insets))
   {
      // Insets are in points, the stage is in pixels
      int pointsW = 0;
      int pointsH = 0;
      SDL_GetWindowSize(window, &pointsW, &pointsH);

      float pixelsPerPoint = 1.0f;
      if (pointsW > 0)
      {
         pixelsPerPoint = getStage()->getSize().x / (float)pointsW;
      }

      // The notch moves to the other side when the phone is turned
      // around, and iOS reports the same inset on both sides in
      // landscape, so the HUD is kept clear of both edges.
      float inset = (float)(insets.left > insets.right ? insets.left : insets.right) * pixelsPerPoint;
      m_safeInsetLeft = inset;
      m_safeInsetRight = inset;

      logs::messageln("Safe area insets (points): top %.0f, left %.0f, bottom %.0f, right %.0f; HUD inset %.0f px",
         insets.top, insets.left, insets.bottom, insets.right, inset);
   }
#endif
}

float Layout::getSafeInsetLeft(void)
{
   return m_safeInsetLeft;
}

float Layout::getSafeInsetRight(void)
{
   return m_safeInsetRight;
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
   return m_safeInsetLeft + (float)column * m_buttonWidth;
}

float Layout::getXFromRight(int column)
{
   float stageWidth = (getStage()->getSize()).x;
   float x = stageWidth - m_safeInsetRight - ((float)column + 1.0f) * m_buttonWidth;
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

Color Layout::getPhosphorColor(void)
{
   return oxygine::Color(0x55d400ff);
}

float Layout::getHorizontalDpi(void)
{
   return m_hdpi;
}

float Layout::getVerticalDpi(void)
{
   return m_vdpi;
}

float Layout::getMaxDpi(void)
{
   return fmax(m_hdpi, m_vdpi);
}
