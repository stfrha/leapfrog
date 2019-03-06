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
   class ReentryFlameEmitterPropertiesViewModel : SystemViewModelBase
   {
      #region Declarations

      private ReentryFlameEmitterProperties _modelObject;
      private LfShapeViewModel _bodyVm;

      #endregion

      #region Constructors

      public ReentryFlameEmitterPropertiesViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         ReentryFlameEmitterProperties modelObject,
         CoSystemViewModel systemViewModel) :
         base(treeParent, parentVm, mainVm, systemViewModel)
      {
         _modelObject = modelObject;
      }

      #endregion

      #region Properties

      public ReentryFlameEmitterProperties LocalModelObject
      {
         get { return _modelObject; }
      }

      public string BodyName
      {
         get
         {
            if (_modelObject == null) return "notDefined";

            return _modelObject.BodyName;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.BodyName = value;
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


      public double EmitterLineStartX
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.EmitterLineStartX;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.EmitterLineStartX = value;
            OnPropertyChanged("EmitterLineStartX");
         }
      }

      public double EmitterLineStartY
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.EmitterLineStartY;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.EmitterLineStartY = value;
            OnPropertyChanged("EmitterLineStartY");
         }
      }

      public double EmitterLineEndX
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.EmitterLineEndX;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.EmitterLineEndX = value;
            OnPropertyChanged("EmitterLineEndX");
         }
      }

      public double EmitterLineEndY
      {
         get
         {
            if (_modelObject == null) return 0;

            return _modelObject.EmitterLineEndY;
         }
         set
         {
            if (_modelObject == null) return;

            _modelObject.EmitterLineEndY = value;
            OnPropertyChanged("EmitterLineEndY");
         }
      }

      public double Angle
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.Angle;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.Angle = value;
            OnPropertyChanged("Angle");
         }
      }

      public double Intensity
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.Intensity;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.Intensity = value;
            OnPropertyChanged("Intensity");
         }
      }

      public int LifeTime
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.LifeTime;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.LifeTime = value;
            OnPropertyChanged("LifeTime");
         }
      }

      public double MaxWidth
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.MaxWidth;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.MaxWidth = value;
            OnPropertyChanged("MaxWidth");
         }
      }

      public double MaxHeight
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.MaxHeight;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.MaxHeight = value;
            OnPropertyChanged("MaxHeight");
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
         _bodyVm = ParentVm.FindShape(LocalModelObject.BodyName, shapes);
         if (_bodyVm == null)
         {
            MessageBox.Show("The shape pointed to by " + LocalModelObject.BodyName + " does not exists in CO " + ParentVm.Name, "Error parsing file", MessageBoxButton.OK, MessageBoxImage.Error);
         }
      }


      #endregion
   }
}
