using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   // Behaviour "stripper": a small enemy that flies at the Leapfrog, latches
   // onto its hull, takes one resource and leaves (see stripper.h in the game).
   [Serializable]
   public class StripperProperties
   {
      #region Declarations

      private double _maxSpeed = 60;
      private double _acceleration = 90;
      private double _degPerSecTurnRate = 360;
      private double _noseAngle = 0;
      private double _lead = 3;
      private double _brakeDistance = 50;
      private double _latchSpeed = 8;
      private double _ramSpeed = 25;
      private double _shieldDrain = 4;
      private double _latchDistance = 1.5;
      private double _hitPoints = 1;
      private string _steal = "fuel ammo credits";
      private double _ammoAmount = 20;
      private double _shieldAmount = 25;
      private double _fuelAmount = 15;
      private double _creditsAmount = 25;
      private double _stealTime = 2.5;
      private double _gripForce = 0;
      private double _gripTime = 0.6;
      private double _stunTime = 1;
      private double _leaveTime = 2;
      private double _fadeInTime = 0.5;
      private double _leaveSpeed = 60;
      private double _sparkInterval = 0.3;
      private bool _antiGravity = true;
      private string _latchedShape = "";
      private string _flyingShape = "";
      private string _dropFile = "";

      #endregion

      #region Properties

      [XmlAttribute("maxSpeed")]
      public double MaxSpeed
      {
         get { return _maxSpeed; }
         set { _maxSpeed = value; }
      }

      [XmlAttribute("acceleration")]
      public double Acceleration
      {
         get { return _acceleration; }
         set { _acceleration = value; }
      }

      [XmlAttribute("degPerSecTurnRate")]
      public double DegPerSecTurnRate
      {
         get { return _degPerSecTurnRate; }
         set { _degPerSecTurnRate = value; }
      }

      [XmlAttribute("noseAngle")]
      public double NoseAngle
      {
         get { return _noseAngle; }
         set { _noseAngle = value; }
      }

      [XmlAttribute("lead")]
      public double Lead
      {
         get { return _lead; }
         set { _lead = value; }
      }

      [XmlAttribute("brakeDistance")]
      public double BrakeDistance
      {
         get { return _brakeDistance; }
         set { _brakeDistance = value; }
      }

      [XmlAttribute("latchSpeed")]
      public double LatchSpeed
      {
         get { return _latchSpeed; }
         set { _latchSpeed = value; }
      }

      [XmlAttribute("ramSpeed")]
      public double RamSpeed
      {
         get { return _ramSpeed; }
         set { _ramSpeed = value; }
      }

      [XmlAttribute("shieldDrain")]
      public double ShieldDrain
      {
         get { return _shieldDrain; }
         set { _shieldDrain = value; }
      }

      [XmlAttribute("latchDistance")]
      public double LatchDistance
      {
         get { return _latchDistance; }
         set { _latchDistance = value; }
      }

      [XmlAttribute("hitPoints")]
      public double HitPoints
      {
         get { return _hitPoints; }
         set { _hitPoints = value; }
      }

      [XmlAttribute("steal")]
      public string Steal
      {
         get { return _steal; }
         set { _steal = value; }
      }

      [XmlAttribute("ammoAmount")]
      public double AmmoAmount
      {
         get { return _ammoAmount; }
         set { _ammoAmount = value; }
      }

      [XmlAttribute("shieldAmount")]
      public double ShieldAmount
      {
         get { return _shieldAmount; }
         set { _shieldAmount = value; }
      }

      [XmlAttribute("fuelAmount")]
      public double FuelAmount
      {
         get { return _fuelAmount; }
         set { _fuelAmount = value; }
      }

      [XmlAttribute("creditsAmount")]
      public double CreditsAmount
      {
         get { return _creditsAmount; }
         set { _creditsAmount = value; }
      }

      [XmlAttribute("stealTime")]
      public double StealTime
      {
         get { return _stealTime; }
         set { _stealTime = value; }
      }

      [XmlAttribute("gripForce")]
      public double GripForce
      {
         get { return _gripForce; }
         set { _gripForce = value; }
      }

      [XmlAttribute("gripTime")]
      public double GripTime
      {
         get { return _gripTime; }
         set { _gripTime = value; }
      }

      [XmlAttribute("stunTime")]
      public double StunTime
      {
         get { return _stunTime; }
         set { _stunTime = value; }
      }

      [XmlAttribute("leaveTime")]
      public double LeaveTime
      {
         get { return _leaveTime; }
         set { _leaveTime = value; }
      }

      [XmlAttribute("fadeInTime")]
      public double FadeInTime
      {
         get { return _fadeInTime; }
         set { _fadeInTime = value; }
      }

      [XmlAttribute("leaveSpeed")]
      public double LeaveSpeed
      {
         get { return _leaveSpeed; }
         set { _leaveSpeed = value; }
      }

      [XmlAttribute("sparkInterval")]
      public double SparkInterval
      {
         get { return _sparkInterval; }
         set { _sparkInterval = value; }
      }

      [XmlAttribute("antiGravity")]
      public bool AntiGravity
      {
         get { return _antiGravity; }
         set { _antiGravity = value; }
      }

      [XmlAttribute("latchedShape")]
      public string LatchedShape
      {
         get { return _latchedShape; }
         set { _latchedShape = value; }
      }

      public bool ShouldSerializeLatchedShape()
      {
         return !string.IsNullOrEmpty(LatchedShape);
      }

      [XmlAttribute("flyingShape")]
      public string FlyingShape
      {
         get { return _flyingShape; }
         set { _flyingShape = value; }
      }

      public bool ShouldSerializeFlyingShape()
      {
         return !string.IsNullOrEmpty(FlyingShape);
      }

      [XmlAttribute("dropFile")]
      public string DropFile
      {
         get { return _dropFile; }
         set { _dropFile = value; }
      }

      public bool ShouldSerializeDropFile()
      {
         return !string.IsNullOrEmpty(DropFile);
      }

      #endregion
   }
}
