﻿using System;
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
   class ObjectFactoryPropertiesViewModel : SystemViewModelBase, IPositionInterface, IBoxPointsInterface
   {
      #region Declarations

      private ObjectFactoryProperties _modelObject;
      private CompoundObjectViewModel _parent;
      private ObservableCollection<LfPointViewModel> _points = new ObservableCollection<LfPointViewModel>();

      #endregion

      #region Constructors

      public ObjectFactoryPropertiesViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm, 
         ObjectFactoryProperties modelObject) :
         base(treeParent, parentVm, mainVm)
      {
         _modelObject = modelObject;
      }

      #endregion

      #region Properties

      public ObjectFactoryProperties LocalModelObject
      {
         get { return (ObjectFactoryProperties)_modelObject; }
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


      #endregion

      #region public Methods


      #endregion
   }
}
