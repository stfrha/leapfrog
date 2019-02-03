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

      #endregion

      #region Constructors

      public ExplosiveObjectProperties()
      {
         _numOfParticles = 0;
         _particlesRadius = 0;
         _blastPower = 0;
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

      #endregion

   }
}
