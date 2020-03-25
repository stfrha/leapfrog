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
   public class ExplosiveObjectProperties
   {
      #region Declarations

      private int _numOfParticles;
      private double _particlesRadius;
      private double _blastPower;
      private double _damageBulletEqv;
      private bool _impactExplosion;
      private double _impactThreshold;

      #endregion

      #region Constructors

      public ExplosiveObjectProperties()
      {
         _numOfParticles = 0;
         _particlesRadius = 0;
         _blastPower = 0;
         _damageBulletEqv = 1;
         _impactExplosion = true;
         _impactThreshold = 2500;
      }

      #endregion

      #region Properties

      [XmlAttribute("numOfParticles")]
      public int NumOfParticles
      {
         get { return _numOfParticles; }
         set { _numOfParticles = value; }
      }

      [XmlAttribute("particleRadius")]
      public double ParticlesRadius
      {
         get { return _particlesRadius; }
         set { _particlesRadius = value; }
      }

      [XmlAttribute("blastPower")]
      public double BlastPower
      {
         get { return _blastPower; }
         set { _blastPower = value; }
      }

      [XmlAttribute("damageBulletEqv")]
      public double DamageBulletEqv
      {
         get { return _damageBulletEqv; }
         set { _damageBulletEqv = value; }
      }

      [XmlAttribute("impactExplosion")]
      public bool ImpactExplosion
      {
         get { return _impactExplosion; }
         set { _impactExplosion = value; }
      }

      [XmlAttribute("impactThreshold")]
      public double ImpactThreshold
      {
         get { return _impactThreshold; }
         set { _impactThreshold = value; }
      }


      #endregion

   }
}
