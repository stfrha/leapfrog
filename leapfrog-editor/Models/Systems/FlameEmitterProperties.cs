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
   public class FlameEmitterProperties : BodyOriginSystem
   {
      #region Declarations

      private double _angle;
      private double _emitterWidth;
      private int _lifeTime;
      private double _impulseMagnitude;
      private double _radius;

      #endregion

      #region Constructors

      public FlameEmitterProperties()
      {
         _angle = 0;
         _emitterWidth = 10;
         _lifeTime = 60000;
         _impulseMagnitude = 100;
         _radius = 100;
      }

      #endregion

      #region Properties

      [XmlAttribute("angle")]
      public double Angle
      {
         get { return _angle; }
         set { _angle = value; }
      }

      [XmlAttribute("emitterWidth")]
      public double EmitterWidth
      {
         get { return _emitterWidth; }
         set { _emitterWidth = value; }
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

      [XmlAttribute("radius")]
      public double Radius
      {
         get { return _radius; }
         set { _radius = value; }
      }

      #endregion

   }
}
