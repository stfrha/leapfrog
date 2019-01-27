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
   public class PlanetActorProperties
   {
      #region Declarations

      private string _texture;
      private double _posY;
      private double _posX;
      private double _scale;
      private ObservableCollection<LandingSite> _landingSites = new ObservableCollection<LandingSite>();

      #endregion

      #region Constructors

      public PlanetActorProperties()
      {
         _texture = "default";
         _posX = -30;
         _posY = 20;
         _scale = 0.3;
      }

      #endregion

      #region Properties

      [XmlAttribute("texture")]
      public string Texture
      {
         get { return _texture; }
         set { _texture = value; }
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

      [XmlAttribute("scale")]
      public double Scale
      {
         get { return _scale; }
         set { _scale = value; }
      }

      [XmlArray("landingSites"), XmlArrayItem("landingSite")]
      public ObservableCollection<LandingSite> LandingSites
      {
         get { return _landingSites; }
         set { _landingSites = value; }
      }


      #endregion

   }
}
