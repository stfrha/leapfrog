using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace LeapfrogEditor
{
   public class ImportedObjectCollectionViewModel : CollectionViewModelBase
   {
      #region Declarations

      private ObservableCollection<CompoundObjectViewModel> _children = new ObservableCollection<CompoundObjectViewModel>();

      #endregion

      #region Constructors

      public ImportedObjectCollectionViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
      }

      #endregion

      #region Properties

      public ObservableCollection<CompoundObjectViewModel> Children
      {
         get { return _children; }
         set { _children = value; }
      }

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      #endregion
   }
}
