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
   // TODO: The object factory has a position, width and height
   class ObjectFactoryPropertiesViewModel : SystemViewModelBase, IPositionInterface, IBoxPointsInterface, ISpawnObjectParentVmInterface
   {
      #region Declarations

      private ObjectFactoryProperties _modelObject;
      private CompoundObjectViewModel _parent;
      private ObservableCollection<LfPointViewModel> _points = new ObservableCollection<LfPointViewModel>();
      private LfShapeViewModel _bodyObject = null;
      private ObservableCollection<SpawnObjectViewModel> _spawnObjects = new ObservableCollection<SpawnObjectViewModel>();

      #endregion

      #region Constructors

      public ObjectFactoryPropertiesViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm, 
         ObjectFactoryProperties modelObject,
         CoSystemViewModel systemViewModel) :
         base(treeParent, parentVm, mainVm, systemViewModel)
      {
         _modelObject = modelObject;

         if (_modelObject != null)
         {
            _bodyObject = ParentVm.FindBodyObject(_modelObject.Body);
         }

         foreach (SpawnObject so in LocalModelObject.SpawnObjects)
         {
            SpawnObjectViewModel sovm = new SpawnObjectViewModel(this, parentVm, mainVm, this, so);
            SpawnObjects.Add(sovm);
         }

         UpdateCornerPoints();
      }

      #endregion

      #region Properties

      public ObjectFactoryProperties LocalModelObject
      {
         get { return (ObjectFactoryProperties)_modelObject; }
      }

      public ISpawnObjectParentInterface SpawnParentModelObject
      {
         get { return _modelObject as ISpawnObjectParentInterface; }
      }

      public CompoundObjectViewModel Parent
      {
         get { return _parent; }
         set
         {
            _parent = value;
            OnPropertyChanged("");
         }
      }

      public string Body
      {
         get { return _modelObject.Body; }
         set
         {
            _modelObject.Body = value;
            _bodyObject = ParentVm.FindBodyObject(_modelObject.Body);
            OnPropertyChanged("Body");
         }
      }

      public LfShapeViewModel BodyObject
      {
         get
         {
            return _bodyObject;
         }
      }



      public ObservableCollection<SpawnObjectViewModel> SpawnObjects
      {
         get { return _spawnObjects; }
         set
         {
            _spawnObjects = value;
            OnPropertyChanged("SpawnObjects");
         }
      }

      public double PosX
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.PosX;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.PosX = value;
            OnPropertyChanged("PosX");
            OnPropertyChanged("AbsPosX");
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = Parent;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p.InvalidateJoints();
               p = p.ParentVm;
            }
         }
      }

      public double AbsPosX
      {
         get
         {
            double parentPos = 0;

            if (ParentVm != null)
            {
               parentPos = ParentVm.AbsPosX;
            }

            if (_modelObject == null) return parentPos;

            return parentPos + _modelObject.PosX;
         }
      }

      public double PosY
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.PosY;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.PosY = value;
            OnPropertyChanged("PosY");
            OnPropertyChanged("AbsPosY");
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = Parent;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p.InvalidateJoints();
               p = p.ParentVm;
            }
         }
      }

      public double AbsPosY
      {
         get
         {
            double parentPos = 0;

            if (ParentVm != null)
            {
               parentPos = ParentVm.AbsPosY;
            }

            if (_modelObject == null) return parentPos;

            return parentPos + _modelObject.PosY;
         }
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

            CompoundObjectViewModel p = Parent;

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

            CompoundObjectViewModel p = Parent;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.ParentVm;
            }
         }
      }

      public double Angle
      {
         get
         {
            if (_bodyObject != null)
            {
               return _bodyObject.Angle;
            }

            return 0;
         }
      }

      public ObservableCollection<LfPointViewModel> PointVms
      {
         get { return _points; }
         set { _points = value; ; }
      }

      public int SpawnInitial
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.SpawnInitial;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.SpawnInitial = value;
            OnPropertyChanged("SpawnInitial");
         }
      }

      public double Intensity
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.Intensity;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.Intensity = value;
            OnPropertyChanged("Intensity");
         }
      }

      public int LifeTime
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.LifeTime;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.LifeTime = value;
            OnPropertyChanged("LifeTime");
         }
      }

      // We also expose some properties of the 

      #endregion

      #region private Methods

      private void UpdateCornerPoints()
      {
         _points.Clear();

         _points.Add(new LfPointViewModel(MainVm, this, new Point(0, 0)));
         _points.Add(new LfPointViewModel(MainVm, this, new Point(Width, 0)));
         _points.Add(new LfPointViewModel(MainVm, this, new Point(Width, Height)));
         _points.Add(new LfPointViewModel(MainVm, this, new Point(0, Height)));

         OnPropertyChanged("PointVms");
      }

      #endregion

      #region protected Methods


      #endregion

      #region public Methods

      #endregion
   }
}
