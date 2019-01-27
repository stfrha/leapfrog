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
   public class BreakableObjectProperties
   {
      #region Declarations

      private int _breakBulletDamage;
      private int _numberOfSpawns;
      private ObservableCollection<SpawnObject> _spawnObjects = new ObservableCollection<SpawnObject>();

      #endregion

      #region Constructors

      public BreakableObjectProperties()
      {
         _breakBulletDamage = 0;
         _numberOfSpawns = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("breakBulletDamage")]
      public int BreakBulletDamage
      {
         get { return _breakBulletDamage; }
         set { _breakBulletDamage = value; }
      }

      [XmlAttribute("numberOfSpawns")]
      public int NumberOfSpawns
      {
         get { return _numberOfSpawns; }
         set { _numberOfSpawns = value; }
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
