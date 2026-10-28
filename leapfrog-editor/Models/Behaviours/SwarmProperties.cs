using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   // Behaviour "swarm": spawns enemies (e.g. strippers) from all sides of the
   // scene, ramping up over time; Lua controls it (see swarm.h in the game).
   [Serializable]
   public class SwarmProperties
   {
      #region Declarations

      private string _spawn = "stripper.xml";
      private bool _active = true;
      private double _startRate = 0.2;
      private double _rate = 2;
      private double _startMaxAlive = 2;
      private double _maxAlive = 25;
      private double _rampTime = 180;
      private int _initialSpawn = 1;
      private string _sides = "";
      private double _spawnInset = 30;
      private double _minDistance = 200;

      #endregion

      #region Properties

      [XmlAttribute("spawn")]
      public string Spawn
      {
         get { return _spawn; }
         set { _spawn = value; }
      }

      [XmlAttribute("active")]
      public bool Active
      {
         get { return _active; }
         set { _active = value; }
      }

      [XmlAttribute("startRate")]
      public double StartRate
      {
         get { return _startRate; }
         set { _startRate = value; }
      }

      [XmlAttribute("rate")]
      public double Rate
      {
         get { return _rate; }
         set { _rate = value; }
      }

      [XmlAttribute("startMaxAlive")]
      public double StartMaxAlive
      {
         get { return _startMaxAlive; }
         set { _startMaxAlive = value; }
      }

      [XmlAttribute("maxAlive")]
      public double MaxAlive
      {
         get { return _maxAlive; }
         set { _maxAlive = value; }
      }

      [XmlAttribute("rampTime")]
      public double RampTime
      {
         get { return _rampTime; }
         set { _rampTime = value; }
      }

      [XmlAttribute("initialSpawn")]
      public int InitialSpawn
      {
         get { return _initialSpawn; }
         set { _initialSpawn = value; }
      }

      [XmlAttribute("sides")]
      public string Sides
      {
         get { return _sides; }
         set { _sides = value; }
      }

      public bool ShouldSerializeSides()
      {
         return !string.IsNullOrEmpty(Sides);
      }

      [XmlAttribute("spawnInset")]
      public double SpawnInset
      {
         get { return _spawnInset; }
         set { _spawnInset = value; }
      }

      [XmlAttribute("minDistance")]
      public double MinDistance
      {
         get { return _minDistance; }
         set { _minDistance = value; }
      }

      #endregion
   }
}
