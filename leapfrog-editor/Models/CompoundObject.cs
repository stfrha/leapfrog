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
   public class CompoundObject : CompoundShapesObject
   {
      #region Declarations

      private bool _needsGroupIndex = false;

      //Behaviour
      private CoBehaviour _behaviour = new CoBehaviour();

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

      public CompoundObject() : base()
      {
      }

      #endregion

      #region Properties
           
      [XmlAttribute("needsGroupIndex")]
      public bool NeedsGroupIndex
      {
         get { return _needsGroupIndex; }
         set { _needsGroupIndex = value; }
      }

      [XmlElement("behaviour")]
      public CoBehaviour Behaviour
      {
         get { return _behaviour; }
         set { _behaviour = value; }
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

      public void AnalyseForChildFiles(string path)
      {
         // Iterate ChildObjects to load all child objects
         foreach (ChildObject cor in ChildObjects)
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
               else
               {
                  //...however, the child objects may have behaviours or systems that
                  // has spawn objects or child objects that need to be loaded
                  sp.Properties.CompObj.AnalyseForChildFiles(path);
               }
            }
         }

         // Iterate SpawnObjects of BreakableObject Behaviour properties to load all spawn objects
         if (Behaviour.Type == "breakableObject")
         {
            foreach (SpawnObject so in Behaviour.BreakableObjProps.SpawnObjects)
            {
               ChildObject cor = so.MyChildObject;

               // Iterate all state properties
               foreach (TStateProperties<ChildObjectStateProperties> sp in cor.StateProperties)
               {
                  // There may be null child objects and those will have a properties 
                  // set to null. Ignore these.
                  if (sp.Properties != null)
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
         }


         // Iterate SpawnObjects of ObejctFactories in Systems.Propertiesto load all spawn objects
         foreach (CoSystem sys in Systems)
         {
            if (sys.Type == "objectFactory")
            {
               foreach (SpawnObject so in sys.ObjFactStateProperties.SpawnObjects)
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
         }

         // Iterate ClippedWindows to load all each child objects
         foreach (ClippedWindowRef cwr in ClippedWindows)
         {
            // Iterate all state properties
            foreach (TStateProperties<ClippedWindowProperties> sp in cwr.StateProperties)
            {
               string newFile = System.IO.Path.Combine(path, sp.Properties.SpaceSceneFile);

               CompoundObject childCo = CompoundObject.ReadFromFile(newFile);

               sp.Properties.CompObj = childCo;
            }
         }
      }

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

         co.AnalyseForChildFiles(path);

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
