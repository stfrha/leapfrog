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
   public class LfScalableTexturePolygonViewModel : LfPolygonViewModel
   {
      #region Declarations

      #endregion

      #region Constructors

      public LfScalableTexturePolygonViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
      }

      #endregion

      #region Properties

      public new LfScalableTexturePolygon LocalModelObject
      {
         get { return (LfScalableTexturePolygon)ModelObject; }
      }


      public double TextureWidth
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.TextureWidth;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.TextureWidth = value;
            OnPropertyChanged("TextureWidth");
         }
      }

      public double TextureHeight
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.TextureHeight;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.TextureHeight = value;
            OnPropertyChanged("TextureHeight");
         }
      }

      public double TextureOffsetX
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.TextureOffsetX;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.TextureOffsetX = value;
            OnPropertyChanged("TextureOffsetX");
         }
      }

      public double TextureOffsetY
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.TextureOffsetY;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.TextureOffsetY = value;
            OnPropertyChanged("TextureOffsetY");
         }
      }


      #endregion

   }
}
