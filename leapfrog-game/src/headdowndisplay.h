#pragma once

#include "oxygine-framework.h"
#include "Box2D/Box2D.h"
#include "scales.h"
#include "physdispconvert.h"
#include "collisionentity.h"
#include "layout.h"

#include <string>
#include <vector>
#include <unordered_set>

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
   Affiliation m_affiliation;
   bool m_shownOnMap;
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

   // The square's colour. It starts from the type (me is leapfrog,
   // friendly*, neutral* and enemy* are friendly, neutral and enemy) and
   // can be changed later; the shape (moving or stationary) stays.
   void setAffiliation(Affiliation affiliation);
   Affiliation getAffiliation(void);
   static Affiliation affiliationFromType(MapItemTypeEnum type);

   // Hidden items stay in the map's collection (so they keep their state
   // and colour) but are not drawn. Kept when the map is rebuilt.
   void setShownOnMap(bool shown);

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


DECLARE_SMART(TerrainSensorLayer, spTerrainSensorLayer);

// The terrain sensor's picture on the map: thin phosphor lines where the
// Leapfrog's range-finder (a sweeping beam of rays, see LeapFrog::
// updateTerrainSensor) has hit the ground. Lines fade out over the
// persistence time, like the afterglow of a radar screen, and the beam is
// drawn as a faint line. Lives on the map, in scene coordinates.
//
// With memory on, the ground stays on the map once scanned: a dim line
// under the live traces, for the rest of the scene. Only ground the memory
// doesn't cover yet is added (covered = a grid of c_memoryCell cells), so
// sweeping the same ground again and again adds nothing.
class TerrainSensorLayer : public oxygine::Actor
{
private:
   struct Trace
   {
      oxygine::spColorRectSprite m_sprite;
      float m_age;            // [s]
   };

   std::vector<Trace> m_traces;
   std::vector<oxygine::spColorRectSprite> m_pool;

   // Remembered ground
   oxygine::spActor m_memoryLayer;            // under the live traces
   std::vector<oxygine::spColorRectSprite> m_memory;
   std::unordered_set<long long> m_covered;   // grid cells with remembered ground
   bool m_remember;
   unsigned char m_memoryAlpha;
   oxygine::spColorRectSprite m_beam;
   float m_beamAge;           // [s] since the beam was last set
   float m_lineWidth;         // [scene units]
   float m_persistence;       // [s]
   oxygine::Vector2 m_bounds; // the scene's size: nothing is drawn outside

   // Clips a to b to the scene; false if nothing of it is inside
   bool clip(oxygine::Vector2& a, oxygine::Vector2& b) const;
   void placeLine(oxygine::ColorRectSprite* sprite, const oxygine::Vector2& a, const oxygine::Vector2& b);
   long long cellKey(const oxygine::Vector2& p) const;
   void remember(const oxygine::Vector2& a, const oxygine::Vector2& b);

public:
   TerrainSensorLayer();

   void setLineWidth(float width);
   void setPersistence(float seconds);
   void setBounds(const oxygine::Vector2& sceneSize);

   // Ground stays on the map once scanned (dim, alpha 0..255)
   void setMemory(bool on, unsigned char alpha);
   size_t memorySize(void) const { return m_memory.size(); }

   // A trace from a to b (a dot if they are the same), scene coordinates
   void addTrace(const oxygine::Vector2& a, const oxygine::Vector2& b);

   // The beam, from origin at angle [rad] (y down), length in scene units
   void setBeam(const oxygine::Vector2& origin, float angle, float length);

   void clearTraces(void);

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
   bool m_inOrbit;      // goToOrbit until goToMap: no map, so map items
                        // added meanwhile (e.g. the orbit scene's Leapfrog)
                        // wait unattached instead of showing in the
                        // orbit display's corner
   HudModeType m_hudMode;
   std::vector<spMapItem> m_mapActors;

   float m_itemScale;
   int m_itemIdRepository;

   spHddInstruments m_instruments;
   spTerrainSensorLayer m_terrainSensor;
   void attachTerrainSensor(void);

   void orbitStartTransitionComplete(oxygine::Event* event);
   void menuStartTransitionComplete(oxygine::Event* event);
   void mapTransitionComplete(oxygine::Event* event);

   bool m_squeezed;

public:
   float m_mapScale;
   float m_mapSizeX;
   float m_mapSizeY;
   float m_orbitSizeX;
   float m_orbitSizeY;
   float m_orbitPosX;
   float m_menuSizeX;
   float m_menuSizeY;

   float m_sceneWidth;
   float m_sceneHeight;

   oxygine::Vector2 m_mapPos;

   HeadDownDisplay();

   // Squeeze the display (map or orbit overlay, and the instruments around
   // the map) flat into its bottom edge, or grow it back, e.g. to make room
   // for the expanded message display. delay in ms before it starts.
   void squeeze(bool squeezed, oxygine::timeMS duration, oxygine::timeMS delay = 0);
   static const oxygine::timeMS c_squeezeMs = 200;

   // Where the display is in orbit (screen coordinates): between the
   // message display's column on the left and the booster/fire column
   // on the right, the whole height (where the map is in other scenes,
   // only taller). The orbit scene lays its parts out inside it:
   //   orbitSpaceViewRect  the close-up of the Leapfrog, an inset in the
   //                       display's top right corner (empty sky)
   //   orbitBurnBarRect    the burn bar and its caret, at the display's
   //                       right edge under the inset
   //   orbitPictureRect    room for the planet, trajectory and sites:
   //                       the display less a margin and the bar's column
   static oxygine::RectF orbitRect(void);
   static oxygine::RectF orbitSpaceViewRect(void);
   static oxygine::RectF orbitBurnBarRect(void);
   static oxygine::RectF orbitPictureRect(void);

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

   void setAffiliation(int itemId, Affiliation affiliation);
   void setAffiliation(oxygine::spActor actor, Affiliation affiliation);

   void setItemShown(oxygine::spActor actor, bool shown);

   void goToOrbit(void);
   void goToMenu(void);
   void goToMap(void);

   void resetHudMode(void);

   // True when the map is shown and no size/position transition is running
   bool isShowingMap(void);

   // Called by the Leapfrog every update
   void setLeapfrogTelemetry(const LeapfrogTelemetry& telemetry);

   // The terrain sensor (LeapFrog::updateTerrainSensor) reports what its
   // beam hits, in physics coordinates. persistence: how long a trace
   // stays [s]. Traces are cleared with the map (a new scene).
   void sensorTrace(const b2Vec2& a, const b2Vec2& b);
   void sensorBeam(const b2Vec2& origin, float angle, float range, float persistence, bool memory = false);

   // Event log, see HudLog. Safe to call at any time (ignored before
   // the first scene has set up the HDD).
   void logMessage(const std::string& text, bool critical = false);
   void logAmount(const std::string& key, const std::string& format, int amount);

protected:
	void doUpdate(const oxygine::UpdateState& us);
};

extern spHeadDownDisplay g_HeadDownDisplay;

