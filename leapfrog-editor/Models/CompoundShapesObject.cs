using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Xml;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   [Serializable]
   [XmlRoot("compoundObject")]
   public class CompoundShapesObject
   {
      #region Declarations

      // Shapes
      private ObservableCollection<LfSpriteBox> _spriteBoxes = new ObservableCollection<LfSpriteBox>();
      private ObservableCollection<LfSpritePolygon> _spritePolygons = new ObservableCollection<LfSpritePolygon>();
      private ObservableCollection<LfStaticCircle> _staticCircles = new ObservableCollection<LfStaticCircle>();
      private ObservableCollection<LfStaticBox> _staticBoxes = new ObservableCollection<LfStaticBox>();
      private ObservableCollection<LfStaticPolygon> _staticPolygons = new ObservableCollection<LfStaticPolygon>();
      private ObservableCollection<LfDynamicBox> _dynamicBoxes = new ObservableCollection<LfDynamicBox>();
      private ObservableCollection<LfDynamicCircle> _dynamicCircles = new ObservableCollection<LfDynamicCircle>();
      private ObservableCollection<LfDynamicPolygon> _dynamicPolygons = new ObservableCollection<LfDynamicPolygon>();
      private ObservableCollection<LfStaticBoxedSpritePolygon> _staticBoxedSpritePolygons = new ObservableCollection<LfStaticBoxedSpritePolygon>();
      private ObservableCollection<LfDynamicBoxedSpritePolygon> _dynamicBoxedSpritePolygons = new ObservableCollection<LfDynamicBoxedSpritePolygon>();


      #endregion

      #region Constructor

      public CompoundShapesObject()
      {
      }

      #endregion

      #region Properties
           
      [XmlElement("spriteBox")]
      public ObservableCollection<LfSpriteBox> SpriteBoxes
      {
         get { return _spriteBoxes; }
         set { _spriteBoxes = value; }
      }

      [XmlElement("spritePolygon")]
      public ObservableCollection<LfSpritePolygon> SpritePolygons
      {
         get { return _spritePolygons; }
         set { _spritePolygons = value; }
      }

      [XmlElement("staticBox")]
      public ObservableCollection<LfStaticBox> StaticBoxes
      {
         get { return _staticBoxes; }
         set { _staticBoxes = value; }
      }

      [XmlElement("staticCircle")]
      public ObservableCollection<LfStaticCircle> StaticCircles
      {
         get { return _staticCircles; }
         set { _staticCircles = value; }
      }

      [XmlElement("staticPolygon")]
      public ObservableCollection<LfStaticPolygon> StaticPolygons
      {
         get { return _staticPolygons; }
         set { _staticPolygons = value; }
      }

      [XmlElement("dynamicBox")]
      public ObservableCollection<LfDynamicBox> DynamicBoxes
      {
         get { return _dynamicBoxes; }
         set { _dynamicBoxes = value; }
      }

      [XmlElement("dynamicCircle")]
      public ObservableCollection<LfDynamicCircle> DynamicCircles
      {
         get { return _dynamicCircles; }
         set { _dynamicCircles = value; }
      }

      [XmlElement("dynamicPolygon")]
      public ObservableCollection<LfDynamicPolygon> DynamicPolygons
      {
         get { return _dynamicPolygons; }
         set { _dynamicPolygons = value; }
      }

      [XmlElement("staticBoxedSpritePolygonBody")]
      public ObservableCollection<LfStaticBoxedSpritePolygon> StaticBoxedSpritePolygons
      {
         get { return _staticBoxedSpritePolygons; }
         set { _staticBoxedSpritePolygons = value; }
      }

      [XmlElement("dynamicBoxedSpritePolygonBody")]
      public ObservableCollection<LfDynamicBoxedSpritePolygon> DynamicBoxedSpritePolygons
      {
         get { return _dynamicBoxedSpritePolygons; }
         set { _dynamicBoxedSpritePolygons = value; }
      }

      #endregion

      #region Public Methods

      public void RemoveShape(object shape)
      {
         // Below it is important to start with the classes
         // that are of lowest decendants since they will trigger
         // in the base classes otherwise

         if (shape is LfSpriteBox)
         {
            SpriteBoxes.Remove((LfSpriteBox)shape);
         }
         else if (shape is LfSpritePolygon)
         {
            SpritePolygons.Remove((LfSpritePolygon)shape);
         }
         else if (shape is LfDynamicBox)
         {
            DynamicBoxes.Remove((LfDynamicBox)shape);
         }
         else if (shape is LfDynamicCircle)
         {
            DynamicCircles.Remove((LfDynamicCircle)shape);
         }
         else if (shape is LfDynamicPolygon)
         {
            DynamicPolygons.Remove((LfDynamicPolygon)shape);
         }
         else if (shape is LfDynamicBoxedSpritePolygon)
         {
            DynamicBoxedSpritePolygons.Remove((LfDynamicBoxedSpritePolygon)shape);
         }
         else if (shape is LfStaticBox)
         {
            StaticBoxes.Remove((LfStaticBox)shape);
         }
         else if (shape is LfStaticCircle)
         {
            StaticCircles.Remove((LfStaticCircle)shape);
         }
         else if (shape is LfStaticPolygon)
         {
            StaticPolygons.Remove((LfStaticPolygon)shape);
         }
         else if (shape is LfStaticBoxedSpritePolygon)
         {
            StaticBoxedSpritePolygons.Remove((LfStaticBoxedSpritePolygon)shape);
         }
      }

      #endregion
   }
}
