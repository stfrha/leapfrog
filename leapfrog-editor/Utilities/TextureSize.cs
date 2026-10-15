using System;
using System.IO;
using System.Windows;
using System.Windows.Media.Imaging;

namespace LeapfrogEditor
{
   // internal, because IWidthHeightInterface is internal
   internal static class TextureSize
   {
      // Returns the size of a texture in pixels, read from
      // <ImageDirectory>\<texture>.png. Returns false if the file is missing
      // or can't be decoded.
      //
      // PixelWidth/PixelHeight are used rather than Width/Height, because the
      // latter are in device independent units and depend on the DPI stored
      // in the PNG. The file is read through a stream and the WPF image cache
      // is bypassed, so a re-rendered texture is picked up without restarting
      // the editor, and the file is not kept locked.
      public static bool TryGetPixelSize(string texture, out int pixelWidth, out int pixelHeight)
      {
         pixelWidth = 0;
         pixelHeight = 0;

         string fullFileName = TextureImages.FullFileName(texture);

         if (!File.Exists(fullFileName))
         {
            return false;
         }

         try
         {
            using (FileStream fs = new FileStream(fullFileName, FileMode.Open, FileAccess.Read, FileShare.ReadWrite))
            {
               BitmapDecoder decoder = BitmapDecoder.Create(
                  fs,
                  BitmapCreateOptions.IgnoreImageCache | BitmapCreateOptions.DelayCreation,
                  BitmapCacheOption.None);

               BitmapFrame frame = decoder.Frames[0];
               pixelWidth = frame.PixelWidth;
               pixelHeight = frame.PixelHeight;
            }
         }
         catch (Exception)
         {
            return false;
         }

         return (pixelWidth > 0) && (pixelHeight > 0);
      }

      // Sets width and height of a shape from its texture's pixel size,
      // using GlobalConstants.TexturePixelsPerUnit.
      public static void SetSizeFromTexture(IWidthHeightInterface iwh, string texture)
      {
         int pw, ph;

         if (!TryGetPixelSize(texture, out pw, out ph))
         {
            MessageBox.Show(
               "Could not read texture '" + texture + ".png' in " + GlobalConstants.ImageDirectory +
               " or " + GlobalConstants.EditorImageDirectory,
               "Set size from texture",
               MessageBoxButton.OK,
               MessageBoxImage.Warning);
            return;
         }

         iwh.Width = pw / GlobalConstants.TexturePixelsPerUnit;
         iwh.Height = ph / GlobalConstants.TexturePixelsPerUnit;
      }
   }
}
