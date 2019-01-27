using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
   public class BorderTextureViewModel : MicroMvvm.ViewModelBase
   {
      #region Declarations

      private LfStaticPolygonViewModel _parentVm;

      private BorderTexture _modelObject;

      #endregion

      #region Constructors

      public BorderTextureViewModel(LfStaticPolygonViewModel parentVm, BorderTexture modelObject)
      {
         _parentVm = parentVm;
         _modelObject = modelObject;
      }

      #endregion

      #region Properties

      public LfStaticPolygonViewModel ParentVm
      {
         get { return _parentVm; }
         set
         {
            _parentVm = value;
            OnPropertyChanged("ParentVm");
         }
      }


      public BorderTexture ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            OnPropertyChanged("ModelObject");
            ParentVm.InvalidateAllVertices();
         }
      }


      // Changes to any of the properties below must be reflected
      // to all vertices of the polygon. 
      // Lets do that in a private method.

      public string Texture
      {
         get { return _modelObject.Texture; }
         set
         {
            _modelObject.Texture = value;
            OnPropertyChanged("Texture");
            ParentVm.InvalidateAllVertices();
         }
      }

      public double HorisontalOffset
      {
         get { return _modelObject.HorisontalOffset; }
         set
         {
            _modelObject.HorisontalOffset = value;
            OnPropertyChanged("HorisontalOffset");
            ParentVm.InvalidateAllVertices();
         }
      }

      public double TextureWidth
      {
         get { return _modelObject.TextureWidth; }
         set
         {
            _modelObject.TextureWidth = value;
            OnPropertyChanged("TextureWidth");
            ParentVm.InvalidateAllVertices();
         }
      }

      public double TextureHeight
      {
         get { return _modelObject.TextureHeight; }
         set
         {
            _modelObject.TextureHeight = value;
            OnPropertyChanged("TextureHeight");
            ParentVm.InvalidateAllVertices();
         }
      }

      #endregion
   }
}
