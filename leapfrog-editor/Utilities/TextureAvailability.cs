using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text.RegularExpressions;
using System.Xml.Linq;

namespace LeapfrogEditor
{
   // Which textures a scene or object file can use in the game.
   //
   // The game looks up a scene object's texture in the scene's resource set
   // first, then in res.xml (GraphicResources::getResAnim), and throws when
   // neither has it. A scene's set comes from data\scene_resources.xml;
   // scenes not listed there have res.xml only. So for a file the editor
   // works out which resource files will be loaded when it is shown:
   //
   //  1. A scene listed in scene_resources.xml: res.xml + its set.
   //  2. A file whose top <compoundObject> has resources="..." (e.g.
   //     resources="res_mining.xml"): res.xml + those. For objects that
   //     Lua adds to a scene (c_addPositionedChildObject,
   //     c_addMissionStateSceneObjects), which the editor can't trace.
   //  3. A file other data files include (childObject file=, spawn=,
   //     dropFile=, surfaceFile=, spaceSceneFile=...): what every one of
   //     them has (strict: the textures common to all, so it loads in every
   //     scene it is used in).
   //  4. Anything else: res.xml only (safe in every scene). This includes
   //     the Leapfrog files, which are used in every scene.
   //
   // The data files are re-read when any of them has changed.
   public class TextureContext
   {
      // The edited file's name (e.g. "home_scene.xml")
      public string FileName = "";

      // Resource files that will be loaded, res.xml first
      public List<string> ResourceFiles = new List<string>();

      // Where that comes from, e.g. "from scene_resources.xml"
      public string Reason = "";

      // "res.xml + res_mining.xml"
      public string ResourceText
      {
         get { return string.Join(" + ", ResourceFiles); }
      }

      // "res.xml + res_mining.xml (from scene_resources.xml)"
      public string Description
      {
         get { return ResourceText + " (" + Reason + ")"; }
      }

      public bool IsAvailable(string texture)
      {
         return WhyUnavailable(texture) == null;
      }

      // null if the game can load the texture here, else a short reason
      public string WhyUnavailable(string texture)
      {
         if (TextureAvailability.IsNoTexture(texture))
         {
            return null;
         }

         List<string> inFiles = TextureDensity.ResourceFilesOf(texture);
         foreach (string f in inFiles)
         {
            if (ResourceFiles.Contains(f, StringComparer.OrdinalIgnoreCase))
            {
               return null;
            }
         }

         if (inFiles.Count > 0)
         {
            return "only in " + string.Join(", ", inFiles);
         }

         if (TextureImages.IsEditorOnly(texture))
         {
            return "editor only (not in the game)";
         }

         return "not in any resource file";
      }
   }

   // A texture a file uses that its context can't load
   public class TextureProblem
   {
      public string Texture;
      public string File;     // the file it is used in (the checked file or one it includes)
      public string Why;
   }

   public static class TextureAvailability
   {
      public const string MainResourceFile = "res.xml";
      public const string SceneResourceMapFile = "scene_resources.xml";

      private class DataFile
      {
         public string Name;                                  // file name, e.g. "launch_site.xml"
         public string Declared;                              // resources attribute, null if none
         public List<string> Includes = new List<string>();   // data files it includes
         public List<string> Textures = new List<string>();   // texture attribute values (no "none" values)
      }

      private static Dictionary<string, DataFile> _files = null;
      private static Dictionary<string, List<string>> _sceneMap = null;
      private static Dictionary<string, List<string>> _includedBy = null;
      private static string _stamp = null;

      // Values that mean "no texture" in the data files
      public static bool IsNoTexture(string texture)
      {
         if (string.IsNullOrEmpty(texture))
         {
            return true;
         }

         return string.Equals(texture, "notDefined", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(texture, "noTexture", StringComparison.OrdinalIgnoreCase) ||
            string.Equals(texture, "none", StringComparison.OrdinalIgnoreCase);
      }

      // Resource files named in a resources attribute (space or comma
      // separated); res.xml is always loaded, so it's left out.
      private static List<string> ParseResourceList(string s)
      {
         List<string> l = new List<string>();
         if (s == null)
         {
            return l;
         }

         foreach (string part in s.Split(new char[] { ' ', ',', ';', '\t' }, StringSplitOptions.RemoveEmptyEntries))
         {
            if (!string.Equals(part, MainResourceFile, StringComparison.OrdinalIgnoreCase) &&
               !l.Contains(part, StringComparer.OrdinalIgnoreCase))
            {
               l.Add(part);
            }
         }
         return l;
      }

      private static XDocument LoadXml(string path)
      {
         try
         {
            using (FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
            {
               return XDocument.Load(fs);
            }
         }
         catch (Exception)
         {
            // Being written or broken
            return null;
         }
      }

      private static void Refresh()
      {
         string dir = TextureDensity.FullDataDirectory;
         string[] paths = Directory.Exists(dir) ? Directory.GetFiles(dir, "*.xml") : new string[0];
         Array.Sort(paths, StringComparer.OrdinalIgnoreCase);

         string stamp = string.Join("|", paths.Select(p => p + "@" + File.GetLastWriteTimeUtc(p).Ticks));
         if ((_files != null) && (stamp == _stamp))
         {
            return;
         }

         // scene_resources.xml
         Dictionary<string, List<string>> sceneMap = new Dictionary<string, List<string>>(StringComparer.OrdinalIgnoreCase);
         XDocument mapDoc = LoadXml(Path.Combine(dir, SceneResourceMapFile));
         if (mapDoc != null)
         {
            foreach (XElement sc in mapDoc.Descendants("scene"))
            {
               string file = (string)sc.Attribute("file");
               if (!string.IsNullOrEmpty(file))
               {
                  sceneMap[file] = ParseResourceList((string)sc.Attribute("resources"));
               }
            }
         }

         // Every compound object file: what it declares, includes and uses
         Dictionary<string, DataFile> files = new Dictionary<string, DataFile>(StringComparer.OrdinalIgnoreCase);
         Dictionary<string, XElement> roots = new Dictionary<string, XElement>(StringComparer.OrdinalIgnoreCase);
         foreach (string p in paths)
         {
            XDocument doc = LoadXml(p);
            if ((doc == null) || (doc.Root == null) || (doc.Root.Name.LocalName != "compoundObject"))
            {
               continue;
            }

            string name = Path.GetFileName(p);
            XAttribute decl = doc.Root.Attribute("resources");
            files[name] = new DataFile { Name = name, Declared = (decl != null) ? decl.Value : null };
            roots[name] = doc.Root;
         }

         foreach (KeyValuePair<string, XElement> kv in roots)
         {
            DataFile df = files[kv.Key];
            foreach (XElement el in kv.Value.DescendantsAndSelf())
            {
               foreach (XAttribute a in el.Attributes())
               {
                  if (a.Name.LocalName == "texture")
                  {
                     if (!IsNoTexture(a.Value) && !df.Textures.Contains(a.Value, StringComparer.OrdinalIgnoreCase))
                     {
                        df.Textures.Add(a.Value);
                     }
                     continue;
                  }

                  foreach (string inc in ReferencedFiles(a.Value, files.Keys))
                  {
                     if (!string.Equals(inc, df.Name, StringComparison.OrdinalIgnoreCase) &&
                        !df.Includes.Contains(inc, StringComparer.OrdinalIgnoreCase))
                     {
                        df.Includes.Add(inc);
                     }
                  }
               }
            }
         }

         Dictionary<string, List<string>> includedBy = new Dictionary<string, List<string>>(StringComparer.OrdinalIgnoreCase);
         foreach (DataFile df in files.Values.OrderBy(f => f.Name, StringComparer.OrdinalIgnoreCase))
         {
            foreach (string inc in df.Includes)
            {
               List<string> l;
               if (!includedBy.TryGetValue(inc, out l))
               {
                  l = new List<string>();
                  includedBy[inc] = l;
               }
               l.Add(df.Name);
            }
         }

         _files = files;
         _sceneMap = sceneMap;
         _includedBy = includedBy;
         _stamp = stamp;
      }

      // The data files an attribute value names: tokens ending in .xml
      // (spawn="a.xml b.xml" names several); "{...}" is a wildcard, as in
      // the stripper's dropFile="stripper_drop_{loot}.xml".
      private static IEnumerable<string> ReferencedFiles(string value, IEnumerable<string> known)
      {
         List<string> result = new List<string>();
         if (string.IsNullOrEmpty(value) || (value.IndexOf(".xml", StringComparison.OrdinalIgnoreCase) < 0))
         {
            return result;
         }

         foreach (string tok in value.Split(new char[] { ' ', ',', ';', '\t' }, StringSplitOptions.RemoveEmptyEntries))
         {
            if (!tok.EndsWith(".xml", StringComparison.OrdinalIgnoreCase))
            {
               continue;
            }

            // File name part (not Path.GetFileName: it throws on characters
            // that can't be in a path, and this is any attribute's text)
            string t = tok.Substring(tok.LastIndexOfAny(new char[] { '/', '\\' }) + 1);
            if (t.Contains("{"))
            {
               Regex re = new Regex("^" + Regex.Replace(Regex.Escape(t), @"\\\{[^}]*}", ".*") + "$", RegexOptions.IgnoreCase);
               result.AddRange(known.Where(k => re.IsMatch(k)));
            }
            else if (known.Contains(t, StringComparer.OrdinalIgnoreCase))
            {
               result.Add(known.First(k => string.Equals(k, t, StringComparison.OrdinalIgnoreCase)));
            }
         }
         return result;
      }

      private static TextureContext Make(string fileName, IEnumerable<string> extra, string reason)
      {
         TextureContext c = new TextureContext { FileName = fileName, Reason = reason };
         c.ResourceFiles.Add(MainResourceFile);
         foreach (string r in extra.OrderBy(r => r, StringComparer.OrdinalIgnoreCase))
         {
            if (!c.ResourceFiles.Contains(r, StringComparer.OrdinalIgnoreCase))
            {
               c.ResourceFiles.Add(r);
            }
         }
         return c;
      }

      // The context of a data file. declared: the file's resources attribute
      // as the editor has it now (it may not be saved yet); null = none.
      public static TextureContext ForFile(string fileName, string declared)
      {
         Refresh();
         return Compute(fileName ?? "", declared, true, new HashSet<string>(StringComparer.OrdinalIgnoreCase));
      }

      // (Not cached: there are a few dozen files and includes are shallow.)
      private static TextureContext Compute(string fileName, string declared, bool useDeclaredArg, HashSet<string> visiting)
      {
         TextureContext result;
         List<string> mapped;
         DataFile df;
         _files.TryGetValue(fileName, out df);
         string decl = useDeclaredArg ? declared : ((df != null) ? df.Declared : null);

         if (_sceneMap.TryGetValue(fileName, out mapped))
         {
            result = Make(fileName, mapped, "from " + SceneResourceMapFile);
         }
         else if (!string.IsNullOrWhiteSpace(decl))
         {
            result = Make(fileName, ParseResourceList(decl), "the file's Resources");
         }
         else
         {
            List<string> parents;
            List<string> used = new List<string>();
            HashSet<string> common = null;

            if (_includedBy.TryGetValue(fileName, out parents))
            {
               visiting.Add(fileName);
               foreach (string p in parents)
               {
                  if (visiting.Contains(p))
                  {
                     continue;
                  }

                  TextureContext pc = Compute(p, null, false, visiting);
                  HashSet<string> extra = new HashSet<string>(pc.ResourceFiles.Skip(1), StringComparer.OrdinalIgnoreCase);
                  if (common == null)
                  {
                     common = extra;
                  }
                  else
                  {
                     common.IntersectWith(extra);
                  }
                  used.Add(p);
               }
               visiting.Remove(fileName);
            }

            if (used.Count > 0)
            {
               result = Make(fileName, common, (used.Count == 1 ? "included by " : "common to ") + string.Join(", ", used));
            }
            else
            {
               result = Make(fileName, new string[0],
                  "not included by any scene file; set Resources if Lua adds it to a scene");
            }
         }

         return result;
      }

      // Textures the file and the files it includes use that the context
      // can't load (each texture once per file)
      public static List<TextureProblem> Check(TextureContext context)
      {
         Refresh();
         List<TextureProblem> problems = new List<TextureProblem>();
         HashSet<string> done = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
         Stack<string> todo = new Stack<string>();
         todo.Push(context.FileName);

         while (todo.Count > 0)
         {
            string f = todo.Pop();
            DataFile df;
            if (!done.Add(f) || !_files.TryGetValue(f, out df))
            {
               continue;
            }

            foreach (string t in df.Textures)
            {
               string why = context.WhyUnavailable(t);
               if (why != null)
               {
                  problems.Add(new TextureProblem { Texture = t, File = f, Why = why });
               }
            }

            foreach (string inc in df.Includes)
            {
               todo.Push(inc);
            }
         }

         return problems
            .OrderBy(p => string.Equals(p.File, context.FileName, StringComparison.OrdinalIgnoreCase) ? 0 : 1)
            .ThenBy(p => p.File, StringComparer.OrdinalIgnoreCase)
            .ThenBy(p => p.Texture, StringComparer.OrdinalIgnoreCase)
            .ToList();
      }
   }
}
