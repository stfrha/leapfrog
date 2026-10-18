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
   //
   // For an animation strip (rows/cols in its resource entry, see
   // TextureDensity) only the first frame is returned, so the editor shows
   // and fits bodies to one frame, as the game shows it.
   public static class TextureImages
   {
      private class Entry
      {
         public DateTime WriteTime;
         public BitmapImage Image;
         public int Cols = 1;
         public int Rows = 1;
         public ImageSource Frame;      // first frame (Image itself for a still image)
      }

      // The first frame of an image laid out as cols x rows frames
      private static ImageSource FirstFrame(BitmapImage image, int cols, int rows)
      {
         if ((cols <= 1) && (rows <= 1))
         {
            return image;
         }

         int w = image.PixelWidth / Math.Max(1, cols);
         int h = image.PixelHeight / Math.Max(1, rows);
         if ((w <= 0) || (h <= 0))
         {
            return image;
         }

         CroppedBitmap cb = new CroppedBitmap(image, new System.Windows.Int32Rect(0, 0, w, h));
         cb.Freeze();
         return cb;
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

      // Index of texture name -> file, over data\images and editor_images
      // including subfolders (images are organised in folders like
      // images\mining; the texture name is the file name without folder, as
      // in the game). data\images wins if a name exists in both.
      private static Dictionary<string, string> _index = null;
      private static List<string> _duplicates = new List<string>();

      public static void RebuildIndex()
      {
         Dictionary<string, string> idx = new Dictionary<string, string>(StringComparer.OrdinalIgnoreCase);
         List<string> dups = new List<string>();

         foreach (string dir in TextureDirectories)
         {
            if (!Directory.Exists(dir))
            {
               continue;
            }

            foreach (string file in Directory.GetFiles(dir, "*.png", SearchOption.AllDirectories))
            {
               string name = Path.GetFileNameWithoutExtension(file);
               string existing;
               if (idx.TryGetValue(name, out existing))
               {
                  // Same name in data\images and editor_images is fine only
                  // when one of them is editor-only; within one root it's
                  // ambiguous for the game.
                  bool sameRoot = existing.StartsWith(dir, StringComparison.OrdinalIgnoreCase);
                  if (sameRoot)
                  {
                     dups.Add(name + ":\n   " + existing + "\n   " + file);
                  }
               }
               else
               {
                  idx[name] = file;
               }
            }
         }

         _index = idx;
         _duplicates = dups;
      }

      // All texture names (builds the index if needed)
      public static ICollection<string> TextureNames
      {
         get
         {
            if (_index == null)
            {
               RebuildIndex();
            }
            return _index.Keys;
         }
      }

      // Texture names that exist in more than one subfolder of the same root
      public static List<string> Duplicates
      {
         get { return _duplicates; }
      }

      // Path of a texture's PNG, wherever it is. (If it can't be found,
      // the path it would have directly in data\images.)
      public static string FullFileName(string texture)
      {
         if (_index == null)
         {
            RebuildIndex();
         }

         string f;
         if (_index.TryGetValue(texture, out f) && File.Exists(f))
         {
            return f;
         }

         // Moved or new since the last scan
         RebuildIndex();
         if (_index.TryGetValue(texture, out f))
         {
            return f;
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

         int cols, rows;
         TextureDensity.Frames(texture, out cols, out rows);

         Entry e;
         if (_cache.TryGetValue(fileName, out e) && (e.WriteTime == writeTime))
         {
            if ((e.Cols != cols) || (e.Rows != rows) || (e.Frame == null))
            {
               // The resource entry's rows/cols changed
               e.Cols = cols;
               e.Rows = rows;
               e.Frame = FirstFrame(e.Image, cols, rows);
            }
            return e.Frame;
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

            ImageSource frame = FirstFrame(bi, cols, rows);
            _cache[fileName] = new Entry { WriteTime = writeTime, Image = bi, Cols = cols, Rows = rows, Frame = frame };
            return frame;
         }
         catch (Exception)
         {
            // Probably being written right now; the file watcher will
            // trigger a new attempt when the write is done.
            return (e != null) ? (e.Frame ?? e.Image) : null;
         }
      }
   }
}
