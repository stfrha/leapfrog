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
   class SteerableObjectPropertiesViewModel : BehaviourViewModelBase
   {
      #region Declarations

      private SteerableObjectProperties ModelObject;

      #endregion

      #region Constructors

      public SteerableObjectPropertiesViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         SteerableObjectProperties modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public SteerableObjectProperties LocalModelObject
      {
         get { return ModelObject; }
      }

      public string SteeringState
      {
         get { return LocalModelObject.SteeringState; }
         set
         {
            LocalModelObject.SteeringState = value;
            OnPropertyChanged("SteeringState");
         }
      }

      public string BodyName
      {
         get { return LocalModelObject.BodyName; }
         set
         {
            LocalModelObject.BodyName = value;
            OnPropertyChanged("BodyName");
         }
      }

      public double BodyToBoosterAngle
      {
         get { return LocalModelObject.BodyToBoosterAngle; }
         set
         {
            LocalModelObject.BodyToBoosterAngle = value;
            OnPropertyChanged("BodyToBoosterAngle");
         }
      }

      public double SeekTargetX
      {
         get { return LocalModelObject.SeekTargetX; }
         set
         {
            LocalModelObject.SeekTargetX = value;
            OnPropertyChanged("SeekTargetX");
         }
      }

      public double SeekTargetY
      {
         get { return LocalModelObject.SeekTargetY; }
         set
         {
            LocalModelObject.SeekTargetY = value;
            OnPropertyChanged("SeekTargetY");
         }
      }

      public double MaxBoosterForce
      {
         get { return LocalModelObject.MaxBoosterForce; }
         set
         {
            LocalModelObject.MaxBoosterForce = value;
            OnPropertyChanged("MaxBoosterForce");
         }
      }

      public double MaxSpeed
      {
         get { return LocalModelObject.MaxSpeed; }
         set
         {
            LocalModelObject.MaxSpeed = value;
            OnPropertyChanged("MaxSpeed");
         }
      }

      public double DegPerSecMaxRotateSpeed
      {
         get { return LocalModelObject.DegPerSecMaxRotateSpeed; }
         set
         {
            LocalModelObject.DegPerSecMaxRotateSpeed = value;
            OnPropertyChanged("DegPerSecMaxRotateSpeed");
         }
      }

      public double LinearDamping
      {
         get { return LocalModelObject.LinearDamping; }
         set
         {
            LocalModelObject.LinearDamping = value;
            OnPropertyChanged("LinearDamping");
         }
      }

      #endregion

      #region private Methods

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      #endregion
   }
}
