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
   public class ObjectFactoryProperties : ISpawnObjectParentInterface
   {
      #region Declarations

      private string _body;
      private double _posX;
      private double _posY;
      private double _width;
      private double _height;
      private int _spawnInitial;
      private double _intensity;
      private int _lifeTime;
      private int _maxAlive = 0;
      private ObservableCollection<SpawnObject> _spawnObjects = new ObservableCollection<SpawnObject>();

      #endregion

      #region Constructors

      public ObjectFactoryProperties()
      {
         _body = "notDefined";
         _posX = 0;
         _posY = 0;
         _width = 100;
         _height = 100;
         _spawnInitial = 150;
         _intensity = 0.4;
         _lifeTime = 60000;
      }

      #endregion

      #region Properties

      [XmlAttribute("body")]
      public string Body
      {
         get { return _body; }
         set { _body = value; }
      }

      [XmlAttribute("posX")]
      public double PosX
      {
         get { return _posX; }
         set { _posX = value; }
      }

      [XmlAttribute("posY")]
      public double PosY
      {
         get { return _posY; }
         set { _posY = value; }
      }

      [XmlAttribute("width")]
      public double Width
      {
         get { return _width; }
         set { _width = value; }
      }

      [XmlAttribute("height")]
      public double Height
      {
         get { return _height; }
         set { _height = value; }
      }

      [XmlAttribute("spawnInitial")]
      public int SpawnInitial
      {
         get { return _spawnInitial; }
         set { _spawnInitial = value; }
      }
      
      [XmlAttribute("intensity")]
      public double Intensity
      {
         get { return _intensity; }
         set { _intensity = value; }
      }

      [XmlAttribute("lifeTime")]
      public int LifeTime
      {
         get { return _lifeTime; }
         set { _lifeTime = value; }
      }

      // At most this many spawned objects alive at a time; a new one is
      // spawned (at the intensity) only while below it. 0 = no limit.
      [XmlAttribute("maxAlive")]
      public int MaxAlive
      {
         get { return _maxAlive; }
         set { _maxAlive = value; }
      }

      public bool ShouldSerializeMaxAlive()
      {
         return MaxAlive > 0;
      }

      [XmlElement("spawnObject")]
      public ObservableCollection<SpawnObject> SpawnObjects
      {
         get { return _spawnObjects; }
         set { _spawnObjects = value; }
      }


      #endregion

   }
}
