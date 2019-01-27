using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   [Serializable]
   [XmlRoot("collisionEntities")]
   public class CollisionEntities
   {
      #region Declarations

      private ObservableCollection<CollisionEntity> _entities = new ObservableCollection<CollisionEntity>();

      #endregion

      #region Constructor

      public CollisionEntities()
      {

      }

      #endregion

      #region Properties

      [XmlElement("collisionEntity")]
      public ObservableCollection<CollisionEntity> Entities
      {
         get { return _entities; }
         set { _entities = value; }
      }

      #endregion

      #region Public Methods

      public static CollisionEntities ReadFromFile(string fileName)
      {
         string path = Path.GetDirectoryName(fileName);

         XmlSerializer ser = new XmlSerializer(typeof(CollisionEntities));
         FileStream fs = new FileStream(fileName, FileMode.Open);
         XmlReader reader = XmlReader.Create(fs);
         CollisionEntities zl = (CollisionEntities)ser.Deserialize(reader);

         fs.Close();

         return zl;
      }

      public void WriteToFile(string fileName)
      {
         XmlWriterSettings settings = new XmlWriterSettings();
         settings.Indent = true;
         settings.NewLineOnAttributes = true;

         string path = Path.GetDirectoryName(fileName);

         XmlSerializerNamespaces ns = new XmlSerializerNamespaces();
         ns.Add("", "");

         XmlSerializer ser = new XmlSerializer(typeof(CollisionEntities));
         FileStream fs = new FileStream(fileName, FileMode.Create);
         XmlWriter writer = XmlWriter.Create(fs, settings);
         ser.Serialize(writer, this, ns);

         fs.Close();
      }

      #endregion

   }
}
