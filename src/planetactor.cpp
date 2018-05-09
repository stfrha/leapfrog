#include "planetactor.h"


using namespace oxygine;
using namespace pugi;

PlanetActor::PlanetActor(
   Resources& gameResources,
   Actor* sceneParent,
   CompoundObject* parentObject,
   xml_node& objectNode) :
   m_gameResources(&gameResources),
   m_state(PAS_INITITAL),
   m_orbitStartPos(Vector2(500.0f, 150.0f))
{
   //// I should probably load resources that are uniuqe here
   //setPanorateMode(PME_TOP_LEFT);

   m_planet = new Actor();
   m_planet->attachTo(this);
   m_planet->setAnchor(0.5f, 0.5f);
   m_planet->setPosition(Vector2(500.0f, 4500.0f));

   spMaskedSprite maskedSprite = new MaskedSprite();
   maskedSprite->attachTo(m_planet);
   maskedSprite->setAnchor(0.5f, 0.5f);
   maskedSprite->setPosition(0.0f, 0.0f);

   spBox9Sprite planetSprite = new Box9Sprite();
   planetSprite->setResAnim(gameResources.getResAnim("planet_rock"));
//   planetSprite->setResAnim(gameResources.getResAnim(objectNode.attribute("texture").as_string()));
   planetSprite->setSize(7000.0f, 7000.0f);
   planetSprite->setAnchor(0.5f, 0.5f);
   planetSprite->setGuides(0, 0, 0, 0);
   planetSprite->setHorizontalMode(Box9Sprite::TILING_FULL);
   planetSprite->setVerticalMode(Box9Sprite::TILING_FULL);
   planetSprite->attachTo(maskedSprite);

   spSprite mask = new Sprite();
   mask->setResAnim(gameResources.getResAnim("planet_mask"));
   mask->setSize(7000.0f, 7000.0f);
   mask->setAnchor(0.5f, 0.5f);
   mask->attachTo(maskedSprite);
   mask->setVisible(false);

   maskedSprite->setMask(mask, true);

   for (auto it = objectNode.child("landingSites").children("landingSite").begin();
      it != objectNode.child("landingSites").children("landingSite").end();
      ++it)
   {
      LandingSite ls(*it);
      m_landingSites.insert(ls);
   }


   setAnchor(objectNode.attribute("anchorX").as_float(), objectNode.attribute("anchorY").as_float());
   setPosition(objectNode.attribute("posX").as_float(), objectNode.attribute("posY").as_float());
   setScale(objectNode.attribute("scale").as_float());
   attachTo(sceneParent);

   spTween rotTween = m_planet->addTween(Actor::TweenRotation(2.0f * MATH_PI), 90000, -1);

}


void PlanetActor::orbitEstablished(void)
{
   // Generate predicted landing sites
   // TODO: Generate real landing sites also
   for (auto it = m_landingSites.begin(); it != m_landingSites.end(); ++it)
   {
      spSprite crs = new Sprite();
      crs->setResAnim(m_gameResources->getResAnim("predicted_site"));
      crs->setSize(100.0f, 30.0f);
      crs->setAnchor(0.5f, 0.5f);

      float planetAngle = m_planet->getRotation();
      float alpha = -100.0f / 180.0f * MATH_PI - planetAngle + it->m_angle;
      crs->setPosition(Vector2(3500.0f * cos(alpha), 3500.0f * sin(alpha)));
      crs->setRotation(alpha + MATH_PI / 2.0f);
      crs->attachTo(m_planet);
   }

   // generate the trajectory, without angles set to zero, to begin with
   spSprite trajectory[44];

   for (int i = 0; i < 44; i++)
   {
      trajectory[i] = new Sprite();
      trajectory[i]->setResAnim(m_gameResources->getResAnim("trajectory"));
      trajectory[i]->setSize(50.0f, 8.0f);
      trajectory[i]->setAnchor(0.0f, 0.5f);
      trajectory[i]->setPosition(c_trajectoryPositions[i] + m_orbitStartPos);
      trajectory[i]->setRotation(c_trajectoryAngles[i]);
      trajectory[i]->attachTo(this);
   }

   m_positionIndicator = new Sprite();
   m_positionIndicator->setResAnim(m_gameResources->getResAnim("lf_position"));
   m_positionIndicator->setSize(40.0f, 40.0f);
   m_positionIndicator->setAnchor(0.5f, 0.5f);
   m_positionIndicator->setPosition(c_trajectoryPositions[0] + m_orbitStartPos);
   m_positionIndicator->attachTo(this);

   spColorRectSprite burnFrame = new ColorRectSprite();
   burnFrame->setAnchor(0.5f, 0.5f);
   burnFrame->setSize(100.0f, 500.0f);
   burnFrame->setColor(Color::Fuchsia);
   burnFrame->setPosition(2800.0f, 900.0f);
   burnFrame->attachTo(this);

   spColorRectSprite burnHaze = new ColorRectSprite();
   burnHaze->setAnchor(0.5f, 0.5f);
   burnHaze->setSize(96.0f, 496.0f);
   burnHaze->setColor(Color::Purple);
   burnHaze->setPosition(2800.0f, 900.0f);
   burnHaze->attachTo(this);

   spColorRectSprite safeZoom = new ColorRectSprite();
   safeZoom->setAnchor(0.5f, 0.5f);
   safeZoom->setSize(96.0f, 100.0f);
   safeZoom->setColor(Color::Fuchsia);
   safeZoom->setPosition(2800.0f, 800.0f);
   safeZoom->setAlpha(96);
   safeZoom->attachTo(this);

   m_burnIndicator = new ProgressBar();
   m_burnIndicator->setAnchor(0.5f, 0.5f);
   m_burnIndicator->setSize(100.0f, 500.0f);
   m_burnIndicator->setColor(Color::Fuchsia);
   m_burnIndicator->setDirection(ProgressBar::dir_90);
   m_burnIndicator->setPosition(2800.0f, 900.0f);
   m_burnIndicator->attachTo(this);
   m_burnIndicator->setProgress(0.0f);

   spSprite level = new Sprite();
   level->setResAnim(m_gameResources->getResAnim("reentry_burn_level"));
   level->setAnchor(0.5f, 0.5f);
   level->setSize(150.0f, 34.0f);
   level->setPosition(2800.0f, 800.0f);
   level->attachTo(this);
}

void PlanetActor::doUpdate(const UpdateState& us)
{
   if (m_state == PAS_BURN)
   {
      m_burnIndicator->setProgress(m_burnIndicator->getProgress() + 0.01f);
   }
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

   spTweenQueue frogTweenQueue = new TweenQueue;
   if (relativeBurnDeviation > 1.0f)
   {
      // Create bouncy trajectory
      for (int i = 1; i < 44; i++)
      {
         Vector2 idelaPos = c_bouncyTrajectoryPositions[i] + m_orbitStartPos;
         Vector2 burnDeviation = Vector2(i * relativeBurnDeviation * deviationDistance / 44.0f, 0.0f);
         frogTweenQueue->add(Actor::TweenPosition(idelaPos + burnDeviation), 220);
      }
   }
   else
   {
      // Create planetary trajectory
      for (int i = 1; i < 44; i++)
      {
         Vector2 idelaPos = c_trajectoryPositions[i] + m_orbitStartPos;
         Vector2 burnDeviation = Vector2(i * relativeBurnDeviation * deviationDistance / 44.0f, 0.0f);
         frogTweenQueue->add(Actor::TweenPosition(idelaPos + burnDeviation), 220);
      }
   }
   m_positionIndicator->addTween(frogTweenQueue);
}

void PlanetActor::startBurn(void)
{
   m_state = PAS_BURN;
}

void PlanetActor::stopBurn(void)
{
   m_state = PAS_REENTRY;
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
