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
      private double _textureMeterWidth = 0.0;
      private double _textureMeterHeight = 0.0;
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

      // The size of one tile of the ground texture, in the surface's units
      // (as textured polygons' textureMeterWidth/Height). 0 = the texture's
      // own size in pixels (one pixel a unit); a height of 0 with a width
      // keeps the texture's proportions.
      [XmlAttribute("textureMeterWidth")]
      public double TextureMeterWidth
      {
         get { return _textureMeterWidth; }
         set { _textureMeterWidth = value; }
      }

      public bool ShouldSerializeTextureMeterWidth()
      {
         return TextureMeterWidth > 0.0;
      }

      [XmlAttribute("textureMeterHeight")]
      public double TextureMeterHeight
      {
         get { return _textureMeterHeight; }
         set { _textureMeterHeight = value; }
      }

      public bool ShouldSerializeTextureMeterHeight()
      {
         return TextureMeterHeight > 0.0;
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
