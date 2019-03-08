using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   [Serializable]
   public class PickupObjectProperties
   {
      //public enum PickupTypeEnum
      //{
      //   notDef,
      //   resourceAmmo,
      //   resourceFuel,
      //   resourceShield,
      //   resourceCoin,
      //   inventory
      //}

      //public enum PickupResourceAmountTypeEnum
      //{
      //   notDefAmount,
      //   fix,
      //   random
      //};

      #region Declarations

      private string _pickupType;
      private string _pickupAmountType;
      private double _amount;
      private double _amountMax;
      private double _amountMin;

      #endregion

      #region Constructors

      public PickupObjectProperties()
      {
         _pickupType = "notDef";
         _pickupAmountType = "notDefAmount";
         _amount = 0;
         _amountMax = 0;
         _amountMin = 0;
      }

   #endregion

   #region Properties

      [XmlAttribute("pickupType")]
      public string PickupType
      {
         get { return _pickupType; }
         set { _pickupType = value; }
      }

      [XmlAttribute("amountType")]
      public string PickupAmountType
      {
         get { return _pickupAmountType; }
         set { _pickupAmountType = value; }
      }

      [XmlAttribute("amount")]
      public double Amount
      {
         get { return _amount; }
         set { _amount = value; }
      }

      [XmlAttribute("amountMax")]
      public double AmountMax
      {
         get { return _amountMax; }
         set { _amountMax = value; }
      }

      [XmlAttribute("amountMin")]
      public double AmountMin
      {
         get { return _amountMin; }
         set { _amountMin = value; }
      }

      #endregion

   }
}
