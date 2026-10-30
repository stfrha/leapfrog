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
      private StripperProperties _stripperProps = new StripperProperties();
      private SwarmProperties _swarmProps = new SwarmProperties();
      private SceneProperties _sceneProperties = new SceneProperties();
      private PlanetSurfaceProperties _planetSurfaceProps = new PlanetSurfaceProperties();
      private OrbitSceneProperties _orbitSceneProps = new OrbitSceneProperties();
      private bool _active = true;
      private string _fuel = "none";
      private double _fuelRate = 12.0;
      private double _fuelPrice = 5.0;
      private double _fuelSupply = 100.0;

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

      // Landing pads and launch sites: an inactive one can be landed on, but
      // nothing happens there (no events, no refuelling). Lua can change it
      // (property 2). Written only when false, so existing files don't change.
      [XmlAttribute("active")]
      public bool Active
      {
         get { return _active; }
         set { _active = value; }
      }

      public bool ShouldSerializeActive()
      {
         return HasActiveFlag(Type) && !Active;
      }

      public static bool HasActiveFlag(string type)
      {
         return (type == "landingPad") || (type == "launchSite");
      }

      // Landing pads: fuel service. "none" (default), "sell" (for credits,
      // fuelPrice credits per unit) or "free" (from a supply of fuelSupply,
      // -1 = no limit). fuelRate is fuel per second (the tank holds 100).
      // Only written for pads that have fuel.
      [XmlAttribute("fuel")]
      public string Fuel
      {
         get { return _fuel; }
         set { _fuel = value; }
      }

      public bool ShouldSerializeFuel()
      {
         return (Type == "landingPad") && (Fuel != "none");
      }

      [XmlAttribute("fuelRate")]
      public double FuelRate
      {
         get { return _fuelRate; }
         set { _fuelRate = value; }
      }

      public bool ShouldSerializeFuelRate()
      {
         return ShouldSerializeFuel();
      }

      [XmlAttribute("fuelPrice")]
      public double FuelPrice
      {
         get { return _fuelPrice; }
         set { _fuelPrice = value; }
      }

      public bool ShouldSerializeFuelPrice()
      {
         return ShouldSerializeFuel() && (Fuel == "sell");
      }

      [XmlAttribute("fuelSupply")]
      public double FuelSupply
      {
         get { return _fuelSupply; }
         set { _fuelSupply = value; }
      }

      public bool ShouldSerializeFuelSupply()
      {
         return ShouldSerializeFuel() && (Fuel == "free");
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
         return (Type == "pickupObject");
      }

      [XmlElement("stripperProperties")]
      public StripperProperties StripperProps
      {
         get { return _stripperProps; }
         set { _stripperProps = value; }
      }

      public bool ShouldSerializeStripperProps()
      {
         return (Type == "stripper");
      }

      [XmlElement("swarmProperties")]
      public SwarmProperties SwarmProps
      {
         get { return _swarmProps; }
         set { _swarmProps = value; }
      }

      public bool ShouldSerializeSwarmProps()
      {
         return (Type == "swarm");
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

      // A planet's surface, edited unrolled (see PlanetSurfaceProperties)
      [XmlElement("planetSurfaceProperties")]
      public PlanetSurfaceProperties PlanetSurfaceProps
      {
         get { return _planetSurfaceProps; }
         set { _planetSurfaceProps = value; }
      }

      public bool ShouldSerializePlanetSurfaceProps()
      {
         return (Type == "planetSurface");
      }

      // An orbit (re-entry) scene's planet (see OrbitSceneProperties)
      [XmlElement("properties")]
      public OrbitSceneProperties OrbitSceneProps
      {
         get { return _orbitSceneProps; }
         set { _orbitSceneProps = value; }
      }

      public bool ShouldSerializeOrbitSceneProps()
      {
         return (Type == "orbitScene");
      }

      #endregion

   }
}
