using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

// This class is used to provide a tree view hierarcical
// level under the CompoundObject that holds headlines for
// Behaviour, Shapes, Joints, Systems and child objects

// TODO: This class has nothing to do with states anymore and should be 
// renamed accordingly

namespace LeapfrogEditor
{
   public class CollectionViewModelBase : TreeViewViewModel
   {
      #region Declarations

      #endregion

      #region Constructors

      public CollectionViewModelBase(
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

      #region protected Methods

      protected virtual void DeselectAllChildren()
      {
         // Only polygon shapes has children (so far), se we do nothing here
      }

      #endregion

      #region public Methods

      //public virtual void InvalidateAll()
      //{
      //   OnPropertyChanged("");
      //}

      #endregion
   }
}
