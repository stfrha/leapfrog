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
   public class LfDynamicCircle : LfStaticCircle
   {
      #region Declarations

      private double _density;
      private double _friction;

      #endregion

      #region Constructors

      public LfDynamicCircle()
      {
         _density = 1;
      }

      #endregion

      #region Properties

      [XmlAttribute("density")]
      public double Density
      {
         get { return _density; }
         set { _density = value; }
      }

      [XmlAttribute("friction")]
      public double Friction
      {
         get { return _friction; }
         set { _friction = value; }
      }

      #endregion

   }
}
