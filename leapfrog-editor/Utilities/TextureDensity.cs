using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Xml.Linq;

namespace LeapfrogEditor
{
   // Per-texture information from the game's resource files
   // (data\res.xml and data\res_*.xml):
   //
   //    <image file="amb_hut.png" ppu="10" rows="2" .../>
   //
   //  * ppu: pixels per game unit the image was rendered at. The Blender
   //    sprite kit writes it. Images without it use
   //    GlobalConstants.TexturePixelsPerUnit (20).
   //  * rows / cols: the image is an animation strip; one frame is the
   //    image size divided by these.
   //
   // oxygine ignores attributes it doesn't know, so ppu is only read here.
   // The files are re-read when any of them has changed.
   public static class TextureDensity
   {
      private class Info
      {
         public double Ppu;
         public int Rows;
         public int Cols;
      }

      private static Dictionary<string, Info> _infos = null;
      private static string _stamp = null;

      public static string FullDataDirectory
      {
         get
         {
            string exeDir = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
            return Path.GetFullPath(Path.Combine(exeDir, GlobalConstants.DataDirectory));
         }
      }

      // res.xml first, then res_*.xml by name (the same order as the
      // sprite kit's project_files.py)
      private static List<string> ResourceFiles()
      {
         List<string> files = new List<string>();
         string dir = FullDataDirectory;
         if (!Directory.Exists(dir))
         {
            return files;
         }

         files.AddRange(Directory.GetFiles(dir, "res*.xml").OrderBy(f => Path.GetFileName(f), StringComparer.OrdinalIgnoreCase));
         string main = files.FirstOrDefault(f => string.Equals(Path.GetFileName(f), "res.xml", StringComparison.OrdinalIgnoreCase));
         if (main != null)
         {
            files.Remove(main);
            files.Insert(0, main);
         }
         return files;
      }

      private static void Refresh()
      {
         List<string> files = ResourceFiles();
         string stamp = string.Join("|", files.Select(f => f + "@" + File.GetLastWriteTimeUtc(f).Ticks));
         if ((_infos != null) && (stamp == _stamp))
         {
            return;
         }

         Dictionary<string, Info> infos = new Dictionary<string, Info>(StringComparer.OrdinalIgnoreCase);
         foreach (string f in files)
         {
            XDocument doc;
            try
            {
               using (FileStream fs = new FileStream(f, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
               {
                  doc = XDocument.Load(fs);
               }
            }
            catch (Exception)
            {
               // Being written or broken; try again next time
               continue;
            }

            foreach (XElement im in doc.Descendants("image"))
            {
               string file = (string)im.Attribute("file");
               if (string.IsNullOrEmpty(file))
               {
                  continue;
               }

               string name = Path.GetFileNameWithoutExtension(file);
               if (infos.ContainsKey(name))
               {
                  continue;
               }

               infos[name] = new Info
               {
                  Ppu = ParseDouble((string)im.Attribute("ppu"), GlobalConstants.TexturePixelsPerUnit),
                  Rows = ParseInt((string)im.Attribute("rows"), 1),
                  Cols = ParseInt((string)im.Attribute("cols"), 1)
               };
            }
         }

         _infos = infos;
         _stamp = stamp;
      }

      private static double ParseDouble(string s, double def)
      {
         double d;
         if (!string.IsNullOrEmpty(s) && double.TryParse(s, NumberStyles.Float, CultureInfo.InvariantCulture, out d) && (d > 0.0))
         {
            return d;
         }
         return def;
      }

      private static int ParseInt(string s, int def)
      {
         int i;
         if (!string.IsNullOrEmpty(s) && int.TryParse(s, NumberStyles.Integer, CultureInfo.InvariantCulture, out i) && (i > 0))
         {
            return i;
         }
         return def;
      }

      private static Info Lookup(string texture)
      {
         Refresh();
         Info info;
         if (!string.IsNullOrEmpty(texture) && _infos.TryGetValue(texture, out info))
         {
            return info;
         }
         return null;
      }

      // Pixels per game unit for a texture (20 unless its entry says otherwise)
      public static double PixelsPerUnit(string texture)
      {
         Info info = Lookup(texture);
         return (info != null) ? info.Ppu : GlobalConstants.TexturePixelsPerUnit;
      }

      // Animation frames across and down (1, 1 for a still image)
      public static void Frames(string texture, out int cols, out int rows)
      {
         Info info = Lookup(texture);
         cols = (info != null) ? info.Cols : 1;
         rows = (info != null) ? info.Rows : 1;
      }
   }
}
