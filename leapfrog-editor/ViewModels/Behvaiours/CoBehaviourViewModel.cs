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
   public class CoBehaviourViewModel : MicroMvvm.ViewModelBase
   {
      #region Declarations

      private CompoundObjectViewModel _parentVm;

      private CoBehaviour _modelObject;
      private SteerableObjectPropertiesViewModel _steerableObjProperties;
      private BreakableObjectPropertiesViewModel _breakableObjProperties;
      private ExplosiveObjectPropertiesViewModel _explosiveObjProperties;
      private MagneticMinePropertiesViewModel _magneticMineProperties;
      private PickupObjectPropertiesViewModel _pickupProperties;
      private StripperPropertiesViewModel _stripperProperties;
      private SwarmPropertiesViewModel _swarmProperties;
      private ScenePropertiesViewModel _sceneProperties;


      #endregion

      #region Constructors

      public CoBehaviourViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         CoBehaviour modelObject) 
      {
         _modelObject = modelObject;

         _parentVm = parentVm;
         _sceneProperties = new ScenePropertiesViewModel(treeParent, parentVm, mainVm, ModelObject.SceneProperties);
         _steerableObjProperties = new SteerableObjectPropertiesViewModel(treeParent, parentVm, mainVm, ModelObject.SteerableObjProps);
         _breakableObjProperties = new BreakableObjectPropertiesViewModel(treeParent, parentVm, mainVm, ModelObject.BreakableObjProps);
         _explosiveObjProperties = new ExplosiveObjectPropertiesViewModel(treeParent, parentVm, mainVm, ModelObject.ExplosiveObjProps);
         _magneticMineProperties = new MagneticMinePropertiesViewModel(treeParent, parentVm, mainVm, ModelObject.MagneticMineProps);
         _pickupProperties = new PickupObjectPropertiesViewModel(treeParent, parentVm, mainVm, ModelObject.PickupObjProps);
         _stripperProperties = new StripperPropertiesViewModel(treeParent, parentVm, mainVm, ModelObject.StripperProps);
         _swarmProperties = new SwarmPropertiesViewModel(treeParent, parentVm, mainVm, ModelObject.SwarmProps);
      }

      #endregion

      #region Properties

      public CoBehaviour ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            OnPropertyChanged("ModelObject");
         }
      }

      public string Type
      {
         get { return ModelObject.Type; }
         set
         {
            ModelObject.Type = value;
            OnPropertyChanged("Type");
            OnPropertyChanged("BehaviourProperties");
            OnPropertyChanged("HasActiveFlag");
            OnPropertyChanged("HasFuelSettings");
            OnPropertyChanged("ShowsFuelRate");
            OnPropertyChanged("ShowsFuelPrice");
            OnPropertyChanged("ShowsFuelSupply");
         }
      }

      // Landing pads: fuel service (see CoBehaviour.Fuel)
      private static readonly List<string> _fuelServices =
         new List<string> { "none", "sell", "free" };

      public List<string> FuelServices
      {
         get { return _fuelServices; }
      }

      public int SelectedFuelIndex
      {
         get
         {
            int i = _fuelServices.IndexOf(ModelObject.Fuel);
            return (i < 0) ? 0 : i;
         }
         set
         {
            ModelObject.Fuel = _fuelServices[((value < 0) || (value >= _fuelServices.Count)) ? 0 : value];
            OnPropertyChanged("SelectedFuelIndex");
            OnPropertyChanged("ShowsFuelRate");
            OnPropertyChanged("ShowsFuelPrice");
            OnPropertyChanged("ShowsFuelSupply");
         }
      }

      public double FuelRate
      {
         get { return ModelObject.FuelRate; }
         set
         {
            ModelObject.FuelRate = value;
            OnPropertyChanged("FuelRate");
         }
      }

      public double FuelPrice
      {
         get { return ModelObject.FuelPrice; }
         set
         {
            ModelObject.FuelPrice = value;
            OnPropertyChanged("FuelPrice");
         }
      }

      public double FuelSupply
      {
         get { return ModelObject.FuelSupply; }
         set
         {
            ModelObject.FuelSupply = value;
            OnPropertyChanged("FuelSupply");
         }
      }

      public bool HasFuelSettings
      {
         get { return Type == "landingPad"; }
      }

      public bool ShowsFuelRate
      {
         get { return HasFuelSettings && (ModelObject.Fuel != "none"); }
      }

      public bool ShowsFuelPrice
      {
         get { return HasFuelSettings && (ModelObject.Fuel == "sell"); }
      }

      public bool ShowsFuelSupply
      {
         get { return HasFuelSettings && (ModelObject.Fuel == "free"); }
      }

      // Landing pads and launch sites have an Active flag (see CoBehaviour)
      public bool Active
      {
         get { return ModelObject.Active; }
         set
         {
            ModelObject.Active = value;
            OnPropertyChanged("Active");
         }
      }

      public bool HasActiveFlag
      {
         get { return CoBehaviour.HasActiveFlag(Type); }
      }

      // To be general, all behaviour need to provide the DisplayedStateIndex
      // property even though it is only scenes that actually has some
      // meaningfull content. For all other behaviour types, index 0 is returned
      // and the property can not be set.


      public BehaviourViewModelBase BehaviourProperties
      {
         get
         {
            if (Type == "scene")
            {
               return _sceneProperties;
            }
            else if (Type == "steerableObject")
            {
               return _steerableObjProperties;
            }
            else if (Type == "breakableObject")
            {
               return _breakableObjProperties;
            }
            else if (Type == "explosiveObject")
            {
               return _explosiveObjProperties;
            }
            else if (Type == "magneticMine")
            {
               return _magneticMineProperties;
            }
            else if (Type == "pickupObject")
            {
               return _pickupProperties;
            }
            else if (Type == "stripper")
            {
               return _stripperProperties;
            }
            else if (Type == "swarm")
            {
               return _swarmProperties;
            }

            return null;
         }
      }

      #endregion

      #region private Methods

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      //public StateViewModel FindStateVM(string stateName)
      //{
      //   if (Type != "scene")
      //   {
      //      return _defaultState;
      //   }

      //   foreach (StateViewModel svm in States)
      //   {
      //      if (svm.StateName == stateName)
      //      {
      //         return svm;
      //      }
      //   }

      //   return null;

      //}

      #endregion
   }
}
