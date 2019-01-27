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
   class LfDynamicBoxViewModel : LfStaticBoxViewModel
   {
      #region Declarations

      #endregion

      #region Constructors

      public LfDynamicBoxViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         LfDynamicBox modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, modelObject, enabled)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public new LfDynamicBox LocalModelObject
      {
         get { return (LfDynamicBox)ModelObject; }
      }


      public double Density
      {
         get
         {
            if (LocalModelObject == null) return 1;

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
