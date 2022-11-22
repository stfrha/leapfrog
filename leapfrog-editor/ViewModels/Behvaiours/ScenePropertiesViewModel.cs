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
      private int _selectedSceneTypeIndex;


      // The following fields are item 0 and item 1 in the PropertyCollection collection.
      // They exists to get easy access
      //private ScenePropsStateCollectionViewModel _stateCollection;

      private ObservableCollection<ScenePropertyCollectionBaseViewModel> _propertyCollection = new ObservableCollection<ScenePropertyCollectionBaseViewModel>();

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

         SelectedSceneTypeIndex = SceneTypes.IndexOf(ModelObject.SceneType);
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

      public List<string> SceneTypes
      {
         get
         {
            return new List<string>()
            {
               "notDef",
               "ground",
               "space",
               "orbitSpaceScene",
               "hyperspace"
            };
         }
      }

      public int SelectedSceneTypeIndex
      {
         get
         {
            return _selectedSceneTypeIndex;
         }
         set
         {
            if (value == -1)
            {
               return;
               // _selectedSceneTypeIndex = 0;
            }
            else
            {
               _selectedSceneTypeIndex = value;
            }
            OnPropertyChanged("SelectedSceneTypeIndex");

            SceneType = SceneTypes[_selectedSceneTypeIndex];
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
