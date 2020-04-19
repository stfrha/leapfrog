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
   public class ChildObject
   {
      #region Declarations

      private string _name;
      private string _file = "undef_file.xml";
      private double _posX = 0;
      private double _posY = 0;
      private CompoundObject _compObj = null;

      #endregion

      #region Constructor

      public ChildObject()
      {
         _name = "noName";
         //CompObj = new CompoundObject();
      }

      #endregion

      #region Properties

      [XmlAttribute("name")]
      public string Name
      {
         get { return _name; }
         set { _name = value; }
      }

      [XmlAttribute("file")]
      public string File
      {
         get { return _file; }
         set { _file = value; }
      }

      public bool ShouldSerializeFile()
      {
         return ((_file != "") && (_file != "undef_file.xml"));
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

      [XmlElement("compoundObject")]
      public CompoundObject CompObj
      {
         get { return _compObj; }
         set { _compObj = value; }
      }

      public bool ShouldSerializeCompObj()
      {
         return ((_file == "") || (_file == "undef_file.xml"));
      }


      #endregion
   }
}
