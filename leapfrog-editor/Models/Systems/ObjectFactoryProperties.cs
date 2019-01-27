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
   public class ObjectFactoryProperties
   {
      #region Declarations

      private double _posX;
      private double _posY;
      private double _width;
      private double _height;
      private int _spawnInitial;
      private double _intensity;
      private int _lifeTime;
      private ObservableCollection<SpawnObject> _spawnObjects = new ObservableCollection<SpawnObject>();

      #endregion

      #region Constructors

      public ObjectFactoryProperties()
      {
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

      [XmlElement("spawnObject")]
      public ObservableCollection<SpawnObject> SpawnObjects
      {
         get { return _spawnObjects; }
         set { _spawnObjects = value; }
      }


      #endregion

   }
}
