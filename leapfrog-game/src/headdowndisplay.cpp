#include "headdowndisplay.h"

#include "mainactor.h"
#include "actoruserdata.h"
#include "bodyuserdata.h"
#include "layout.h"
#include "graphicresources.h"

#include <cmath>
#include <cstdio>

using namespace std;
using namespace oxygine;


// Instanciate global instance of the HDD
spHeadDownDisplay g_HeadDownDisplay;

MapItem::MapItem(
   MapItemTypeEnum type,
   MapItemStateEnum state,
   int id,
   oxygine::spActor actor) :
   m_itemId(id),
   m_type(type),
   m_state(state),
   m_parentMap(NULL),
   m_realActor(actor)
{
   int a = 10;
}


MapItem::MapItem(
   HeadDownDisplay* hddMapActor,
   MapItemTypeEnum type,
   MapItemStateEnum state,
   int id,
   oxygine::spActor actor,
   float scale) :
   m_itemId(id),
   m_type(type),
   m_state(state),
   m_parentMap(hddMapActor),
   m_realActor(actor)
{
   switch (m_type)
   {
   case MapItemTypeEnum::me:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_stationary");
      break;
   case MapItemTypeEnum::enemyMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("enemy_moving");
      break;
   case MapItemTypeEnum::enemyStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("enemy_stationary");
      break;
   case MapItemTypeEnum::neutralMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("neutral_moving");
      break;
   case MapItemTypeEnum::neutralStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("neutral_stationary");
      break;
   }

   setState(m_state);

   setPriority(231);
   setTouchChildrenEnabled(false);

   setPosition(m_realActor->getPosition());
   setSize(10.0f, 10.0f);
   setAnchor(0.5f, 0.5f);
   setAlpha(128);
   setScale(scale);
   attachTo(hddMapActor);
}

void MapItem::addToMapActor(
   HeadDownDisplay* hddMapActor,
   float scale)
{
   m_parentMap = hddMapActor;

   switch (m_type)
   {
   case MapItemTypeEnum::me:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_moving");
      break;
   case MapItemTypeEnum::friendlyStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("friendly_stationary");
      break;
   case MapItemTypeEnum::enemyMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("enemy_moving");
      break;
   case MapItemTypeEnum::enemyStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("enemy_stationary");
      break;
   case MapItemTypeEnum::neutralMoving:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("neutral_moving");
      break;
   case MapItemTypeEnum::neutralStationary:
      m_resAnim = g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("neutral_stationary");
      break;
   }

   setState(m_state);

   setPriority(231);
   setTouchChildrenEnabled(false);

   setPosition(m_realActor->getPosition());
   setSize(10.0f, 10.0f);
   setAnchor(0.5f, 0.5f);
   setAlpha(128);
   setScale(scale);
   attachTo(hddMapActor);
}


MapItem::MapItemTypeEnum MapItem::getType(void)
{
   return m_type;
}

oxygine::spActor MapItem::getActor(void)
{
   return m_realActor;
}

int MapItem::getId(void)
{
   return m_itemId;
}

void MapItem::setState(MapItem::MapItemStateEnum state)
{
   switch (state)
   {
   case hollow:
      removeTweens();
      setResAnim(m_resAnim, 0, 0);
      break;
   case filled:
      removeTweens();
      setResAnim(m_resAnim, 1, 0);
      break;
   case flashingSlow:
      setResAnim(m_resAnim, 0, 0);
      removeTweens();
      addTween(TweenAnim(m_resAnim), 1000, -1);
      break;
   case flashingFast:
      setResAnim(m_resAnim, 0, 0);
      removeTweens();
      addTween(TweenAnim(m_resAnim), 250, -1);
      break;
   }
}


void MapItem::doUpdate(const oxygine::UpdateState& us)
{
   Vector2 pos = m_realActor->getPosition();

   if (pos.x < 0.0f)
   {
      pos.x = 0.0f;
   }

   if (pos.y < 0.0f)
   {
      pos.y = 0.0f;
   }

   if (pos.x > m_parentMap->m_sceneWidth)
   {
      pos.x = m_parentMap->m_sceneWidth;
   }

   if (pos.y > m_parentMap->m_sceneHeight)
   {
      pos.y = m_parentMap->m_sceneHeight;
   }

   setPosition(pos);
}


// ---------------------------------------------------------------------------
// HDD instrument tuning
// ---------------------------------------------------------------------------

namespace
{
   // Speed bars
   const float c_speedBarFullScale = 40.0f;           // [m/s] at the end of a bar
   const float c_safeHorizontalLandingSpeed = 2.0f;   // [m/s] tick marks, no game rule yet
   const float c_safeVerticalLandingSpeed = 4.0f;     // [m/s] descent, tick mark below centre
   const int c_telemetryTimeoutMs = 300;              // bars show "--" without telemetry
   const unsigned char c_barFrameAlpha = 200;
   const unsigned char c_barFillAlpha = 170;
   const unsigned char c_barTickAlpha = 140;

   // Event log
   const int c_logMaxLines = 6;
   const float c_logLifetime = 6.0f;                  // [s] normal lines
   const float c_logCriticalLifetime = 9.0f;          // [s] critical lines
   const float c_logFadeOutTime = 1.2f;               // [s] at the end of the lifetime
   const float c_logDriftLinesPerSecond = 0.33f;      // own upward drift
   const float c_logMergeWindow = 2.5f;               // [s] for summing amounts
   const unsigned char c_logNormalAlpha = 150;
   const unsigned char c_logCriticalAlpha = 235;

   // Warning thresholds (values are 0..100 like the status bars)
   const float c_ammoLow = 15.0f;
   const float c_ammoCritical = 5.0f;
   const float c_shieldLow = 30.0f;
   const float c_shieldCritical = 10.0f;
   const float c_fuelLow = 20.0f;
   const float c_fuelCritical = 10.0f;
   const float c_damageHigh = 50.0f;
   const float c_damageCritical = 75.0f;
   const float c_damageFailure = 100.0f;
   const float c_warningHysteresis = 3.0f;

   const ResFont* hudFont(void)
   {
      return g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResFont("lf_font");
   }

   spTextField makeHudText(int fontSize, const Vector2& size, bool alignRight)
   {
      spTextField t = new TextField();
      TextStyle style = TextStyle(hudFont()).withColor(Color::White).alignTop();
      style = alignRight ? style.alignRight() : style.alignLeft();
      style.fontSize = fontSize;
      t->setStyle(style);
      t->setColor(g_Layout.getPhosphorColor());
      t->setAnchor(0.0f, 0.0f);
      t->setSize(size);
      t->setTouchEnabled(false);
      return t;
   }

   // Level for a resource that is bad when low: 0 = ok, 1 = low,
   // 2 = critical, 3 = empty. Getting better needs a small margin
   // so a value hovering at a threshold doesn't spam the log.
   int lowResourceLevel(float value, int current, float low, float critical)
   {
      int level = 0;
      if (value <= 0.0f) level = 3;
      else if (value < critical) level = 2;
      else if (value < low) level = 1;

      if (level < current)
      {
         float threshold = (current == 3) ? 0.0f : (current == 2) ? critical : low;
         float margin = (current == 3) ? 0.5f : c_warningHysteresis;
         if (value < threshold + margin)
         {
            level = current;
         }
      }

      return level;
   }

   // Same for damage, which is bad when high
   int damageLevel(float value, int current)
   {
      int level = 0;
      if (value >= c_damageFailure) level = 3;
      else if (value >= c_damageCritical) level = 2;
      else if (value >= c_damageHigh) level = 1;

      if (level < current)
      {
         float threshold = (current == 3) ? c_damageFailure : (current == 2) ? c_damageCritical : c_damageHigh;
         if (value > threshold - c_warningHysteresis)
         {
            level = current;
         }
      }

      return level;
   }
}


// ---------------------------------------------------------------------------
// SpeedBar
// ---------------------------------------------------------------------------

SpeedBar::SpeedBar(
   bool vertical,
   const Vector2& size,
   float fullScale,
   float safeLimit,
   bool safeBothSides,
   const string& label,
   int fontSize,
   const Vector2& readoutPos,
   bool readoutAlignRight) :
   m_vertical(vertical),
   m_barSize(size),
   m_fullScale(fullScale),
   m_displayed(0.0f),
   m_readoutValue(1.0e9f),
   m_readoutTimer(0),
   m_label(label)
{
   setSize(size);
   setTouchEnabled(false, false);

   spBox9Sprite frame = new Box9Sprite();
   frame->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_thin"));
   frame->setVerticalMode(Box9Sprite::STRETCHING);
   frame->setHorizontalMode(Box9Sprite::STRETCHING);
   frame->setAnchor(0.0f, 0.0f);
   frame->setPosition(0.0f, 0.0f);
   frame->setSize(size);
   frame->setAlpha(c_barFrameAlpha);
   frame->setTouchEnabled(false);
   frame->attachTo(this);

   m_fill = new ColorRectSprite();
   m_fill->setColor(g_Layout.getPhosphorColor());
   m_fill->setAlpha(c_barFillAlpha);
   m_fill->setAnchor(0.0f, 0.0f);
   m_fill->setTouchEnabled(false);
   m_fill->attachTo(this);

   // Zero mark, then the safe landing limits
   addTick(0.0f, 255);
   addTick(-safeLimit, c_barTickAlpha);
   if (safeBothSides)
   {
      addTick(safeLimit, c_barTickAlpha);
   }

   float readoutWidth = fontSize * 8.0f;
   Vector2 pos = readoutPos;
   if (readoutAlignRight)
   {
      pos.x -= readoutWidth;
   }

   m_readout = makeHudText(fontSize, Vector2(readoutWidth, fontSize * 1.4f), readoutAlignRight);
   m_readout->setPosition(pos);
   m_readout->setAlpha(c_barFrameAlpha);
   m_readout->attachTo(this);

   layoutFill();
   setNoSignal();
}

float SpeedBar::toFraction(float speed) const
{
   float s = fabs(speed) / m_fullScale;
   if (s > 1.0f)
   {
      s = 1.0f;
   }
   float f = sqrt(s);
   return speed < 0.0f ? -f : f;
}

void SpeedBar::addTick(float speed, unsigned char alpha)
{
   const float inset = 3.0f;
   spColorRectSprite tick = new ColorRectSprite();
   tick->setColor(g_Layout.getPhosphorColor());
   tick->setAlpha(alpha);
   tick->setTouchEnabled(false);

   float width = (speed == 0.0f) ? 2.0f : 1.0f;

   if (m_vertical)
   {
      float half = (m_barSize.y - 2.0f * inset) / 2.0f;
      float y = m_barSize.y / 2.0f - toFraction(speed) * half;
      tick->setAnchor(0.0f, 0.5f);
      tick->setSize(m_barSize.x, width);
      tick->setPosition(0.0f, y);
   }
   else
   {
      float half = (m_barSize.x - 2.0f * inset) / 2.0f;
      float x = m_barSize.x / 2.0f + toFraction(speed) * half;
      tick->setAnchor(0.5f, 0.0f);
      tick->setSize(width, m_barSize.y);
      tick->setPosition(x, 0.0f);
   }

   tick->attachTo(this);
}

void SpeedBar::layoutFill(void)
{
   const float inset = 3.0f;
   float f = toFraction(m_displayed);

   if (m_vertical)
   {
      float half = (m_barSize.y - 2.0f * inset) / 2.0f;
      float len = f * half;   // positive is up
      float cy = m_barSize.y / 2.0f;
      m_fill->setPosition(inset, len >= 0.0f ? cy - len : cy);
      m_fill->setSize(m_barSize.x - 2.0f * inset, fabs(len));
   }
   else
   {
      float half = (m_barSize.x - 2.0f * inset) / 2.0f;
      float len = f * half;   // positive is right
      float cx = m_barSize.x / 2.0f;
      m_fill->setPosition(len >= 0.0f ? cx : cx + len, inset);
      m_fill->setSize(fabs(len), m_barSize.y - 2.0f * inset);
   }
}

void SpeedBar::setReadout(float speed)
{
   if (fabs(speed) < 0.05f)
   {
      speed = 0.0f;
   }

   if (fabs(speed - m_readoutValue) < 0.05f)
   {
      return;
   }

   m_readoutValue = speed;

   char text[32];
   snprintf(text, sizeof(text), "%s %+.1f", m_label.c_str(), speed);
   m_readout->setText(text);
}

void SpeedBar::setSpeed(float speed, int dtMs)
{
   float k = dtMs / 1000.0f * 12.0f;
   if (k > 1.0f)
   {
      k = 1.0f;
   }

   m_displayed += (speed - m_displayed) * k;
   layoutFill();

   // Text updates at 10 Hz is plenty and keeps the digits readable
   m_readoutTimer += dtMs;
   if (m_readoutTimer >= 100)
   {
      m_readoutTimer = 0;
      setReadout(speed);
   }
}

void SpeedBar::setNoSignal(void)
{
   m_displayed = 0.0f;
   layoutFill();
   m_readoutValue = 1.0e9f;
   m_readout->setText(m_label + " --");
}


// ---------------------------------------------------------------------------
// HudLog
// ---------------------------------------------------------------------------

HudLog::HudLog(const Vector2& size, int fontSize) :
   m_fontSize(fontSize)
{
   m_lineHeight = fontSize * 1.35f;
   m_padding = fontSize * 0.5f;

   setSize(size);
   setTouchEnabled(false, false);
}

void HudLog::setLineText(Line& line, const string& text)
{
   line.m_text->setText(text);
   line.m_shadow->setText(text);
}

float HudLog::targetY(int index, const Line& line) const
{
   // Line 0 (newest) sits at the bottom. Each line also drifts up by
   // its own age. Older lines always have a larger age, so they stay
   // at least one line height above the newer ones.
   return getHeight() - m_padding - (index + 1) * m_lineHeight
      - line.m_age * c_logDriftLinesPerSecond * m_lineHeight;
}

void HudLog::pushLine(const string& text, Priority prio, const string& key, const string& format, int amount)
{
   Line line;
   line.m_key = key;
   line.m_format = format;
   line.m_amount = amount;
   line.m_age = 0.0f;
   line.m_lifetime = (prio == critical) ? c_logCriticalLifetime : c_logLifetime;
   line.m_priority = prio;

   Vector2 textSize(getWidth() - 2.0f * m_padding, m_lineHeight);

   line.m_actor = new Actor();
   line.m_actor->setSize(textSize);
   line.m_actor->setTouchEnabled(false, false);
   line.m_actor->setAlpha(0);

   // A dark copy one pixel down-right keeps the text readable over
   // bright map items without needing an outlined font
   line.m_shadow = makeHudText(m_fontSize, textSize, false);
   line.m_shadow->setColor(Color(0, 0, 0, 255));
   line.m_shadow->setAlpha(160);
   line.m_shadow->setPosition(1.0f, 1.0f);
   line.m_shadow->attachTo(line.m_actor);

   line.m_text = makeHudText(m_fontSize, textSize, false);
   line.m_text->setPosition(0.0f, 0.0f);
   line.m_text->attachTo(line.m_actor);

   setLineText(line, text);

   // Slide in from just below the bottom line
   line.m_y = targetY(0, line) + m_lineHeight * 0.6f;
   line.m_actor->setPosition(m_padding, line.m_y);
   line.m_actor->attachTo(this);

   m_lines.insert(m_lines.begin(), line);

   // Too many lines: let the oldest ones fade out quickly
   for (size_t i = c_logMaxLines; i < m_lines.size(); i++)
   {
      Line& old = m_lines[i];
      float quickEnd = old.m_age + 0.4f;
      if (quickEnd < old.m_lifetime)
      {
         old.m_lifetime = quickEnd;
      }
   }
}

void HudLog::addMessage(const string& text, Priority prio)
{
   // Same message again while it is the newest line: keep it alive
   // instead of stacking copies
   if (!m_lines.empty())
   {
      Line& newest = m_lines.front();
      if (newest.m_key.empty() && (newest.m_format == text) && (newest.m_age < c_logMergeWindow))
      {
         float lifetime = (prio == critical) ? c_logCriticalLifetime : c_logLifetime;
         newest.m_lifetime = newest.m_age + lifetime;
         return;
      }
   }

   // Plain lines keep their text in m_format so repeats can be detected
   pushLine(text, prio, "", text, 0);
}

void HudLog::addAmount(const string& key, const string& format, int amount, Priority prio)
{
   char text[128];

   if (!m_lines.empty())
   {
      Line& newest = m_lines.front();
      if ((newest.m_key == key) && (newest.m_age < c_logMergeWindow))
      {
         newest.m_amount += amount;
         snprintf(text, sizeof(text), newest.m_format.c_str(), newest.m_amount);
         setLineText(newest, text);
         newest.m_lifetime = newest.m_age + ((prio == critical) ? c_logCriticalLifetime : c_logLifetime);
         return;
      }
   }

   snprintf(text, sizeof(text), format.c_str(), amount);
   pushLine(text, prio, key, format, amount);
}

void HudLog::doUpdate(const UpdateState& us)
{
   float dt = us.dt / 1000.0f;
   if (dt > 0.1f)
   {
      dt = 0.1f;
   }

   float k = dt * 10.0f;
   if (k > 1.0f)
   {
      k = 1.0f;
   }

   for (size_t i = 0; i < m_lines.size(); i++)
   {
      Line& line = m_lines[i];
      line.m_age += dt;

      float target = targetY((int)i, line);
      line.m_y += (target - line.m_y) * k;
      line.m_actor->setPosition(m_padding, line.m_y);

      float base = (line.m_priority == critical) ? c_logCriticalAlpha : c_logNormalAlpha;
      float fadeIn = line.m_age / 0.15f;
      if (fadeIn > 1.0f) fadeIn = 1.0f;
      float fadeOut = (line.m_lifetime - line.m_age) / c_logFadeOutTime;
      if (fadeOut > 1.0f) fadeOut = 1.0f;
      if (fadeOut < 0.0f) fadeOut = 0.0f;

      // Critical lines blink a few times when they arrive
      float blink = 1.0f;
      if ((line.m_priority == critical) && (line.m_age < 1.0f))
      {
         blink = (((int)(line.m_age / 0.125f)) % 2) ? 0.3f : 1.0f;
      }

      line.m_actor->setAlpha((unsigned char)(base * fadeIn * fadeOut * blink));
   }

   // Remove expired lines, oldest are at the end
   for (int i = (int)m_lines.size() - 1; i >= 0; i--)
   {
      if (m_lines[i].m_age >= m_lines[i].m_lifetime)
      {
         m_lines[i].m_actor->detach();
         m_lines.erase(m_lines.begin() + i);
      }
   }
}


// ---------------------------------------------------------------------------
// HddInstruments
// ---------------------------------------------------------------------------

HddInstruments::HddInstruments(const RectF& mapRect, float barThickness, float gap) :
   m_telemetryFresh(false),
   m_haveBaseline(false),
   m_msSinceTelemetry(c_telemetryTimeoutMs),
   m_ammoLevel(0),
   m_shieldLevel(0),
   m_fuelLevel(0),
   m_damageLevel(0)
{
   setPosition(0.0f, 0.0f);
   setSize(g_Layout.getViewPortBounds());
   setTouchEnabled(false, false);

   int fontSize = (int)(g_Layout.getDefaultFontSize() * 0.8f);
   if (fontSize < 8)
   {
      fontSize = 8;
   }

   float readoutHeight = fontSize * 1.4f;

   // Horizontal speed: along the top edge of the map, readout above its left end
   m_horizontalBar = new SpeedBar(
      false,
      Vector2(mapRect.getWidth(), barThickness),
      c_speedBarFullScale,
      c_safeHorizontalLandingSpeed,
      true,
      "H",
      fontSize,
      Vector2(0.0f, -readoutHeight),
      false);
   m_horizontalBar->setPosition(mapRect.getLeft(), mapRect.getTop() - gap - barThickness);
   m_horizontalBar->attachTo(this);

   // Vertical speed: along the right edge of the map, readout to the
   // right of the bar's top end (above it, it read as belonging to the
   // horizontal bar). Up is positive.
   m_verticalBar = new SpeedBar(
      true,
      Vector2(barThickness, mapRect.getHeight()),
      c_speedBarFullScale,
      c_safeVerticalLandingSpeed,
      false,
      "V",
      fontSize,
      Vector2(barThickness + gap, 0.0f),
      false);
   m_verticalBar->setPosition(mapRect.getRight() + gap, mapRect.getTop());
   m_verticalBar->attachTo(this);

   // Event log on top of the map
   m_log = new HudLog(mapRect.size, fontSize);
   m_log->setPosition(mapRect.pos);
   m_log->attachTo(this);
}

HudLog* HddInstruments::getLog(void)
{
   return m_log.get();
}

void HddInstruments::setTelemetry(const LeapfrogTelemetry& telemetry)
{
   m_telemetry = telemetry;
   m_telemetryFresh = true;
   m_msSinceTelemetry = 0;

   evaluateWarnings();
}

void HddInstruments::evaluateWarnings(void)
{
   // The first telemetry of a scene sets the levels. Anything already
   // low is reported then too, which works as a reminder after a jump.

   int ammo = lowResourceLevel(m_telemetry.m_ammo, m_ammoLevel, c_ammoLow, c_ammoCritical);
   if (ammo > m_ammoLevel)
   {
      const char* msg[] = { "", "Ammo low", "Ammo critical", "Out of ammo" };
      m_log->addMessage(msg[ammo], ammo >= 2 ? HudLog::critical : HudLog::normal);
   }
   m_ammoLevel = ammo;

   int fuel = lowResourceLevel(m_telemetry.m_fuel, m_fuelLevel, c_fuelLow, c_fuelCritical);
   if (fuel > m_fuelLevel)
   {
      const char* msg[] = { "", "Fuel low", "Fuel critical", "Out of fuel" };
      m_log->addMessage(msg[fuel], fuel >= 2 ? HudLog::critical : HudLog::normal);
   }
   m_fuelLevel = fuel;

   // The shield isn't used in landing mode, so it is not reported there
   if (m_telemetry.m_shieldInUse)
   {
      int shield = lowResourceLevel(m_telemetry.m_shield, m_shieldLevel, c_shieldLow, c_shieldCritical);
      if (shield > m_shieldLevel)
      {
         const char* msg[] = { "", "Shield low", "Shield critical", "Shield down" };
         m_log->addMessage(msg[shield], shield >= 2 ? HudLog::critical : HudLog::normal);
      }
      else if ((m_shieldLevel == 3) && (shield < 3) && m_haveBaseline)
      {
         m_log->addMessage("Shield online");
      }
      m_shieldLevel = shield;
   }

   int damage = damageLevel(m_telemetry.m_damage, m_damageLevel);
   if (damage > m_damageLevel)
   {
      const char* msg[] = { "", "Hull damaged", "Hull critical", "Hull failure" };
      m_log->addMessage(msg[damage], damage >= 2 ? HudLog::critical : HudLog::normal);
   }
   m_damageLevel = damage;

   m_haveBaseline = true;
}

void HddInstruments::doUpdate(const UpdateState& us)
{
   // Only show the instruments together with the map, not in the
   // orbit overlay, the menu or while the HDD is resizing
   bool show = g_HeadDownDisplay && g_HeadDownDisplay->isShowingMap();
   m_horizontalBar->setVisible(show);
   m_verticalBar->setVisible(show);
   m_log->setVisible(show);

   m_msSinceTelemetry += us.dt;

   if (m_telemetryFresh && (m_msSinceTelemetry < c_telemetryTimeoutMs))
   {
      // Physics y points down; the bar shows climbing as positive
      m_horizontalBar->setSpeed(m_telemetry.m_velocity.x, us.dt);
      m_verticalBar->setSpeed(-m_telemetry.m_velocity.y, us.dt);
   }
   else if (m_telemetryFresh)
   {
      m_telemetryFresh = false;
      m_horizontalBar->setNoSignal();
      m_verticalBar->setNoSignal();
   }
}


// ---------------------------------------------------------------------------
// HeadDownDisplay
// ---------------------------------------------------------------------------

HeadDownDisplay::HeadDownDisplay() :
   m_itemIdRepository(0),
   m_actorExists(false),
   m_hudMode(HudModeType::hudModeInit)
{
}

void HeadDownDisplay::clearDisplay(void)
{
   // since this is class is supposed to be instance once globally
   // (a lazy singleton implementation) we need to clean up all here
   // and recreate all 
   // Clean up current scene
   spActor actor = getFirstChild();

   while (actor)
   {
      spActor next = actor->getNextSibling();
      if (actor.get() != NULL)
      {
         actor->detach();
      }
      actor = next;
   }
}

void HeadDownDisplay::clearMap(void)
{
   clearDisplay();

   // vector is of type spActor so clearing it will delete all items
   m_mapActors.clear();

   // m_actorExists = false;

}


void HeadDownDisplay::initialiseHdd(
   const Vector2& topLeft, 
   const Vector2& bottomRight)
{
   // The HUD can work as game menu, map or orbit overlay. The modes 
   // all have a common ancor on the screen, at the midde bottom. 
   // The game menu and orbit overlay have the top margin the same as the bottom.
   // The orbit overlay goes from left = 0 to right = screen width
   // The game menu goes from TBD to TBD
   // The map is the same size as the game map but scaled to fit into the 
   // map window between the lower buttons
   // In map modes, the size of the map needs to be the same as the scene 
   // but scaled to the map size. This is because that makes the position
   // of map items on the map the same as the real positions in the scene
   // However, when using the orbit overlay or game menu, we want the scale
   // to one. 
   // The animation of the size transitions is done in screen coordinates.
   // so, when shrinking to map size, this is done in scale 1, using the 
   // size as animation. When the map animation is finished, the size and scale
   // is changed in the same step, doing a seamless switch to map scale.

   m_sceneWidth = bottomRight.x - topLeft.x;
   m_sceneHeight = bottomRight.y - topLeft.y;

   // Speed bars go above and to the right of the map. The one on the
   // right needs room between the map and the fire button.
   float barThickness = g_Layout.getButtonWidth() * 0.14f;
   float barGap = g_Layout.getButtonWidth() * 0.06f;

   // Calculate the rect where the map is to be
   float mapLeft = g_Layout.getXFromLeft(2) + g_Layout.getButtonWidth() / 4.0f;
   float mapRight = g_Layout.getXFromRight(1) - g_Layout.getButtonWidth() / 4.0f - barGap - barThickness;
   float mapTop = g_Layout.getYFromBottom(1);
   // float mapBottom = g_Layout.getYFromBottom(-1) - g_Layout.getButtonWidth() / 4.0f;
   float mapBottom = g_Layout.getYFromBottom(-1);

   // Vector2 mapPos((mapRight - mapLeft) / 2.0f + mapLeft, (mapBottom - mapTop) / 2.0f + mapTop);
   m_mapPos = Vector2((mapRight - mapLeft) / 2.0f + mapLeft, mapBottom);

   float mapScaleX = (mapRight - mapLeft) / (m_sceneWidth);
   float mapScaleY = (mapBottom - mapTop) / (m_sceneHeight);

   m_mapScale = fmin(mapScaleX, mapScaleY);
   m_mapSizeX = m_sceneWidth * m_mapScale;
   m_mapSizeY = m_sceneHeight * m_mapScale;

   m_itemScale = 1.0f / m_mapScale * g_Layout.getButtonWidth() / 80.0f;

   m_menuSizeX = g_Layout.getViewPortBounds().x - 4.0f * g_Layout.getButtonWidth()  - 0.2f * g_Layout.getButtonWidth();
   m_menuSizeY = g_Layout.getViewPortBounds().y - 2.0f * (g_Layout.getViewPortBounds().y - mapBottom);

   // m_orbitSizeX = g_Layout.getViewPortBounds().x;
   // From the left safe edge (clear of a camera notch) to the buttons on the right
   m_orbitSizeX = g_Layout.getXFromRight(4) - g_Layout.getXFromLeft(0);
   m_orbitPosX = g_Layout.getXFromLeft(0) + m_orbitSizeX / 2.0f;
   

   m_orbitSizeY = m_menuSizeY;

   float thickness = 8.0f;
   
   setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_fat"));
   setVerticalMode(Box9Sprite::TILING_FULL);
   setHorizontalMode(Box9Sprite::TILING_FULL);
   setSize(m_sceneWidth, m_sceneHeight);
   setAnchor(0.5f, 1.0f);
   setPosition(m_mapPos);
   setScale(m_mapScale);
   setTouchEnabled(false);
   attachTo(g_MainActor);

   // The instruments are recreated with each scene since the main actor
   // removes all its children (and theirs) when a new scene starts.
   RectF mapRect(
      m_mapPos.x - m_mapSizeX / 2.0f,
      m_mapPos.y - m_mapSizeY,
      m_mapSizeX,
      m_mapSizeY);

   m_instruments = new HddInstruments(mapRect, barThickness, barGap);
   m_instruments->attachTo(g_MainActor);

   m_actorExists = true;

   // Some map items has already been added as the xml-file was read
   // but they are not yet attached to the map (since it was just
   // created, how could they?). Now it is time to add them.
   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      (*it)->addToMapActor(this, m_itemScale);
   }

   resetHudMode();
}

int HeadDownDisplay::addMeToMap(
   MapItem::MapItemTypeEnum type,
   spActor actor,
   MapItem::MapItemStateEnum state)
{
   // Check that this actor has not been added before.
   // If so, just change state.

   // We need to know if the map actor exists 
   // or not. 
   spMapItem mi;
   if (m_actorExists)
   {
       mi = new MapItem(
         this,
         type,
         state,
         m_itemIdRepository,
         actor,
         m_itemScale);
   }
   else
   {
      mi = new MapItem(type, state, m_itemIdRepository, actor);
   }

   m_mapActors.push_back(mi);

   return m_itemIdRepository++;
}

void HeadDownDisplay::removeMeFromMap(spActor removeMe)
{
   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      if ((*it)->getActor() == removeMe)
      {
         spActor d = (*it)->getActor();
         (*it)->detach();
         m_mapActors.erase(it);
         return;
      }
   }
}


void HeadDownDisplay::removeMeFromMap(int itemId)
{
   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      if ((*it)->getId() == itemId)
      {
         spActor d = (*it)->getActor();
         (*it)->detach();
         m_mapActors.erase(it);
         return;
      }
   }
}

void HeadDownDisplay::setState(int itemId, MapItem::MapItemStateEnum state)
{
   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      if ((*it)->getId() == itemId)
      {
         (*it)->setState(state);
         return;
      }
   }
}

void HeadDownDisplay::setState(oxygine::spActor actor, MapItem::MapItemStateEnum state)
{
   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      if ((*it)->getActor() == actor)
      {
         (*it)->setState(state);
         return;
      }
   }
}


void HeadDownDisplay::doUpdate(const oxygine::UpdateState& us)
{
}

void HeadDownDisplay::goToOrbit(void)
{

   if ((m_hudMode == HudModeType::hudModeMap) || (m_hudMode == HudModeType::hudModeInit))
   {
      clearMap();
      setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_thin"));
      setVerticalMode(Box9Sprite::TILING_FULL);
      setHorizontalMode(Box9Sprite::TILING_FULL);
      setSize(m_mapSizeX, m_mapSizeY);
      setScale(1.0f);
   }
   else
   {
      clearDisplay();
   }

   spTween tween = addTween(Actor::TweenSize(m_orbitSizeX, m_orbitSizeY), 500);
   tween->setDoneCallback(CLOSURE(this, &HeadDownDisplay::orbitStartTransitionComplete));

   spTween moveTween = addTween(Actor::TweenPosition(m_orbitPosX, m_orbitSizeY), 500);
}

void HeadDownDisplay::goToMenu(void)
{
   if (m_hudMode == HudModeType::hudModeMap)
   {
      clearDisplay();
      setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_thin"));
      setVerticalMode(Box9Sprite::TILING_FULL);
      setHorizontalMode(Box9Sprite::TILING_FULL);
      setSize(m_mapSizeX, m_mapSizeY);
      setScale(1.0f);
   }
   
   if (m_hudMode == HudModeType::hudModeOrbit)
   {
      spTween moveTween = addTween(Actor::TweenPosition(m_mapPos), 750);
   }

   m_hudMode = HudModeType::hudModeMenu;
   spTween tween = addTween(Actor::TweenSize(m_menuSizeX, m_menuSizeY), 750, 1, false, 0, oxygine::Tween::EASE::ease_outBounce);
   tween->setDoneCallback(CLOSURE(this, &HeadDownDisplay::menuStartTransitionComplete));


}

void HeadDownDisplay::resetHudMode(void)
{
   m_hudMode = HudModeType::hudModeInit;
}

bool HeadDownDisplay::isShowingMap(void)
{
   return (m_hudMode == HudModeType::hudModeMap) && !getFirstTween();
}

void HeadDownDisplay::setLeapfrogTelemetry(const LeapfrogTelemetry& telemetry)
{
   if (m_instruments)
   {
      m_instruments->setTelemetry(telemetry);
   }
}

void HeadDownDisplay::logMessage(const std::string& text, bool critical)
{
   if (m_instruments)
   {
      m_instruments->getLog()->addMessage(text, critical ? HudLog::critical : HudLog::normal);
   }
}

void HeadDownDisplay::logAmount(const std::string& key, const std::string& format, int amount)
{
   if (m_instruments)
   {
      m_instruments->getLog()->addAmount(key, format, amount);
   }
}

void HeadDownDisplay::goToMap(void)
{
   if (m_hudMode != HudModeType::hudModeMap)
   {
      clearDisplay();
      spTween tween = addTween(Actor::TweenSize(m_mapSizeX, m_mapSizeY), 500);
      tween->setDoneCallback(CLOSURE(this, &HeadDownDisplay::mapTransitionComplete));

   }

   if (m_hudMode == HudModeType::hudModeMap)
   {
      spTween moveTween = addTween(Actor::TweenPosition(m_mapPos), 500);
   }
}

void HeadDownDisplay::menuStartTransitionComplete(Event* event)
{
   // TODO: Light up buttons (in mainactor) only at this time
   g_MainActor->menuStartTransitionComplete();
}

void HeadDownDisplay::orbitStartTransitionComplete(oxygine::Event* event)
{
   if (m_hudMode != HudModeType::hudModeInit)
   {
      g_MainActor->restartedFromMenu();
   }

   m_hudMode = HudModeType::hudModeOrbit;
}


void HeadDownDisplay::mapTransitionComplete(oxygine::Event* event)
{
   setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_fat"));
   setVerticalMode(Box9Sprite::TILING_FULL);
   setHorizontalMode(Box9Sprite::TILING_FULL);
   setSize(m_sceneWidth, m_sceneHeight);
   setScale(m_mapScale);

   for (auto it = m_mapActors.begin(); it != m_mapActors.end(); ++it)
   {
      (*it)->addToMapActor(this, m_itemScale);
   }

   if (m_hudMode != HudModeType::hudModeInit)
   {
      g_MainActor->restartedFromMenu();
   }

   m_hudMode = HudModeType::hudModeMap;
}
