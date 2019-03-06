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
   public class GunProperties : BodyOriginSystem
   {
      #region Declarations

      private double _angle;
      private double _fireRate;
      private int _lifeTime;
      private double _impulseMagnitude;
      private bool _bouncy;

      #endregion

      #region Constructors

      public GunProperties()
      {
         _angle = 0;
         _fireRate = 4;
         _lifeTime = 2000;
         _impulseMagnitude = 10000;
         _bouncy = false;
      }

      #endregion

      #region Properties

      [XmlAttribute("angle")]
      public double Angle
      {
         get { return _angle; }
         set { _angle = value; }
      }

      [XmlAttribute("fireRate")]
      public double FireRate
      {
         get { return _fireRate; }
         set { _fireRate = value; }
      }

      [XmlAttribute("lifeTime")]
      public int LifeTime
      {
         get { return _lifeTime; }
         set { _lifeTime = value; }
      }

      [XmlAttribute("impulseMagnitude")]
      public double ImpulseMagnitude
      {
         get { return _impulseMagnitude; }
         set { _impulseMagnitude = value; }
      }

      [XmlAttribute("bouncy")]
      public bool Bouncy
      {
         get { return _bouncy; }
         set { _bouncy = value; }
      }

      #endregion

   }
}
