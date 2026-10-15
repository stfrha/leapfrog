using System;
using System.Collections.Generic;
using System.IO;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace LeapfrogEditor
{
   // Loads texture images for display.
   //
   // WPF's own image loading (from a path or Uri) caches images by path for
   // the lifetime of the application, so a PNG that is re-rendered while the
   // editor is open never shows up. Here each image is cached together with
   // the file's last write time and reloaded when the file has changed. The
   // file is read completely and closed (BitmapCacheOption.OnLoad), so the
   // editor never keeps a PNG locked and Blender can overwrite it.
   public static class TextureImages
   {
      private class Entry
      {
         public DateTime WriteTime;
         public BitmapImage Image;
      }

      private static Dictionary<string, Entry> _cache = new Dictionary<string, Entry>(StringComparer.OrdinalIgnoreCase);

      public static string FullImageDirectory
      {
         get { return FullPath(GlobalConstants.ImageDirectory); }
      }

      public static string FullEditorImageDirectory
      {
         get { return FullPath(GlobalConstants.EditorImageDirectory); }
      }

      // All folders textures are read from: the game's images first, then
      // the editor-only images.
      public static string[] TextureDirectories
      {
         get { return new string[] { FullImageDirectory, FullEditorImageDirectory }; }
      }

      private static string FullPath(string relative)
      {
         string exeDir = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
         return Path.GetFullPath(Path.Combine(exeDir, relative));
      }

      // Path of a texture's PNG: data\images if it's there, otherwise
      // editor_images. (If it exists in neither, the data\images path.)
      public static string FullFileName(string texture)
      {
         foreach (string dir in TextureDirectories)
         {
            string f = Path.Combine(dir, texture + ".png");
            if (File.Exists(f))
            {
               return f;
            }
         }
         return Path.Combine(FullImageDirectory, texture + ".png");
      }

      // Returns the image for a texture name, or null if it doesn't exist or
      // can't be read (e.g. while it is being written).
      public static ImageSource Get(string texture)
      {
         if (string.IsNullOrEmpty(texture))
         {
            return null;
         }

         string fileName = FullFileName(texture);

         if (!File.Exists(fileName))
         {
            _cache.Remove(fileName);
            return null;
         }

         DateTime writeTime = File.GetLastWriteTimeUtc(fileName);

         Entry e;
         if (_cache.TryGetValue(fileName, out e) && (e.WriteTime == writeTime))
         {
            return e.Image;
         }

         try
         {
            BitmapImage bi = new BitmapImage();
            bi.BeginInit();
            bi.CacheOption = BitmapCacheOption.OnLoad;
            bi.CreateOptions = BitmapCreateOptions.IgnoreImageCache;
            bi.UriSource = new Uri(fileName);
            bi.EndInit();
            bi.Freeze();

            _cache[fileName] = new Entry { WriteTime = writeTime, Image = bi };
            return bi;
         }
         catch (Exception)
         {
            // Probably being written right now; the file watcher will
            // trigger a new attempt when the write is done.
            return (e != null) ? e.Image : null;
         }
      }
   }
}
