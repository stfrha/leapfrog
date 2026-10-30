#include "statusbar.h"
#include "gamestatus.h"
#include "sceneactor.h"
#include "objectpropertyevents.h"
#include "leapfrogevents.h"
#include "layout.h"
#include "graphicresources.h"

using namespace oxygine;
using namespace std;

// The headline's share of a status row's width (the bar or value gets the
// rest)
const float c_statusLabelFraction = 0.45f;

StatusBar::StatusBar(
   oxygine::Actor* eventActor,
   SceneActor* sceneActor,
   Actor* attachToMe,
   int propertyId,
   const Vector2& pos,
   const Vector2& size,
   const int fontSize,
   const float maxProgress,
   const float initialProgress,
   string headline,
   ObjectResourcesTypeEnum statusType) :
   m_sceneActor(sceneActor),
   m_eventActor(eventActor),
   m_propertyId(propertyId),
   m_maxProgess(maxProgress),
   m_headline(headline),
   m_statusType(statusType)
{
   setPosition(pos + Vector2(-2.0f, 0.0f));
   setAnchor(0.0f, 0.0f);
   setPriority(163);
   attachTo(attachToMe);

   // One row: the headline on the left, the bar to its right (they used
   // to be stacked, twice the height)
   float labelWidth = size.x * c_statusLabelFraction;
   float barWidth = size.x - labelWidth;

   spTextField t = new TextField();
   t->setAnchor(0.0f, 0.0f);
   t->setSize(labelWidth, size.y);
   t->setColor(g_Layout.getPhosphorColor());
   t->setPriority(255);

   TextStyle style = TextStyle(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResFont("lf_font")).withColor(Color::White).alignLeft().alignMiddleV();
   style.fontSize = fontSize;

   t->setStyle(style);
   t->setText(m_headline);
   t->setPosition(0.0f, 0.0f);
   t->attachTo(this);

   spActor theBar = new Actor();
   theBar->setAnchor(0.0f, 0.0f);
   theBar->setSize(barWidth, size.y);
   theBar->setPosition(labelWidth, 0.0f);
   theBar->attachTo(this);

   spBox9Sprite frame = new Box9Sprite();
   frame->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_thin"));
   frame->setVerticalMode(Box9Sprite::STRETCHING);
   frame->setHorizontalMode(Box9Sprite::STRETCHING);
   frame->setSize(barWidth, size.y);
   frame->setAnchor(0.0f, 0.0f);
   frame->setPosition(0.0f, 0.0f);
   frame->setGuides(8, 120, 8, 120);
   frame->setTouchEnabled(false);
   frame->attachTo(theBar);

   m_progressBar = new ProgressBar();
   m_progressBar->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("progress_bar"));
   m_progressBar->setAnchor(0.0f, 0.0f);
   m_progressBar->setSize(Vector2(barWidth, size.y) - Vector2(4.0f, 4.0f));
   m_progressBar->setPosition(Vector2(2.0f, 2.0f));
   m_progressBar->attachTo(theBar);

   m_progressBar->setDirection(ProgressBar::direction::dir_0);
   m_progressBar->setProgress(initialProgress/maxProgress);

//   m_eventActor->addEventListener(StatusChangedEvent::EVENT, CLOSURE(this, &StatusBar::statusChangedListner));
   if (m_eventActor != NULL)
   {
      m_eventActor->addEventListener(ObjectPropertyChangedEvent::EVENT, CLOSURE(this, &StatusBar::statusChangedListner));
   }
}

void StatusBar::disconnectListner(void)
{
   if (m_eventActor != NULL)
   {
      m_eventActor->removeEventListener(ObjectPropertyChangedEvent::EVENT);
   }
}

void StatusBar::doUpdate(const oxygine::UpdateState& us)
{
}

void StatusBar::statusChangedListner(Event *ev)
{
   ObjectPropertyChangedEvent* e = (ObjectPropertyChangedEvent*)ev;

   if (e->m_propId == m_propertyId)
   {
      m_progressBar->setProgress(e->m_property->getProperty() / m_maxProgess);
   }
}

void StatusBar::dummyListner(oxygine::Event *ev)
{
   logs::messageln("Got dummy event inside StatusBar");
}


StatusLiteral::StatusLiteral(
   oxygine::Actor* eventActor,
   SceneActor* sceneActor,
   Actor* attachToMe,
   int propertyId,
   const Vector2& pos,
   const Vector2& size,
   const int fontSize,
   const int initialLiteral,
   string headline,
   ObjectResourcesTypeEnum statusType) :
   m_sceneActor(sceneActor),
   m_eventActor(eventActor),
   m_propertyId(propertyId),
   m_headline(headline),
   m_statusType(statusType)
{
   setPosition(pos + Vector2(-2.0f, 0.0f));
   setAnchor(0.0f, 0.0f);
   setPriority(163);
   attachTo(attachToMe);

   // One row: the headline on the left, the value to its right
   float labelWidth = size.x * c_statusLabelFraction;

   spTextField t = new TextField();
   t->setAnchor(0.0f, 0.0f);
   t->setSize(labelWidth, size.y);
   t->setColor(g_Layout.getPhosphorColor());
   t->setPriority(255);

   TextStyle style = TextStyle(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResFont("lf_font")).withColor(Color::White).alignLeft().alignMiddleV();
   style.fontSize = fontSize;

   t->setStyle(style);
   t->setText(m_headline);
   t->setPosition(0.0f, 0.0f);
   t->attachTo(this);

   m_tf = new TextField();
   m_tf->setAnchor(0.0f, 0.0f);
   m_tf->setSize(size.x - labelWidth, size.y);
   m_tf->setColor(g_Layout.getPhosphorColor());
   m_tf->setPriority(255);

   TextStyle literalStyle = TextStyle(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResFont("lf_font")).withColor(Color::White).alignLeft().alignMiddleV();
   literalStyle.fontSize = fontSize;

   m_tf->setStyle(literalStyle);
   setLiteral(initialLiteral);
   m_tf->setPosition(Vector2(labelWidth, 0.0f));
   m_tf->attachTo(this);

   if (m_eventActor != NULL)
   {
      m_eventActor->addEventListener(ObjectPropertyChangedEvent::EVENT, CLOSURE(this, &StatusLiteral::statusChangedListner));
   }
}

void StatusLiteral::setLiteral(int value)
{
   char text[32];
   sprintf(text, "%d", value);
   m_tf->setText(text);

   // Credits below zero are a debt (to Bruno): shown in red
   if (m_statusType == ObjectResourcesTypeEnum::credits)
   {
      m_tf->setColor((value < 0) ? oxygine::Color(0xff3030ff) : g_Layout.getPhosphorColor());
   }
}

void StatusLiteral::doUpdate(const oxygine::UpdateState& us)
{
}

void StatusLiteral::disconnectListner(void)
{
   if (m_eventActor != NULL)
   {
      m_eventActor->removeEventListener(ObjectPropertyChangedEvent::EVENT);
   }
}

void StatusLiteral::statusChangedListner(Event *ev)
{
   ObjectPropertyChangedEvent* e = (ObjectPropertyChangedEvent*)ev;

   if (e->m_propId == m_propertyId)
   {
      setLiteral((int)e->m_property->getProperty());
   }
}

void StatusLiteral::dummyListner(oxygine::Event *ev)
{
   logs::messageln("Got dummy event inside StatusBar");
}


// ---------------------------------------------------------------------------
// LegStatus

namespace
{
   // The picture, in its own units (Fredrik's sketch): the body, and the
   // joints in the Leapfrog's order: left hip, knee, foot, right hip,
   // knee, foot
   const RectF c_legBody(150.0f, 15.0f, 480.0f, 100.0f);

   const Vector2 c_legJoints[LegStatus::c_numJoints] =
   {
      Vector2(322.0f, 113.0f), Vector2(157.0f, 220.0f), Vector2(157.0f, 437.0f),
      Vector2(470.0f, 113.0f), Vector2(620.0f, 220.0f), Vector2(620.0f, 437.0f)
   };

   // The feet: a bar through each foot joint
   const float c_legFootHalfWidth = 108.0f;
   const float c_legFootThickness = 28.0f;
   const float c_legLimbThickness = 28.0f;
   const float c_legJointRadius = 48.0f;
   const float c_legRingThickness = 18.0f;

   // The picture's extent (feet and joint rings included)
   const RectF c_legExtent(42.0f, 15.0f, 683.0f, 470.0f);

   const Color c_legFailColor(0xff3030ff);
}

LegStatus::LegStatus(Actor* attachToMe, const Vector2& pos, const Vector2& size)
{
   setAnchor(0.0f, 0.0f);
   setPosition(pos);
   setSize(size);
   setPriority(163);
   setTouchEnabled(false);
   setTouchChildrenEnabled(false);
   attachTo(attachToMe);

   float scale = std::min(size.x / c_legExtent.getWidth(), size.y / c_legExtent.getHeight());
   Vector2 offset = (size - c_legExtent.getSize() * scale) / 2.0f - c_legExtent.getLeftTop() * scale;

   auto at = [&](const Vector2& p) { return offset + p * scale; };

   Color green = g_Layout.getPhosphorColor();

   // Body
   spColorRectSprite body = new ColorRectSprite();
   body->setColor(green);
   body->setAnchor(0.0f, 0.0f);
   body->setPosition(at(c_legBody.getLeftTop()));
   body->setSize(c_legBody.getSize() * scale);
   body->attachTo(this);

   // Legs: hip to knee to foot, and the feet
   for (int side = 0; side < 2; side++)
   {
      const Vector2& hip = c_legJoints[side * 3];
      const Vector2& knee = c_legJoints[side * 3 + 1];
      const Vector2& foot = c_legJoints[side * 3 + 2];

      addSegment(at(hip), at(knee), c_legLimbThickness * scale, green);
      addSegment(at(knee), at(foot), c_legLimbThickness * scale, green);
      addSegment(
         at(foot - Vector2(c_legFootHalfWidth, 0.0f)),
         at(foot + Vector2(c_legFootHalfWidth, 0.0f)),
         c_legFootThickness * scale, green);
   }

   // Joints on top: a dark disk (covers the limb ends) with a ring
   for (int i = 0; i < c_numJoints; i++)
   {
      addDisk(at(c_legJoints[i]), c_legJointRadius * scale, Color(0, 0, 0, 255));
      addRing(i, at(c_legJoints[i]), c_legJointRadius * scale, c_legRingThickness * scale);
      m_shownHealth[i] = -1.0f;
      showHealth(i, g_GameStatus.getLegHealth(i));
   }
}

void LegStatus::addSegment(const Vector2& a, const Vector2& b, float thickness, const Color& color)
{
   Vector2 d = b - a;

   spColorRectSprite r = new ColorRectSprite();
   r->setColor(color);
   r->setAnchor(0.0f, 0.5f);
   r->setSize(d.length(), thickness);
   r->setPosition(a);
   r->setRotation(atan2f(d.y, d.x));
   r->attachTo(this);
}

void LegStatus::addDisk(const Vector2& centre, float radius, const Color& color)
{
   // Squares turned in steps: their union lies between a/2 and a/sqrt(2)
   // from the centre (a the side), so with a = sqrt(2) radius it fills the
   // inside of the ring and its corners stay under the ring
   const int squares = 6;
   float side = radius * 1.41421356f;

   for (int k = 0; k < squares; k++)
   {
      spColorRectSprite r = new ColorRectSprite();
      r->setColor(color);
      r->setAnchor(0.5f, 0.5f);
      r->setSize(side, side);
      r->setPosition(centre);
      r->setRotation((float)k * MATH_PI / 2.0f / (float)squares);
      r->attachTo(this);
   }
}

void LegStatus::addRing(int joint, const Vector2& centre, float radius, float thickness)
{
   // Segments round the circle, clockwise from the top
   float mid = radius - thickness / 2.0f;
   float length = 2.0f * MATH_PI * mid / (float)c_ringSegments * 1.15f;

   for (int k = 0; k < c_ringSegments; k++)
   {
      float a = -MATH_PI / 2.0f + ((float)k + 0.5f) * 2.0f * MATH_PI / (float)c_ringSegments;

      spColorRectSprite r = new ColorRectSprite();
      r->setColor(g_Layout.getPhosphorColor());
      r->setAnchor(0.5f, 0.5f);
      r->setSize(length, thickness);
      r->setPosition(centre + Vector2(cosf(a), sinf(a)) * mid);
      r->setRotation(a + MATH_PI / 2.0f);
      r->attachTo(this);

      m_rings[joint].push_back(r);
   }
}

void LegStatus::showHealth(int joint, float health)
{
   health = std::max(0.0f, std::min(1.0f, health));

   if (fabs(health - m_shownHealth[joint]) < 0.005f)
   {
      return;
   }

   m_shownHealth[joint] = health;

   // Red for what is lost, rounded up so any damage shows
   int red = (health <= 0.0f) ? c_ringSegments : (int)ceilf((1.0f - health) * (float)c_ringSegments - 0.01f);

   for (int k = 0; k < c_ringSegments; k++)
   {
      m_rings[joint][k]->setColor(k < red ? c_legFailColor : g_Layout.getPhosphorColor());
   }
}

void LegStatus::doUpdate(const UpdateState& us)
{
   for (int i = 0; i < c_numJoints; i++)
   {
      showHealth(i, g_GameStatus.getLegHealth(i));
   }
}
