#include "statusbar.h"
#include "gamestatus.h"
#include "sceneactor.h"
#include "gamestatusevents.h"
#include "leapfrogevents.h"
#include "layout.h"

using namespace oxygine;
using namespace std;

StatusBar::StatusBar(
   Resources& gameResources,
   oxygine::Actor* eventActor,
   SceneActor* sceneActor,
   const Vector2& pos,
   const Vector2& size,
   const int fontSize,
   const float maxProgress,
   const float initialProgress,
   string headline,
   GameStatusTypeEnum statusType) :
   m_sceneActor(sceneActor),
   m_eventActor(eventActor),
   m_maxProgess(maxProgress),
   m_headline(headline),
   m_statusType(statusType)
{
   setPosition(pos + Vector2(-2.0f, 0.0f));
   setAnchor(0.0f, 0.0f);
   setPriority(163);
   attachTo(sceneActor->getParent());

   // Status bar has headline and progress bar beneath


   spTextField t = new TextField();
   t->setAnchor(0.0f, 0.0f);
   t->setSize(size.x, size.y / 2.0f);
   t->setColor(g_Layout.getPhosphorColor());
   t->setPriority(255);

//   TextStyle style = TextStyle(gameResources.getResFont("lf_font")).withColor(Color::Fuchsia).alignLeft();
   TextStyle style = TextStyle(gameResources.getResFont("lf_font")).withColor(Color::White).alignLeft();
   //style.fontSize = size.y / 2.0f / 8.0f * 6.0f;
   style.fontSize = fontSize;
   
   t->setStyle(style);
   
   t->setText(m_headline);
   t->setPosition(0.0f, size.y / 2.0f / 8.0f );
   
   // Set size of text to available height. Fonst size is 32.
//   t->setScale(0.5f);

   t->attachTo(this);

   spActor theBar = new Actor();
   theBar->setAnchor(0.0f, 0.0f);
   theBar->setSize(size.x, size.y / 2.0f);
   theBar->setPosition(-2.0f, size.y / 2.0f);
   theBar->attachTo(this);

   //spColorRectSprite top = new ColorRectSprite();
   //top->setColor(Color::Fuchsia);
   //top->setAnchor(0.0f, 0.0f);
   //top->setSize(size.x, 1.0f);
   //top->setPosition(0.0f, 0.0f);
   //top->attachTo(theBar);

   //spColorRectSprite bottom = new ColorRectSprite();
   //bottom->setColor(Color::Fuchsia);
   //bottom->setAnchor(0.0f, 0.0f);
   //bottom->setSize(size.x, 1.0f);
   //bottom->setPosition(0.0f, size.y / 2.0f);
   //bottom->attachTo(theBar);

   //spColorRectSprite left = new ColorRectSprite();
   //left->setColor(Color::Fuchsia);
   //left->setAnchor(0.0f, 0.0f);
   //left->setSize(1.0f, size.y / 2.0f);
   //left->setPosition(0.0f, 0.0f);
   //left->attachTo(theBar);

   //spColorRectSprite right = new ColorRectSprite();
   //right->setColor(Color::Fuchsia);
   //right->setAnchor(0.0f, 0.0f);
   //right->setSize(1.0f, size.y / 2.0f);
   //right->setPosition(size.x, 0.0f);
   //right->attachTo(theBar);

   spBox9Sprite hej = new Box9Sprite();
   hej->setResAnim(gameResources.getResAnim("display_thin"));
   hej->setVerticalMode(Box9Sprite::STRETCHING);
   hej->setHorizontalMode(Box9Sprite::STRETCHING);
   hej->setSize(size.x, size.y / 2.0f);
   hej->setAnchor(0.0f, 0.0f);
   hej->setPosition(0.0f, 0.0f);
   hej->setGuides(8, 120, 8, 120);
   hej->setTouchEnabled(false);
   hej->attachTo(theBar);

   m_progressBar = new ProgressBar();
   m_progressBar->setResAnim(gameResources.getResAnim("progress_bar"));
   m_progressBar->setAnchor(0.0f, 0.0f);
   m_progressBar->setSize(Vector2(size.x, size.y / 2.0f) - Vector2(4.0f, 4.0f));
//   m_progressBar->setPosition(pos + Vector2(0.0f, size.y / 2.0f));
   m_progressBar->setPosition(Vector2(2.0f, 2.0f));
//   m_progressBar->attachTo(sceneActor->getParent());
   m_progressBar->attachTo(theBar);

   m_progressBar->setDirection(ProgressBar::direction::dir_0);
   m_progressBar->setProgress(initialProgress/maxProgress);

   m_eventActor->addEventListener(StatusChangedEvent::EVENT, CLOSURE(this, &StatusBar::statusChangedListner));
}

void StatusBar::disconnectListner(void)
{
   m_eventActor->removeEventListener(StatusChangedEvent::EVENT);
}

void StatusBar::doUpdate(const oxygine::UpdateState& us)
{
}

void StatusBar::statusChangedListner(Event *ev)
{
   StatusChangedEvent* e = (StatusChangedEvent*)ev;

   if (e->m_type == m_statusType)
   {
      m_progressBar->setProgress(e->m_value / e->m_max);
   }
}

void StatusBar::dummyListner(oxygine::Event *ev)
{
   logs::messageln("Got dummy event inside StatusBar");
}


StatusLiteral::StatusLiteral(
   Resources& gameResources,
   oxygine::Actor* eventActor,
   SceneActor* sceneActor,
   const Vector2& pos,
   const Vector2& size,
   const int fontSize,
   const int initialLiteral,
   string headline,
   GameStatusTypeEnum statusType) :
   m_sceneActor(sceneActor),
   m_eventActor(eventActor),
   m_headline(headline),
   m_statusType(statusType)
{
   setPosition(pos + Vector2(-2.0f, 0.0f));
   setAnchor(0.0f, 0.0f);
   setPriority(163);
   attachTo(sceneActor->getParent());

   // Status bar has headline and literal string beneath
   spTextField t = new TextField();
   t->setAnchor(0.0f, 0.0f);
   t->setSize(size.x, size.y / 2.0f);
   t->setColor(g_Layout.getPhosphorColor());
   t->setPriority(255);

   TextStyle style = TextStyle(gameResources.getResFont("lf_font")).withColor(Color::White).alignLeft();
   style.fontSize = fontSize;

   t->setStyle(style);

   t->setText(m_headline);
   t->setPosition(0.0f, size.y / 2.0f / 8.0f);

   // Set size of text to available height. Fonst size is 32.
   //   t->setScale(0.5f);

   t->attachTo(this);

   m_tf = new TextField();
   m_tf->setAnchor(0.0f, 0.0f);
   m_tf->setSize(size.x, size.y / 2.0f);
   m_tf->setColor(g_Layout.getPhosphorColor());
   m_tf->setPriority(255);

   TextStyle literalStyle = TextStyle(gameResources.getResFont("lf_font")).withColor(Color::White).alignLeft();
   literalStyle.fontSize = fontSize + 4;

   m_tf->setStyle(literalStyle);
   setLiteral(initialLiteral);
   m_tf->setPosition(Vector2(0.0f, size.y / 2.0f));
   m_tf->attachTo(this);

   spActor theBar = new Actor();
   theBar->setAnchor(0.0f, 0.0f);
   theBar->setSize(size.x, size.y / 2.0f);
   theBar->setPosition(-2.0f, size.y / 2.0f);
   theBar->attachTo(this);

   m_eventActor->addEventListener(StatusChangedEvent::EVENT, CLOSURE(this, &StatusLiteral::statusChangedListner));
}

void StatusLiteral::setLiteral(int value)
{
   char text[32];
   sprintf(text, "%d", value);
   m_tf->setText(text);
}

void StatusLiteral::doUpdate(const oxygine::UpdateState& us)
{
}

void StatusLiteral::disconnectListner(void)
{
   m_eventActor->removeEventListener(StatusChangedEvent::EVENT);
}

void StatusLiteral::statusChangedListner(Event *ev)
{
   StatusChangedEvent* e = (StatusChangedEvent*)ev;

   if (e->m_type == m_statusType)
   {
      setLiteral(e->m_value);
   }
}

void StatusLiteral::dummyListner(oxygine::Event *ev)
{
   logs::messageln("Got dummy event inside StatusBar");
}
