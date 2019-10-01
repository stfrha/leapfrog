﻿using System;
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
      private ScenePropsPBCollectionViewModel _pbCollection;

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

         // Create the ScenePropsPBCollectionViewModel
         _pbCollection = new ScenePropsPBCollectionViewModel(this, parentVm, mainVm, enabled);

         // Now populate the _pbCollection with ParallaxBackgroundViewModels from 
         // the source states
         foreach (ParallaxBackground pb in ModelObject.Backgrounds)
         {
            ParallaxBackgroundViewModel pbvm = new ParallaxBackgroundViewModel(this, parentVm, mainVm, pb, enabled);
            _pbCollection.Backgrounds.Add(pbvm);
         }

         // Add the background collection to the PropertyCollection
         _propertyCollection.Add(_pbCollection);


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

      public ObservableCollection<ScenePropertyCollectionBaseViewModel> PropertyCollection
      {
         get { return _propertyCollection; }
         set { _propertyCollection = value; }
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
