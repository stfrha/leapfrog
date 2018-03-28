#include "planetactor.h"


using namespace oxygine;

PlanetActor::PlanetActor(Resources& gameResources) : 
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

   spTween rotTween = m_planet->addTween(Actor::TweenRotation(2.0f * MATH_PI), 90000);

}


void PlanetActor::orbitEstablished(void)
{
   spSprite crs = new Sprite();
   crs->setResAnim(m_gameResources->getResAnim("predicted_site"));
   crs->setSize(100.0f, 30.0f);
   crs->setAnchor(0.5f, 0.5f);

   float alpha = -100.0 / 180.0f * MATH_PI;
   crs->setPosition(Vector2(3500.0f * cos(alpha), 3500.0f * sin(alpha)));
   crs->setRotation(alpha + MATH_PI / 2.0f);
   crs->attachTo(m_planet);

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
}


void PlanetActor::startReentry(void)
{
   spTweenQueue frogTweenQueue = new TweenQueue;
   for (int i = 1; i < 44; i++)
   {
      frogTweenQueue->add(Actor::TweenPosition(c_trajectoryPositions[i] + m_orbitStartPos), 220);
   }
   m_positionIndicator->addTween(frogTweenQueue);
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
   -1.522234345f,
   -1.402040594f,
   -1.314482639f,
   -1.235633278f,
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


