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
   public class StateViewModel : TreeViewViewModel
   {
      #region Declarations

      private string _stateName;
      private ScenePropertiesViewModel _sceneVm;

      #endregion

      #region Constructors

      public StateViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         ScenePropertiesViewModel sceneVm,
         string stateName,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         _sceneVm = sceneVm;
         StateName = stateName;
      }

      #endregion

      #region Properties

      public string StateIndicator
      {
         get
         {
            if ((_sceneVm != null) && (_sceneVm.IsOnDisplay(this)))
            {
               return _stateName + " <--";
            }

            return _stateName;
         }
         set
         {
            _stateName = value;
            OnPropertyChanged("StateIndicator");
         }
      }

      public string StateName
      {
         get
         {
            return _stateName;
         }
         set
         {
            _stateName = value;
            OnPropertyChanged("StateName");
            OnPropertyChanged("StateIndicator");
         }
      }

      public bool IsOnDisplay
      {
         get { return _sceneVm.IsOnDisplay(this); }
      }

      #endregion

      #region private Methods

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      public void SetOnDisplay()
      {
         _sceneVm.SetStateOnDisplay(this);
         OnPropertyChanged("StateIndicator");
      }

      #endregion
   }
}
