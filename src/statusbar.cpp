#include "statusbar.h"
#include "gamestatus.h"
#include "sceneactor.h"
#include "gamestatusevents.h"

using namespace oxygine;
using namespace std;

StatusBar::StatusBar(
   Resources& gameResources,
   SceneActor* sceneActor,
   const Vector2& pos,
   const Vector2& size,
   const float maxProgress,
   const float initialProgress,
   string headline,
   StatusChangedEvent::GameStatusTypeEnum statusType) :
   m_sceneActor(sceneActor),
   m_maxProgess(maxProgress),
   m_headline(headline),
   m_statusType(statusType)
{
   setPosition(pos);
   setAnchor(0.0f, 0.0f);
   setPriority(163);

   // Status bar has headline and progress bar beneath


   spTextField t = new TextField();
   t->setAnchor(0.0f, 0.0f);
   t->setSize(size.x, size.y / 2.0f);
   t->setPriority(255);

//   TextStyle style = TextStyle(gameResources.getResFont("lf_font")).withColor(Color::Fuchsia).alignLeft();
   TextStyle style = TextStyle(gameResources.getResFont("lf_font")).withColor(Color::White).alignLeft();
   t->setStyle(style);

   t->setText(m_headline);
   t->setPosition(pos);
   t->setScale(0.5f);

   t->attachTo(sceneActor->getParent());

   spActor theBar = new Actor();
   theBar->setAnchor(0.0f, 0.0f);
   theBar->setSize(size.x, size.y / 2.0f);
   theBar->setPosition(pos + Vector2(-2.0f, size.y / 2.0f));
   theBar->attachTo(sceneActor->getParent());

   spColorRectSprite top = new ColorRectSprite();
   top->setColor(Color::Fuchsia);
   top->setAnchor(0.0f, 0.0f);
   top->setSize(size.x, 1.0f);
   top->setPosition(0.0f, 0.0f);
   top->attachTo(theBar);

   spColorRectSprite bottom = new ColorRectSprite();
   bottom->setColor(Color::Fuchsia);
   bottom->setAnchor(0.0f, 0.0f);
   bottom->setSize(size.x, 1.0f);
   bottom->setPosition(0.0f, size.y / 2.0f);
   bottom->attachTo(theBar);

   spColorRectSprite left = new ColorRectSprite();
   left->setColor(Color::Fuchsia);
   left->setAnchor(0.0f, 0.0f);
   left->setSize(1.0f, size.y / 2.0f);
   left->setPosition(0.0f, 0.0f);
   left->attachTo(theBar);

   spColorRectSprite right = new ColorRectSprite();
   right->setColor(Color::Fuchsia);
   right->setAnchor(0.0f, 0.0f);
   right->setSize(1.0f, size.y / 2.0f);
   right->setPosition(size.x, 0.0f);
   right->attachTo(theBar);

   m_progressBar = new ProgressBar();
   m_progressBar->setColor(Color::Fuchsia);
   m_progressBar->setAnchor(0.0f, 0.0f);
   m_progressBar->setSize(Vector2(size.x, size.y / 2.0f) - Vector2(4.0f, 4.0f));
//   m_progressBar->setPosition(pos + Vector2(0.0f, size.y / 2.0f));
   m_progressBar->setPosition(Vector2(2.0f, 2.0f));
//   m_progressBar->attachTo(sceneActor->getParent());
   m_progressBar->attachTo(theBar);

   m_progressBar->setDirection(ProgressBar::direction::dir_0);
   m_progressBar->setProgress(initialProgress/maxProgress);

   addEventListener(StatusChangedEvent::EVENT, CLOSURE(this, &StatusBar::statusChangedListner));

}

void StatusBar::doUpdate(const oxygine::UpdateState& us)
{
}

void StatusBar::statusChangedListner(Event *ev)
{
   StatusChangedEvent* e = (StatusChangedEvent*)ev;

   if (e->type == m_statusType)
   {
      m_progressBar->setProgress(e->m_value / e->m_max);
   }
}
