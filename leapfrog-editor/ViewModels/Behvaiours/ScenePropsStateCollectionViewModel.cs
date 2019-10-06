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
   public class ScenePropsStateCollectionViewModel : ScenePropertyCollectionBaseViewModel
   {
      #region Declarations

      private ObservableCollection<StateViewModel> _states = new ObservableCollection<StateViewModel>();

      #endregion

      #region Constructors

      public ScenePropsStateCollectionViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
      }

      #endregion

      #region Properties

      public ObservableCollection<StateViewModel> States
      {
         get { return _states; }
         set { _states = value; }
      }

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      //public virtual void InvalidateAll()
      //{
      //   OnPropertyChanged("");
      //}

      #endregion
   }
}
