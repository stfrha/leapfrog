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
   class PickupObjectPropertiesViewModel : BehaviourViewModelBase
   {
      #region Declarations

      private PickupObjectProperties ModelObject;
      private int _selectedPickupTypeIndex;
      private int _selectedAmountTypeIndex;

      #endregion

      #region Constructors

      public PickupObjectPropertiesViewModel(
         TreeViewViewModel treeParent, 
         CompoundObjectViewModel parentVm, 
         MainViewModel mainVm,
         PickupObjectProperties modelObject,
         bool enabled = true) :
         base(treeParent, parentVm, mainVm, enabled)
      {
         ModelObject = modelObject;

         SelectedPickupTypeIndex = PickupTypes.IndexOf(LocalModelObject.PickupType);
         SelectedAmountTypeIndex = AmountTypes.IndexOf(LocalModelObject.PickupAmountType);
      }

      #endregion

      #region Properties

      public PickupObjectProperties LocalModelObject
      {
         get { return ModelObject; }
      }

      public string PickupType
      {
         get { return LocalModelObject.PickupType; }
         set
         {
            LocalModelObject.PickupType = value;
            OnPropertyChanged("PickupType");
         }
      }

      public string PickupAmountType
      {
         get { return LocalModelObject.PickupAmountType; }
         set
         {
            LocalModelObject.PickupAmountType = value;
            OnPropertyChanged("PickupAmountType");
         }
      }

      public double Amount
      {
         get { return LocalModelObject.Amount; }
         set
         {
            LocalModelObject.Amount = value;
            OnPropertyChanged("Amount");
         }
      }

      public double AmountMax
      {
         get { return LocalModelObject.AmountMax; }
         set
         {
            LocalModelObject.AmountMax = value;
            OnPropertyChanged("AmountMax");
         }
      }

      public double AmountMin
      {
         get { return LocalModelObject.AmountMin; }
         set
         {
            LocalModelObject.AmountMin = value;
            OnPropertyChanged("AmountMin");
         }
      }

      public List<string> PickupTypes
      {
         get
         {
            return new List<string>()
            {
               "notDef",
               "resourceAmmo",
               "resourceFuel",
               "resourceShield",
               "resourceCoin",
               "inventory"
            };
         }
      }

      public int SelectedPickupTypeIndex
      {
         get
         {
            return _selectedPickupTypeIndex;
         }
         set
         {
            if (value == -1)
            {
               return; // Why is -1 being written to it?
               // _selectedPickupTypeIndex = 0;
            }
            else
            {
               _selectedPickupTypeIndex = value;
            }
            OnPropertyChanged("SelectedPickupTypeIndex");

            PickupType = PickupTypes[_selectedPickupTypeIndex];
         }
      }

      public List<string> AmountTypes
      {
         get
         {
            return new List<string>()
            {
               "notDefAmount",
               "fix",
               "random"
            };
         }
      }

      public int SelectedAmountTypeIndex
      {
         get
         {
            return _selectedAmountTypeIndex;
         }
         set
         {
            if (value == -1)
            {
               return;
               // _selectedAmountTypeIndex = 0;
            }
            else
            {
               _selectedAmountTypeIndex = value;
            }
            OnPropertyChanged("SelectedAmountTypeIndex");

            PickupAmountType = AmountTypes[_selectedAmountTypeIndex];
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
