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
   public class ClippedWindowProperties
   {
      #region Declarations

      private string _background;
      private double _posY;
      private double _posX;
      private double _width;
      private double _height;
      private double _zoomScale;
      private double _frameThickness;
      private string _spaceSceneFile;
      private CompoundObject _compObj = null;


      #endregion

      #region Constructors

      public ClippedWindowProperties()
      {
         _background = "starfield";
         _posX = 550;
         _posY = 0;
         _width = 300;
         _height = 150;
         _zoomScale = 0.4;
         _spaceSceneFile = "noScene";
      }

      #endregion

      #region Properties

      [XmlAttribute("background")]
      public string Background
      {
         get { return _background; }
         set { _background = value; }
      }

      [XmlAttribute("posX")]
      public double PosX
      {
         get { return _posX; }
         set { _posX = value; }
      }

      [XmlAttribute("posY")]
      public double PosY
      {
         get { return _posY; }
         set { _posY = value; }
      }

      [XmlAttribute("width")]
      public double Width
      {
         get { return _width; }
         set { _width = value; }
      }

      [XmlAttribute("height")]
      public double Height
      {
         get { return _height; }
         set { _height = value; }
      }

      [XmlAttribute("zoomScale")]
      public double ZoomScale
      {
         get { return _zoomScale; }
         set { _zoomScale = value; }
      }


      [XmlAttribute("frameThickness")]
      public double FrameThickness
      {
         get { return _frameThickness; }
         set { _frameThickness = value; }
      }

      [XmlAttribute("spaceSceneFile")]
      public string SpaceSceneFile
      {
         get { return _spaceSceneFile; }
         set { _spaceSceneFile = value; }
      }

      [XmlIgnore]
      public CompoundObject CompObj
      {
         get { return _compObj; }
         set { _compObj = value; }
      }

      #endregion

   }
}
