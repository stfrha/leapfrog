using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   [Serializable]
   public class ZLevelItem
   {
      #region Declarations

      private uint _id;
      private string _name;

      #endregion

      #region Constructor

      public ZLevelItem()
      {
         _id = 0;
         _name = "noName";
      }

      public ZLevelItem(uint id, string name)
      {
         _id = id;
         _name = name;
      }

      #endregion

      #region Properties

      [XmlAttribute("id")]
      public uint Id
      {
         get { return _id; }
         set { _id = value; }
      }

      [XmlAttribute("name")]
      public string Name
      {
         get { return _name; }
         set { _name = value; }
      }

      #endregion
   }
}
