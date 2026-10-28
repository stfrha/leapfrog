using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
   class StripperPropertiesViewModel : BehaviourViewModelBase
   {
      private StripperProperties ModelObject;

      public StripperPropertiesViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         StripperProperties modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;
      }

      public StripperProperties LocalModelObject
      {
         get { return ModelObject; }
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

      public double Acceleration
      {
         get { return LocalModelObject.Acceleration; }
         set
         {
            LocalModelObject.Acceleration = value;
            OnPropertyChanged("Acceleration");
         }
      }

      public double DegPerSecTurnRate
      {
         get { return LocalModelObject.DegPerSecTurnRate; }
         set
         {
            LocalModelObject.DegPerSecTurnRate = value;
            OnPropertyChanged("DegPerSecTurnRate");
         }
      }

      public double NoseAngle
      {
         get { return LocalModelObject.NoseAngle; }
         set
         {
            LocalModelObject.NoseAngle = value;
            OnPropertyChanged("NoseAngle");
         }
      }

      public double Lead
      {
         get { return LocalModelObject.Lead; }
         set
         {
            LocalModelObject.Lead = value;
            OnPropertyChanged("Lead");
         }
      }

      public double BrakeDistance
      {
         get { return LocalModelObject.BrakeDistance; }
         set
         {
            LocalModelObject.BrakeDistance = value;
            OnPropertyChanged("BrakeDistance");
         }
      }

      public double LatchSpeed
      {
         get { return LocalModelObject.LatchSpeed; }
         set
         {
            LocalModelObject.LatchSpeed = value;
            OnPropertyChanged("LatchSpeed");
         }
      }

      public double RamSpeed
      {
         get { return LocalModelObject.RamSpeed; }
         set
         {
            LocalModelObject.RamSpeed = value;
            OnPropertyChanged("RamSpeed");
         }
      }

      public double ShieldDrain
      {
         get { return LocalModelObject.ShieldDrain; }
         set
         {
            LocalModelObject.ShieldDrain = value;
            OnPropertyChanged("ShieldDrain");
         }
      }

      public double LatchDistance
      {
         get { return LocalModelObject.LatchDistance; }
         set
         {
            LocalModelObject.LatchDistance = value;
            OnPropertyChanged("LatchDistance");
         }
      }

      public double HitPoints
      {
         get { return LocalModelObject.HitPoints; }
         set
         {
            LocalModelObject.HitPoints = value;
            OnPropertyChanged("HitPoints");
         }
      }

      public string Steal
      {
         get { return LocalModelObject.Steal; }
         set
         {
            LocalModelObject.Steal = value;
            OnPropertyChanged("Steal");
         }
      }

      public double AmmoAmount
      {
         get { return LocalModelObject.AmmoAmount; }
         set
         {
            LocalModelObject.AmmoAmount = value;
            OnPropertyChanged("AmmoAmount");
         }
      }

      public double ShieldAmount
      {
         get { return LocalModelObject.ShieldAmount; }
         set
         {
            LocalModelObject.ShieldAmount = value;
            OnPropertyChanged("ShieldAmount");
         }
      }

      public double FuelAmount
      {
         get { return LocalModelObject.FuelAmount; }
         set
         {
            LocalModelObject.FuelAmount = value;
            OnPropertyChanged("FuelAmount");
         }
      }

      public double CreditsAmount
      {
         get { return LocalModelObject.CreditsAmount; }
         set
         {
            LocalModelObject.CreditsAmount = value;
            OnPropertyChanged("CreditsAmount");
         }
      }

      public double StealTime
      {
         get { return LocalModelObject.StealTime; }
         set
         {
            LocalModelObject.StealTime = value;
            OnPropertyChanged("StealTime");
         }
      }

      public double GripForce
      {
         get { return LocalModelObject.GripForce; }
         set
         {
            LocalModelObject.GripForce = value;
            OnPropertyChanged("GripForce");
         }
      }

      public double GripTime
      {
         get { return LocalModelObject.GripTime; }
         set
         {
            LocalModelObject.GripTime = value;
            OnPropertyChanged("GripTime");
         }
      }

      public double StunTime
      {
         get { return LocalModelObject.StunTime; }
         set
         {
            LocalModelObject.StunTime = value;
            OnPropertyChanged("StunTime");
         }
      }

      public double LeaveTime
      {
         get { return LocalModelObject.LeaveTime; }
         set
         {
            LocalModelObject.LeaveTime = value;
            OnPropertyChanged("LeaveTime");
         }
      }

      public double FadeInTime
      {
         get { return LocalModelObject.FadeInTime; }
         set
         {
            LocalModelObject.FadeInTime = value;
            OnPropertyChanged("FadeInTime");
         }
      }

      public double LeaveSpeed
      {
         get { return LocalModelObject.LeaveSpeed; }
         set
         {
            LocalModelObject.LeaveSpeed = value;
            OnPropertyChanged("LeaveSpeed");
         }
      }

      public double SparkInterval
      {
         get { return LocalModelObject.SparkInterval; }
         set
         {
            LocalModelObject.SparkInterval = value;
            OnPropertyChanged("SparkInterval");
         }
      }

      public bool AntiGravity
      {
         get { return LocalModelObject.AntiGravity; }
         set
         {
            LocalModelObject.AntiGravity = value;
            OnPropertyChanged("AntiGravity");
         }
      }

      public string LatchedShape
      {
         get { return LocalModelObject.LatchedShape; }
         set
         {
            LocalModelObject.LatchedShape = value;
            OnPropertyChanged("LatchedShape");
         }
      }

      public string FlyingShape
      {
         get { return LocalModelObject.FlyingShape; }
         set
         {
            LocalModelObject.FlyingShape = value;
            OnPropertyChanged("FlyingShape");
         }
      }

      public string DropFile
      {
         get { return LocalModelObject.DropFile; }
         set
         {
            LocalModelObject.DropFile = value;
            OnPropertyChanged("DropFile");
         }
      }
   }
}
