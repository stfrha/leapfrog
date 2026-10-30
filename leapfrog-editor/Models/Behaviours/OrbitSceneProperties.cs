using System;
using System.Collections.ObjectModel;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   // An orbit (re-entry) scene (behaviour "orbitScene", <properties>):
   //   texture               the planet's texture (the surface file's
   //                         wins)
   //   spaceSceneBackground  the space view's background
   //   spaceSceneFile        the space view's scene
   //   posX, posY            (not used by the game)
   //   surfaceFile           the planet's surface (behaviour
   //                         "planetSurface"): its art and landing sites
   //   landingSites          without a surface file
   // Modelled so that the editor keeps it all when it saves the file.
   [Serializable]
   public class OrbitSceneProperties
   {
      private string _texture = "";
      private string _spaceSceneBackground = "";
      private string _spaceSceneFile = "";
      private double _posX = 0.0;
      private double _posY = 0.0;
      private string _surfaceFile = "";
      private ObservableCollection<LandingSite> _landingSites = new ObservableCollection<LandingSite>();

      [XmlAttribute("texture")]
      public string Texture
      {
         get { return _texture; }
         set { _texture = value; }
      }

      [XmlAttribute("spaceSceneBackground")]
      public string SpaceSceneBackground
      {
         get { return _spaceSceneBackground; }
         set { _spaceSceneBackground = value; }
      }

      [XmlAttribute("spaceSceneFile")]
      public string SpaceSceneFile
      {
         get { return _spaceSceneFile; }
         set { _spaceSceneFile = value; }
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

      [XmlAttribute("surfaceFile")]
      public string SurfaceFile
      {
         get { return _surfaceFile; }
         set { _surfaceFile = value; }
      }

      public bool ShouldSerializeSurfaceFile()
      {
         return !string.IsNullOrEmpty(SurfaceFile);
      }

      [XmlArray("landingSites"), XmlArrayItem("landingSite")]
      public ObservableCollection<LandingSite> LandingSites
      {
         get { return _landingSites; }
         set { _landingSites = value; }
      }

      public bool ShouldSerializeLandingSites()
      {
         return LandingSites.Count > 0;
      }
   }
}
