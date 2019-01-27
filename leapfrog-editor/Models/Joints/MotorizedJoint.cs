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
   public class MotorizedJoint : WeldJoint
   {
      // All is inherited in this class except XML element name and view

      #region Declarations

      private bool _lockJoint;
      private bool _useMotor;
      private double _motorSpeed;

      #endregion

      #region Constructors

      public MotorizedJoint() : base()
      {
         _lockJoint = false;
         _useMotor = false;
         _motorSpeed = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("lockJoint")]
      public bool LockJoint
      {
         get { return _lockJoint; }
         set { _lockJoint = value; }
      }

      [XmlAttribute("useMotor")]
      public bool UseMotor
      {
         get { return _useMotor; }
         set { _useMotor = value; }
      }

      [XmlAttribute("motorSpeed")]
      public double MotorSpeed
      {
         get { return _motorSpeed; }
         set { _motorSpeed = value; }
      }

      
      #endregion

   }
}
