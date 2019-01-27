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
   public class SpawnObject
   {
      #region Declarations

      private double _probabilityFactor;
      private ChildObject _childObject;

      #endregion

      #region Constructors

      public SpawnObject()
      {
         _probabilityFactor = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("probabilityFactor")]
      public double ProbabilityFactor
      {
         get { return _probabilityFactor; }
         set { _probabilityFactor = value; }
      }

      [XmlElement("childObject")]
      public ChildObject MyChildObject
      {
         get { return _childObject; }
         set { _childObject = value; }
      }


      #endregion

   }
}
