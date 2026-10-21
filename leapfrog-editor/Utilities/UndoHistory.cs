using System;
using System.Collections.Generic;
using System.IO;
using System.Reflection;
using System.Xml;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   // Undo and redo for one edited object.
   //
   // The history keeps the object's states as the XML it would be saved as
   // (the same serialisation as the object files). A checkpoint compares the
   // object with the last kept state; if anything differs, the last state
   // becomes an undo step. MainViewModel takes a checkpoint shortly after
   // each mouse button release, key release and wheel turn, so a drag, a
   // paste, a deletion or an edited property each become one step, without
   // every editing action having to report itself.
   //
   // Undoing reads a kept state back and puts its contents into the edited
   // object's model (CopyContents), after which its view models are built
   // again (CompoundObjectViewModel.RestoreModelContents).
   //
   // Child objects that refer to a file are saved as the file name only.
   // Their objects are shared with the open object (they come from their
   // files and aren't edited here), so the history remembers them by file
   // name and puts them back when a state is read.
   public class UndoHistory
   {
      // Oldest steps are dropped beyond these
      public const int MaxSteps = 200;
      public const long MaxChars = 50000000;

      private static XmlSerializer _serializer;

      private readonly List<string> _undo = new List<string>();
      private readonly List<string> _redo = new List<string>();
      private string _current;
      private readonly Dictionary<string, CompoundObject> _fileObjects = new Dictionary<string, CompoundObject>(StringComparer.OrdinalIgnoreCase);

      public UndoHistory(CompoundObject model)
      {
         RememberFileObjects(model);
         _current = Serialize(model);
      }

      public bool CanUndo
      {
         get { return _undo.Count > 0; }
      }

      public bool CanRedo
      {
         get { return _redo.Count > 0; }
      }

      // Makes any change since the last kept state an undo step. Returns
      // true if there was a change.
      public bool Checkpoint(CompoundObject model)
      {
         string now = Serialize(model);

         if (now == _current)
         {
            return false;
         }

         RememberFileObjects(model);
         _undo.Add(_current);
         _redo.Clear();
         _current = now;
         Trim();
         return true;
      }

      // The state before the last step, or null if there is none. Changes
      // not yet checkpointed become a step first, so they can be redone.
      public CompoundObject Undo(CompoundObject model)
      {
         Checkpoint(model);

         if (_undo.Count == 0)
         {
            return null;
         }

         _redo.Add(_current);
         _current = _undo[_undo.Count - 1];
         _undo.RemoveAt(_undo.Count - 1);

         return Load(_current);
      }

      // The state the last undo went back from, or null if there is none
      // (also when something was changed after the undo).
      public CompoundObject Redo(CompoundObject model)
      {
         Checkpoint(model);

         if (_redo.Count == 0)
         {
            return null;
         }

         _undo.Add(_current);
         _current = _redo[_redo.Count - 1];
         _redo.RemoveAt(_redo.Count - 1);

         return Load(_current);
      }

      // Takes the object as it is now as the current state without making
      // the difference a step: after restoring a state (building the view
      // models may tidy the model), and after saving (which regenerates the
      // polygons' triangles).
      public void Rebaseline(CompoundObject model)
      {
         RememberFileObjects(model);
         _current = Serialize(model);
      }

      // Puts the contents of `from` into `to`: every property that is saved
      // in the object files. `to` keeps its identity, since its parent child
      // object (or the open file) refers to it.
      public static void CopyContents(CompoundObject from, CompoundObject to)
      {
         foreach (PropertyInfo p in typeof(CompoundObject).GetProperties(BindingFlags.Public | BindingFlags.Instance))
         {
            if (!p.CanRead || !p.CanWrite || (p.GetIndexParameters().Length > 0))
            {
               continue;
            }

            if (p.IsDefined(typeof(XmlIgnoreAttribute), true))
            {
               continue;
            }

            p.SetValue(to, p.GetValue(from, null), null);
         }
      }

      private static XmlSerializer Serializer
      {
         get
         {
            if (_serializer == null)
            {
               _serializer = new XmlSerializer(typeof(CompoundObject));
            }

            return _serializer;
         }
      }

      private static string Serialize(CompoundObject model)
      {
         XmlSerializerNamespaces ns = new XmlSerializerNamespaces();
         ns.Add("", "");

         using (StringWriter sw = new StringWriter())
         {
            Serializer.Serialize(sw, model, ns);
            return sw.ToString();
         }
      }

      private CompoundObject Load(string xml)
      {
         CompoundObject co;

         using (StringReader sr = new StringReader(xml))
         {
            co = (CompoundObject)Serializer.Deserialize(sr);
         }

         RestoreFileObjects(co);
         return co;
      }

      private void Trim()
      {
         long chars = 0;
         foreach (string s in _undo)
         {
            chars += s.Length;
         }

         while ((_undo.Count > MaxSteps) || ((_undo.Count > 1) && (chars > MaxChars)))
         {
            chars -= _undo[0].Length;
            _undo.RemoveAt(0);
         }
      }

      // A child object that refers to a file (rather than holding its
      // object in this one)
      private static bool IsFileReference(ChildObject cho)
      {
         return (cho != null) && !string.IsNullOrEmpty(cho.File) && (cho.File != "undef_file.xml");
      }

      private void RememberFileObjects(CompoundObject co)
      {
         ForEachChildObject(co, cho =>
         {
            if (IsFileReference(cho) && (cho.CompObj != null))
            {
               _fileObjects[cho.File] = cho.CompObj;
            }
         });
      }

      private void RestoreFileObjects(CompoundObject co)
      {
         ForEachChildObject(co, cho =>
         {
            if (IsFileReference(cho) && (cho.CompObj == null))
            {
               CompoundObject fileObject;
               if (!_fileObjects.TryGetValue(cho.File, out fileObject))
               {
                  fileObject = ReadFileObject(cho.File);
                  if (fileObject != null)
                  {
                     _fileObjects[cho.File] = fileObject;
                  }
               }

               cho.CompObj = fileObject;
            }
         });
      }

      // As the editor opens object files (MainViewModel.OpenFile)
      private static CompoundObject ReadFileObject(string file)
      {
         try
         {
            string s = GlobalConstants.DataDirectory + file;
            string fullPath = Path.GetDirectoryName(Assembly.GetExecutingAssembly().Location);
            return CompoundObject.ReadFromFile(Path.Combine(fullPath, s));
         }
         catch (Exception)
         {
            return null;
         }
      }

      // Calls `action` for every child object in the object: its child
      // objects and the spawn objects of its breakable object behaviour and
      // object factory systems, and those inside them. The objects of file
      // references are not entered (they belong to their files).
      private static void ForEachChildObject(CompoundObject co, Action<ChildObject> action)
      {
         if (co == null)
         {
            return;
         }

         foreach (ChildObject cho in co.ChildObjects)
         {
            VisitChild(cho, action);
         }

         if ((co.Behaviour != null) && (co.Behaviour.Type == "breakableObject") && (co.Behaviour.BreakableObjProps != null))
         {
            foreach (SpawnObject so in co.Behaviour.BreakableObjProps.SpawnObjects)
            {
               VisitChild(so.MyChildObject, action);
            }
         }

         foreach (CoSystem sys in co.Systems)
         {
            if ((sys.Type == "objectFactory") && (sys.ObjFactStateProperties != null))
            {
               foreach (SpawnObject so in sys.ObjFactStateProperties.SpawnObjects)
               {
                  VisitChild(so.MyChildObject, action);
               }
            }
         }
      }

      private static void VisitChild(ChildObject cho, Action<ChildObject> action)
      {
         if (cho == null)
         {
            return;
         }

         action(cho);

         if (!IsFileReference(cho))
         {
            ForEachChildObject(cho.CompObj, action);
         }
      }
   }
}
