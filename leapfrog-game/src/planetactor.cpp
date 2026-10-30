#include "planetactor.h"
#include "sceneactor.h"
#include "layout.h"
#include "graphicresources.h"
#include "headdowndisplay.h"
#include "polygonvertices.h"

using namespace oxygine;
using namespace pugi;

// The planet turns once in 90 s in orbit
const float PlanetActor::c_orbitSpinPerMs = 2.0f * MATH_PI / 90000.0f;

// Arriving in orbit: the planet's spin-up and the Leapfrog marker's braking
// take as long (the marker's tween used to take 6 s too)
const float PlanetActor::c_spinUpMs = 6000.0f;

// A site's solution counts as missed when its predicted marker is this far
// past it (about 180 planet units, nearly two marker widths)
const float PlanetActor::c_missedAngle = 3.0f / 180.0f * MATH_PI;

// The planet's radius in its picture's units (the disc is 7000 wide)
const float PlanetActor::c_surfaceRadius = 3500.0f;

// Where the marker comes in from: right of the picture (it shows 50..2100)
const float PlanetActor::c_arrivalStartX = 2250.0f;

PlanetActor::PlanetActor(
   CompoundObject* parentObject,
   const xml_node& objectNode) :
   CompoundObject((SceneActor*)this, parentObject),
   m_parentObject(parentObject),
   m_state(PAS_INITITAL),
   m_stateChange(true),
   m_orbitStartPos(Vector2(500.0f, 150.0f)),
   // 11.4 seconds from start of burn to normal landing
   // Planet turn at 0.0698 rad/s
   // angle offset is 0.0698 * 11.4 = 0.796
   c_predictedAngleOffset(0.796f),
   c_startAngleOffset(-100.0f / 180.0f * MATH_PI),
   c_landingPointAngle(-1.142f),
   c_angleZeroRotation(0.952f + 100.0f / 180.0f * MATH_PI),
   c_safeLandingMaxRadDeviation(100.0f / 3500.0f / 2.0f),
   m_spinElapsedMs(0.0f),
   m_revolutionPending(false)
{
   //// I should probably load resources that are uniuqe here
   //setPanorateMode(topLeft);

   m_planet = new Actor();
   m_planet->attachTo(this);
   m_planet->setAnchor(0.5f, 0.5f);
   m_planet->setPosition(Vector2(500.0f, 4500.0f));

   spMaskedSprite maskedSprite = new MaskedSprite();
   maskedSprite->attachTo(m_planet);
   maskedSprite->setAnchor(0.5f, 0.5f);
   maskedSprite->setPosition(0.0f, 0.0f);

   // The surface layer, over the planet's disc (see m_surface)
   m_surface = new Actor();
   m_surface->setPriority(1);
   m_surface->attachTo(m_planet);

   // The sites and the surface's art; the planet's texture, unless the
   // surface file gives its own
   std::string texture = objectNode.attribute("texture").as_string();
   float tileWidth = 0.0f;
   float tileHeight = 0.0f;
   loadSurface(objectNode, texture, tileWidth, tileHeight);

   // Where the orbit will be established: the planet spins up over
   // c_spinUpMs and the orbit is established about 12 s after the start
   // (OrbitSpaceScene), so it has turned about as far as 9 s at full
   // speed. orbitEstablished corrects it (by about a frame's turn).
   m_surface->setRotation(surfaceRotationFor(c_orbitSpinPerMs * 9000.0f));

   spBox9Sprite planetSprite = new Box9Sprite();
   planetSprite->setResAnim(g_GraphRes.getResAnim(texture.c_str()));
   // The texture's tiles: the surface file's size (textureMeterWidth and
   // Height, in planet units), or the texture's own size in pixels. The
   // tiling is done at the texture's size, so the sprite is scaled to make
   // the tiles that big, and sized to still cover the disc.
   float tileScaleX = 1.0f;
   float tileScaleY = 1.0f;
   ResAnim* groundAnim = g_GraphRes.getResAnim(texture.c_str());
   if ((groundAnim != NULL) && (groundAnim->getWidth() > 0.0f) && (groundAnim->getHeight() > 0.0f))
   {
      if (tileWidth > 0.0f)
      {
         tileScaleX = tileWidth / groundAnim->getWidth();
         tileScaleY = tileScaleX;
      }
      if (tileHeight > 0.0f)
      {
         tileScaleY = tileHeight / groundAnim->getHeight();
      }
   }
   planetSprite->setSize(7000.0f / tileScaleX, 7000.0f / tileScaleY);
   planetSprite->setScale(tileScaleX, tileScaleY);
   planetSprite->setAnchor(0.5f, 0.5f);
   planetSprite->setGuides(0, 0, 0, 0);
   planetSprite->setHorizontalMode(Box9Sprite::TILING_FULL);
   planetSprite->setVerticalMode(Box9Sprite::TILING_FULL);
   planetSprite->attachTo(maskedSprite);

   spSprite mask = new Sprite();
   mask->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::game).getResAnim("planet_mask"));
   mask->setSize(7000.0f, 7000.0f);
   mask->setAnchor(0.5f, 0.5f);
   mask->attachTo(maskedSprite);
   mask->setVisible(false);

   // Test
   spColorRectSprite mask2 = new ColorRectSprite();
   mask2->setColor(Color::Black);
   mask2->setSize(6000.0f, 6000.0f);
   mask2->setAnchor(0.5f, 0.5f);
   mask2->attachTo(mask);

   maskedSprite->setMask(mask, true);

   setAnchor(0.5f, 0.5f);

   // The planet, the trajectory and the markers are drawn in the head-down
   // display, which in orbit sits between the message display and the
   // booster/fire buttons (HeadDownDisplay::orbitRect). Fit the part of
   // the picture that matters into its room (orbitPictureRect: left of
   // the burn bar), and clip the planet to the display. The space view
   // (OrbitScene) is an inset in the display's top right corner, over
   // the sky above the trajectory.
   float bw = g_Layout.getButtonWidth();
   RectF hdd = HeadDownDisplay::orbitRect();

   // Inside the display's frame
   float frame = bw * 0.08f;
   spClipRectActor clip = new ClipRectActor();
   clip->setPosition(hdd.getX() + frame, hdd.getY() + frame);
   clip->setSize(hdd.getWidth() - 2.0f * frame, hdd.getHeight() - 2.0f * frame);
   clip->attachTo(m_parentObject);

   RectF room = HeadDownDisplay::orbitPictureRect();
   float roomLeft = room.getX();
   float roomRight = room.getRight();
   float roomTop = room.getY();
   float roomBottom = room.getBottom();

   // What must be seen, in the planet's units: the Leapfrog's start (100,
   // 150), the trajectory (m_orbitStartPos + up to (1498, 1252)), the site
   // markers on the surface where it ends, and a strip of the planet below
   float viewLeft = 50.0f;
   float viewTop = 100.0f;
   float viewWidth = 2050.0f;
   float viewHeight = 1650.0f;

   float roomWidth = roomRight - roomLeft;
   float roomHeight = roomBottom - roomTop;
   float scale = fmin(roomWidth / viewWidth, roomHeight / viewHeight);

   // The space view covers the room's top right corner. Where the
   // trajectory would run under it (a narrow display, as with large
   // buttons on a 16:9 screen), the picture goes to the room's bottom
   // left instead of its centre, and if that isn't enough it shrinks
   // until the trajectory passes clear of the view.
   RectF spaceView = HeadDownDisplay::orbitSpaceViewRect();
   float clearance = bw * 0.15f;
   RectF keepOut(
      spaceView.getX() - clearance,
      spaceView.getY() - clearance,
      spaceView.getWidth() + 2.0f * clearance,
      spaceView.getHeight() + 2.0f * clearance);

   auto trajectoryUnderView = [&](float s, float px, float py)
   {
      // Trajectory sprites are 50 x 8 units from their position
      for (int i = 0; i < 44; i++)
      {
         Vector2 p = c_trajectoryPositions[i] + m_orbitStartPos;
         RectF dot(px + p.x * s, py + (p.y - 4.0f) * s, 50.0f * s, 8.0f * s);
         if (keepOut.isIntersecting(dot))
         {
            return true;
         }
      }
      return false;
   };

   // Centred in the room; positions are in the screen's coordinates
   float x = roomLeft + (roomWidth - viewWidth * scale) / 2.0f - viewLeft * scale;
   float y = roomTop + (roomHeight - viewHeight * scale) / 2.0f - viewTop * scale;

   if (trajectoryUnderView(scale, x, y))
   {
      for (int step = 0; step < 40; step++)
      {
         // Bottom left of the room
         x = roomLeft - viewLeft * scale;
         y = roomBottom - (viewHeight + viewTop) * scale;

         if (!trajectoryUnderView(scale, x, y))
         {
            break;
         }

         scale *= 0.98f;
      }
   }

   setPosition(x - clip->getX(), y - clip->getY());
   setScale(scale);

   attachTo(clip);

   m_positionIndicator = new Sprite();
   m_positionIndicator->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("lf_position"));
   m_positionIndicator->setSize(40.0f, 40.0f);
   m_positionIndicator->setAnchor(0.5f, 0.5f);
   // Arriving: the Leapfrog brakes into orbit. It comes in from the right,
   // beyond the picture (past the space view), and slows to the
   // trajectory's start while the planet, still at first, spins up to its
   // orbit speed under it (see doUpdate): in the orbit the Leapfrog stays
   // put and the ground moves on (the planet turns clockwise, so the
   // Leapfrog travels leftwards over it, as the stars in the space view
   // show), and arriving it is the Leapfrog that moves and the planet
   // that starts still. Its speed over the ground goes down smoothly from
   // the arrival to the orbit speed.
   m_positionIndicator->setPosition(Vector2(c_arrivalStartX, 150.0f));
   m_positionIndicator->attachTo(this);

   m_positionIndicator->addTween(Actor::TweenPosition(c_trajectoryPositions[0] + m_orbitStartPos), c_spinUpMs, 1, false, 0, Tween::ease_outQuad);

   // The planet's rotation is advanced in doUpdate (it used to be a tween
   // turning at the orbit speed from the start)

   m_spaceSceneBackground = objectNode.attribute("spaceSceneBackground").as_string();
   m_spaceSceneFile = objectNode.attribute("spaceSceneFile").as_string();

}


void PlanetActor::doUpdate(const UpdateState& us)
{
   // The planet's spin: from still to the orbit speed over c_spinUpMs, as
   // the Leapfrog brakes into orbit (its marker's ease-out tween takes as
   // long), then steady. The orbit speed is what the predicted sites and
   // the re-entry timing were made for (a turn in 90 s), and orbit is
   // established (OrbitSpaceScene) well after the spin-up.
   float dtMs = us.dtf * 1000.0f;
   m_spinElapsedMs += dtMs;
   float spinUp = fmin(1.0f, m_spinElapsedMs / c_spinUpMs);
   m_planet->setRotation(m_planet->getRotation() + c_orbitSpinPerMs * spinUp * dtMs);

   //if (m_stateChange)
   //{
   //   logs::message("State changed in %d, ", us.time);

   //   switch (m_state)
   //   {
   //   case PAS_INITITAL:
   //      logs::messageln("PAS_INITITAL");
   //      break;
   //   case PAS_ORBIT_ESTABLISHED:
   //      logs::messageln("PAS_ORBIT_ESTABLISHED");
   //      break;
   //   case PAS_BURN:
   //      logs::messageln("PAS_BURN");
   //      break;
   //   case PAS_REENTRY:
   //      logs::messageln("PAS_REENTRY");
   //      break;
   //   case PAS_LANDED:
   //      logs::messageln("PAS_LANDED");
   //      break;
   //   }
   //   m_stateChange = false;
   //}

   if (m_state == PAS_BURN)
   {
      m_burnIndicator->setProgress(m_burnIndicator->getProgress() + 0.01f);
   }
   else if (m_state == PAS_REENTRY)
   {
//      logs::messageln("Time in reentry: %d", us.time);
   }
}

void PlanetActor::orbitEstablished(void)
{
   m_planetAngleAtOrbitEstablished = m_planet->getRotation();
   m_state = PAS_ORBIT_ESTABLISHED;
   m_stateChange = true;

   // The sites exactly where the re-entry timing expects them now
   m_surface->setRotation(surfaceRotationFor(m_planetAngleAtOrbitEstablished));
   placeSolutionMarkers(false);

   // generate the trajectory, without angles set to zero, to begin with
   spSprite trajectory[44];

   for (int i = 0; i < 44; i++)
   {
      trajectory[i] = new Sprite();
      trajectory[i]->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("trajectory"));
      trajectory[i]->setSize(50.0f, 8.0f);
      trajectory[i]->setAnchor(0.0f, 0.5f);
      trajectory[i]->setPosition(c_trajectoryPositions[i] + m_orbitStartPos);
      trajectory[i]->setRotation(c_trajectoryAngles[i]);
      trajectory[i]->attachTo(this);
   }

   m_positionIndicator->setPosition(c_trajectoryPositions[0] + m_orbitStartPos);

   // At the orbit display's right edge, under the space view
   RectF barRect = HeadDownDisplay::orbitBurnBarRect();
   float barHeight = barRect.getHeight();
   float barVertCenter = barRect.getY() + barHeight / 2.0f;
   float barWidth = barHeight * 0.2f;
   float barHorCenter = barRect.getX() + barRect.getWidth() / 2.0f;
   float barCaretWidth = barHeight * 0.3f;
   float barCaretHeight = barHeight * 0.068f;
   float barCaretVertCenter = barVertCenter - barHeight * 0.2f; // ALso center of safe zone
   float barSafeZoneHeight = barHeight * 0.2f;
   float barInsideFrameWidth = barWidth - barHeight * 0.008f;
   float barInsideFrameHeight = barHeight - barHeight * 0.008f;

   spBox9Sprite burnFrame = new Box9Sprite();
   burnFrame->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_thin"));
   burnFrame->setVerticalMode(Box9Sprite::TILING_FULL);
   burnFrame->setHorizontalMode(Box9Sprite::TILING_FULL);
   burnFrame->setAnchor(0.5f, 0.5f);
   burnFrame->setSize(barWidth, barHeight);
   burnFrame->setPosition(barHorCenter, barVertCenter);
   burnFrame->setGuides(8, 120, 8, 120);
   burnFrame->attachTo(m_parentObject);

   //spBox9Sprite burnHaze = new Box9Sprite();
   //burnHaze->setVerticalMode(Box9Sprite::STRETCHING);
   //burnHaze->setHorizontalMode(Box9Sprite::STRETCHING);
   //burnHaze->setResAnim(m_gameResources->getResAnim("display_thin"));
   //burnHaze->setGuides(8, 120, 8, 120);
   //burnHaze->setAnchor(0.5f, 0.5f);
   //burnHaze->setSize(barInsideFrameWidth, barInsideFrameHeight);
   //burnHaze->setColor(Color::Purple);
   //burnHaze->setPosition(barHorCenter, barVertCenter);
   //burnHaze->setAlpha(64);
   //burnHaze->attachTo(m_parentObject);

   spBox9Sprite safeZoom = new Box9Sprite();
   safeZoom->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("display_thin"));
   safeZoom->setVerticalMode(Box9Sprite::TILING_FULL);
   safeZoom->setHorizontalMode(Box9Sprite::TILING_FULL);
   safeZoom->setGuides(8, 120, 8, 120);
   safeZoom->setAnchor(0.5f, 0.5f);
   safeZoom->setSize(barInsideFrameWidth, barSafeZoneHeight);
   safeZoom->setPosition(barHorCenter, barCaretVertCenter);
   safeZoom->attachTo(m_parentObject);

   m_burnIndicator = new ProgressBar();
   m_burnIndicator->setAnchor(0.5f, 0.5f);
   m_burnIndicator->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("progress_bar"));
   m_burnIndicator->setSize(barInsideFrameWidth, barInsideFrameHeight);
   m_burnIndicator->setDirection(ProgressBar::dir_90);
   m_burnIndicator->setPosition(barHorCenter, barVertCenter);
   m_burnIndicator->attachTo(m_parentObject);
   m_burnIndicator->setProgress(0.0f);

   spSprite level = new Sprite();
   level->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("reentry_burn_level"));
   level->setAnchor(0.5f, 0.5f);
   level->setSize(barCaretWidth, barCaretHeight);
   level->setPosition(barHorCenter, barCaretVertCenter);
   level->attachTo(m_parentObject);
}


// The surface layer's rotation that puts each site where the re-entry
// timing expects it (its solution marker at c_startAngleOffset, the site
// c_predictedAngleOffset behind) with the planet turned planetRotation
float PlanetActor::surfaceRotationFor(float planetRotation)
{
   return c_startAngleOffset - c_predictedAngleOffset - planetRotation;
}

// The landing sites, and the art of the planet's surface. The orbit
// scene's properties name the surface file (surfaceFile); without one, the
// sites are in the properties (<landingSites>), and there is no art.
void PlanetActor::loadSurface(const pugi::xml_node& orbitProperties, std::string& texture, float& tileWidth, float& tileHeight)
{
   pugi::xml_document doc;
   pugi::xml_node sitesNode = orbitProperties.child("landingSites");
   pugi::xml_node root;

   std::string surfaceFile = orbitProperties.attribute("surfaceFile").as_string();

   if (!surfaceFile.empty())
   {
      ox::file::buffer bf;

      if (ox::file::exists(surfaceFile.c_str()))
      {
         ox::file::read(surfaceFile.c_str(), bf);
      }

      if ((bf.size() > 0) && doc.load_buffer(&bf.data[0], bf.size()))
      {
         root = doc.child("compoundObject");
         pugi::xml_node props = root.child("behaviour").child("planetSurfaceProperties");
         sitesNode = props;

         std::string surfaceTexture = props.attribute("texture").as_string();
         if (!surfaceTexture.empty())
         {
            texture = surfaceTexture;
         }

         tileWidth = props.attribute("textureMeterWidth").as_float(0.0f);
         tileHeight = props.attribute("textureMeterHeight").as_float(0.0f);
      }
      else
      {
         logs::warning("Planet: can't read the surface file %s", surfaceFile.c_str());
      }
   }

   for (pugi::xml_node node = sitesNode.child("landingSite"); node; node = node.next_sibling("landingSite"))
   {
      LandingSite ls(node);
      m_landingSites.push_back(ls);

      if (!ls.m_texture.empty())
      {
         spSprite marker = new Sprite();
         marker->setResAnim(g_GraphRes.getResAnim(ls.m_texture));
         marker->setSize(100.0f, 30.0f);
         marker->setAnchor(0.5f, 0.5f);
         marker->setPosition(Vector2(c_surfaceRadius * cos(ls.m_angle), c_surfaceRadius * sin(ls.m_angle)));
         marker->setRotation(ls.m_angle + MATH_PI / 2.0f);
         marker->attachTo(m_surface);
      }
   }

   if (root)
   {
      for (pugi::xml_node node = root.child("spriteBox"); node; node = node.next_sibling("spriteBox"))
      {
         placeSurfaceSprite(node);
      }

      for (pugi::xml_node node = root.child("spritePolygon"); node; node = node.next_sibling("spritePolygon"))
      {
         placeSurfacePolygon(node);
      }
   }
}

// A spriteBox of the unrolled surface on the planet: its centre (posX,
// posY) at the angle posX / radius, height -posY over the ground, turned
// so its top faces away from the planet (plus its own angle, degrees).
// Sprites are not bent: under a 600 wide one the surface sags only ~13.
void PlanetActor::placeSurfaceSprite(pugi::xml_node& node)
{
   spSprite sprite = new Sprite();
   doCommonShapeDefinitions(sprite.get(), node);
   sprite->setSize(node.attribute("width").as_float(), node.attribute("height").as_float());
   sprite->setAnchor(0.5f, 0.5f);

   float a = node.attribute("posX").as_float() / c_surfaceRadius;
   float r = c_surfaceRadius - node.attribute("posY").as_float();
   sprite->setPosition(Vector2(r * cos(a), r * sin(a)));
   sprite->setRotation(a + MATH_PI / 2.0f + node.attribute("angle").as_float() * MATH_PI / 180.0f);
   sprite->attachTo(m_surface);
}

// A point of the unrolled surface (x along the ground, y the height,
// negative above it) on the planet, in the surface layer's coordinates
Vector2 PlanetActor::bendToSurface(const Vector2& stripPoint)
{
   float a = stripPoint.x / c_surfaceRadius;
   float r = c_surfaceRadius - stripPoint.y;
   return Vector2(r * cos(a), r * sin(a));
}

namespace
{
   // A triangle of a surface polygon, in the polygon's own (unbent)
   // coordinates
   struct SurfaceTriangle
   {
      Vector2 p[3];
   };

   // Splits triangles into four (at their edges' midpoints) until no edge
   // is longer than maxEdge, so that bent onto the planet they follow the
   // curve: a 120 long edge sags only half a unit under the arc
   void tessellate(const SurfaceTriangle& t, float maxEdge, int depth, std::vector<SurfaceTriangle>& out)
   {
      float longest = 0.0f;
      for (int i = 0; i < 3; i++)
      {
         longest = fmax(longest, (t.p[(i + 1) % 3] - t.p[i]).length());
      }

      if ((longest <= maxEdge) || (depth <= 0))
      {
         out.push_back(t);
         return;
      }

      Vector2 m01 = (t.p[0] + t.p[1]) * 0.5f;
      Vector2 m12 = (t.p[1] + t.p[2]) * 0.5f;
      Vector2 m20 = (t.p[2] + t.p[0]) * 0.5f;

      SurfaceTriangle a = { { t.p[0], m01, m20 } };
      SurfaceTriangle b = { { m01, t.p[1], m12 } };
      SurfaceTriangle c = { { m20, m12, t.p[2] } };
      SurfaceTriangle d = { { m01, m12, m20 } };
      tessellate(a, maxEdge, depth - 1, out);
      tessellate(b, maxEdge, depth - 1, out);
      tessellate(c, maxEdge, depth - 1, out);
      tessellate(d, maxEdge, depth - 1, out);
   }
}

// A spritePolygon of the unrolled surface on the planet: its triangles (as
// the editor triangulated them) split up and bent round the planet, the
// texture laid out as in a scene (textureMeterWidth/Height and offsets, on
// the unbent polygon, so it follows the bend). Its own angle is ignored,
// as for sprite polygons in scenes; fades and borders are not drawn here.
void PlanetActor::placeSurfacePolygon(pugi::xml_node& node)
{
   size_t vertexCount = (size_t)std::distance(node.child("vertices").children("vertex").begin(), node.child("vertices").children("vertex").end());
   size_t triangleCount = (size_t)std::distance(node.child("triangles").children("triangle").begin(), node.child("triangles").children("triangle").end());
   if ((vertexCount < 3) || (triangleCount < 1))
   {
      return;
   }

   std::vector<Vector2> vertices(vertexCount);
   std::vector<VectorT3<int> > triangles(triangleCount);
   PolygonVertices::readVertices(vertices, node);
   PolygonVertices::readTriangles(triangles, node);

   std::vector<SurfaceTriangle> pieces;
   for (const VectorT3<int>& tri : triangles)
   {
      int idx[3] = { tri.x - 1, tri.y - 1, tri.z - 1 };
      bool valid = true;
      for (int i = 0; i < 3; i++)
      {
         valid = valid && (idx[i] >= 0) && ((size_t)idx[i] < vertexCount);
      }
      if (!valid)
      {
         continue;
      }

      SurfaceTriangle t = { { vertices[idx[0]], vertices[idx[1]], vertices[idx[2]] } };
      tessellate(t, 120.0f, 6, pieces);
   }

   if (pieces.empty())
   {
      return;
   }

   spPolygon polygon = new oxygine::Polygon();
   doCommonShapeDefinitions(polygon.get(), node);

   Vector2 pos(node.attribute("posX").as_float(), node.attribute("posY").as_float());
   Vector2 texSize(node.attribute("textureMeterWidth").as_float(), node.attribute("textureMeterHeight").as_float());
   Vector2 texOffset(node.attribute("textureOffsetMeterX").as_float(), node.attribute("textureOffsetMeterY").as_float());
   if (texSize.x <= 0.0f) texSize.x = 1.0f;
   if (texSize.y <= 0.0f) texSize.y = 1.0f;

   // Tint in the vertices (oxygine::Polygon draws with them), premultiplied
   const unsigned int vertexColor = polygon->getColor().premultiplied().rgba();

   // Four vertices a triangle: oxygine draws strips, the last is repeated
   // (a degenerate triangle), as PolygonVertices does
   vertexPCT2* verts = new vertexPCT2[pieces.size() * 4];
   vertexPCT2* v = verts;

   for (const SurfaceTriangle& t : pieces)
   {
      for (int i = 0; i < 4; i++)
      {
         const Vector2& local = t.p[(i < 3) ? i : 2];
         Vector2 bent = bendToSurface(pos + local);

         v->color = vertexColor;
         v->x = bent.x;
         v->y = bent.y;
         v->z = 0;
         v->u = (local.x - texOffset.x) / texSize.x;
         v->v = (local.y - texOffset.y) / texSize.y;
         ++v;
      }
   }

   polygon->setVertices(verts, sizeof(vertexPCT2) * pieces.size() * 4, vertexPCT2::FORMAT, true);
   polygon->setTouchEnabled(false);
   polygon->attachTo(m_surface);
}

// The sites' solution markers (the predicted marker, with the site's name
// under it), c_predictedAngleOffset ahead of each site on the surface
// layer. fadeIn for a new revolution (restartRevolution).
void PlanetActor::placeSolutionMarkers(bool fadeIn)
{
   for (spActor& marker : m_solutionMarkers)
   {
      marker->detach();
   }
   m_solutionMarkers.clear();

   for (auto it = m_landingSites.begin(); it != m_landingSites.end(); ++it)
   {
      spSprite predSite = new Sprite();
      predSite->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("predicted_site"));
      predSite->setSize(100.0f, 30.0f);
      predSite->setAnchor(0.5f, 0.5f);
      predSite->setPriority(250);

      float alpha = it->m_angle + c_predictedAngleOffset;
      predSite->setPosition(Vector2(c_surfaceRadius * cos(alpha), c_surfaceRadius * sin(alpha)));
      predSite->setRotation(alpha + MATH_PI / 2.0f);
      predSite->attachTo(m_surface);
      m_solutionMarkers.push_back(predSite);

      // The site's name under the marker (towards the ground; it turns
      // with the planet). The planet picture is scaled down to fit the
      // display, so the font is scaled up to read like the HUD's text.
      float pictureScale = getScale().x > 0.0f ? getScale().x : 1.0f;
      spTextField label = new TextField();
      TextStyle style = TextStyle(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResFont("lf_font"))
         .withColor(Color::White).alignMiddleH().alignTop();
      style.fontSize = (int)(g_Layout.getDefaultFontSize() / pictureScale);
      label->setStyle(style);
      label->setColor(g_Layout.getPhosphorColor());
      label->setAnchor(0.0f, 0.0f);
      label->setSize(100.0f, 30.0f);
      label->setPosition(0.0f, 30.0f + 6.0f);
      label->setTouchEnabled(false);
      label->setText(it->m_label);
      label->attachTo(predSite);

      if (fadeIn)
      {
         predSite->setAlpha(0);
         predSite->addTween(Actor::TweenAlpha(255), 600);
      }
   }
}

// No burn while the sites came by: another revolution. The planet goes on
// turning; the surface (its art, the sites and their solution markers)
// fades out and comes again where it was when the orbit was established
// (Fredrik: no new arrival, just the next turn around the planet).
void PlanetActor::restartRevolution(void)
{
   if (m_revolutionPending)
   {
      return;
   }

   m_revolutionPending = true;
   m_surface->removeTweens();
   spTween fade = m_surface->addTween(Actor::TweenAlpha(0), 400);
   fade->setDoneCallback([this](Event*)
   {
      // Not if a burn has started meanwhile: then the sites stay put
      if (m_state == PAS_ORBIT_ESTABLISHED)
      {
         m_surface->setRotation(surfaceRotationFor(m_planet->getRotation()));
         placeSolutionMarkers(false);
      }

      m_revolutionPending = false;
      m_surface->addTween(Actor::TweenAlpha(255), 600);
   });
}

void PlanetActor::startReentry(float relativeBurnDeviation, float deviationDistance)
// The relativeBurnDeviation is the relative error from the 
// ideal brun rate. If this is 0, the trajectory is perfect
// If it is -1 the burn was just on the edge of bouncing
// off of the atmosphere. It it is 1 the burn was just on 
// the edge to make the leapfrog bounce back into space
// This value is used to check what trajectory to use, 
// the ideal with a deviation of the bouncy one and also
// to apply a small deviation to the x-value of the ideal
// tracjectory points. The small deviation to use in each point
// should sum up to the supplied deviationDistance.
{
   // TODO: Check if the relativeBurnDeviation is above 1
   // in which case a bouncy trajectory should be used.

   m_frogTweenQueue = new TweenQueue;
   if (relativeBurnDeviation > 1.0f)
   {
      // Create bouncy trajectory
      for (int i = 1; i < 44; i++)
      {
         Vector2 idelaPos = c_bouncyTrajectoryPositions[i] + m_orbitStartPos;
         Vector2 burnDeviation = Vector2(i * relativeBurnDeviation * deviationDistance / 44.0f, 0.0f);
         m_frogTweenQueue->add(Actor::TweenPosition(idelaPos + burnDeviation), 220);
      }
   }
   else
   {
      // Create planetary trajectory
      for (int i = 1; i < 44; i++)
      {
         Vector2 idelaPos = c_trajectoryPositions[i] + m_orbitStartPos;
         Vector2 burnDeviation = Vector2(i * relativeBurnDeviation * deviationDistance / 44.0f, 0.0f);
         m_frogTweenQueue->add(Actor::TweenPosition(idelaPos + burnDeviation), 220);
      }
   }
   m_positionIndicator->addTween(m_frogTweenQueue);
}

void PlanetActor::surfaceReached(void)
{
   m_planetAngleAtSurface = m_planet->getRotation();

   // Where on the surface the Leapfrog came down: the end of its path
   // (c_landingPointAngle on the screen) in the surface layer's angles
   float landingAngle = c_landingPointAngle - m_planetAngleAtSurface - m_surface->getRotation();

   logs::messageln("Landing angle: %f", landingAngle);
   
   float minAngleToLandingSite = 2.0f * MATH_PI;
   int index = 0;

   // Check the closest landing site
   for (auto it = m_landingSites.begin(); it != m_landingSites.end(); ++it, ++index)
   {
      // The other way round the planet if that is shorter
      float diff = remainderf(it->m_angle - landingAngle, 2.0f * MATH_PI);
      float absDiff = fabs(diff);
      if (absDiff < minAngleToLandingSite)
      {
         minAngleToLandingSite = absDiff;
         m_results.m_angleFromClosestSite = diff;
         m_results.m_distanceFromClosestSite = absDiff * 3500.0f;
         m_results.m_closestSite = it->m_name;
         m_results.m_closestSiteIndex = index;
         m_results.m_closestSiteScene = it->m_scene;

         if (absDiff < c_safeLandingMaxRadDeviation)
         {
            m_results.m_landingSuccessfull = true;
         }
         else
         {
            m_results.m_landingSuccessfull = false;
         }
      }
   }

   m_state = PAS_LANDED;
   m_stateChange = true;

   m_frogTweenQueue->remove();
}

bool PlanetActor::allSolutionsPassed(void)
{
   if ((m_state != PAS_ORBIT_ESTABLISHED) || m_landingSites.empty() || m_revolutionPending)
   {
      return false;
   }

   float turned = m_planet->getRotation() + m_surface->getRotation();

   for (const LandingSite& site : m_landingSites)
   {
      if (site.m_angle + c_predictedAngleOffset + turned <= c_landingPointAngle + c_missedAngle)
      {
         return false;
      }
   }

   return true;
}

LandingResult PlanetActor::getLandingResult()
{
   return m_results;
}

void PlanetActor::startBurn(void)
{
   m_planetAngleAtBurnStart = m_planet->getRotation();

   m_state = PAS_BURN;
   m_stateChange = true;
}

void PlanetActor::stopBurn(void)
{
   m_state = PAS_REENTRY;
   m_stateChange = true;
}

float PlanetActor::getBurnAmount(void)
{
   return m_burnIndicator->getProgress();
}

float PlanetActor::getReentryLeapfrogAngle(float time)
{
   float index = time / 220.0f;
   int i = (int)index;
   if (i < 0) i = 0;
   if (i > 44) i = 44;
   return c_trajectoryAngles[i];

}


// Data is from trajectory_ellipse.xlsx
// The total distance to travel here is 2241 m. 
// Let the animation to the m_planet take about
// ten seconds. That means about 2241/10 = 224 m/s
// Time of one 50 m leg is the t = 50 / 224 = 0.22 seconds

const float PlanetActor::c_relativeTrajectoryAngles[] =
{
   0.0f,
   0.011113742f,
   0.022250852f,
   0.022318419f,
   0.022428004f,
   0.022580486f,
   0.022777072f,
   0.023019315f,
   0.023309136f,
   0.023648852f,
   0.024041207f,
   0.024489409f,
   0.024997179f,
   0.025568797f,
   0.026209163f,
   0.026923857f,
   0.027719214f,
   0.028602392f,
   0.029581456f,
   0.030665455f,
   0.031864488f,
   0.033189773f,
   0.034653668f,
   0.03626967f,
   0.038052338f,
   0.040017124f,
   0.042180079f,
   0.044557395f,
   0.047164778f,
   0.050016674f,
   0.053125556f,
   0.056501768f,
   0.060155259f,
   0.064102581f,
   0.068388272f,
   0.073149849f,
   0.078849361f,
   0.087557955f,
   0.130158081f,
   0.018600318f,
   0.130158081f,
   0.087557955f,
   0.078849361f,
   0.073149849f
};

const float PlanetActor::c_trajectoryAngles[] =
{
   0.0f,
   0.011113742f,
   0.033364593f,
   0.055683013f,
   0.078111017f,
   0.100691503f,
   0.123468575f,
   0.14648789f,
   0.169797027f,
   0.193445879f,
   0.217487086f,
   0.241976495f,
   0.266973673f,
   0.29254247f,
   0.318751633f,
   0.345675491f,
   0.373394704f,
   0.401997096f,
   0.431578552f,
   0.462244007f,
   0.494108495f,
   0.527298268f,
   0.561951936f,
   0.598221605f,
   0.636273943f,
   0.676291067f,
   0.718471146f,
   0.763028541f,
   0.810193319f,
   0.860209992f,
   0.913335549f,
   0.969837316f,
   1.029992576f,
   1.094095157f,
   1.162483429f,
   1.235633278f,
   1.314482639f,
   1.402040594f,
   1.532198675f,
   1.550798993f,
   1.619358309f,
   1.73955206f,
   1.827110015f,
   1.905959376f,
   1.905959376f,
   1.905959376f,
   1.905959376f
};




const Vector2 PlanetActor::c_trajectoryPositions[] =
{
   { 0.0f, 0.0f },
{ 50.0f, 0.555709962f },
{ 99.98764517f, 2.224146567f },
{ 149.938128f, 5.008418174f },
{ 199.8263639f, 8.913183688f },
{ 249.6268141f, 13.94468182f },
{ 299.3133014f, 20.11076644f },
{ 348.8588159f, 27.4209483f },
{ 398.2353089f, 35.88644312f },
{ 447.4134701f, 45.52022622f },
{ 496.3624862f, 56.33709354f },
{ 545.0497754f, 68.35372894f },
{ 593.4406943f, 81.58877709f },
{ 641.4982122f, 96.06292163f },
{ 689.1825459f, 111.7989671f },
{ 736.4507499f, 128.8219233f },
{ 783.2562542f, 147.1590899f },
{ 829.5483409f, 166.840138f },
{ 875.2715515f, 187.8971846f },
{ 920.3650146f, 210.364855f },
{ 964.761683f, 234.2803254f },
{ 1008.387471f, 259.6833383f },
{ 1051.160278f, 286.6161796f },
{ 1092.988899f, 315.1236068f },
{ 1133.7718f, 345.2527188f },
{ 1173.39578f, 377.0527599f },
{ 1211.734511f, 410.5748661f },
{ 1248.647f, 445.8717878f },
{ 1283.975997f, 482.9976841f },
{ 1317.546454f, 522.0082089f },
{ 1349.164103f, 562.9613682f },
{ 1378.61433f, 605.9201841f },
{ 1405.661437f, 650.959458f },
{ 1430.048353f, 698.18202f },
{ 1451.49649f, 747.7585463f },
{ 1469.704393f, 800.0344496f },
{ 1484.340492f, 855.8807552f },
{ 1495.011794f, 918.5145271f },
{ 1497.0f, 970.0f },
{ 1498.0f, 1020.0f },
{ 1495.011794f, 1081.485473f },
{ 1484.340492f, 1144.119245f },
{ 1469.704393f, 1199.96555f },
{ 1451.49649f, 1252.241454f }
};


const Vector2 PlanetActor::c_bouncyTrajectoryPositions[] =
{
   { 0.0f, 0.0f },
{50.0f, 0.555709962f },
{99.98764517f, 2.224146567f },
{149.938128f, 5.008418174f },
{199.8263639f, 8.913183688f },
{249.6268141f, 13.94468182f },
{299.3133014f, 20.11076644f },
{348.8588159f, 27.4209483f },
{398.2353089f, 35.88644312f },
{447.4134701f, 45.52022622f },
{496.3624862f, 56.33709354f },
{545.0497754f, 68.35372894f },
{593.4406943f, 81.58877709f },
{641.4982122f, 96.06292163f },
{689.1825459f, 111.7989671f },
{736.4507499f, 128.8219233f },
{783.2562542f, 147.1590899f },
{829.5483409f, 166.840138f },
{875.2715515f, 187.8971846f },
{920.3650146f, 210.364855f },
{965.4584776f, 210.364855f },
{1010.551941f, 187.8971846f },
{1055.645404f, 166.840138f },
{1100.738867f, 147.1590899f },
{1145.83233f, 128.8219233f },
{1190.925793f, 111.7989671f },
{1236.019256f, 96.06292163f },
{1281.112719f, 81.58877709f },
{1326.206182f, 68.35372894f },
{1371.299645f, 56.33709354f },
{1416.393108f, 45.52022622f },
{1461.486571f, 35.88644312f },
{1506.580034f, 27.4209483f },
{1551.673497f, 20.11076644f },
{1596.76696f, 13.94468182f },
{1641.860424f, 8.913183688f },
{1686.953887f, 5.008418174f },
{1732.04735f, 2.224146567f },
{1777.140813f, 0.555709962f },
{1822.234276f, 0.0f },
{ 1822.234276f, 0.0f },
{ 1822.234276f, 0.0f },
{ 1822.234276f, 0.0f },
{ 1822.234276f, 0.0f },
{ 1822.234276f, 0.0f },
{ 1822.234276f, 0.0f },
{ 1822.234276f, 0.0f },
{ 1822.234276f, 0.0f },
};
