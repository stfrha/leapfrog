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
   public class ReentryFlameEmitterProperties
   {
      #region Declarations

      private string _bodyName;
      private double _emitterLineStartX;
      private double _emitterLineStartY;
      private double _emitterLineEndX;
      private double _emitterLineEndY;
      private double _angle;
      private double _intensity;
      private int _lifeTime;
      private double _maxWidth;
      private double _maxHeight;

      #endregion

      #region Constructors

      public ReentryFlameEmitterProperties()
      {
         _bodyName = "notDefined";
         _emitterLineStartX = 0;
         _emitterLineStartY = 0;
         _emitterLineEndX = 0;
         _emitterLineEndY = 0;
         _angle = 0;
         _intensity = 0;
         _lifeTime = 60000;
         _maxWidth = 0;
         _maxHeight = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("body")]
      public string BodyName
      {
         get { return _bodyName; }
         set { _bodyName = value; }
      }

      [XmlAttribute("emitterLineStartX")]
      public double EmitterLineStartX
      {
         get { return _emitterLineStartX; }
         set { _emitterLineStartX = value; }
      }

      [XmlAttribute("emitterLineStartY")]
      public double EmitterLineStartY
      {
         get { return _emitterLineStartY; }
         set { _emitterLineStartY = value; }
      }

      [XmlAttribute("emitterLineEndX")]
      public double EmitterLineEndX
      {
         get { return _emitterLineEndX; }
         set { _emitterLineEndX = value; }
      }

      [XmlAttribute("emitterLineEndY")]
      public double EmitterLineEndY
      {
         get { return _emitterLineEndY; }
         set { _emitterLineEndY = value; }
      }

      [XmlAttribute("angle")]
      public double Angle
      {
         get { return _angle; }
         set { _angle = value; }
      }

      [XmlAttribute("intensity")]
      public double Intensity
      {
         get { return _intensity; }
         set { _intensity = value; }
      }

      [XmlAttribute("lifeTime")]
      public int LifeTime
      {
         get { return _lifeTime; }
         set { _lifeTime = value; }
      }

      [XmlAttribute("maxWidth")]
      public double MaxWidth
      {
         get { return _maxWidth; }
         set { _maxWidth = value; }
      }

      [XmlAttribute("maxHeight")]
      public double MaxHeight
      {
         get { return _maxHeight; }
         set { _maxHeight = value; }
      }


      #endregion

   }
}
