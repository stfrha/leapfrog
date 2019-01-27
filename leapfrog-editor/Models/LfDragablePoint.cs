using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   [Serializable]
   public class LfDragablePoint
    {
      #region Declarations

      private uint _id;
      private double _posX;
      private double _posY;

      #endregion

      #region Constructors

      public LfDragablePoint()
      {
         _id = 0;
         _posX = 0;
         _posY = 0;
      }

      public LfDragablePoint(uint id, double posX, double posY)
      {
         _id = id;
         _posX = posX;
         _posY = posY;
      }

      #endregion

      #region Properties

      [XmlAttribute("id")]
      public uint Id
      {
         get { return _id; }
         set { _id = value; }
      }

      [XmlAttribute("x")]
      public double PosX
      {
         get { return _posX; }
         set { _posX = value; }
      }

      [XmlAttribute("y")]
      public double PosY
      {
         get { return _posY; }
         set { _posY = value; }
      }

      #endregion
   }
}
