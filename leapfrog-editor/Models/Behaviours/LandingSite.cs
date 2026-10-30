using System;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   // A landing site on a planet (planetSurfaceProperties, or an orbit
   // scene's landingSites). The game numbers the sites in file order (the
   // orbit scene's exit parameter).
   //   name     for the scripts (c_lastReentry's siteName)
   //   label    shown under the site's solution marker in orbit ("" = name)
   //   angle    along the surface in degrees, growing to the right
   //            (clockwise), as on the planet editor's ruler: the strip's
   //            x / PlanetSurfaceProperties.Radius
   //   texture  the site's marker on the ground ("" = none, e.g. when the
   //            surface's art shows the site)
   //   scene    the landing scene the site leads to
   [Serializable]
   public class LandingSite
   {
      private string _name = "site";
      private string _label = "";
      private double _angle = 0.0;
      private string _texture = "";
      private string _scene = "";

      [XmlAttribute("name")]
      public string Name
      {
         get { return _name; }
         set { _name = value; }
      }

      [XmlAttribute("label")]
      public string Label
      {
         get { return _label; }
         set { _label = value; }
      }

      public bool ShouldSerializeLabel()
      {
         return !string.IsNullOrEmpty(Label);
      }

      [XmlAttribute("angle")]
      public double Angle
      {
         get { return _angle; }
         set { _angle = value; }
      }

      [XmlAttribute("texture")]
      public string Texture
      {
         get { return _texture; }
         set { _texture = value; }
      }

      [XmlAttribute("scene")]
      public string Scene
      {
         get { return _scene; }
         set { _scene = value; }
      }
   }
}
