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
   public class LfStaticPolygonViewModel : LfScalableTexturePolygonViewModel
   {
      #region Declarations


      #endregion

      #region Constructors

      public LfStaticPolygonViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         LfStaticPolygon modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;

         CreateBorderVms();
      }

      #endregion

      #region Properties

      public new LfStaticPolygon LocalModelObject
      {
         get { return (LfStaticPolygon)ModelObject; }
      }

      #endregion

      #region public methods

      #endregion
   }
}
