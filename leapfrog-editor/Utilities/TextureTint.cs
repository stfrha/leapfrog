using System;
using System.Collections.Generic;
using System.Globalization;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace LeapfrogEditor
{
   // Shape tint, as the game applies it (see CompoundObject::parseHexColor
   // and doCommonShapeDefinitions in the game):
   //   color     multiplies the texture, "rrggbb" or "rrggbbaa" (default white)
   //   addColor  is added on top, scaled by the texture's alpha, "rrggbb" or
   //             "rrggbbaa" (alpha defaults to 0)
   // Tinted images are made once per texture and colour pair and cached.
   public static class TextureTint
   {
      // Parses "rrggbb" / "rrggbbaa" (optional leading '#'). Returns false for
      // anything else; empty means "not set".
      public static bool TryParse(string s, byte defaultAlpha, out byte r, out byte g, out byte b, out byte a)
      {
         r = g = b = 255;
         a = defaultAlpha;

         if (string.IsNullOrWhiteSpace(s))
         {
            return false;
         }

         s = s.Trim();
         if (s.StartsWith("#"))
         {
            s = s.Substring(1);
         }

         uint v;
         if (((s.Length != 6) && (s.Length != 8)) ||
            !uint.TryParse(s, NumberStyles.HexNumber, CultureInfo.InvariantCulture, out v))
         {
            return false;
         }

         if (s.Length == 6)
         {
            r = (byte)(v >> 16);
            g = (byte)(v >> 8);
            b = (byte)v;
         }
         else
         {
            r = (byte)(v >> 24);
            g = (byte)(v >> 16);
            b = (byte)(v >> 8);
            a = (byte)v;
         }
         return true;
      }

      // The value to store: lower-case hex without '#', or "" when empty,
      // invalid or the same as no tint at all.
      public static string Normalize(string s, bool isAddColor)
      {
         byte r, g, b, a;
         if (!TryParse(s, (byte)(isAddColor ? 0 : 255), out r, out g, out b, out a))
         {
            return "";
         }

         bool neutral = isAddColor ? ((r | g | b | a) == 0) : ((r & g & b & a) == 255);
         if (neutral)
         {
            return "";
         }

         string t = s.Trim().TrimStart('#').ToLowerInvariant();
         return t;
      }

      private class CacheEntry
      {
         public ImageSource Source;
         public ImageSource Tinted;
      }

      private static Dictionary<string, CacheEntry> _cache = new Dictionary<string, CacheEntry>(StringComparer.OrdinalIgnoreCase);

      // The texture's image (first frame for animations) with the tint applied
      public static ImageSource Get(string texture, string color, string addColor)
      {
         ImageSource source = TextureImages.Get(texture);
         if (source == null)
         {
            return null;
         }

         byte cr, cg, cb, ca, ar, ag, ab, aa;
         bool hasColor = TryParse(color, 255, out cr, out cg, out cb, out ca);
         bool hasAdd = TryParse(addColor, 0, out ar, out ag, out ab, out aa);
         if (!hasColor && !hasAdd)
         {
            return source;
         }
         if (!hasAdd)
         {
            ar = ag = ab = aa = 0;
         }

         string key = texture + "|" + color + "|" + addColor;
         CacheEntry e;
         if (_cache.TryGetValue(key, out e) && ReferenceEquals(e.Source, source))
         {
            return e.Tinted;
         }

         BitmapSource bs = source as BitmapSource;
         if (bs == null)
         {
            return source;
         }

         try
         {
            FormatConvertedBitmap pbgra = new FormatConvertedBitmap(bs, PixelFormats.Pbgra32, null, 0);
            int w = pbgra.PixelWidth;
            int h = pbgra.PixelHeight;
            int stride = w * 4;
            byte[] px = new byte[stride * h];
            pbgra.CopyPixels(px, stride, 0);

            // Premultiplied multiply colour, like the game's vertex colours
            double mA = ca / 255.0;
            double mR = cr / 255.0 * mA;
            double mG = cg / 255.0 * mA;
            double mB = cb / 255.0 * mA;

            for (int i = 0; i < px.Length; i += 4)
            {
               double b = px[i] * mB;
               double g = px[i + 1] * mG;
               double r = px[i + 2] * mR;
               double a = px[i + 3] * mA;

               // res += add_color * res.a
               double k = a / 255.0;
               b += ab * k;
               g += ag * k;
               r += ar * k;
               a += aa * k;

               px[i] = (byte)Math.Min(255.0, b + 0.5);
               px[i + 1] = (byte)Math.Min(255.0, g + 0.5);
               px[i + 2] = (byte)Math.Min(255.0, r + 0.5);
               px[i + 3] = (byte)Math.Min(255.0, a + 0.5);
            }

            BitmapSource tinted = BitmapSource.Create(w, h, bs.DpiX, bs.DpiY, PixelFormats.Pbgra32, null, px, stride);
            tinted.Freeze();

            _cache[key] = new CacheEntry { Source = source, Tinted = tinted };
            return tinted;
         }
         catch (Exception)
         {
            return source;
         }
      }
   }
}
