#include "stripper.h"

#include <cstdlib>
#include <sstream>

#include "sceneactor.h"
#include "leapfrog.h"
#include "leapfrogevents.h"
#include "objectresources.h"
#include "shield.h"
#include "blastemitter.h"
#include "bodyuserdata.h"
#include "actoruserdata.h"
#include "headdowndisplay.h"
#include "mainactor.h"
#include "steeringmanager.h"

using namespace std;
using namespace oxygine;
using namespace pugi;

Stripper::Stripper(
   SceneActor* sceneActor,
   CompoundObject* parentObject,
   b2World* world,
   const oxygine::Vector2& pos,
   pugi::xml_node& root,
   int groupIndex) :
   CompoundObject(sceneActor, parentObject),
   m_world(world),
   m_body(NULL),
   m_state(approach),
   m_stateTime(0.0f),
   m_sparkTime(0.0f),
   m_weld(NULL),
   m_gripLocal(0.0f, 0.0f),
   m_numOwnFilters(0),
   m_loot(lootNone),
   m_lootAmount(0.0f),
   m_health(1.0f),
   m_shotDown(false),
   m_mapItemId(-1),
   m_maxSpeed(60.0f),
   m_acceleration(90.0f),
   m_turnRate(6.0f),
   m_noseAngle(0.0f),
   m_lead(3.0f),
   m_brakeDistance(50.0f),
   m_latchSpeed(8.0f),
   m_latchDistance(1.5f),
   m_ramSpeed(25.0f),
   m_shieldDrain(4.0f),
   m_stealTime(2.5f),
   m_gripForce(0.0f),
   m_gripTime(0.6f),
   m_strainTime(0.0f),
   m_stunTime(1.0f),
   m_leaveTime(2.0f),
   m_fadeInTime(0.5f),
   m_leaveSpeed(60.0f),
   m_sparkInterval(0.3f),
   m_antiGravity(true),
   m_ammoAmount(20.0f),
   m_shieldAmount(25.0f),
   m_fuelAmount(15.0f),
   m_creditsAmount(25.0f)
{
   initCompoundObjectParts(sceneActor, sceneActor, parentObject, world, pos, root, groupIndex);

   attachTo(m_sceneActor);

   m_properties.push_back(ObjectProperty(this, NULL, propState, (float)approach, true));
   m_properties.push_back(ObjectProperty(this, NULL, propXPos, 0.0f, true));
   m_properties.push_back(ObjectProperty(this, NULL, propYPos, 0.0f, true));

   xml_node propNode = root.child("behaviour").child("stripperProperties");

   readStripperNode(propNode);

   m_body = getBody("mainBody");

   if (m_body == NULL)
   {
      logs::messageln("Stripper: the object has no body called mainBody");
      return;
   }

   m_body->SetLinearDamping(0.0f);

   // Keep its own collision filters, to restore them when it lets go
   for (b2Fixture* f = m_body->GetFixtureList(); f && m_numOwnFilters < 8; f = f->GetNext())
   {
      m_ownFilters[m_numOwnFilters++] = f->GetFilterData();
   }

   // Soft scene boundaries push it back in, as for other enemies
   m_sceneActor->addBoundingBody(m_body);

   if (g_HeadDownDisplay)
   {
      m_mapItemId = g_HeadDownDisplay->addMeToMap(
         MapItem::MapItemTypeEnum::enemyMoving,
         getActor("mainBody"),
         MapItem::MapItemStateEnum::flashingSlow);
   }

   showStateShapes();

   // Fade in, as it fades out when it leaves
   if (m_fadeInTime > 0.0f)
   {
      for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it)
      {
         unsigned char alpha = (*it)->getAlpha();
         (*it)->setAlpha(0);
         (*it)->addTween(Actor::TweenAlpha(alpha), (timeMS)(m_fadeInTime * 1000.0f));
      }
   }
}

void Stripper::readStripperNode(const xml_node& node)
{
   m_maxSpeed = node.attribute("maxSpeed").as_float(m_maxSpeed);
   m_acceleration = node.attribute("acceleration").as_float(m_acceleration);
   m_turnRate = node.attribute("degPerSecTurnRate").as_float(360.0f) * MATH_PI / 180.0f;
   m_noseAngle = node.attribute("noseAngle").as_float(0.0f) * MATH_PI / 180.0f;
   m_lead = node.attribute("lead").as_float(m_lead);
   m_brakeDistance = node.attribute("brakeDistance").as_float(m_brakeDistance);
   m_latchSpeed = node.attribute("latchSpeed").as_float(m_latchSpeed);
   m_latchDistance = node.attribute("latchDistance").as_float(m_latchDistance);
   m_ramSpeed = node.attribute("ramSpeed").as_float(m_ramSpeed);
   m_shieldDrain = node.attribute("shieldDrain").as_float(m_shieldDrain);
   m_stealTime = node.attribute("stealTime").as_float(m_stealTime);
   m_gripForce = node.attribute("gripForce").as_float(m_gripForce);
   m_gripTime = node.attribute("gripTime").as_float(m_gripTime);
   m_stunTime = node.attribute("stunTime").as_float(m_stunTime);
   m_leaveTime = node.attribute("leaveTime").as_float(m_leaveTime);
   m_fadeInTime = node.attribute("fadeInTime").as_float(m_fadeInTime);
   m_leaveSpeed = node.attribute("leaveSpeed").as_float(m_leaveSpeed);
   m_sparkInterval = node.attribute("sparkInterval").as_float(m_sparkInterval);
   m_antiGravity = node.attribute("antiGravity").as_bool(true);
   m_health = node.attribute("hitPoints").as_float(1.0f);
   m_ammoAmount = node.attribute("ammoAmount").as_float(m_ammoAmount);
   m_shieldAmount = node.attribute("shieldAmount").as_float(m_shieldAmount);
   m_fuelAmount = node.attribute("fuelAmount").as_float(m_fuelAmount);
   m_creditsAmount = node.attribute("creditsAmount").as_float(m_creditsAmount);
   m_latchedShapeName = node.attribute("latchedShape").as_string("");
   m_flyingShapeName = node.attribute("flyingShape").as_string("");
   m_dropFile = node.attribute("dropFile").as_string("");

   // What it may take, e.g. steal="fuel ammo credits"
   string steal = node.attribute("steal").as_string("fuel ammo credits");
   istringstream words(steal);
   string w;

   while (words >> w)
   {
      if (w == "ammo") m_stealList.push_back(lootAmmo);
      else if (w == "shield") m_stealList.push_back(lootShield);
      else if (w == "fuel") m_stealList.push_back(lootFuel);
      else if (w == "credits") m_stealList.push_back(lootCredits);
      else logs::messageln("Stripper: unknown steal item '%s'", w.c_str());
   }
}

void Stripper::preSolve(b2Contact* contact)
{
   if ((BodyUserData::getCollisionType(contact->GetFixtureA()->GetUserData()) == CollisionEntity::stripper) &&
      (BodyUserData::getCollisionType(contact->GetFixtureB()->GetUserData()) == CollisionEntity::stripper))
   {
      contact->SetEnabled(false);
   }
}

const char* Stripper::lootName(int loot)
{
   switch (loot)
   {
   case lootAmmo: return "ammo";
   case lootShield: return "shield";
   case lootFuel: return "fuel";
   case lootCredits: return "credits";
   }

   return "nothing";
}

void Stripper::setState(StripperState state)
{
   m_state = state;
   m_stateTime = 0.0f;
   m_properties[propState].setProperty((float)state);

   if (g_HeadDownDisplay && (m_mapItemId >= 0))
   {
      switch (state)
      {
      case approach:
         g_HeadDownDisplay->setState(m_mapItemId, MapItem::MapItemStateEnum::flashingSlow);
         break;
      case latched:
         g_HeadDownDisplay->setState(m_mapItemId, MapItem::MapItemStateEnum::flashingFast);
         break;
      default:
         g_HeadDownDisplay->setState(m_mapItemId, MapItem::MapItemStateEnum::hollow);
         break;
      }
   }

   showStateShapes();
}

void Stripper::showStateShapes(void)
{
   if (m_latchedShapeName != "")
   {
      spActor a = getActor(m_latchedShapeName);
      if (a) a->setVisible(m_state == latched);
   }

   if (m_flyingShapeName != "")
   {
      spActor a = getActor(m_flyingShapeName);
      if (a) a->setVisible(m_state != latched);
   }
}

LeapFrog* Stripper::getLeapfrog(void)
{
   LeapFrog* lf = m_sceneActor->m_leapfrog;

   if (lf && lf->m_resources && !lf->m_isDead)
   {
      return lf;
   }

   return NULL;
}

b2Body* Stripper::getLeapfrogBody(LeapFrog* lf)
{
   if (lf == NULL)
   {
      return NULL;
   }

   return lf->getBody("lfMainBody");
}

// Is the shield bubble there? (it is switched off on the ground)
bool Stripper::shieldUp(LeapFrog* lf)
{
   return lf->m_shield &&
      lf->m_shield->m_body &&
      lf->m_shield->m_body->IsActive() &&
      (lf->m_resources->getShield() > 0.0f);
}

// The weld goes away with either body (Box2D destroys a body's joints),
// so check that it is still there before using the pointer.
bool Stripper::weldIsValid(void)
{
   if (m_weld == NULL)
   {
      return false;
   }

   for (b2JointEdge* je = m_body->GetJointList(); je; je = je->next)
   {
      if (je->joint == m_weld)
      {
         return true;
      }
   }

   m_weld = NULL;
   return false;
}

static bool isHullPart(CollisionEntity::CollisionEntityTypeEnum type)
{
   switch (type)
   {
   case CollisionEntity::leapfrog:
   case CollisionEntity::lfBigLeg:
   case CollisionEntity::lfSmallLeg:
   case CollisionEntity::lfBooster:
   case CollisionEntity::lfLeftFoot:
   case CollisionEntity::lfRightFoot:
   case CollisionEntity::lfSteerBooster:
      return true;
   default:
      return false;
   }
}

// Closest hull part of one Leapfrog along a ray
class StripperHullRayCallback : public b2RayCastCallback
{
public:
   LeapFrog* m_leapfrog;
   b2Fixture* m_fixture;
   b2Vec2 m_point;
   float m_fraction;

   StripperHullRayCallback(LeapFrog* lf) :
      m_leapfrog(lf), m_fixture(NULL), m_fraction(1.0f) {}

   float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float32 fraction) override
   {
      if (!isHullPart(BodyUserData::getCollisionType(fixture->GetUserData())) ||
         (BodyUserData::getParentObjectOfType<LeapFrog*>(fixture->GetBody()->GetUserData()) != m_leapfrog))
      {
         return -1.0f;     // not a part of this Leapfrog: go on
      }

      m_fixture = fixture;
      m_point = point;
      m_fraction = fraction;
      return fraction;     // keep looking for closer ones
   }
};

// How far its own shapes reach from its centre in a direction
float Stripper::reach(const b2Vec2& dir)
{
   float r = 0.0f;
   b2Vec2 c = m_body->GetWorldCenter();

   for (b2Fixture* f = m_body->GetFixtureList(); f; f = f->GetNext())
   {
      b2Shape* s = f->GetShape();

      if (s->GetType() == b2Shape::e_polygon)
      {
         b2PolygonShape* p = static_cast<b2PolygonShape*>(s);
         for (int i = 0; i < p->m_count; i++)
         {
            r = max(r, b2Dot(m_body->GetWorldPoint(p->m_vertices[i]) - c, dir));
         }
      }
      else if (s->GetType() == b2Shape::e_circle)
      {
         b2CircleShape* cs = static_cast<b2CircleShape*>(s);
         r = max(r, b2Dot(m_body->GetWorldPoint(cs->m_p) - c, dir) + cs->m_radius);
      }
   }

   return r;
}

// Is it within latchDistance of a hull part of the Leapfrog (main body,
// legs, feet, boosters; not the shield), looking towards its centre, or
// touching one? Then point is where to grip and part the body to hold.
bool Stripper::findHull(LeapFrog* lf, b2Body* lfBody, b2Vec2& point, b2Body*& part)
{
   b2Vec2 from = m_body->GetWorldCenter();
   b2Vec2 to = lfBody->GetWorldCenter();
   b2Vec2 dir = to - from;
   float len = dir.Normalize();

   if (len > 0.001f)
   {
      StripperHullRayCallback cb(lf);
      m_world->RayCast(&cb, from, to);

      if (cb.m_fixture && (cb.m_fraction * len - reach(dir) <= m_latchDistance))
      {
         point = cb.m_point;
         part = cb.m_fixture->GetBody();
         return true;
      }
   }

   // Touching a part the ray didn't see (e.g. a leg beside it)
   for (b2ContactEdge* ce = m_body->GetContactList(); ce; ce = ce->next)
   {
      b2Contact* c = ce->contact;

      if (!c->IsTouching() || !c->IsEnabled())
      {
         continue;
      }

      b2Fixture* other = (c->GetFixtureA()->GetBody() == m_body) ? c->GetFixtureB() : c->GetFixtureA();

      if (!isHullPart(BodyUserData::getCollisionType(other->GetUserData())) ||
         (BodyUserData::getParentObjectOfType<LeapFrog*>(other->GetBody()->GetUserData()) != lf))
      {
         continue;
      }

      b2WorldManifold wm;
      c->GetWorldManifold(&wm);
      point = (c->GetManifold()->pointCount > 0) ? wm.points[0] : m_body->GetPosition();

      // Weld to the part it hit (a leg too: then the leg carries it)
      part = other->GetBody();
      return true;
   }

   return false;
}

// Is it touching the Leapfrog's shield bubble?
bool Stripper::touchingShield(LeapFrog* lf)
{
   b2Body* sb = lf->m_shield ? lf->m_shield->m_body : NULL;

   if (sb == NULL)
   {
      return false;
   }

   for (b2ContactEdge* ce = m_body->GetContactList(); ce; ce = ce->next)
   {
      if ((ce->other == sb) && ce->contact->IsTouching() && ce->contact->IsEnabled())
      {
         return true;
      }
   }

   return false;
}

void Stripper::latchOn(LeapFrog* lf, b2Body* partBody, const b2Vec2& point)
{
   b2WeldJointDef wd;
   wd.Initialize(partBody, m_body, point);
   wd.collideConnected = false;

   m_weld = static_cast<b2WeldJoint*>(m_world->CreateJoint(&wd));
   m_gripLocal = partBody->GetLocalPoint(point);

   // While latched it shall not collide with any part of the Leapfrog
   // (the weld holds it overlapping the hull): join the Leapfrog's
   // non-colliding group.
   b2Body* lfBody = getLeapfrogBody(lf);
   int16 lfGroup = 0;

   if (lfBody && lfBody->GetFixtureList())
   {
      lfGroup = lfBody->GetFixtureList()->GetFilterData().groupIndex;
   }

   if (lfGroup < 0)
   {
      for (b2Fixture* f = m_body->GetFixtureList(); f; f = f->GetNext())
      {
         b2Filter filt = f->GetFilterData();
         filt.groupIndex = lfGroup;
         f->SetFilterData(filt);
      }
   }

   m_sparkTime = 0.0f;
   m_strainTime = 0.0f;
   setState(latched);
   spark(point);
}

// Lets go of the Leapfrog. It stays in the Leapfrog's collision group
// (it would be pushed out of the hull, knocking the Leapfrog) until it
// approaches again (restoreFilters).
void Stripper::letGo(void)
{
   if (weldIsValid())
   {
      m_world->DestroyJoint(m_weld);
   }

   m_weld = NULL;
}

void Stripper::restoreFilters(void)
{
   int i = 0;
   for (b2Fixture* f = m_body->GetFixtureList(); f && i < m_numOwnFilters; f = f->GetNext(), i++)
   {
      f->SetFilterData(m_ownFilters[i]);
   }
}

// Takes one of the resources in the steal list that the Leapfrog has,
// picked at random. Returns what it took (lootNone if nothing).
int Stripper::takeLoot(LeapFrog* lf)
{
   ObjectResources* r = lf->m_resources;
   vector<int> candidates;

   for (auto it = m_stealList.begin(); it != m_stealList.end(); ++it)
   {
      switch (*it)
      {
      case lootAmmo:    if (r->getShots() > 0) candidates.push_back(*it); break;
      case lootShield:  if (r->getShield() > 0.0f) candidates.push_back(*it); break;
      case lootFuel:    if (r->getFuel() > 0.0f) candidates.push_back(*it); break;
      case lootCredits: if (r->getCredits() > 0) candidates.push_back(*it); break;
      }
   }

   if (candidates.empty())
   {
      return lootNone;
   }

   int loot = candidates[rand() % candidates.size()];
   float taken = 0.0f;

   switch (loot)
   {
   case lootAmmo:
   {
      int n = min(r->getShots(), (int)m_ammoAmount);
      r->deltaShots(-n);
      taken = (float)n;
      break;
   }
   case lootShield:
      taken = min(r->getShield(), m_shieldAmount);
      r->deltaShield(-taken);
      break;
   case lootFuel:
      taken = min(r->getFuel(), m_fuelAmount);
      r->deltaFuel(-taken);
      break;
   case lootCredits:
   {
      int n = min(r->getCredits(), (int)m_creditsAmount);
      r->deltaCredits(-n);
      taken = (float)n;
      break;
   }
   }

   if (g_HeadDownDisplay)
   {
      char msg[64];
      snprintf(msg, sizeof(msg), "Stripper took %d %s", (int)(taken + 0.5f), lootName(loot));
      g_HeadDownDisplay->logMessage(msg, true);
   }

   m_lootAmount = taken;

   LeapfrogStrippedEvent ev(loot, taken);
   lf->dispatchEvent(&ev);

   return loot;
}

// Velocity steering with a limited acceleration
void Stripper::steerTowards(const b2Vec2& desiredVelocity, float dt)
{
   b2Vec2 change = desiredVelocity - m_body->GetLinearVelocity();
   float maxChange = m_acceleration * dt;

   if (change.Length() > maxChange)
   {
      change.Normalize();
      change *= maxChange;
   }

   m_body->ApplyLinearImpulseToCenter(m_body->GetMass() * change, true);
}

// Turns the nose towards direction, at most m_turnRate
void Stripper::faceDirection(const b2Vec2& direction, float dt)
{
   if (direction.LengthSquared() < 0.0001f)
   {
      return;
   }

   float desired = atan2f(direction.y, direction.x) - m_noseAngle;
   float err = desired - m_body->GetAngle();

   while (err < -MATH_PI) err += 2.0f * MATH_PI;
   while (err > MATH_PI) err -= 2.0f * MATH_PI;

   // Reach the angle in about 0.15 s, no faster than the turn rate
   float w = err / 0.15f;
   w = max(-m_turnRate, min(m_turnRate, w));

   m_body->ApplyAngularImpulse(m_body->GetInertia() * (w - m_body->GetAngularVelocity()), true);
}

void Stripper::cancelGravity(void)
{
   if (m_antiGravity)
   {
      m_body->ApplyForceToCenter(-m_body->GetMass() * m_world->GetGravity(), true);
   }
}

void Stripper::spark(const b2Vec2& worldPoint)
{
   spBlastEmitter blast = new BlastEmitter(
      PhysDispConvert::convert(worldPoint, 1.0f),
      200.0f,     // particles / s
      100,        // emitter lifetime, ms
      300,        // particle lifetime, ms
      12.0f,      // particle distance
      0.5f);      // particle size
   blast->attachTo(m_sceneActor);
}

void Stripper::vanish(void)
{
   if (m_state == gone)
   {
      return;
   }

   letGo();
   setState(gone);
   m_isDead = true;

   if (g_HeadDownDisplay && (m_mapItemId >= 0))
   {
      g_HeadDownDisplay->removeMeFromMap(m_mapItemId);
      m_mapItemId = -1;
   }

   addMeToDeathList();
}

void Stripper::hitByBullet(b2Contact* contact, float bulletEqvDamage)
{
   if ((m_state == gone) || (m_body == NULL))
   {
      return;
   }

   m_health -= bulletEqvDamage;

   spark(m_body->GetPosition());

   // Called during the physics step (contact listener), when joints can't
   // be destroyed: it goes in the next update.
   if (m_health <= 0.0f)
   {
      m_shotDown = true;
   }
}

// Shot down with loot: drop the pickup in dropFile, made to give back
// what it took.
void Stripper::dropLoot(void)
{
   if ((m_loot == lootNone) || (m_lootAmount <= 0.0f) || (m_dropFile == ""))
   {
      return;
   }

   // "{loot}" in the name becomes ammo, shield, fuel or credits, so each
   // kind can have its own look: dropFile="stripper_drop_{loot}.xml"
   string fileName = m_dropFile;
   size_t at = fileName.find("{loot}");
   if (at != string::npos)
   {
      fileName.replace(at, 6, lootName(m_loot));
   }

   if (!ox::file::exists(fileName))
   {
      logs::messageln("Stripper: drop file %s not found", fileName.c_str());
      return;
   }

   ox::file::buffer bf;
   ox::file::read(fileName.c_str(), bf);

   xml_document fileDoc;
   if (!fileDoc.load_buffer(&bf.data[0], bf.size()))
   {
      return;
   }

   // <spawnObject><childObject><compoundObject .../></childObject></spawnObject>
   xml_document spawnDoc;
   xml_node so = spawnDoc.append_child("spawnObject");
   so.append_attribute("probabilityFactor") = 1;
   xml_node cho = so.append_child("childObject");
   cho.append_attribute("posX") = 0;
   cho.append_attribute("posY") = 0;
   xml_node co = cho.append_copy(fileDoc.child("compoundObject"));

   xml_node props = co.child("behaviour").child("pickupObjectProperties");

   if (props.empty())
   {
      logs::messageln("Stripper: %s is not a pickupObject", fileName.c_str());
      return;
   }

   const char* type = "resourceFuel";
   switch (m_loot)
   {
   case lootAmmo: type = "resourceAmmo"; break;
   case lootShield: type = "resourceShield"; break;
   case lootFuel: type = "resourceFuel"; break;
   case lootCredits: type = "resourceCoin"; break;
   }

   auto setAttr = [&](const char* name, const char* value)
   {
      xml_attribute a = props.attribute(name);
      if (!a) a = props.append_attribute(name);
      a.set_value(value);
   };

   char amount[32];
   snprintf(amount, sizeof(amount), "%g", m_lootAmount);

   setAttr("pickupType", type);
   setAttr("amountType", "fix");
   setAttr("amount", amount);

   spSpawnObjectList list = new SpawnObjectList();
   list->readSpawnObjectsNode(spawnDoc);

   Vector2 p = PhysDispConvert::convert(m_body->GetPosition(), 1.0f);
   m_sceneActor->addObjectToSpawnList(1, p - Vector2(1.0f, 1.0f), Vector2(2.0f, 2.0f), list);

   m_loot = lootNone;
}

void Stripper::doUpdate(const oxygine::UpdateState& us)
{
   if ((m_body == NULL) || (m_state == gone) || g_MainActor->isInPause())
   {
      return;
   }

   if (m_shotDown)
   {
      dropLoot();

      spBlastEmitter blast = new BlastEmitter(
         PhysDispConvert::convert(m_body->GetPosition(), 1.0f),
         300.0f, 200, 500, 40.0f, 0.8f);
      blast->attachTo(m_sceneActor);

      vanish();
      return;
   }

   float dt = us.dtf;
   m_stateTime += dt;

   b2Vec2 pos = m_body->GetPosition();
   m_properties[propXPos].setProperty(pos.x);
   m_properties[propYPos].setProperty(pos.y);

   LeapFrog* lf = getLeapfrog();
   b2Body* lfBody = getLeapfrogBody(lf);

   if ((m_state == latched) && (!weldIsValid() || (lfBody == NULL)))
   {
      // The part it held on to is gone
      letGo();
      setState(stunned);
   }

   switch (m_state)
   {
   case approach:
   {
      cancelGravity();

      if (lfBody == NULL)
      {
         // Nobody to strip: slow down and wait
         steerTowards(b2Vec2(0.0f, 0.0f), dt);
         break;
      }

      bool shield = shieldUp(lf);
      b2Vec2 point;
      b2Body* part = NULL;

      if (!shield && findHull(lf, lfBody, point, part))
      {
         latchOn(lf, part, point);
         break;
      }

      if (shield && touchingShield(lf))
      {
         // Rammed the shield: it drains it (on top of the knock itself),
         // bounces back, then comes again
         lf->m_resources->deltaShield(-m_shieldDrain);

         b2Vec2 away = pos - lfBody->GetPosition();
         away.Normalize();
         m_body->SetLinearVelocity(lfBody->GetLinearVelocity() + 0.8f * m_ramSpeed * away);
         setState(stunned);
         break;
      }

      b2Vec2 lfPos = lfBody->GetPosition();
      b2Vec2 lfVel = lfBody->GetLinearVelocity();
      b2Vec2 toTarget = lfPos - pos;
      float dist = toTarget.Length();

      // Far: full speed towards the point where it can meet the
      // Leapfrog (up to m_lead s ahead). Head-on, that point is in front
      // of it, so it flies straight into the Leapfrog's path.
      b2Vec2 meet = SteeringManager::interceptPoint(pos, m_maxSpeed, lfPos, lfVel, m_lead);
      b2Vec2 chase = meet - pos;
      chase.Normalize();
      chase *= m_maxSpeed;

      // Close: move sideways with the Leapfrog and close in on it at
      // ramSpeed (shield up) or latchSpeed (shield down). If the Leapfrog
      // is already coming at it faster than that, it holds its ground
      // (does not back off) and grabs it as it arrives.
      b2Vec2 dir = toTarget;
      dir.Normalize();
      float lfAlong = b2Dot(lfVel, dir);
      b2Vec2 lfSide = lfVel - lfAlong * dir;
      float closing = shield ? m_ramSpeed : m_latchSpeed;
      b2Vec2 close = lfSide + max(0.0f, lfAlong + closing) * dir;

      // Full chase beyond brakeDistance from the Leapfrog's centre, all
      // close-in at half that
      float half = 0.5f * m_brakeDistance;
      float f = (half > 0.0f) ? (dist - half) / half : 1.0f;
      f = max(0.0f, min(1.0f, f));

      b2Vec2 desired = f * chase + (1.0f - f) * close;

      steerTowards(desired, dt);

      // Arms forward: along the chase far out, at the Leapfrog close in
      faceDirection((f > 0.5f) ? chase : toTarget, dt);
      break;
   }

   case latched:
   {
      b2Body* part = m_weld->GetBodyA();
      b2Vec2 grip = part->GetWorldPoint(m_gripLocal);

      // Shaken off by hard manoeuvres: the weld has carried more than
      // gripForce for gripTime s in a row. (Not in the first 0.25 s: the
      // weld takes a short hard tug when it stops the stripper's own speed.)
      if ((m_gripForce > 0.0f) && (m_stateTime > 0.25f) &&
         (m_weld->GetReactionForce(1.0f / Scales::c_updateStep).Length() > m_gripForce))
      {
         m_strainTime += dt;
      }
      else
      {
         m_strainTime = 0.0f;
      }

      if ((m_gripForce > 0.0f) && (m_strainTime >= m_gripTime))
      {
         letGo();
         spark(grip);
         setState(stunned);
         break;
      }

      // Blown off when the shield comes back
      if (shieldUp(lf))
      {
         letGo();
         b2Vec2 away = pos - lfBody->GetPosition();
         away.Normalize();
         m_body->ApplyLinearImpulseToCenter(m_body->GetMass() * m_maxSpeed * away, true);
         spark(grip);
         setState(stunned);
         break;
      }

      m_sparkTime += dt;

      if ((m_sparkInterval > 0.0f) && (m_sparkTime >= m_sparkInterval))
      {
         m_sparkTime = 0.0f;
         spark(grip);
      }

      if (m_stateTime >= m_stealTime)
      {
         m_loot = takeLoot(lf);

         letGo();

         // Push off from the hull
         b2Vec2 away = pos - lfBody->GetPosition();
         away.Normalize();
         m_body->SetLinearVelocity(lfBody->GetLinearVelocity() + 0.5f * m_leaveSpeed * away);

         setState(leave);

         // Fade out while it leaves
         for (auto it = m_shapes.begin(); it != m_shapes.end(); ++it)
         {
            (*it)->addTween(Actor::TweenAlpha(0), (timeMS)(m_leaveTime * 1000.0f));
         }
      }
      break;
   }

   case stunned:
      // Tumbles without thrust (but still hovers), then tries again
      cancelGravity();

      if (m_stateTime >= m_stunTime)
      {
         restoreFilters();
         setState(approach);
      }
      break;

   case leave:
   {
      cancelGravity();

      b2Vec2 away = (lfBody != NULL) ? (pos - lfBody->GetPosition()) : m_body->GetLinearVelocity();
      if (away.LengthSquared() < 0.0001f) away = b2Vec2(0.0f, -1.0f);
      away.Normalize();

      steerTowards(m_leaveSpeed * away, dt);
      faceDirection(away, dt);

      if (m_stateTime >= m_leaveTime)
      {
         vanish();
      }
      break;
   }

   default:
      break;
   }
}
