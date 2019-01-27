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
   public class LfStaticCircle : LfShape
   {
      #region Declarations

      private double _radius;

      #endregion

      #region Constructors

      public LfStaticCircle()
      {
         _radius = 100;
      }

      #endregion

      #region Properties

      [XmlAttribute("radius")]
      public double Radius
      {
         get { return _radius; }
         set { _radius = value; }
      }

      #endregion

   }
}
