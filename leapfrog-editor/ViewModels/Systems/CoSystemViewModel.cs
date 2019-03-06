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
   public class CoSystemViewModel : TreeViewViewModel
   {
      #region Declarations

      private CoSystem _modelObject;

      private int _selectedSystemIndex;


      // Only one object is in use per instance of this class. The object
      // in use is defined by the type string of the system.
      private ObjectFactoryPropertiesViewModel _objFactStatesProps = null;
      private FlameEmitterPropertiesViewModel _flameEmitterStatesProps = null;
      private ReentryFlameEmitterPropertiesViewModel _reentryFlameEmitterStatesProps = null;
      private GunPropertiesViewModel _gunStatesProps = null;
      private ShieldPropertiesViewModel _shieldStatesProps = null;

      #endregion

      #region Constructors

      public CoSystemViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm, 
         CoSystem modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         LocalModelObject = modelObject;

         SelectedSystemIndex = Systems.IndexOf(LocalModelObject.Type);

         if (LocalModelObject.Type == "objectFactory")
         {
            _objFactStatesProps = new ObjectFactoryPropertiesViewModel(treeParent, parentVm, mainVm, LocalModelObject.ObjFactStateProperties, this);
         }
         else if (LocalModelObject.Type == "flameEmitter")
         {
            _flameEmitterStatesProps = new FlameEmitterPropertiesViewModel(treeParent, parentVm, mainVm, LocalModelObject.FlameEmitterStateProperties, this);
         }
         else if (LocalModelObject.Type == "reentryFlameEmitter")
         {
            _reentryFlameEmitterStatesProps = new ReentryFlameEmitterPropertiesViewModel(treeParent, parentVm, mainVm, LocalModelObject.ReentryFlameEmitterStateProperties, this);
         }
         else if (LocalModelObject.Type == "gun")
         {
            _gunStatesProps = new GunPropertiesViewModel(treeParent, parentVm, mainVm, LocalModelObject.GunStateProperties, this);
         }
         else if (LocalModelObject.Type == "shield")
         {
            _shieldStatesProps = new ShieldPropertiesViewModel(treeParent, parentVm, mainVm, LocalModelObject.ShieldStateProperties, this);
         }

      }

      #endregion

      #region Properties

      public CoSystem LocalModelObject
      {
         get { return _modelObject; }
         set
         {
            _modelObject = value;
            OnPropertyChanged("LocalModelObject");
         }
      }

      public string Name
      {
         get { return LocalModelObject.Name; }
         set
         {
            LocalModelObject.Name = value;
            OnPropertyChanged("Name");
            Properties.OnPropertyChanged("ObjectName");
         }
      }

      public string Type
      {
         get { return LocalModelObject.Type; }
         set
         {
            LocalModelObject.Type = value;
            OnPropertyChanged("Type");
            OnPropertyChanged("Properties");
         }
      }

      public SystemViewModelBase Properties
      {
         get
         {
            if (Type == "objectFactory")
            {
               return _objFactStatesProps;
            }
            else if (Type == "flameEmitter")
            {
               return _flameEmitterStatesProps;
            }
            else if (Type == "reentryFlameEmitter")
            {
               return _reentryFlameEmitterStatesProps;
            }
            else if (Type == "gun")
            {
               return _gunStatesProps;
            }
            else if (Type == "shield")
            {
               return _shieldStatesProps;
            }

            return null;
         }
         set
         {
            if (Type == "objectFactory")
            {
               _objFactStatesProps = (ObjectFactoryPropertiesViewModel)value;
            }
            else if (Type == "flameEmitter")
            {
               _flameEmitterStatesProps = (FlameEmitterPropertiesViewModel)value;
            }
            else if (Type == "gun")
            {
               _gunStatesProps = (GunPropertiesViewModel)value;
            }
            else if (Type == "shield")
            {
               _shieldStatesProps = (ShieldPropertiesViewModel)value;
            }
         }
      }

      public List<string> Systems
      {
         get
         {
            return new List<string>()
            {
               "notApplicable",
               "objectFactory",
               "gun",
               "flameEmitter",
               "reentryFlameEmitter",
               "shield"
            };
         }
      }

      public int SelectedSystemIndex
      {
         get
         {
            return _selectedSystemIndex;
         }
         set
         {
            if (value == -1)
            {
               _selectedSystemIndex = 0;
            }
            else
            {
               _selectedSystemIndex = value;
            }
            OnPropertyChanged("SelectedSystemIndex");

            Type = Systems[_selectedSystemIndex];
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
