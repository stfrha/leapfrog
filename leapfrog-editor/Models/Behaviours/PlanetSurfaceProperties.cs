using System;
using System.Collections.ObjectModel;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   // A planet's surface, edited unrolled (behaviour "planetSurface"): x is
   // the distance along the surface (0 = angle 0), y the height (0 = the
   // ground, negative above it). The game bends it round the planet: a
   // point (x, y) is at the angle x / Radius and Radius - y from the
   // planet's centre (PlanetActor). Its spriteBoxes are the surface's art;
   // the landing sites are here. An orbit scene names the file
   // (surfaceFile).
   [Serializable]
   public class PlanetSurfaceProperties
   {
      // The game's planet (PlanetActor): one degree is about 61.1 units
      public const double Radius = 3500.0;

      private string _texture = "";
      private ObservableCollection<LandingSite> _landingSites = new ObservableCollection<LandingSite>();

      // The planet's ground texture ("" = the orbit scene's)
      [XmlAttribute("texture")]
      public string Texture
      {
         get { return _texture; }
         set { _texture = value; }
      }

      public bool ShouldSerializeTexture()
      {
         return !string.IsNullOrEmpty(Texture);
      }

      [XmlElement("landingSite")]
      public ObservableCollection<LandingSite> LandingSites
      {
         get { return _landingSites; }
         set { _landingSites = value; }
      }

      public static double AngleToX(double angleDegrees)
      {
         return angleDegrees * Math.PI / 180.0 * Radius;
      }

      public static double XToAngle(double x)
      {
         return x / Radius * 180.0 / Math.PI;
      }
   }
}
