#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"

#include <string>
#include <vector>

class HeadDownDisplay;

DECLARE_SMART(MapItem, spMapItem);

class MapItem : public oxygine::Sprite
{
public:
   enum MapItemTypeEnum
   {
      me,
      friendlyMoving,
      friendlyStationary,
      neutralMoving,
      neutralStationary,
      enemyMoving,
      enemyStationary
   };

   enum MapItemStateEnum
   {
      hollow,
      filled,
      flashingSlow,
      flashingFast
   };


private:
   oxygine::ResAnim* m_resAnim;
   MapItemTypeEnum m_type;
   oxygine::spActor m_realActor;
   HeadDownDisplay* m_parentMap;
   MapItem::MapItemStateEnum m_state;
   int m_itemId;

public:
   // There are two constructors.
   // The first one is for 
   // creating map items without adding them 
   // to the map actor (just inserting them
   // in the collection of items). This is done when scene
   // files are read and all objects are created. At this
   // time there is no map yet. Some function needs to 
   // iterate the map item collection and add them all
   // to the map when it is created
   // The other is complete and used when obejcts are 
   // added at a later time, by the lua-script. At this
   // time the map actor will exist
   MapItem(
      MapItemTypeEnum type,
      MapItemStateEnum state,
      int id,
      oxygine::spActor actor);

   MapItem(
      HeadDownDisplay* hddMapActor,
      MapItemTypeEnum type,
      MapItemStateEnum state,
      int id,
      oxygine::spActor actor,
      float scale);

   void addToMapActor(
      HeadDownDisplay* hddMapActor,
      float scale);

   MapItemTypeEnum getType(void);
   oxygine::spActor getActor(void);
   int getId(void);
   void setState(MapItem::MapItemStateEnum state);

protected:
   void doUpdate(const oxygine::UpdateState& us);

};


// ---------------------------------------------------------------------------
// HDD instruments: speed bars and the event log.
// They live in screen coordinates on top of the map (the map itself is
// scaled to scene coordinates) and are only visible in map mode.
// ---------------------------------------------------------------------------

// What the Leapfrog reports to the HDD every update. Pushing values
// (instead of the HDD holding a pointer to the Leapfrog) means nothing
// dangles when the Leapfrog or its b2World goes away.
struct LeapfrogTelemetry
{
   b2Vec2 m_velocity;      // [m/s], physics coordinates (y down)
   bool m_shieldInUse;     // false on the ground, where the shield bar is dimmed
   float m_ammo;
   float m_shield;
   float m_fuel;
   float m_damage;         // 0 = intact, grows with hits
};


DECLARE_SMART(SpeedBar, spSpeedBar);

// A bar centred on zero that fills towards the sign of the speed.
// The scale is square-root shaped so that the small speeds that
// matter when landing are easy to read while large speeds still fit.
class SpeedBar : public oxygine::Actor
{
private:
   bool m_vertical;
   oxygine::Vector2 m_barSize;
   float m_fullScale;
   float m_displayed;
   float m_readoutValue;
   int m_readoutTimer;
   std::string m_label;

   oxygine::spColorRectSprite m_fill;
   oxygine::spTextField m_readout;

   float toFraction(float speed) const;
   void addTick(float speed, unsigned char alpha);
   void layoutFill(void);
   void setReadout(float speed);

public:
   // size: the bar itself. readoutPos: top left corner of the readout
   // text, relative to the bar. Positive speed is right / up.
   SpeedBar(
      bool vertical,
      const oxygine::Vector2& size,
      float fullScale,
      float safeLimit,
      bool safeBothSides,
      const std::string& label,
      int fontSize,
      const oxygine::Vector2& readoutPos,
      bool readoutAlignRight);

   void setSpeed(float speed, int dtMs);
   void setNoSignal(void);
};


DECLARE_SMART(HudLog, spHudLog);

// The scrolling event log in the bottom left corner of the map.
// New lines appear at the bottom and push older ones up. Every line
// also drifts slowly upwards on its own and fades out, so the log
// empties itself even if nothing new happens.
class HudLog : public oxygine::ClipRectActor
{
public:
   enum Priority
   {
      normal,
      critical
   };

private:
   struct Line
   {
      oxygine::spActor m_actor;
      oxygine::spTextField m_text;
      oxygine::spTextField m_shadow;
      std::string m_key;      // non-empty for lines that merge amounts
      std::string m_format;   // printf format with one %d
      int m_amount;
      float m_age;            // [s]
      float m_lifetime;       // [s]
      float m_y;
      Priority m_priority;
   };

   std::vector<Line> m_lines;    // newest first
   float m_lineHeight;
   float m_padding;
   int m_fontSize;

   void pushLine(const std::string& text, Priority prio, const std::string& key, const std::string& format, int amount);
   void setLineText(Line& line, const std::string& text);
   float targetY(int index, const Line& line) const;

public:
   HudLog(const oxygine::Vector2& size, int fontSize);

   // A one-off message, e.g. "Shield critical". Repeating the newest
   // line just refreshes it instead of adding a copy.
   void addMessage(const std::string& text, Priority prio = normal);

   // A message with an amount that is summed if the same key is
   // logged again while its line is the newest, so three quick ammo
   // pickups read "Picked up 12 ammo" rather than three lines.
   void addAmount(const std::string& key, const std::string& format, int amount, Priority prio = normal);

protected:
   void doUpdate(const oxygine::UpdateState& us) override;
};


DECLARE_SMART(HddInstruments, spHddInstruments);

class HddInstruments : public oxygine::Actor
{
private:
   spSpeedBar m_horizontalBar;
   spSpeedBar m_verticalBar;
   spHudLog m_log;

   LeapfrogTelemetry m_telemetry;
   bool m_telemetryFresh;
   bool m_haveBaseline;
   int m_msSinceTelemetry;

   // Warning level per resource: 0 = ok, 1 = low, 2 = critical, 3 = empty
   int m_ammoLevel;
   int m_shieldLevel;
   int m_fuelLevel;
   int m_damageLevel;

   void evaluateWarnings(void);

public:
   HddInstruments(const oxygine::RectF& mapRect, float barThickness, float gap);

   void setTelemetry(const LeapfrogTelemetry& telemetry);
   HudLog* getLog(void);

protected:
   void doUpdate(const oxygine::UpdateState& us) override;
};


DECLARE_SMART(HeadDownDisplay, spHeadDownDisplay);

class HeadDownDisplay : public oxygine::Box9Sprite
{
private:

   enum HudModeType
   {
      hudModeInit,
      hudModeMap,
      hudModeOrbit,
      hudModeMenu
   };

   bool m_actorExists;
   HudModeType m_hudMode;
   std::vector<spMapItem> m_mapActors;

   float m_itemScale;
   int m_itemIdRepository;

   spHddInstruments m_instruments;

   void orbitStartTransitionComplete(oxygine::Event* event);
   void menuStartTransitionComplete(oxygine::Event* event);
   void mapTransitionComplete(oxygine::Event* event);

public:
   float m_mapScale;
   float m_mapSizeX;
   float m_mapSizeY;
   float m_orbitSizeX;
   float m_orbitSizeY;
   float m_menuSizeX;
   float m_menuSizeY;

   float m_sceneWidth;
   float m_sceneHeight;

   oxygine::Vector2 m_mapPos;

   HeadDownDisplay();

   void initialiseHdd(
      const oxygine::Vector2& topLeft, 
      const oxygine::Vector2& bottomRight);

   void clearMap(void);
   void clearDisplay(void);

   int addMeToMap(
      MapItem::MapItemTypeEnum type,
      oxygine::spActor actor,
      MapItem::MapItemStateEnum state);

   void removeMeFromMap(oxygine::spActor removeMe);
   void removeMeFromMap(int itemId);

   // Fast version, caller must have remembered the item idenitfier.
   void setState(int itemId, MapItem::MapItemStateEnum state);

   // Slow version, the item is looked up
   void setState(oxygine::spActor actor, MapItem::MapItemStateEnum state);

   void goToOrbit(void);
   void goToMenu(void);
   void goToMap(void);

   void resetHudMode(void);

   // True when the map is shown and no size/position transition is running
   bool isShowingMap(void);

   // Called by the Leapfrog every update
   void setLeapfrogTelemetry(const LeapfrogTelemetry& telemetry);

   // Event log, see HudLog. Safe to call at any time (ignored before
   // the first scene has set up the HDD).
   void logMessage(const std::string& text, bool critical = false);
   void logAmount(const std::string& key, const std::string& format, int amount);

protected:
	void doUpdate(const oxygine::UpdateState& us);
};

extern spHeadDownDisplay g_HeadDownDisplay;

