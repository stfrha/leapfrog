#include "planetactor.h"
#include "sceneactor.h"
#include "layout.h"
#include "graphicresources.h"

using namespace oxygine;
using namespace pugi;

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
   c_safeLandingMaxRadDeviation(100.0f / 3500.0f / 2.0f)
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

   spBox9Sprite planetSprite = new Box9Sprite();
//   planetSprite->setResAnim(gameResources.getResAnim("planet_rock"));
   planetSprite->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::game).getResAnim(objectNode.attribute("texture").as_string()));
   planetSprite->setSize(7000.0f, 7000.0f);
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

   for (auto it = objectNode.child("landingSites").children("landingSite").begin();
      it != objectNode.child("landingSites").children("landingSite").end();
      ++it)
   {
      LandingSite ls(*it);
      m_landingSites.insert(ls);
   }
   
   setAnchor(0.5f, 0.5f);

   //float height = getStage()->getSize().y;
   //float width = getStage()->getSize().x;

   // Originally the position was read from XML and trimmed to 
   // -30,50 for a screen resolution of 960,640.
   // To maintain the position relative to the bottom of 
   // the screen the 20 value is recalculated.
   float yp = 20.0f / 640.0f * g_Layout.getViewPortBounds().y;
   setPosition(-30.0f, yp);
   
   //setPosition(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());

   // The scale set in xml-file (0.3) was empirically found
   // for the size of the screen at the time. 
   // For another screen size this does not apply.
   // The screen size at the time was 640. So lets
   // set scale with this relationship
   // This means that we no longer want to control this from the XML. 
   // Lets remove it from there and hardcode scale here
   float vScale = 0.3f / 640.0f * g_Layout.getViewPortBounds().y;
   float hScale = 0.3f / 960.0f * g_Layout.getViewPortBounds().x;


   setScale(fmin(vScale, hScale));

   attachTo(m_parentObject);

   m_positionIndicator = new Sprite();
   m_positionIndicator->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("lf_position"));
   m_positionIndicator->setSize(40.0f, 40.0f);
   m_positionIndicator->setAnchor(0.5f, 0.5f);
   m_positionIndicator->setPosition(Vector2(100.0f, 150.0f));
   m_positionIndicator->attachTo(this);

   m_positionIndicator->addTween(Actor::TweenPosition(c_trajectoryPositions[0] + m_orbitStartPos), 6000, 1, false, 0, Tween::ease_outQuad);

   spTween rotTween = m_planet->addTween(Actor::TweenRotation(2.0f * MATH_PI), 90000, -1);

   m_spaceSceneBackground = objectNode.attribute("spaceSceneBackground").as_string();
   m_spaceSceneFile = objectNode.attribute("spaceSceneFile").as_string();

}


void PlanetActor::doUpdate(const UpdateState& us)
{
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

   // Generate predicted landing sites

   for (auto it = m_landingSites.begin(); it != m_landingSites.end(); ++it)
   {
      spSprite predSite = new Sprite();
      predSite->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::hud).getResAnim("predicted_site"));
      predSite->setSize(100.0f, 30.0f);
      predSite->setAnchor(0.5f, 0.5f);


      float alpha = c_startAngleOffset - m_planetAngleAtOrbitEstablished - it->m_angle;
      predSite->setPosition(Vector2(3500.0f * cos(alpha), 3500.0f * sin(alpha)));
      predSite->setRotation(alpha + MATH_PI / 2.0f);
      predSite->attachTo(m_planet);

      spSprite crs = new Sprite();
      crs->setResAnim(g_GraphRes.getResources(GraphicResources::ResourceTypeEnum::game).getResAnim(it->m_texture));
      crs->setSize(100.0f, 30.0f);
      crs->setAnchor(0.5f, 0.5f);

      alpha = c_startAngleOffset - m_planetAngleAtOrbitEstablished - it->m_angle - c_predictedAngleOffset;
      crs->setPosition(Vector2(3500.0f * cos(alpha), 3500.0f * sin(alpha)));
      crs->setRotation(alpha + MATH_PI / 2.0f);
      crs->attachTo(m_planet);
   }

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

   float barHeight = 2.0f * g_Layout.getButtonWidth();
   float barVertCenter = g_Layout.getYFromBottom(3) + barHeight / 2.0f;
   float barWidth = barHeight * 0.2f;
   // float barHorCenter = g_Layout.getXFromLeft(2) + g_Layout.getButtonWidth() / 2 + barWidth / 2.0f;
   float barHorCenter = g_Layout.getXFromRight(5) + g_Layout.getButtonWidth() / 2;
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

   float landingAngle = 
      m_planetAngleAtSurface - 
      c_predictedAngleOffset - 
      (c_landingPointAngle - c_startAngleOffset) - 
      m_planetAngleAtOrbitEstablished;

   logs::messageln("Landing angle: %f", landingAngle);
   
   float minAngleToLandingSite = 2.0f * MATH_PI;

   // Check the closest landing site
   for (auto it = m_landingSites.begin(); it != m_landingSites.end(); ++it)
   {
      float absDiff = fabs(it->m_angle - landingAngle);
      if (absDiff < minAngleToLandingSite)
      {
         minAngleToLandingSite = absDiff;
         m_results.m_angleFromClosestSite = it->m_angle - landingAngle;
         m_results.m_closestSite = it->m_name;

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
