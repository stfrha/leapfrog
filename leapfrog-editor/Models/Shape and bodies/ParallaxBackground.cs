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
   public class ParallaxBackground : CompoundShapesObject
   {
      #region Declarations

      private double _parallaxAmount;
      private string _backgroundName;

      // Sprites
      private ObservableCollection<LfSpriteBox> _spriteBoxes = new ObservableCollection<LfSpriteBox>();
      private ObservableCollection<LfSpritePolygon> _spritePolygons = new ObservableCollection<LfSpritePolygon>();


      #endregion

      #region Constructors

      public ParallaxBackground()
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

      [XmlAttribute("name")]
      public string BackgroundName
      {
         get { return _backgroundName; }
         set { _backgroundName = value; }
      }

      #endregion

   }
}
