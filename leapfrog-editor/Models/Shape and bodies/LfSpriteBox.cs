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

      #endregion

      #region Constructors

      public LfSpriteBox()
      {
         _width = 200;
         _height = 200;
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

      #endregion

   }
}
