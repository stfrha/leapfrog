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
   class LfStaticCircleViewModel : LfShapeViewModel, ICircleInterface
   {
      #region Declarations

      #endregion

      #region Constructors

      public LfStaticCircleViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         LfStaticCircle modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public LfStaticCircle LocalModelObject
      {
         get { return (LfStaticCircle)ModelObject; }
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
            OnPropertyChanged("BoundingBox");

            CompoundObjectViewModel p = ParentVm;

            while (p != null)
            {
               p.OnPropertyChanged("BoundingBox");
               p = p.ParentVm;
            }
         }
      }

      #endregion

      #region protected Methods

      protected override Rect GetBoundingBox()
      {
         Rect r = new Rect(-Radius, -Radius, Radius * 2, Radius * 2);
         //r.Offset(new Vector(-Radius, -Radius));
         return r;
      }

      #endregion


   }
}
