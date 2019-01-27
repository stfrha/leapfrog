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
   public class PolygonBorder
   {
      #region Declarations

      private double _leftGroundAngle;
      private double _rightGroundAngle;
      private double _leftCeilingAngle;
      private double _rightCeilingAngle;
      private BorderTexture _groundBorder = new BorderTexture();
      private BorderTexture _ceilingBorder = new BorderTexture();
      private BorderTexture _leftWallBorder = new BorderTexture();
      private BorderTexture _rightWallBorder = new BorderTexture();

      #endregion

      #region Constructors

      public PolygonBorder()
      {
         _leftGroundAngle = 0;
         _rightGroundAngle = 0;
         _leftCeilingAngle = 0;
         _rightCeilingAngle = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("leftGroundAngle")]
      public double LeftGroundAngle
      {
         get { return _leftGroundAngle; }
         set { _leftGroundAngle = value; }
      }

      [XmlAttribute("rightGroundAngle")]
      public double RightGroundAngle
      {
         get { return _rightGroundAngle; }
         set { _rightGroundAngle = value; }
      }

      [XmlAttribute("leftCeilingAngle")]
      public double LeftCeilingAngle
      {
         get { return _leftCeilingAngle; }
         set { _leftCeilingAngle = value; }
      }

      [XmlAttribute("rightCeilingAngle")]
      public double RightCeilingAngle
      {
         get { return _rightCeilingAngle; }
         set { _rightCeilingAngle = value; }
      }

      [XmlElement("groundBorder")]
      public BorderTexture GroundBorder
      {
         get { return _groundBorder; }
         set { _groundBorder = value; }
      }

      [XmlElement("ceilingBorder")]
      public BorderTexture CeilingBorder
      {
         get { return _ceilingBorder; }
         set { _ceilingBorder = value; }
      }

      [XmlElement("leftWallBorder")]
      public BorderTexture LeftWallBorder
      {
         get { return _leftWallBorder; }
         set { _leftWallBorder = value; }
      }

      [XmlElement("rightWallBorder")]
      public BorderTexture RightWallBorder
      {
         get { return _rightWallBorder; }
         set { _rightWallBorder = value; }
      }

      #endregion

   }
}
