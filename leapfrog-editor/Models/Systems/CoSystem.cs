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
   public class CoSystem
   {
      #region Declarations

      private string _name;
      private string _type;
      private ObjectFactoryProperties _objFactStateProperties;
      private GunProperties _gunStateProperties;
      private ShieldProperties _shieldStateProperties;
      private FlameEmitterProperties _flameEmitterStateProperties;
      private ReentryFlameEmitterProperties _reentryFlameEmitterStateProperties;

      #endregion

      #region Constructors

      public CoSystem()
      {
         _name = "notDefined";
         _type = "notDefined";
      }

      #endregion

      #region Properties

      [XmlAttribute("name")]
      public string Name
      {
         get { return _name; }
         set { _name = value; }
      }

      [XmlAttribute("type")]
      public string Type
      {
         get { return _type; }
         set { _type = value; }
      }

      [XmlElement("objectFactoryProperties")]
      public ObjectFactoryProperties ObjFactStateProperties
      {
         get { return _objFactStateProperties; }
         set { _objFactStateProperties = value; }
      }

      public bool ShouldSerializeObjFactStateProperties()
      {
         return (Type == "objectFactory");
      }

      [XmlElement("gunProperties")]
      public GunProperties GunStateProperties
      {
         get { return _gunStateProperties; }
         set { _gunStateProperties = value; }
      }

      public bool ShouldSerializeGunStateProperties()
      {
         return (Type == "gun");
      }

      [XmlElement("shieldProperties")]
      public ShieldProperties ShieldStateProperties
      {
         get { return _shieldStateProperties; }
         set { _shieldStateProperties = value; }
      }

      public bool ShouldSerializeShieldStateProperties()
      {
         return (Type == "shield");
      }


      [XmlElement("flameEmitterProperties")]
      public FlameEmitterProperties FlameEmitterStateProperties
      {
         get { return _flameEmitterStateProperties; }
         set { _flameEmitterStateProperties = value; }
      }

      public bool ShouldSerializeFlameEmitterStateProperties()
      {
         return (Type == "flameEmitter");
      }

      [XmlElement("reentryFlameEmitterProperties")]
      public ReentryFlameEmitterProperties ReentryFlameEmitterStateProperties
      {
         get { return _reentryFlameEmitterStateProperties; }
         set { _reentryFlameEmitterStateProperties = value; }
      }

      public bool ShouldSerializeReentryFlameEmitterStateProperties()
      {
         return (Type == "reentryFlameEmitter");
      }



      #endregion

   }
}
