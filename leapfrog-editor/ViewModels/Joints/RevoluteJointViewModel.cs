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
   class RevoluteJointViewModel : MotorizedJointViewModel, IMainVmInterface
   {
      #region Declarations

      private RevoluteJoint _modelObject;

      #endregion

      #region Constructors

      public RevoluteJointViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         RevoluteJoint modelObject,
         bool enabled = true)  :
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

      public new RevoluteJoint ModelObject
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

      public double MaxMotorTorque
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.MaxMotorTorque;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.MaxMotorTorque = value;
            OnPropertyChanged("MaxMotorTorque");
         }
      }



      #endregion

      #region public Methods

      #endregion
   }
}
