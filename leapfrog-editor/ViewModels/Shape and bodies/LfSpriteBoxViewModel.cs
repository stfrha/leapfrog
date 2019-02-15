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
   class LfSpriteBoxViewModel : LfShapeViewModel, IWidthHeightInterface, IBoxPointsInterface
   {
      #region Declarations

      private ObservableCollection<LfPointViewModel> _points = new ObservableCollection<LfPointViewModel>();

      #endregion

      #region Constructors

      public LfSpriteBoxViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         LfSpriteBox modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;
         UpdateCornerPoints();
      }

      #endregion

      #region Properties

      public LfSpriteBox LocalModelObject
      {
         get { return (LfSpriteBox)ModelObject; }
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
            UpdateCornerPoints();
            OnPropertyChanged("Width");
            OnPropertyChanged("BoundingBox");

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
            UpdateCornerPoints();
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

      public ObservableCollection<LfPointViewModel> PointVms
      {
         get { return _points; }
         set { _points = value; ; }
      }


      #endregion

      #region private Methods

      private void UpdateCornerPoints()
      {
         _points.Clear();

         _points.Add(new LfPointViewModel(MainVm, this, new Point(-Width / 2, -Height / 2)));
         _points.Add(new LfPointViewModel(MainVm, this, new Point(Width / 2, -Height / 2)));
         _points.Add(new LfPointViewModel(MainVm, this, new Point(Width / 2, Height / 2)));
         _points.Add(new LfPointViewModel(MainVm, this, new Point(-Width / 2, Height / 2)));

         OnPropertyChanged("PointVms");
      }

      #endregion

      #region protected Methods

      protected override Rect GetBoundingBox()
      {
         double l = double.MaxValue;
         double r = double.MinValue;
         double t = double.MaxValue;
         double b = double.MinValue;

         foreach (LfPointViewModel p in PointVms)
         {
            // Convert point according to angle
            Point rtp = CoordinateTransformations.RotatedPointFromLocal(new Point(p.PosX, p.PosY), Angle);


            if (rtp.X < l)
            {
               l = rtp.X;
            }

            if (rtp.X > r)
            {
               r = rtp.X;
            }

            if (rtp.Y < t)
            {
               t = rtp.Y;
            }

            if (rtp.Y > b)
            {
               b = rtp.Y;
            }
         }
         Rect tr = new Rect(new Point(l, t), new Point(r, b));

         return tr;
      }
      
      #endregion

      #region public Methods

      public override void InvalidateAll()
      {
         UpdateCornerPoints();
         OnPropertyChanged("");

      }

      public void SetWidthToTextureAspectRatio()
      {
         // Get bitmap file and path
         string s = GlobalConstants.DataDirectory +"images\\" + Texture + ".png";
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
