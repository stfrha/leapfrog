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
   public class SystemCollectionViewModel : CollectionViewModelBase
   {
      #region Declarations

      private ObservableCollection<CoSystemViewModel> _systems = new ObservableCollection<CoSystemViewModel>();

      #endregion

      #region Constructors

      public SystemCollectionViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
      }

      #endregion

      #region Properties

      public ObservableCollection<CoSystemViewModel> Systems
      {
         get { return _systems; }
         set { _systems = value; }
      }

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      #endregion
   }
}
