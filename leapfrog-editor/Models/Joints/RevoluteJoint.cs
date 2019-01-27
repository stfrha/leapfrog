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
   public class RevoluteJoint : MotorizedJoint
   {
      #region Declarations

      private double _maxMotorTorque;

      #endregion

      #region Constructors

      public RevoluteJoint() : base()
      {
         _maxMotorTorque = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("maxMotorTorque")]
      public double MaxMotorTorque
      {
         get { return _maxMotorTorque; }
         set { _maxMotorTorque = value; }
      }

      #endregion

   }
}
