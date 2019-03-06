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
   public class ShieldProperties : BodyOriginSystem
   {
      #region Declarations

      private double _radius;
      private int _zLevel;

      #endregion

      #region Constructors

      public ShieldProperties()
      {
         _radius = 100;
         _zLevel = 147;
      }

      #endregion

      #region Properties

      [XmlAttribute("radius")]
      public double Radius
      {
         get { return _radius; }
         set { _radius = value; }
      }

      [XmlAttribute("zLevel")]
      public int ZLevel
      {
         get { return _zLevel; }
         set { _zLevel = value; }
      }

      #endregion

   }
}
