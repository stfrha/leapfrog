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
   class MotorizedJointViewModel : WeldJointViewModel, IMainVmInterface
   {
      #region Declarations

      private MotorizedJoint _modelObject;

      #endregion

      #region Constructors

      public MotorizedJointViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         MotorizedJoint modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, modelObject, enabled)
      {
         ModelObject = modelObject;

         //_aVm = ParentVm.FindShape(ModelObject.AName);
         //if (_aVm == null)
         //{
         //   MessageBox.Show("The shape A pointed to by " + ModelObject.Name + " does not exists in CO " + ParentVm.Name, "Error parsing file", MessageBoxButton.OK, MessageBoxImage.Error);
         //}

         //_bVm = ParentVm.FindShape(ModelObject.BName);
         //if (_bVm == null)
         //{
         //   MessageBox.Show("The shape B pointed to by " + ModelObject.Name + " does not exists in CO " + ParentVm.Name, "Error parsing file", MessageBoxButton.OK, MessageBoxImage.Error);
         //}
      }

      #endregion

      #region Properties

      public new MotorizedJoint ModelObject
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

      public bool LockJoint
      {
         get
         {
            if (_modelObject == null) return false;

            return _modelObject.LockJoint;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.LockJoint = value;
            OnPropertyChanged("LockJoint");
         }
      }

      public bool UseMotor
      {
         get
         {
            if (_modelObject == null) return false;

            return _modelObject.UseMotor;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.UseMotor = value;
            OnPropertyChanged("UseMotor");
         }
      }

      public double MotorSpeed
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.MotorSpeed;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.MotorSpeed = value;
            OnPropertyChanged("MotorSpeed");
         }
      }

      #endregion

      #region public Methods

      #endregion
   }
}
