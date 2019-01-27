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
   class StaticBoxViewModel : MicroMvvm.ViewModelBase, IAnchorableInterface, IShapeInterface
   {
      #region Declarations

      private MainViewModel _mainVm;
      private StaticBox _modelObject;
      private CompoundObjectViewModel _parent;

      private bool _isSelected;


      #endregion

      #region Constructors

      public StaticBoxViewModel(MainViewModel mainVm, CompoundObjectViewModel parent, StaticBox modelObject )
      {
         MainVm = mainVm;
         Parent = parent;
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public MainViewModel MainVm
      {
         get { return _mainVm; }
         set { _mainVm = value; }
      }

      public StaticBox ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            OnPropertyChanged("");
         }
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

      public string Name
      {
         get { return _modelObject.Name; }
         set
         {
            _modelObject.Name = value;
            OnPropertyChanged("Name");
         }
      }

      public string Texture
      {
         get { return _modelObject.Texture; }
         set
         {
            _modelObject.Texture = value;
            OnPropertyChanged("Texture");
         }
      }

      public double PosX
      {
         get
         {
            return _modelObject.PosX;
         }
         set
         {
            _modelObject.PosX = value;
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


      public double PosY
      {
         get
         {
            return _modelObject.PosY;
         }
         set
         {
            _modelObject.PosY = value;
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

      public double Angle
      {
         get { return _modelObject.Angle; }
         set
         {
            _modelObject.Angle = value;
            OnPropertyChanged("Angle");
         }
      }

      public double Width
      {
         get { return _modelObject.Width; }
         set
         {
            _modelObject.Width = value;
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
      
      public double Height
      {
         get { return _modelObject.Height; }
         set
         {
            _modelObject.Height = value;
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

      public double AnchorX
      {
         get { return _modelObject.AnchorX; }
         set
         {
            _modelObject.AnchorX = value;
            OnPropertyChanged("AnchorX");
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = Parent;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.Parent;
            }
         }
      }

      public double AnchorY
      {
         get { return _modelObject.AnchorY; }
         set
         {
            _modelObject.AnchorY = value;
            OnPropertyChanged("AnchorY");
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = Parent;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.Parent;
            }
         }
      }

      public string CollisionEntity
      {
         get { return _modelObject.CollisionEntity; }
         set
         {
            _modelObject.CollisionEntity = value;
            OnPropertyChanged("CollisionEntity");
         }
      }

      public uint ZLevel
      {
         get { return _modelObject.ZLevel; }
         set
         {
            _modelObject.ZLevel = value;
            OnPropertyChanged("ZLevel");
         }
      }

      public uint CollisionCategory
      {
         get { return _modelObject.CollisionCategory; }
         set
         {
            _modelObject.CollisionCategory = value;
            OnPropertyChanged("CollisionCategory");
         }
      }

      public uint CollisionMask
      {
         get { return _modelObject.CollisionMask; }
         set
         {
            _modelObject.CollisionMask = value;
            OnPropertyChanged("CollisionMask");
         }
      }

      public Rect BoundingBox
      {
         get
         {
            Rect r = new Rect(0, 0, Width, Height);
            r.Offset(new Vector(-AnchorX * Width, -AnchorY * Height));
            return r;
         }
         set
         { }
      }

      public bool IsSelected
      {
         get { return _isSelected; }
         set
         {
            _isSelected = value;
            OnPropertyChanged("IsSelected");
         }
      }

      #endregion

   }
}
