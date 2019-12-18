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
   public class ParallaxBackgroundPropertiesViewModel : BehaviourViewModelBase
   {
      #region Declarations

      private ParallaxBackgroundProperties _modelObject;

      #endregion

      #region Constructors

      public ParallaxBackgroundPropertiesViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         ParallaxBackgroundProperties modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public ParallaxBackgroundProperties ModelObject
      {
         get
         {
            if (_modelObject == null) return null;
            return _modelObject;
         }
         set
         {
            _modelObject = value;
            OnPropertyChanged("");
         }
      }

      public double ParallaxAmount
      {
         get
         {
            if (_modelObject == null) return 0;
            return _modelObject.ParallaxAmount;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.ParallaxAmount = value;
            OnPropertyChanged("");
         }
      }

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      #endregion
   }
}
