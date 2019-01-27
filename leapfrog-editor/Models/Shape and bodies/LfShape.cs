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
   public class LfShape
   {
      #region Declarations

      private string _name;
      private string _texture;
      private double _posY;
      private double _posX;
      private double _angle;
      private uint _zLevel;
      private string _collisionEntity;
      private uint _collisionCategory;
      private uint _collisionMask;

      #endregion

      #region Constructors

      public LfShape()
      {
         _name = "noName";
         _texture = "default";
         _posX = 0;
         _posY = 0;
         _angle = 0;
         //_anchorX = 0;
         //_anchorY = 0;
         _zLevel = 0;
         _collisionEntity = "notApplicable";
         _collisionCategory = 1;
         _collisionMask = 65534;
      }

      #endregion

      #region Properties

      [XmlAttribute("name")]
      public string Name
      {
         get { return _name; }
         set { _name = value; }
      }

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

      [XmlAttribute("angle")]
      public double Angle
      {
         get { return _angle; }
         set { _angle = value; }
      }

      [XmlAttribute("zLevel")]
      public uint ZLevel
      {
         get { return _zLevel; }
         set { _zLevel = value; }
      }

      [XmlAttribute("collisionEntity")]
      public string CollisionEntity
      {
         get { return _collisionEntity; }
         set { _collisionEntity = value; }
      }

      [XmlAttribute("collisionCategory")]
      public uint CollisionCategory
      {
         get { return _collisionCategory; }
         set { _collisionCategory = value; }
      }

      [XmlAttribute("collisionMask")]
      public uint CollisionMask
      {
         get { return _collisionMask; }
         set { _collisionMask = value; }
      }

      #endregion

   }
}
