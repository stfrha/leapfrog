using System;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   // A fade over a polygon and its border decorations, saved as the
   // polygon's <fade> element (see PolygonFade in the game's
   // polygonvertices.h, which reads it):
   //   amount  0..1, how strong the fade is where it is complete (0 = off)
   //   angle   direction the fade grows in, degrees: 0 = down, 90 = right,
   //           180 = up, 270 = left
   //   start   where it begins, 0..1 of the polygon's extent in that direction
   //   end     where it is complete, 0..1
   //   color   rrggbb to fade towards (haze); empty fades to transparent
   [Serializable]
   public class PolygonFade
   {
      public PolygonFade()
      {
         Amount = 0;
         Angle = 0;
         Start = 0;
         End = 1;
         Color = "";
      }

      [XmlAttribute("amount")]
      public double Amount { get; set; }

      [XmlAttribute("angle")]
      public double Angle { get; set; }

      [XmlAttribute("start")]
      public double Start { get; set; }

      [XmlAttribute("end")]
      public double End { get; set; }

      [XmlAttribute("color")]
      public string Color { get; set; }

      public bool ShouldSerializeColor()
      {
         return !string.IsNullOrEmpty(Color);
      }
   }
}
