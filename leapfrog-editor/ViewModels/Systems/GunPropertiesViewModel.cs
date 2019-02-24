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
   class GunPropertiesViewModel : SystemViewModelBase
   {
      #region Declarations

      private GunProperties ModelObject;
      private LfShapeViewModel _bodyVm;


      #endregion

      #region Constructors

      public GunPropertiesViewModel(
         TreeViewViewModel treeParent,
         CompoundObjectViewModel parentVm,
         MainViewModel mainVm,
         GunProperties modelObject,
         CoSystemViewModel systemViewModel) :
         base(treeParent, parentVm, mainVm, systemViewModel)
      {
         ModelObject = modelObject;
      }

      #endregion

      #region Properties

      public GunProperties LocalModelObject
      {
         get { return (GunProperties)ModelObject; }
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

      public double FireRate
      {
         get
         {
            if (LocalModelObject == null) return 0;

            return LocalModelObject.FireRate;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.FireRate = value;
            OnPropertyChanged("FireRate");
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

      public bool Bouncy
      {
         get
         {
            if (LocalModelObject == null) return false;

            return LocalModelObject.Bouncy;
         }
         set
         {
            if (LocalModelObject == null) return;

            LocalModelObject.Bouncy = value;
            OnPropertyChanged("Bouncy");
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
