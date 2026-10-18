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
      // The polygon border is in LfScalableTexturePolygon, shared with sprite polygons

      #region Constructors

      public LfStaticPolygon()
      {
      }

      #endregion

   }
}
