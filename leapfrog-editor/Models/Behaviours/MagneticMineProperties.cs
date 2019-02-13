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
   public class MagneticMineProperties
   {
      #region Declarations

      private double _magneticDistance;
      private double _magneticForce;
      private int _numOfParticles;
      private double _particlesRadius;
      private double _blastPower;

      #endregion

      #region Constructors

      public MagneticMineProperties()
      {
         _magneticForce = 0;
         _magneticForce = 0;
         _numOfParticles = 0;
         _particlesRadius = 0;
         _blastPower = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("magneticDistance")]
      public double MagneticDistance
      {
         get { return _magneticDistance; }
         set { _magneticDistance = value; }
      }

      [XmlAttribute("magneticForce")]
      public double MagneticForce
      {
         get { return _magneticForce; }
         set { _magneticForce = value; }
      }

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


      #endregion

   }

}
