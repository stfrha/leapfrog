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
   public class ScenePropsPBCollectionViewModel : ScenePropertyCollectionBaseViewModel
   {
      #region Declarations

      private ObservableCollection<ParallaxBackgroundViewModel> _backgrounds = new ObservableCollection<ParallaxBackgroundViewModel>();

      #endregion

      #region Constructors

      public ScenePropsPBCollectionViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
      }

      #endregion

      #region Properties

      public ObservableCollection<ParallaxBackgroundViewModel> Backgrounds
      {
         get { return _backgrounds; }
         set { _backgrounds = value; }
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
