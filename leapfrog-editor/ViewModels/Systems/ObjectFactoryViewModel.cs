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
   class ObjectFactoryViewModel : LfShapeViewModel, IWidthHeightInterface, IBoxPointsInterface
   {
      #region Declarations

      private int _selectedStateIndex = 0;

      // Children collection is two dimensional to accomondate for all State properties
      private ObservableCollection<ObservableCollection<ObjectFactoryProperties>> _childObjects = new ObservableCollection<ObservableCollection<ObjectFactoryProperties>>();

      private new ObjectFactoryRef ModelObject;

      private ObservableCollection<LfPointViewModel> _points = new ObservableCollection<LfPointViewModel>();


      #endregion

      #region Constructors

      public ObjectFactoryViewModel(MainViewModel mainVm, CompoundObjectViewModel parent, ObjectFactoryRef modelObject) :
         base(mainVm, parent)
      {
         ModelObject = modelObject;
         UpdateCornerPoints();
      }

      #endregion

      #region Properties

      public ObjectFactoryRef LocalModelObject
      {
         get { return (ObjectFactoryRef)ModelObject; }
      }

      public override string Name
      {
         get { return LocalModelObject.Name; }
         set
         {
            LocalModelObject.Name = value;
            OnPropertyChanged("Name");
         }
      }

      public override double PosX
      {
         get
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > 0))
               {
                  return LocalModelObject.StateProperties[_selectedStateIndex].Properties.PosX;
               }
            }
            return 0;
         }
         set
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > 0))
               {
                  LocalModelObject.StateProperties[_selectedStateIndex].Properties.PosX = value;

                  OnPropertyChanged("PosX");
                  OnPropertyChanged("BoundingBox");

                  CompoundObjectViewModel p = Parent;

                  while (p != null)
                  {
                     p.OnPropertyChanged("BoundingBox");
                     p = p.Parent;
                  }
               }
            }
         }
      }

      public override double PosY
      {
         get
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > _selectedStateIndex))
               {
                  return LocalModelObject.StateProperties[_selectedStateIndex].Properties.PosY;
               }
            }
            return 0;
         }
         set
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > 0))
               {
                  LocalModelObject.StateProperties[_selectedStateIndex].Properties.PosY = value;

                  OnPropertyChanged("PosY");
                  OnPropertyChanged("BoundingBox");

                  CompoundObjectViewModel p = Parent;

                  while (p != null)
                  {
                     p.OnPropertyChanged("BoundingBox");
                     p = p.Parent;
                  }
               }
            }
         }
      }

      public double Width
      {
         get
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > _selectedStateIndex))
               {
                  return LocalModelObject.StateProperties[_selectedStateIndex].Properties.Width;
               }
            }
            return 0;
         }
         set
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > 0))
               {
                  LocalModelObject.StateProperties[_selectedStateIndex].Properties.Width = value;

                  UpdateCornerPoints();
                  OnPropertyChanged("Width");
                  OnPropertyChanged("BoundingBox");

                  CompoundObjectViewModel p = Parent;

                  while (p != null)
                  {
                     p.OnPropertyChanged("BoundingBox");
                     p = p.Parent;
                  }
               }
            }
         }
      }

      public double Height
      {
         get
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > _selectedStateIndex))
               {
                  return LocalModelObject.StateProperties[_selectedStateIndex].Properties.Height;
               }
            }
            return 0;
         }
         set
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > 0))
               {
                  LocalModelObject.StateProperties[_selectedStateIndex].Properties.Height = value;

                  UpdateCornerPoints();
                  OnPropertyChanged("Height");
                  OnPropertyChanged("BoundingBox");

                  CompoundObjectViewModel p = Parent;

                  while (p != null)
                  {
                     p.OnPropertyChanged("BoundingBox");
                     p = p.Parent;
                  }
               }
            }
         }
      }

      public int SpawnInitial
      {
         get
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > _selectedStateIndex))
               {
                  return LocalModelObject.StateProperties[_selectedStateIndex].Properties.SpawnInitial;
               }
            }
            return 0;
         }
         set
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > 0))
               {
                  LocalModelObject.StateProperties[_selectedStateIndex].Properties.SpawnInitial = value;

                  OnPropertyChanged("SpawnInitial");
               }
            }
         }
      }

      public double Intensity
      {
         get
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > _selectedStateIndex))
               {
                  return LocalModelObject.StateProperties[_selectedStateIndex].Properties.Intensity;
               }
            }
            return 0;
         }
         set
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > 0))
               {
                  LocalModelObject.StateProperties[_selectedStateIndex].Properties.Intensity = value;

                  OnPropertyChanged("Intensity");
               }
            }
         }
      }

      public int LifeTime
      {
         get
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > _selectedStateIndex))
               {
                  return LocalModelObject.StateProperties[_selectedStateIndex].Properties.LifeTime;
               }
            }
            return 0;
         }
         set
         {
            if (LocalModelObject != null)
            {
               if ((_selectedStateIndex >= 0) && (LocalModelObject.StateProperties.Count > 0))
               {
                  LocalModelObject.StateProperties[_selectedStateIndex].Properties.LifeTime = value;

                  OnPropertyChanged("LifeTime");
               }
            }
         }
      }


      public int SelectedStateIndex
      {
         get
         {
            return _selectedStateIndex;
         }
         set
         {
            if (value == -1)
            {
               _selectedStateIndex = 0;
            }
            else
            {
               _selectedStateIndex = value;
            }

            UpdateCornerPoints();

            OnPropertyChanged("");

            CompoundObjectViewModel p = Parent;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.Parent;
            }

         }
      }

      public ObservableCollection<string> States
      {
         get
         {
            ObservableCollection<string> s = new ObservableCollection<string>();

            foreach (TStateProperties<ObjectFactoryProperties> sp in LocalModelObject.StateProperties)
            {
               s.Add(sp.State);
            }

            return s;
         }
         set { }
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

         _points.Add(new LfPointViewModel(MainVm, this, new Point(- Width / 2, - Height / 2)));
         _points.Add(new LfPointViewModel(MainVm, this, new Point(Width / 2, - Height / 2)));
         _points.Add(new LfPointViewModel(MainVm, this, new Point(Width / 2, Height / 2)));
         _points.Add(new LfPointViewModel(MainVm, this, new Point(- Width / 2, Height / 2)));

         //_points.Add(new LfPointViewModel(MainVm, this, new Point(PosX - Width / 2, PosY - Height / 2)));
         //_points.Add(new LfPointViewModel(MainVm, this, new Point(PosX + Width / 2, PosY - Height / 2)));
         //_points.Add(new LfPointViewModel(MainVm, this, new Point(PosX + Width / 2, PosY + Height / 2)));
         //_points.Add(new LfPointViewModel(MainVm, this, new Point(PosX - Width / 2, PosY + Height / 2)));

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
            Point rtp = RotatedPointFromLocal(new Point(p.PosX, p.PosY));

            //rtp.Offset(PosX, PosY);

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

      public void SetWidthToTextureAspectRatio()
      {
         //   // Get bitmap file and path
         //   string s = @".\..\..\..\leapfrog\data\images\" + Texture + ".png";
         //   string fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
         //   string fullFileName = System.IO.Path.Combine(fullPath, s);

         //   // Get Bitmap width and height
         //   Uri u = new Uri(fullFileName);
         //   BitmapImage bi = new BitmapImage(u);

         //   // Set width
         //   Width = bi.Width / bi.Height * Height;

      }

      public void SetHeightToTextureAspectRatio()
      {
         //   // Get bitmap file and path
         //   string s = @".\..\..\..\leapfrog\data\images\" + Texture + ".png";
         //   string fullPath = System.IO.Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
         //   string fullFileName = System.IO.Path.Combine(fullPath, s);

         //   // Get Bitmap width and height
         //   Uri u = new Uri(fullFileName);
         //   BitmapImage bi = new BitmapImage(u);

         //   // Set width
         //   Height = bi.Height / bi.Width * Width;
      }

      public override void InvalidateAll()
      {
         UpdateCornerPoints();
         OnPropertyChanged("");

      }

      #endregion
   }
}
