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
   public class BorderTexture
   {
      #region Declarations

      private string _texture;
      private double _horisontalOffset;
      private double _textureWidth;
      private double _textureHeight;

      #endregion

      #region Constructors

      public BorderTexture()
      {
         _texture = "notDefined";
         _horisontalOffset = 0;
         _textureHeight = 0;
         _textureWidth = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("texture")]
      public string Texture
      {
         get { return _texture; }
         set { _texture = value; }
      }

      [XmlAttribute("horisontalMeterOffset")]
      public double HorisontalOffset
      {
         get { return _horisontalOffset; }
         set { _horisontalOffset = value; }
      }

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

      #endregion

   }
}
