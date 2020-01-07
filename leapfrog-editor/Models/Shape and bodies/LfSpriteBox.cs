using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   [Serializable]
   public class LfSpriteBox : LfShape
   {
      #region Declarations

      private double _width;
      private double _height;
      private double _parallaxAmount;

      #endregion

      #region Constructors

      public LfSpriteBox()
      {
         _width = 200;
         _height = 200;
         _parallaxAmount = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("width")]
      public double Width
      {
         get { return _width; }
         set { _width = value; }
      }

      [XmlAttribute("height")]
      public double Height
      {
         get { return _height; }
         set { _height = value; }
      }

      [XmlAttribute("parallaxAmount")]
      public double ParallaxAmount
      {
         get { return _parallaxAmount; }
         set { _parallaxAmount = value; }
      }

      #endregion

   }
}
