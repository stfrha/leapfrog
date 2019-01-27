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
   public class PrismaticJoint : MotorizedJoint
   {
      #region Declarations

      private double _aAxisX;
      private double _aAxisY;
      private double _refAngle;
      private double _lowerLimit;
      private double _upperLimit;
      private double _maxMotorForce;

      #endregion

      #region Constructors

      public PrismaticJoint() : base()
      {
         _aAxisX = 0.707;
         _aAxisY = 0.707;
         _refAngle = 0;
         _lowerLimit = -50;
         _upperLimit = 25;
      }

      #endregion

      #region Properties

      [XmlAttribute("objectALocalAxisX")]
      public double AAxisX
      {
         get { return _aAxisX; }
         set { _aAxisX = value; }
      }

      [XmlAttribute("objectALocalAxisY")]
      public double AAxisY
      {
         get { return _aAxisY; }
         set { _aAxisY = value; }
      }

      [XmlAttribute("refAngle")]
      public double RefAngle
      {
         get { return _refAngle; }
         set { _refAngle = value; }
      }

      [XmlAttribute("lowerLimit")]
      public double LowerLimit
      {
         get { return _lowerLimit; }
         set { _lowerLimit = value; }
      }

      [XmlAttribute("upperLimit")]
      public double UpperLimit
      {
         get { return _upperLimit; }
         set { _upperLimit = value; }
      }

      [XmlAttribute("maxMotorForce")]
      public double MaxMotorForce
      {
         get { return _maxMotorForce; }
         set { _maxMotorForce = value; }
      }

      #endregion

   }
}
