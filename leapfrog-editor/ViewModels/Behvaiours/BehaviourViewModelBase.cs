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
   // This class is a common base for all Behvaiour properties
   // view models so that we can return something common when 
   // a behaviour properties view model is requested.
   public class BehaviourViewModelBase : StateBehaviourCollectionViewModel
   {
      #region Declarations

      #endregion

      #region Constructors

      public BehaviourViewModelBase(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
      }

      #endregion

      #region Properties


      #endregion

      #region private Methods

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      #endregion
   }
}
