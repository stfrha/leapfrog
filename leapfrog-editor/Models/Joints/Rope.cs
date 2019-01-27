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
   public class Rope : WeldJoint
   {
      // All is inherited in this class except XML element name and view

      #region Declarations

      private string _texture;
      private double _length;
      private double _thickness;
      private uint _noOfSegments;
      private uint _zLevel;
      private string _collisionEntity;
      private uint _collisionCategory;
      private uint _collisionMask;

      #endregion

      #region Constructors

      public Rope() : base()
      {
         _texture = "default";
         _length = 10;
         _thickness = 0.5;
         _noOfSegments = 5;
         _zLevel = 0;
         _collisionEntity = "notApplicable";
         _collisionCategory = 1;
         _collisionMask = 65534;
      }

      #endregion

      #region Properties

      [XmlAttribute("texture")]
      public string Texture
      {
         get { return _texture; }
         set { _texture = value; }
      }

      [XmlAttribute("length")]
      public double Length
      {
         get { return _length; }
         set { _length = value; }
      }

      [XmlAttribute("thickness")]
      public double Thickness
      {
         get { return _thickness; }
         set { _thickness = value; }
      }

      [XmlAttribute("noOfSegments")]
      public uint NoOfSegments
      {
         get { return _noOfSegments; }
         set { _noOfSegments = value; }
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
