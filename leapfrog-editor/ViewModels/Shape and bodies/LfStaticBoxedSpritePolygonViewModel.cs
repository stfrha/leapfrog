using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace LeapfrogEditor
{
   class LfStaticBoxedSpritePolygonViewModel : LfPolygonViewModel, IWidthHeightInterface
   {
      #region Declarations

      #endregion

      #region Constructors

      public LfStaticBoxedSpritePolygonViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         LfStaticBoxedSpritePolygon modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public new LfStaticBoxedSpritePolygon LocalModelObject
      {
         get { return (LfStaticBoxedSpritePolygon)ModelObject; }
      }


      public double Width
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.Width;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.Width = value;
            OnPropertyChanged("Width");
            OnPropertyChanged("BoundingBox");
            OnPropertyChanged("");

            CompoundObjectViewModel p = ParentVm;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.ParentVm;
            }
         }
      }

      public double Height
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.Height;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.Height = value;
            OnPropertyChanged("Height");
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = ParentVm;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.ParentVm;
            }
         }
      }

      #endregion

      #region public Methods

      public void SetWidthToTextureAspectRatio()
      {
         // Get bitmap file and path
         string s = GlobalConstants.ImageDirectory + Texture + ".png";
         string fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
         string fullFileName = System.IO.Path.Combine(fullPath, s);

         // Get Bitmap width and height
         Uri u = new Uri(fullFileName);
         BitmapImage bi = new BitmapImage(u);

         // Set width
         Width = bi.Width / bi.Height * Height;

      }

      public void SetHeightToTextureAspectRatio()
      {
         // Get bitmap file and path
         string s = GlobalConstants.ImageDirectory + Texture + ".png";
         string fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
         string fullFileName = System.IO.Path.Combine(fullPath, s);

         // Get Bitmap width and height
         Uri u = new Uri(fullFileName);
         BitmapImage bi = new BitmapImage(u);

         // Set width
         Height = bi.Height / bi.Width * Width;
      }

      #endregion
   }
}
