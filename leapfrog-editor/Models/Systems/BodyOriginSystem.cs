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
   public class BodyOriginSystem
   {
      #region Declarations

      private string _bodyName;
      private double _systemOriginX;
      private double _systemOriginY;

      #endregion

      #region Constructors

      public BodyOriginSystem()
      {
         _bodyName = "notDefined";
         _systemOriginX = 0;
         _systemOriginY = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("body")]
      public string BodyName
      {
         get { return _bodyName; }
         set { _bodyName = value; }
      }

      [XmlAttribute("systemOriginX")]
      public double SystemOriginX
      {
         get { return _systemOriginX; }
         set { _systemOriginX = value; }
      }

      [XmlAttribute("systemOriginY")]
      public double SystemOriginY
      {
         get { return _systemOriginY; }
         set { _systemOriginY = value; }
      }


      #endregion

   }
}
