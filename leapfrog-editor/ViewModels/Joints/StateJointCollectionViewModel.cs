using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;


// TODO: This class has nothing to do with states anymore and should be 
// renamed accordingly

namespace LeapfrogEditor
{
   public class StateJointCollectionViewModel : StateCollectionViewModelBase
   {
      #region Declarations

      private CompositeCollection _joints;

      #endregion

      #region Constructors

      public StateJointCollectionViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         _joints = new CompositeCollection()
         {
            new CollectionContainer { Collection = new ObservableCollection<WeldJointViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<RevoluteJointViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<PrismaticJointViewModel>() },
            new CollectionContainer { Collection = new ObservableCollection<RopeViewModel>() },
         };
      }

      #endregion

      #region Properties

      public CompositeCollection Joints
      {
         get { return _joints; }
         set { _joints = value; }
      }


      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      #endregion
   }
}
