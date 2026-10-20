using System;
using System.Collections.Generic;
using System.IO;
using System.Text.RegularExpressions;
using System.Xml.Serialization;

namespace LeapfrogEditor
{
   // Copy and paste of shapes, joints and child objects.
   //
   // What is copied is kept as model objects, cloned through the same XML
   // serialisation as the object files, so editing the originals after
   // copying doesn't change what is pasted, and every paste gets objects of
   // its own. The clipboard lives in the editor (not the Windows clipboard),
   // so it works between all files open in the editor.
   public class EditorClipboard
   {
      public List<LfShape> Shapes = new List<LfShape>();
      public List<WeldJoint> Joints = new List<WeldJoint>();
      public List<ChildObject> Children = new List<ChildObject>();

      // Centre of the copied positions (shape and child object positions),
      // in the coordinates of the object they were copied from. Pasting at
      // the mouse moves this point to the mouse.
      public double CenterX;
      public double CenterY;

      public bool IsEmpty
      {
         get { return (Shapes.Count == 0) && (Children.Count == 0); }
      }

      public void Clear()
      {
         Shapes.Clear();
         Joints.Clear();
         Children.Clear();
         CenterX = 0;
         CenterY = 0;
      }

      // A deep copy made by writing the object as XML and reading it back.
      public static T Clone<T>(T obj) where T : class
      {
         if (obj == null)
         {
            return null;
         }

         XmlSerializer ser = new XmlSerializer(obj.GetType());
         using (StringWriter sw = new StringWriter())
         {
            ser.Serialize(sw, obj);
            using (StringReader sr = new StringReader(sw.ToString()))
            {
               return (T)ser.Deserialize(sr);
            }
         }
      }

      // A child object referring to a file is saved as the file name only,
      // so its object doesn't survive the XML copy. Such objects (also deeper
      // down, and in spawn objects) are shared with the original instead;
      // they come from the file and aren't saved with the copy.
      public static ChildObject CloneChild(ChildObject orig)
      {
         ChildObject copy = Clone(orig);
         RestoreFileObjects(orig, copy);
         return copy;
      }

      private static void RestoreFileObjects(ChildObject orig, ChildObject copy)
      {
         if ((orig == null) || (copy == null))
         {
            return;
         }

         if (copy.CompObj == null)
         {
            copy.CompObj = orig.CompObj;
         }
         else
         {
            RestoreFileObjects(orig.CompObj, copy.CompObj);
         }
      }

      private static void RestoreFileObjects(CompoundObject orig, CompoundObject copy)
      {
         if ((orig == null) || (copy == null))
         {
            return;
         }

         int n = Math.Min(orig.ChildObjects.Count, copy.ChildObjects.Count);
         for (int i = 0; i < n; i++)
         {
            RestoreFileObjects(orig.ChildObjects[i], copy.ChildObjects[i]);
         }

         if ((orig.Behaviour != null) && (copy.Behaviour != null) &&
            (orig.Behaviour.Type == "breakableObject") &&
            (orig.Behaviour.BreakableObjProps != null) && (copy.Behaviour.BreakableObjProps != null))
         {
            List<SpawnObject> a = new List<SpawnObject>(orig.Behaviour.BreakableObjProps.SpawnObjects);
            List<SpawnObject> b = new List<SpawnObject>(copy.Behaviour.BreakableObjProps.SpawnObjects);
            for (int i = 0; i < Math.Min(a.Count, b.Count); i++)
            {
               RestoreFileObjects(a[i].MyChildObject, b[i].MyChildObject);
            }
         }

         n = Math.Min(orig.Systems.Count, copy.Systems.Count);
         for (int i = 0; i < n; i++)
         {
            CoSystem sa = orig.Systems[i];
            CoSystem sb = copy.Systems[i];
            if ((sa.Type == "objectFactory") && (sb.Type == "objectFactory") &&
               (sa.ObjFactStateProperties != null) && (sb.ObjFactStateProperties != null))
            {
               List<SpawnObject> a = new List<SpawnObject>(sa.ObjFactStateProperties.SpawnObjects);
               List<SpawnObject> b = new List<SpawnObject>(sb.ObjFactStateProperties.SpawnObjects);
               for (int j = 0; j < Math.Min(a.Count, b.Count); j++)
               {
                  RestoreFileObjects(a[j].MyChildObject, b[j].MyChildObject);
               }
            }
         }
      }

      // A name that isn't in `used`: the name itself if it is free, otherwise
      // with a number added ("rock" -> "rock_2"), or a name that already ends
      // in a number counted up from it ("rock_2" -> "rock_3").
      public static string UniqueName(string name, ICollection<string> used)
      {
         if (string.IsNullOrEmpty(name))
         {
            name = "noName";
         }

         if (!used.Contains(name))
         {
            return name;
         }

         string stem = name;
         long next = 2;
         Match m = Regex.Match(name, @"^(.+)_(\d{1,9})$");
         if (m.Success)
         {
            stem = m.Groups[1].Value;
            next = long.Parse(m.Groups[2].Value) + 1;
         }

         while (true)
         {
            string candidate = stem + "_" + next.ToString();
            if (!used.Contains(candidate))
            {
               return candidate;
            }
            next++;
         }
      }
   }
}
