using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace LeapfrogEditor
{
   // Used to be ConditionalSelectTreeViewViewModel
   public class WeldJointViewModel : TreeViewViewModel
   {
      #region Declarations

      private WeldJoint _modelObject;
      protected LfShapeViewModel _aVm;
      protected LfShapeViewModel _bVm;

      #endregion

      #region Constructors

      public WeldJointViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm,  
         MainViewModel mainVm, 
         WeldJoint modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public WeldJoint ModelObject
      {
         get
         {
            if (_modelObject == null) return null;
            return _modelObject;
         }
         set
         {
            _modelObject = value;
            OnPropertyChanged("");
         }
      }

      public string Name
      {
         get
         {
            if (_modelObject == null) return "";

            return _modelObject.Name;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.Name = value;
            OnPropertyChanged("Name");
         }
      }

      public string AName
      {
         get
         {
            if (_modelObject == null) return "";

            return _modelObject.AName;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.AName = value;
            OnPropertyChanged("AName");
            ConnectToShapes(ParentVm.ShapeCollection);
            OnPropertyChanged("AShapeObject");
         }
      }

      public string BName
      {
         get
         {
            if (_modelObject == null) return "";

            return _modelObject.BName;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.BName = value;
            OnPropertyChanged("BName");
            OnPropertyChanged("ShapeBExists");
            ConnectToShapes(ParentVm.ShapeCollection);
            OnPropertyChanged("BShapeObject");
         }
      }

      public LfShapeViewModel AShapeObject
      {
         get
         {
            return _aVm;
         }
      }

      public LfShapeViewModel BShapeObject
      {
         get
         {
            return _bVm;
         }
      }

      public double AAnchorX
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.AAnchorX;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.AAnchorX = value;
            OnPropertyChanged("AAnchorX");
         }
      }

      public double AAnchorY
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.AAnchorY;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.AAnchorY = value;
            OnPropertyChanged("AAnchorY");
         }
      }

      public double BAnchorX
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.BAnchorX;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.BAnchorX = value;
            OnPropertyChanged("BAnchorX");
         }
      }

      public double BAnchorY
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.BAnchorY;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.BAnchorY = value;
            OnPropertyChanged("BAnchorY");
         }
      }

      #endregion

      #region public Methods

      public void ConnectToShapes(ShapeCollectionViewModel shapes)
      {
         _aVm = ParentVm.FindShape(ModelObject.AName, shapes);
         if (_aVm == null)
         {
            MessageBox.Show("The shape A pointed to by " + ModelObject.Name + " does not exists in CO " + ParentVm.Name, "Error parsing file", MessageBoxButton.OK, MessageBoxImage.Error);
         }

         _bVm = ParentVm.FindShape(ModelObject.BName, shapes);
         if (_bVm == null)
         {
            MessageBox.Show("The shape B pointed to by " + ModelObject.Name + " does not exists in CO " + ParentVm.Name, "Error parsing file", MessageBoxButton.OK, MessageBoxImage.Error);
         }
      }

      #endregion
   }
}
