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
   public class CompoundObject
   {
      #region Declarations

      //Behaviour
      private CoBehaviour _behaviour = new CoBehaviour();

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

      // Joints
      private ObservableCollection<WeldJoint> _weldJoints = new ObservableCollection<WeldJoint>();
      private ObservableCollection<RevoluteJoint> _revoluteJoints = new ObservableCollection<RevoluteJoint>();
      private ObservableCollection<PrismaticJoint> _prismaticJoints = new ObservableCollection<PrismaticJoint>();
      private ObservableCollection<Rope> _ropes= new ObservableCollection<Rope>();

      // Systems
      private ObservableCollection<CoSystem> _systems = new ObservableCollection<CoSystem>();

      // Child objects
      private ObservableCollection<ChildObject> _childObjects = new ObservableCollection<ChildObject>();

      // Following is special cases for landing scenes. Maybe these will be specialised 
      // xml files with specialised format. Or behaviours of a CompoundObject
      private ObservableCollection<PlanetActorRef> _planetActors = new ObservableCollection<PlanetActorRef>();
      private ObservableCollection<ClippedWindowRef> _clippedWindows = new ObservableCollection<ClippedWindowRef>();

      #endregion

      #region Constructor

      public CompoundObject()
      {
      }

      #endregion

      #region Properties

      [XmlElement("behaviour")]
      public CoBehaviour Behaviour
      {
         get { return _behaviour; }
         set { _behaviour = value; }
      }

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

      [XmlElement("weldJoint")]
      public ObservableCollection<WeldJoint> WeldJoints
      {
         get { return _weldJoints; }
         set { _weldJoints = value; }
      }

      [XmlElement("revoluteJoint")]
      public ObservableCollection<RevoluteJoint> RevoluteJoints
      {
         get { return _revoluteJoints; }
         set { _revoluteJoints = value; }
      }

      [XmlElement("prismaticJoint")]
      public ObservableCollection<PrismaticJoint> PrismaticJoints
      {
         get { return _prismaticJoints; }
         set { _prismaticJoints = value; }
      }

      [XmlElement("rope")]
      public ObservableCollection<Rope> Ropes
      {
         get { return _ropes; }
         set { _ropes = value; }
      }

      [XmlElement("system")]
      public ObservableCollection<CoSystem> Systems
      {
         get { return _systems; }
         set { _systems = value; }
      }

      [XmlElement("childObject")]
      public ObservableCollection<ChildObject> ChildObjects
      {
         get { return _childObjects; }
         set { _childObjects = value; }
      }

      // Following is special cases for landing scenes. Maybe these will be specialised 
      // xml files with specialised format.

      [XmlElement("planetActor")]
      public ObservableCollection<PlanetActorRef> PlanetActors
      {
         get { return _planetActors; }
         set { _planetActors = value; }
      }

      // At serialisation of a parent CompoundObject this collection
      // is read. Once read, the spaceSceneFile is used to create 
      // a child CompoundObject.
      [XmlElement("clippedWindow")]
      public ObservableCollection<ClippedWindowRef> ClippedWindows
      {
         get { return _clippedWindows; }
         set { _clippedWindows = value; }
      }

      #endregion

      #region Public Methods

      public static CompoundObject ReadFromFile(string fileName)
         // This method reads an xml file containing a CompoundObject
         // and returns with the generated CompoundObject. 
         // All child objects with other XML-files are also read 
         // (however it will not be possible to edit them without
         // explicitly open thems as separate files.
         // TODO: Error handling is non-existent.
      {
         string path = Path.GetDirectoryName(fileName);

         XmlSerializer ser = new XmlSerializer(typeof(CompoundObject));
         FileStream fs = new FileStream(fileName, FileMode.Open);
         XmlReader reader = XmlReader.Create(fs);
         CompoundObject co = (CompoundObject)ser.Deserialize(reader);

         // Iterate ChildObjects to load all child objects
         foreach (ChildObject cor in co.ChildObjects)
         {
            // Iterate all state properties
            foreach (TStateProperties<ChildObjectStateProperties> sp in cor.StateProperties)
            {
               // Only read files if the child object is referenced from a 
               // separate file, otherwise the serialization will already have
               // populated the child object.
               if ((sp.Properties.File != "") && (sp.Properties.CompObj == null))
               {
                  string newFile = System.IO.Path.Combine(path, sp.Properties.File);

                  CompoundObject childCo = CompoundObject.ReadFromFile(newFile);

                  sp.Properties.CompObj = childCo;

               }
            }
         }

         // Iterate SpawnObjects of BreakableObject Behaviour properties to load all spawn objects
         if (co.Behaviour.Type == "breakableObject")
         {
            foreach (SpawnObject so in co.Behaviour.BreakableObjProps.SpawnObjects)
            {
               ChildObject cor = so.MyChildObject;

               // Iterate all state properties
               foreach (TStateProperties<ChildObjectStateProperties> sp in cor.StateProperties)
               {
                  // Only read files if the child object is referenced from a 
                  // separate file, otherwise the serialization will already have
                  // populated the child object.
                  if ((sp.Properties.File != "") && (sp.Properties.CompObj == null))
                  {
                     string newFile = System.IO.Path.Combine(path, sp.Properties.File);

                     CompoundObject childCo = CompoundObject.ReadFromFile(newFile);

                     sp.Properties.CompObj = childCo;

                  }
               }
            }
         }

         // Iterate ClippedWindows to load all each child objects
         foreach (ClippedWindowRef cwr in co.ClippedWindows)
         {
            // Iterate all state properties
            foreach (TStateProperties<ClippedWindowProperties> sp in cwr.StateProperties)
            {
               string newFile = System.IO.Path.Combine(path, sp.Properties.SpaceSceneFile);

               CompoundObject childCo = CompoundObject.ReadFromFile(newFile);

               sp.Properties.CompObj = childCo;
            }
         }

         fs.Close();

         return co;
      }

      public void WriteToFile(string fileName)
         // This methos writes the CompoundObject to file. The children
         // that contains file references are not written since they 
         // can not be edited under this object. They need to be
         // opened and edited as separate files.
      {
         XmlWriterSettings settings = new XmlWriterSettings();
         settings.Indent = true;
         settings.NewLineOnAttributes = true;

         string path = Path.GetDirectoryName(fileName);

         XmlSerializerNamespaces ns = new XmlSerializerNamespaces();
         ns.Add("", "");

         XmlSerializer ser = new XmlSerializer(typeof(CompoundObject));
         FileStream fs = new FileStream(fileName, FileMode.Create);
         XmlWriter writer = XmlWriter.Create(fs, settings);
         ser.Serialize(writer, this, ns);

         // Iterate ClippedWindowProperties to save all space scenes as child objects
         foreach (ClippedWindowRef cw in ClippedWindows)
         {
            foreach (TStateProperties<ClippedWindowProperties> sp in cw.StateProperties)
            {
               string newFile = System.IO.Path.Combine(path, sp.Properties.SpaceSceneFile);

               sp.Properties.CompObj.WriteToFile(newFile);
            }
         }

         fs.Close();
      }

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

      public void RemoveJoint(object shape)
      {
         if (shape is Rope)
         {
            Ropes.Remove((Rope)shape);
         }
         else if (shape is RevoluteJoint)
         {
            RevoluteJoints.Remove((RevoluteJoint)shape);
         }
         else if (shape is PrismaticJoint)
         {
            PrismaticJoints.Remove((PrismaticJoint)shape);
         }
         else if (shape is WeldJoint)
         {
            WeldJoints.Remove((WeldJoint)shape);
         }
      }

      #endregion
   }
}
