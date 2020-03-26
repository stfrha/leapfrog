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
   public class RopeViewModel : WeldJointViewModel, IMainVmInterface
   {
      #region Declarations

      private Rope _modelObject;

      #endregion

      #region Constructors

      public RopeViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         Rope modelObject,
         bool enabled = true) 
         : base(treeParent, parentVm, mainVm, modelObject, enabled)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public new Rope ModelObject
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

      public string Texture
      {
         get
         {
            if (_modelObject == null) return "";

            return _modelObject.Texture;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.Texture = value;
            OnPropertyChanged("Texture");
         }
      }

      public double Length
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.Length;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.Length = value;
            OnPropertyChanged("Length");
         }
      }

      public double Thickness
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.Thickness;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.Thickness = value;
            OnPropertyChanged("Thickness");
         }
      }

      public uint NoOfSegments
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.NoOfSegments;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.NoOfSegments = value;
            OnPropertyChanged("NoOfSegments");
         }
      }

      public uint ZLevel
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.ZLevel;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.ZLevel = value;
            OnPropertyChanged("ZLevel");
         }
      }

      public string CollisionEntity
      {
         get
         {
            if (_modelObject == null) return "";

            return _modelObject.CollisionEntity;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.CollisionEntity = value;
            OnPropertyChanged("CollisionEntity");
         }
      }

      public uint CollisionCategory
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.CollisionCategory;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.CollisionCategory = value;
            OnPropertyChanged("CollisionCategory");
         }
      }

      public uint CollisionMask
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.CollisionMask;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.CollisionMask = value;
            OnPropertyChanged("CollisionMask");
         }
      }

      public bool ShapeBExists
      {
         get
         {
            return (BName != "notDef");
         }
      }

      #endregion

      #region public Methods

      public new void ConnectToShapes(CompositeCollection coll)
      {
         _aVm = ParentVm.FindShape(ModelObject.AName, coll);
         if (_aVm == null)
         {
            MessageBox.Show("The shape A pointed to by " + ModelObject.Name + " does not exists in CO " + ParentVm.Name, "Error parsing file", MessageBoxButton.OK, MessageBoxImage.Error);
         }

         if (ModelObject.BName != "notDef")
         {
            _bVm = ParentVm.FindShape(ModelObject.BName, coll);
            if (_bVm == null)
            {
               MessageBox.Show("The shape B pointed to by " + ModelObject.Name + " does not exists in CO " + ParentVm.Name, "Error parsing file", MessageBoxButton.OK, MessageBoxImage.Error);
            }
         }
      }

      #endregion
   }
}
