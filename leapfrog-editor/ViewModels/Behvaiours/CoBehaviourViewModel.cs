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
      private ScenePropertiesViewModel _sceneProperties;

      private ObservableCollection<StateViewModel> _defaultStates = new ObservableCollection<StateViewModel>();
      private StateViewModel _defaultState = new StateViewModel(null, null, null, null, "default");


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

         _defaultStates.Add(_defaultState);
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
         }
      }

      // To be general, all behaviour need to provide the DisplayedStateIndex
      // property even though it is only scenes that actually has some
      // meaningfull content. For all other behaviour types, index 0 is returned
      // and the property can not be set.

      public int DisplayedStateIndex
      {
         get
         {
            if (Type == "scene")
            {
               return _sceneProperties.DisplayedStateIndex;
            }

            return 0;
         }
      }

      public ObservableCollection<StateViewModel> States
      {
         get
         {
            if (Type == "scene")
            {
               return _sceneProperties.States;
            }

            ObservableCollection<StateViewModel> svms = new ObservableCollection<StateViewModel>();

            StateViewModel svm = new StateViewModel(null, null, null, null, "default");
            svms.Add(svm);

            return svms;
         }
      }

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

            return null;
         }
      }

      #endregion

      #region private Methods

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      public StateViewModel FindStateVM(string stateName)
      {
         if (Type != "scene")
         {
            return _defaultState;
         }

         foreach (StateViewModel svm in States)
         {
            if (svm.StateName == stateName)
            {
               return svm;
            }
         }

         return null;

      }

      #endregion
   }
}
