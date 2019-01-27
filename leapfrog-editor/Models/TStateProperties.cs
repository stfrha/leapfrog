using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   [Serializable]
   public class TStateProperties<T> 
   {
      #region Declarations

      private string _state = "default";
      private T _properties;

      #endregion

      #region Constructor

      public TStateProperties()
      {

      }

      #endregion

      #region Properties

      [XmlAttribute("state")]
      public string State
      {
         get { return _state; }
         set { _state = value; }
      }

      [XmlElement("properties")]
      public T Properties
      {
         get { return _properties; }
         set { _properties = value; }
      }

      #endregion
   }
}
