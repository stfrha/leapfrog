using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

// This class only reason for existing is to be a specific class 
// that becomes a headline in the tree view with a collection
// under it. 

// TODO: This class has nothing to do with states anymore and should be 
// renamed accordingly


namespace LeapfrogEditor
{
   public class StateShapeCollectionViewModel : StateCollectionViewModelBase
   {
      #region Declarations

      private CompositeCollection _shapes;

      #endregion

      #region Constructors

      public StateShapeCollectionViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         _shapes = new CompositeCollection()
         {
            new CollectionContainer { Collection = new ObservableCollection<LfSpriteBoxViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfSpritePolygonViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfStaticBoxViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfStaticCircleViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfStaticPolygonViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfStaticBoxedSpritePolygonViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfDynamicBoxViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfDynamicCircleViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfDynamicPolygonViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<LfDynamicBoxedSpritePolygonViewModel>() },
         };

      }

      #endregion

      #region Properties

      public CompositeCollection Shapes
      {
         get { return _shapes; }
         set { _shapes = value; }
      }

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      #endregion
   }
}
