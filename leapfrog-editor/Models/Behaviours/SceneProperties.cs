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
      private double _width;
      private double _height;
      private bool _hasUpperBoundary;
      private bool _hasLowerBoundary;
      private bool _hasLeftBoundary;
      private bool _hasRightBoundary;

      private ObservableCollection<string> _states = new ObservableCollection<string>();
      private ObservableCollection<ParallaxBackground> _backgrounds = new ObservableCollection<ParallaxBackground>();


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

      [XmlAttribute("width")]
      public double Width
      {
         get { return _width; }
         set { _width = value; }
      }

      [XmlAttribute("height")]
      public double Height
      {
         get { return _height; }
         set { _height = value; }
      }

      [XmlAttribute("hasUpperBoundary")]
      public bool HasUpperBoundary
      {
         get { return _hasUpperBoundary; }
         set { _hasUpperBoundary = value; }
      }

      [XmlAttribute("hasLowerBoundary")]
      public bool HasLowerBoundary
      {
         get { return _hasLowerBoundary; }
         set { _hasLowerBoundary = value; }
      }

      [XmlAttribute("hasLeftBoundary")]
      public bool HasLeftBoundary
      {
         get { return _hasLeftBoundary; }
         set { _hasLeftBoundary = value; }
      }

      [XmlAttribute("hasRightBoundary")]
      public bool HasRightBoundary
      {
         get { return _hasRightBoundary; }
         set { _hasRightBoundary = value; }
      }

      [XmlArray("states"), XmlArrayItem("state")]
      public ObservableCollection<string> States
      {
         get { return _states; }
         set { _states = value; }
      }

      [XmlElement("parallaxBackground")]
      public ObservableCollection<ParallaxBackground> Backgrounds
      {
         get { return _backgrounds; }
         set { _backgrounds = value; }
      }

      #endregion

   }
}
