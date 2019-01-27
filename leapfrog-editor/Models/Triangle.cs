using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   [Serializable]
   public class Triangle
    {
      #region Declarations

      private uint _id;
      private uint _v1;
      private uint _v2;
      private uint _v3;

      #endregion

      #region Constructors

      public Triangle()
      {
         _id = 0;
         _v1 = 0;
         _v2 = 0;
         _v3 = 0;
      }

      public Triangle(uint id, uint v1, uint v2, uint v3)
      {
         _id = id;
         _v1 = v1;
         _v2 = v2;
         _v3 = v3;
      }

      #endregion

      #region Properties

      [XmlAttribute("id")]
      public uint Id
      {
         get { return _id; }
         set { _id = value; }
      }

      [XmlAttribute("v1")]
      public uint V1
      {
         get { return _v1; }
         set { _v1 = value; }
      }

      [XmlAttribute("v2")]
      public uint V2
      {
         get { return _v2; }
         set { _v2 = value; }
      }

      [XmlAttribute("v3")]
      public uint V3
      {
         get { return _v3; }
         set { _v3 = value; }
      }

      #endregion
   }
}
