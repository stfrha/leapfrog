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
   public class SceneProperties
   {
      #region Declarations

      private string _sceneType;
      private ObservableCollection<string> _states = new ObservableCollection<string>();


      #endregion

      #region Constructors

      public SceneProperties()
      {
         _sceneType = "notDefined";
   }

      #endregion

      #region Properties

      [XmlAttribute("sceneType")]
      public string SceneType
      {
         get { return _sceneType; }
         set { _sceneType = value; }
      }

      [XmlArray("states"), XmlArrayItem("state")]
      public ObservableCollection<string> States
      {
         get { return _states; }
         set { _states = value; }
      }

      #endregion

   }
}
