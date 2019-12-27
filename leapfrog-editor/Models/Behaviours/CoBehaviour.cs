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
   public class CoBehaviour
   {
      #region Declarations

      private string _type;
      private SteerableObjectProperties _steerableObjProps = new SteerableObjectProperties();
      private BreakableObjectProperties _breakableObjProps = new BreakableObjectProperties();
      private ExplosiveObjectProperties _explosiveObjProps = new ExplosiveObjectProperties();
      private MagneticMineProperties _magneticMineProps = new MagneticMineProperties();
      private PickupObjectProperties _pickupProps = new PickupObjectProperties();
      private ParallaxBackgroundProperties _backgroundProps = new ParallaxBackgroundProperties();
      private SceneProperties _sceneProperties = new SceneProperties();

      #endregion

      #region Constructors

      public CoBehaviour()
      {
         _type = "notApplicable";
      }

      #endregion

      #region Properties

      [XmlAttribute("type")]
      public string Type
      {
         get { return _type; }
         set { _type = value; }
      }

      [XmlElement("steerableObjectProperties")]
      public SteerableObjectProperties SteerableObjProps
      {
         get { return _steerableObjProps; }
         set { _steerableObjProps = value; }
      }

      public bool ShouldSerializeSteerableObjProps()
      {
         return (Type == "steerableObject");
      }

      [XmlElement("breakableObjectProperties")]
      public BreakableObjectProperties BreakableObjProps
      {
         get { return _breakableObjProps; }
         set { _breakableObjProps = value; }
      }

      public bool ShouldSerializeBreakableObjProps()
      {
         return (Type == "breakableObject");
      }

      [XmlElement("explosiveObjectProperties")]
      public ExplosiveObjectProperties ExplosiveObjProps
      {
         get { return _explosiveObjProps; }
         set { _explosiveObjProps = value; }
      }

      public bool ShouldSerializeExplosiveObjProps()
      {
         return (Type == "explosiveObject");
      }

      // The line below looks funny but is correct (I hope)
      [XmlElement("magneticMineProperties")]
      public MagneticMineProperties MagneticMineProps
      {
         get { return _magneticMineProps; }
         set { _magneticMineProps = value; }
      }

      public bool ShouldSerializeMagneticMineProps()
      {
         return (Type == "magneticMine");
      }

      [XmlElement("pickupObjectProperties")]
      public PickupObjectProperties PickupObjProps
      {
         get { return _pickupProps; }
         set { _pickupProps = value; }
      }

      public bool ShouldSerializePickupObjProps()
      {
         return (Type == "explosiveObject");
      }

      [XmlElement("parallaxBackgroundProperties")]
      public ParallaxBackgroundProperties BackgroundProps
      {
         get { return _backgroundProps; }
         set { _backgroundProps = value; }
      }

      public bool ShouldSerializeBackgroundProps()
      {
         return (Type == "parallaxBackground");
      }


      [XmlElement("sceneProperties")]
      public SceneProperties SceneProperties
      {
         get { return _sceneProperties; }
         set { _sceneProperties = value; }
      }

      public bool ShouldSerializeSceneProperties()
      {
         return (Type == "scene");
      }

      #endregion

   }
}
