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
   public class DynamicPolygon : ScalableTexturePolygon
   {
      #region Declarations

      private string _name;
      private string _texture;
      private string _collisionEntity;
      private uint _zLevel;
      private uint _collisionCategory;
      private uint _collisionMask;

      #endregion

      #region Constructors

      public DynamicPolygon() :
         base(200, 200, 0, 0)
      {
         _name = "noName";
         _texture = "default";
         _collisionEntity = "notApplicable";
         _zLevel = 0;
         _collisionCategory = 0;
         _collisionMask = 0;
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

      [XmlAttribute("collisionEntity")]
      public string CollisionEntity
      {
         get { return _collisionEntity; }
         set { _collisionEntity = value; }
      }

      [XmlAttribute("zLevel")]
      public uint ZLevel
      {
         get { return _zLevel; }
         set { _zLevel = value; }
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
