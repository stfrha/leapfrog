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
   public class LfStaticPolygon : LfScalableTexturePolygon
   {
      #region Declarations

      private PolygonBorder _polygonBorder = new PolygonBorder();

      #endregion

      #region Constructors

      public LfStaticPolygon()
      {
      }

      #endregion

      #region Properties

      [XmlElement("border")]
      public PolygonBorder PolygonBorder
      {
         get { return _polygonBorder; }
         set { _polygonBorder = value; }
      }

      #endregion

   }
}
