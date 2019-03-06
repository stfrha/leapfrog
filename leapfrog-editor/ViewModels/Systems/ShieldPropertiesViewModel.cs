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
   class ShieldPropertiesViewModel : BodyOriginSystemViewModel
   {
      #region Declarations

      #endregion

      #region Constructors

      public ShieldPropertiesViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         ShieldProperties modelObject,
         CoSystemViewModel systemViewModel) :
         base(treeParent, parentVm, mainVm, modelObject, systemViewModel)
      {
      }

      #endregion

      #region Properties

      public ShieldProperties LocalModelObject
      {
         get { return (ShieldProperties)ModelObject; }
      }

      public double Radius
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.Radius;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.Radius = value;
            OnPropertyChanged("Radius");
         }
      }

      public int ZLevel
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.ZLevel;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.ZLevel = value;
            OnPropertyChanged("ZLevel");
         }
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
