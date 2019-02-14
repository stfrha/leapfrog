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
   class FlameEmitterPropertiesViewModel : SystemViewModelBase
   {
      #region Declarations

      private  FlameEmitterProperties ModelObject;

      #endregion

      #region Constructors

      public FlameEmitterPropertiesViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         FlameEmitterProperties modelObject,
         CoSystemViewModel systemViewModel) :
         base(treeParent, parentVm, mainVm, systemViewModel)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public FlameEmitterProperties LocalModelObject
      {
         get { return ModelObject; }
      }

      public string BodyName
      {
         get
         {
            if (LocalModelObject == null) return "notDefined";

            return LocalModelObject.BodyName;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.BodyName = value;
            OnPropertyChanged("BodyName");
         }
      }

      public double EmitterOriginX
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.EmitterOriginX;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.EmitterOriginX = value;
            OnPropertyChanged("EmitterOriginX");
         }
      }

      public double EmitterOriginY
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.EmitterOriginY;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.EmitterOriginY = value;
            OnPropertyChanged("EmitterOriginY");
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

      public double EmitterWidth
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.EmitterWidth;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.EmitterWidth = value;
            OnPropertyChanged("EmitterWidth");
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

      public double ImpulseMagnitude
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.ImpulseMagnitude;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.ImpulseMagnitude = value;
            OnPropertyChanged("ImpulseMagnitude");
         }
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

      #endregion

      #region private Methods

      #endregion

      #region protected Methods

      #endregion

      #region public Methods

      #endregion
   }
}
