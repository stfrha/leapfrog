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
   public class SteerableObjectProperties
   {
      #region Declarations

      private string _steeringState;
      private string _bodyName;
      private double _bodyToBoosterAngle;
      private double _seekTargetX;
      private double _seekTargetY;
      private double _maxBoosterForce;
      private double _maxSpeed;
      private double _degPerSecMaxRotateSpeed;
      private double _linearDamping;

      #endregion

      #region Constructors

      public SteerableObjectProperties()
      {
         _steeringState = "notDefined";
         _bodyName = "notDefined";
         _bodyToBoosterAngle = 0;
         _seekTargetX = 0;
         _seekTargetY = 0;
         _maxBoosterForce = 0;
         _maxSpeed = 0;
         _degPerSecMaxRotateSpeed = 0;
         _linearDamping = 0;
   }

      #endregion

      #region Properties

      [XmlAttribute("steeringState")]
      public string SteeringState
      {
         get { return _steeringState; }
         set { _steeringState = value; }
      }

      [XmlAttribute("targetBody")]
      public string BodyName
      {
         get { return _bodyName; }
         set { _bodyName = value; }
      }

      [XmlAttribute("bodyToBoosterAngle")]
      public double BodyToBoosterAngle
      {
         get { return _bodyToBoosterAngle; }
         set { _bodyToBoosterAngle = value; }
      }

      [XmlAttribute("seekTargetX")]
      public double SeekTargetX
      {
         get { return _seekTargetX; }
         set { _seekTargetX = value; }
      }

      [XmlAttribute("seekTargetY")]
      public double SeekTargetY
      {
         get { return _seekTargetY; }
         set { _seekTargetY = value; }
      }

      [XmlAttribute("maxBoosterForce")]
      public double MaxBoosterForce
      {
         get { return _maxBoosterForce; }
         set { _maxBoosterForce = value; }
      }

      [XmlAttribute("maxSpeed")]
      public double MaxSpeed
      {
         get { return _maxSpeed; }
         set { _maxSpeed = value; }
      }

      [XmlAttribute("degPerSecMaxRotateSpeed")]
      public double DegPerSecMaxRotateSpeed
      {
         get { return _degPerSecMaxRotateSpeed; }
         set { _degPerSecMaxRotateSpeed = value; }
      }

      [XmlAttribute("linearDamping")]
      public double LinearDamping
      {
         get { return _linearDamping; }
         set { _linearDamping = value; }
      }

      #endregion

   }
}
