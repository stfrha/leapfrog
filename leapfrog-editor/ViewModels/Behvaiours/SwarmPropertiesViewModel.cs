using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
   class SwarmPropertiesViewModel : BehaviourViewModelBase
   {
      private SwarmProperties ModelObject;

      public SwarmPropertiesViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         SwarmProperties modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;
      }

      public SwarmProperties LocalModelObject
      {
         get { return ModelObject; }
      }

      public string Spawn
      {
         get { return LocalModelObject.Spawn; }
         set
         {
            LocalModelObject.Spawn = value;
            OnPropertyChanged("Spawn");
         }
      }

      public bool Active
      {
         get { return LocalModelObject.Active; }
         set
         {
            LocalModelObject.Active = value;
            OnPropertyChanged("Active");
         }
      }

      public double StartRate
      {
         get { return LocalModelObject.StartRate; }
         set
         {
            LocalModelObject.StartRate = value;
            OnPropertyChanged("StartRate");
         }
      }

      public double Rate
      {
         get { return LocalModelObject.Rate; }
         set
         {
            LocalModelObject.Rate = value;
            OnPropertyChanged("Rate");
         }
      }

      public double StartMaxAlive
      {
         get { return LocalModelObject.StartMaxAlive; }
         set
         {
            LocalModelObject.StartMaxAlive = value;
            OnPropertyChanged("StartMaxAlive");
         }
      }

      public double MaxAlive
      {
         get { return LocalModelObject.MaxAlive; }
         set
         {
            LocalModelObject.MaxAlive = value;
            OnPropertyChanged("MaxAlive");
         }
      }

      public double RampTime
      {
         get { return LocalModelObject.RampTime; }
         set
         {
            LocalModelObject.RampTime = value;
            OnPropertyChanged("RampTime");
         }
      }

      public int InitialSpawn
      {
         get { return LocalModelObject.InitialSpawn; }
         set
         {
            LocalModelObject.InitialSpawn = value;
            OnPropertyChanged("InitialSpawn");
         }
      }

      public string Sides
      {
         get { return LocalModelObject.Sides; }
         set
         {
            LocalModelObject.Sides = value;
            OnPropertyChanged("Sides");
         }
      }

      public double SpawnInset
      {
         get { return LocalModelObject.SpawnInset; }
         set
         {
            LocalModelObject.SpawnInset = value;
            OnPropertyChanged("SpawnInset");
         }
      }

      public double MinDistance
      {
         get { return LocalModelObject.MinDistance; }
         set
         {
            LocalModelObject.MinDistance = value;
            OnPropertyChanged("MinDistance");
         }
      }
   }
}
