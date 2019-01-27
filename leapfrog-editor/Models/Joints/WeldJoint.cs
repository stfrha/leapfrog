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
   public class WeldJoint
   {
      #region Declarations

      private string _name;
      private string _aName;
      private string _bName;
      private double _aAnchorX;
      private double _aAnchorY;
      private double _bAnchorX;
      private double _bAnchorY;

      #endregion

      #region Constructors

      public WeldJoint()
      {
         _name = "noJointName";
         _aName = "notDef";
         _bName = "notDef";
         _aAnchorX = 0;
         _aAnchorY = 0;
         _bAnchorX = 0;
         _bAnchorY = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("name")]
      public string Name
      {
         get { return _name; }
         set { _name = value; }
      }

      [XmlAttribute("objectAName")]
      public string AName
      {
         get { return _aName; }
         set { _aName = value; }
      }

      [XmlAttribute("objectBName")]
      public string BName
      {
         get { return _bName; }
         set { _bName = value; }
      }

      [XmlAttribute("objectAAnchorX")]
      public double AAnchorX
      {
         get { return _aAnchorX; }
         set { _aAnchorX = value; }
      }

      [XmlAttribute("objectAAnchorY")]
      public double AAnchorY
      {
         get { return _aAnchorY; }
         set { _aAnchorY = value; }
      }

      [XmlAttribute("objectBAnchorX")]
      public double BAnchorX
      {
         get { return _bAnchorX; }
         set { _bAnchorX = value; }
      }

      [XmlAttribute("objectBAnchorY")]
      public double BAnchorY
      {
         get { return _bAnchorY; }
         set { _bAnchorY = value; }
      }

      #endregion

   }
}
