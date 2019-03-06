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
   public class BodyOriginSystemViewModel : SystemViewModelBase
   {
      #region Declarations

      private BodyOriginSystem _modelObject;
      private LfShapeViewModel _bodyVm;

      #endregion

      #region Constructors

      public BodyOriginSystemViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         BodyOriginSystem modelObject,
         CoSystemViewModel systemViewModel) :
         base(treeParent, parentVm, mainVm, systemViewModel)
      {
         _modelObject = modelObject;
      }

      #endregion

      #region Properties

      public BodyOriginSystem ModelObject
      {
         get { return _modelObject; }
      }

      public string BodyName
      {
         get
         {
            if (ModelObject == null) return "notDefined";

            return ModelObject.BodyName;
         }
         set
         {
            if (ModelObject == null) return;

            ModelObject.BodyName = value;
            OnPropertyChanged("BodyName");
            ConnectToShapes(ParentVm.StateShapes);
            OnPropertyChanged("BodyObject");
         }
      }

      public LfShapeViewModel BodyObject
      {
         get
         {
            return _bodyVm;
         }
      }


      public double SystemOriginX
      {
         get
         {
            if (ModelObject == null) return 0;

            return ModelObject.SystemOriginX;
         }
         set
         {
            if (ModelObject == null) return;

            ModelObject.SystemOriginX = value;
            OnPropertyChanged("SystemOriginX");
         }
      }

      public double SystemOriginY
      {
         get
         {
            if (ModelObject == null) return 0;

            return ModelObject.SystemOriginY;
         }
         set
         {
            if (ModelObject == null) return;

            ModelObject.SystemOriginY = value;
            OnPropertyChanged("SystemOriginY");
         }
      }

      #endregion

      #region private Methods

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      public void ConnectToShapes(StateShapeCollectionViewModel shapes)
      {
         _bodyVm = ParentVm.FindShape(ModelObject.BodyName, shapes);
         if (_bodyVm == null)
         {
            MessageBox.Show("The shape pointed to by " + ModelObject.BodyName + " does not exists in CO " + ParentVm.Name, "Error parsing file", MessageBoxButton.OK, MessageBoxImage.Error);
         }
      }

      #endregion
   }
}
