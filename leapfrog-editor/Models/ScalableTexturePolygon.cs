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
   public class ScalableTexturePolygon : EditablePolygon
    {
      #region Declarations

      private double _textureWidth;
      private double _textureHeight;
      private double _textureOffsetX;
      private double _textureOffsetY;

      #endregion

      #region Constructors

      public ScalableTexturePolygon()
      {
         _textureWidth = 10;
         _textureHeight = 10;
         _textureOffsetX = 0;
         _textureOffsetY = 0;
      }

      public ScalableTexturePolygon(int width, int height, int x, int y )
      {
         _textureWidth = width;
         _textureHeight = height;
         _textureOffsetX = x;
         _textureOffsetY = y;
      }

      #endregion

      #region Properties

      [XmlAttribute("textureMeterWidth")]
      public double TextureWidth
      {
         get { return _textureWidth; }
         set { _textureWidth = value; }
      }

      [XmlAttribute("textureMeterHeight")]
      public double TextureHeight
      {
         get { return _textureHeight; }
         set { _textureHeight = value; }
      }

      [XmlAttribute("textureOffsetMeterX")]
      public double TextureOffsetX
      {
         get { return _textureOffsetX; }
         set { _textureOffsetX = value; }
      }

      [XmlAttribute("textureOffsetMeterY")]
      public double TextureOffsetY
      {
         get { return _textureOffsetY; }
         set { _textureOffsetY = value; }
      }

      #endregion
   }
}
