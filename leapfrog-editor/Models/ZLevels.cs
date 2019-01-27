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
   [XmlRoot("zLevels")]
   public class ZLevels
   {
      #region Declarations

      private ObservableCollection<ZLevelItem> _levels = new ObservableCollection<ZLevelItem>();

      #endregion

      #region Constructor

      public ZLevels()
      {

      }

      #endregion

      #region Properties

      [XmlElement("zLevel")]
      public ObservableCollection<ZLevelItem> Levels
      {
         get { return _levels; }
         set { _levels = value; }
      }

      #endregion

      #region Public Methods

      public static ZLevels ReadFromFile(string fileName)
      {
         string path = Path.GetDirectoryName(fileName);

         XmlSerializer ser = new XmlSerializer(typeof(ZLevels));
         FileStream fs = new FileStream(fileName, FileMode.Open);
         XmlReader reader = XmlReader.Create(fs);
         ZLevels zl = (ZLevels)ser.Deserialize(reader);

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

         XmlSerializer ser = new XmlSerializer(typeof(ZLevels));
         FileStream fs = new FileStream(fileName, FileMode.Create);
         XmlWriter writer = XmlWriter.Create(fs, settings);
         ser.Serialize(writer, this, ns);

         fs.Close();
      }

      #endregion

   }
}
