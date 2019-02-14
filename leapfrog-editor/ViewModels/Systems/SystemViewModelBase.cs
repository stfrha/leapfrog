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
   // This class is a common base for all system properties
   // view models so that we can return something common when 
   // a system properties view model is requested.
   public class SystemViewModelBase : TreeViewViewModel
   {
      #region Declarations

      private CoSystemViewModel _systemViewModel;

      #endregion

      #region Constructors

      public SystemViewModelBase(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         CoSystemViewModel systemViewModel,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         _systemViewModel = systemViewModel;
      }

      #endregion

      #region Properties

      public string ObjectName
      {
         get { return _systemViewModel.Name; }
         set
         {
            _systemViewModel.Name = value;
            OnPropertyChanged("ObjectName");
         }
      }

      public string Type
      {
         get { return _systemViewModel.Type; }
      }

      #endregion

      #region private Methods

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      #endregion
   }
}
