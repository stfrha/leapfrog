using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace LeapfrogEditor
{
   class ScalableTexturePolygonViewModel : EditablePolygonViewModel
   {
      #region Declarations

      private ScalableTexturePolygon _texturePolygonObject;

      #endregion

      #region Constructors

      public ScalableTexturePolygonViewModel(MainViewModel mainVm, CompoundObjectViewModel parent, ScalableTexturePolygon pointObject) :
         base(mainVm, parent, pointObject)
      {
         TexturePolygonObject = pointObject;
      }

      #endregion

      #region Properties

      public ScalableTexturePolygon TexturePolygonObject
      {
         get { return _texturePolygonObject; }
         set
         {
            _texturePolygonObject = value;
            OnPropertyChanged("");
         }
      }

      public double TextureWidth
      {
         get
         {
            return _texturePolygonObject.TextureWidth;
         }
         set
         {
            _texturePolygonObject.TextureWidth = value;
            OnPropertyChanged("TextureWidth");
         }
      }

      public double TextureHeight
      {
         get
         {
            return _texturePolygonObject.TextureHeight;
         }
         set
         {
            _texturePolygonObject.TextureHeight = value;
            OnPropertyChanged("TextureHeight");
         }
      }

      public double TextureOffsetX
      {
         get
         {
            return _texturePolygonObject.TextureOffsetX;
         }
         set
         {
            _texturePolygonObject.TextureOffsetX = value;
            OnPropertyChanged("TextureOffsetX");
         }
      }

      public double TextureOffsetY
      {
         get
         {
            return _texturePolygonObject.TextureOffsetY;
         }
         set
         {
            _texturePolygonObject.TextureOffsetY = value;
            OnPropertyChanged("TextureOffsetY");
         }
      }


      #endregion

      #region public Methods

      #endregion

   }
}
