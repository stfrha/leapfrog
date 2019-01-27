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
   public class ClippedWindowRef
   {
      #region Declarations

      private string _name;
      private ObservableCollection<TStateProperties<ClippedWindowProperties> > _stateProperties = new ObservableCollection<TStateProperties<ClippedWindowProperties> >();

      #endregion

      #region Constructor

      public ClippedWindowRef()
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
      public ObservableCollection<TStateProperties<ClippedWindowProperties> > StateProperties
      {
         get { return _stateProperties; }
         set { _stateProperties = value; }
      }
         
      #endregion
   }
}
