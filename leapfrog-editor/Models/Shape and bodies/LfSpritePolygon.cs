﻿using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   [Serializable]
   public class LfSpritePolygon : LfScalableTexturePolygon
   {
      #region Declarations

      private double _parallaxAmount;

      #endregion

      #region Constructors

      public LfSpritePolygon()
      {
         _parallaxAmount = 0;
      }

      #endregion

      #region Properties

      [XmlAttribute("parallaxAmount")]
      public double ParallaxAmount
      {
         get { return _parallaxAmount; }
         set { _parallaxAmount = value; }
      }

      #endregion

   }
}
