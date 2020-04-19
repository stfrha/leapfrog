using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   [Serializable]
   public class ChildObjectStateProperties
   {
      #region Declarations


      #endregion

      #region Constructor

      public ChildObjectStateProperties()
      {

      }

      #endregion

      #region Properties

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
