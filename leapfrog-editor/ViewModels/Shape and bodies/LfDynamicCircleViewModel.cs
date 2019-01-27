using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace LeapfrogEditor
{
   class LfDynamicCircleViewModel : LfStaticCircleViewModel
   {
      #region Declarations

      #endregion

      #region Constructors

      public LfDynamicCircleViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         LfDynamicCircle modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, modelObject, enabled)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public new LfDynamicCircle LocalModelObject
      {
         get { return (LfDynamicCircle)ModelObject; }
      }


      public double Density
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.Density;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.Density = value;
            OnPropertyChanged("Density");
         }
      }

      #endregion

   }
}
