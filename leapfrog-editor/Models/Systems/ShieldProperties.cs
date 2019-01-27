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
   public class ShieldProperties
   {
      #region Declarations

      private string _bodyName;
      private double _bodyAnchorX;
      private double _bodyAnchorY;
      private double _radius;
      private int _zLevel;

      #endregion

      #region Constructors

      public ShieldProperties()
      {
         _bodyName = "notDefined";
         _bodyAnchorX = 0;
         _bodyAnchorY = 0;
         _radius = 100;
         _zLevel = 147;
      }

      #endregion

      #region Properties

      [XmlAttribute("body")]
      public string BodyName
      {
         get { return _bodyName; }
         set { _bodyName = value; }
      }

      [XmlAttribute("bodyAnchorX")]
      public double BodyAnchorX
      {
         get { return _bodyAnchorX; }
         set { _bodyAnchorX = value; }
      }

      [XmlAttribute("bodyAnchorY")]
      public double BodyAnchorY
      {
         get { return _bodyAnchorY; }
         set { _bodyAnchorY = value; }
      }

      [XmlAttribute("radius")]
      public double Radius
      {
         get { return _radius; }
         set { _radius = value; }
      }

      [XmlAttribute("zLevel")]
      public int ZLevel
      {
         get { return _zLevel; }
         set { _zLevel = value; }
      }

      #endregion

   }
}
