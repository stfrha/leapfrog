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
   public class ScenePropertiesViewModel : BehaviourViewModelBase
   {
      #region Declarations

      private SceneProperties _modelObject;

      // The following fields are item 0 and item 1 in the PropertyCollection collection.
      // They exists to get easy access
      private ScenePropsStateCollectionViewModel _stateCollection;

      private ObservableCollection<ScenePropertyCollectionBaseViewModel> _propertyCollection = new ObservableCollection<ScenePropertyCollectionBaseViewModel>();

      private int _displayedStateIndex = 0;


      #endregion

      #region Constructors

      public ScenePropertiesViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         SceneProperties modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;

         // Automatically create the properties view models
         // To get two different tree-sub branches, we divid the states
         // and ParallaxBackgrounds into two different classes with
         // a common base class (ScenePropertyCollectionBaseViewModel)
         // One instance of each class is added to the PropertyCollection
         // property of this class.

         // Create the ScenePropsStateCollectionViewModel
         _stateCollection = new ScenePropsStateCollectionViewModel(this, parentVm, mainVm, enabled);

         // Now populate the _stateCollection with StateViewModels from 
         // the source states
         foreach (string s in ModelObject.States)
         {
            StateViewModel svm = new StateViewModel(this, parentVm, mainVm, this, s);
            _stateCollection.States.Add(svm);
         }

         // Add the state collection to the PropertyCollection
         _propertyCollection.Add(_stateCollection);
      }

      #endregion

      #region Properties

      public SceneProperties ModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            OnPropertyChanged("ModelObject");
         }
      }

      public string SceneType
      {
         get { return ModelObject.SceneType; }
         set
         {
            ModelObject.SceneType = value;
            OnPropertyChanged("SceneType");
         }
      }

      public double Width
      {
         get { return ModelObject.Width; }
         set
         {
            ModelObject.Width = value;
            OnPropertyChanged("Width");
         }
      }

      public double Height
      {
         get { return ModelObject.Height; }
         set
         {
            ModelObject.Height = value;
            OnPropertyChanged("Height");
         }
      }

      public bool HasUpperBoundary
      {
         get { return ModelObject.HasUpperBoundary; }
         set
         {
            ModelObject.HasUpperBoundary = value;
            OnPropertyChanged("HasUpperBoundary");
         }
      }

      public bool HasLowerBoundary
      {
         get { return ModelObject.HasLowerBoundary; }
         set
         {
            ModelObject.HasLowerBoundary = value;
            OnPropertyChanged("HasLowerBoundary");
         }
      }

      public bool HasLeftBoundary
      {
         get { return ModelObject.HasLeftBoundary; }
         set
         {
            ModelObject.HasLeftBoundary = value;
            OnPropertyChanged("HasLeftBoundary");
         }
      }

      public bool HasRightBoundary
      {
         get { return ModelObject.HasRightBoundary; }
         set
         {
            ModelObject.HasRightBoundary = value;
            OnPropertyChanged("HasRightBoundary");
         }
      }

      public ObservableCollection<ScenePropertyCollectionBaseViewModel> PropertyCollection
      {
         get { return _propertyCollection; }
         set { _propertyCollection = value; }
      }

      public ScenePropsStateCollectionViewModel StateCollection
      {
         get { return _stateCollection; }
      }

      public ObservableCollection<StateViewModel> States
      {
         get { return _stateCollection.States; }
         set { _stateCollection.States = value; }
      }


      public int DisplayedStateIndex
      {
         get
         {
            return _displayedStateIndex;
         }
         set
         {
            if (value == -1)
            {
               _displayedStateIndex = 0;
            }
            else
            {
               _displayedStateIndex = value;
            }

            OnPropertyChanged("DisplayedStateIndex");

            foreach (StateViewModel svm in States)
            {
               svm.OnPropertyChanged("StateIndicator");
            }

            ParentVm.InvalidateChildObjects();
            DeselectAllChildren();

            CompoundObjectViewModel p = ParentVm;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.ParentVm;
            }
         }
      }

      #endregion

      #region private Methods

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      public bool IsOnDisplay(StateViewModel svm)
      {
         return (States.IndexOf(svm) == DisplayedStateIndex);
      }

      public void SetStateOnDisplay(StateViewModel svm)
      {
         int i = States.IndexOf(svm);

         if (i >= 0)
         {
            DisplayedStateIndex = i;
         }
         else
         {
            DisplayedStateIndex = 0;
         }
      }

      #endregion
   }
}
