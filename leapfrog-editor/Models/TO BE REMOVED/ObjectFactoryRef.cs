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
   public class ObjectFactoryRef
   {
      #region Declarations

      private string _name;
      private ObservableCollection<TStateProperties<ObjectFactoryProperties> > _stateProperties = new ObservableCollection<TStateProperties<ObjectFactoryProperties> >();

      #endregion

      #region Constructor

      public ObjectFactoryRef()
      {

      }

      #endregion

      #region Properties

      [XmlAttribute("name")]
      public string Name
      {
         get { return _name; }
         set { _name = value; }
      }

      [XmlElement("stateProperties")]
      public ObservableCollection<TStateProperties<ObjectFactoryProperties> > StateProperties
      {
         get { return _stateProperties; }
         set { _stateProperties = value; }
      }
         
      #endregion
   }
}
