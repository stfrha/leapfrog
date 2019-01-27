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
   class LfDynamicBoxedSpritePolygonViewModel : LfStaticBoxedSpritePolygonViewModel
   {
      #region Declarations

      #endregion

      #region Constructors

      public LfDynamicBoxedSpritePolygonViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         LfDynamicBoxedSpritePolygon modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, modelObject, enabled)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public new LfDynamicBoxedSpritePolygon LocalModelObject
      {
         get { return (LfDynamicBoxedSpritePolygon)ModelObject; }
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
