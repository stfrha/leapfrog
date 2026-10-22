using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;
using System.Windows.Media.Imaging;

namespace LeapfrogEditor
{
   // The opacity mask that shows a polygon fade (PolygonFade) in the editor,
   // as the game draws it: linear from start to end along the fade's
   // direction, in the polygon's own (unrotated) coordinates.
   //
   // Values: [0] AbsPoints (the polygon's points, local), [1] FadeAmount,
   // [2] FadeAngle, [3] FadeStart, [4] FadeEnd, [5] FadeColor, [6] Angle
   // (the shape's rotation, degrees).
   // Parameter:
   //   "mask"    for the polygon itself: fades its opacity when the fade is
   //             to transparent, otherwise no mask (null)
   //   "overlay" for the haze layer on top: its opacity, 0 to amount, when
   //             the fade is to a colour, otherwise fully transparent
   class FadeMaskConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
      {
         bool overlay = ((parameter as string) == "overlay");
         Brush off = overlay ? Brushes.Transparent : null;

         if ((values == null) || (values.Length < 7))
         {
            return off;
         }

         PointCollection points = values[0] as PointCollection;
         double amount = ToDouble(values[1], 0);
         double angleDeg = ToDouble(values[2], 0);
         double start = ToDouble(values[3], 0);
         double end = ToDouble(values[4], 1);
         string color = values[5] as string;
         double shapeAngle = ToDouble(values[6], 0);

         bool toColor = !string.IsNullOrEmpty(color);

         if ((points == null) || (points.Count == 0) || (amount <= 0) || (toColor != overlay))
         {
            return off;
         }

         amount = Math.Min(amount, 1.0);

         // Direction: 0 = down (y axis down), 90 = right
         double a = angleDeg * Math.PI / 180.0;
         Vector dir = new Vector(Math.Sin(a), Math.Cos(a));

         double pmin = double.MaxValue;
         double pmax = double.MinValue;
         foreach (Point p in points)
         {
            double s = p.X * dir.X + p.Y * dir.Y;
            pmin = Math.Min(pmin, s);
            pmax = Math.Max(pmax, s);
         }

         double s0 = pmin + start * (pmax - pmin);
         double s1 = pmin + end * (pmax - pmin);
         if (Math.Abs(s1 - s0) < 1e-3)
         {
            s1 = s0 + 1e-3;
         }

         // A point at s along dir is dir * s
         LinearGradientBrush brush = new LinearGradientBrush();
         brush.MappingMode = BrushMappingMode.Absolute;
         brush.SpreadMethod = GradientSpreadMethod.Pad;
         brush.StartPoint = new Point(dir.X * s0, dir.Y * s0);
         brush.EndPoint = new Point(dir.X * s1, dir.Y * s1);

         byte full = 255;
         byte faded = (byte)Math.Round(255.0 * (overlay ? amount : 1.0 - amount));
         byte first = overlay ? (byte)0 : full;
         brush.GradientStops.Add(new GradientStop(System.Windows.Media.Color.FromArgb(first, 255, 255, 255), 0.0));
         brush.GradientStops.Add(new GradientStop(System.Windows.Media.Color.FromArgb(faded, 255, 255, 255), 1.0));

         // The polygon and its borders are drawn rotated by the shape's angle
         // about its origin; the fade turns with them
         if (shapeAngle != 0)
         {
            brush.Transform = new RotateTransform(shapeAngle);
         }

         brush.Freeze();
         return brush;
      }

      private static double ToDouble(object o, double fallback)
      {
         if (o is double)
         {
            return (double)o;
         }
         return fallback;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
      {
         return null;
      }
   }

   // The opacity mask that shows a polygon's edge fade (PolygonEdgeFade) in
   // the editor, as the game draws it: opacity from the distance to the
   // nearest fading edge, with the same smooth curve. A small bitmap over
   // the polygon's bounds, in the polygon's own (unrotated) coordinates, so
   // it goes on elements drawn with AbsPoints under the shape's rotation.
   //
   // Values: [0] AbsPoints, [1] EdgeFadeWidth, [2] EdgeFadeAmount,
   // [3] EdgeFadeSides. Returns null (no mask) when there is no edge fade.
   class EdgeFadeMaskConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
      {
         if ((values == null) || (values.Length < 4))
         {
            return null;
         }

         PointCollection points = values[0] as PointCollection;
         double width = (values[1] is double) ? (double)values[1] : 0;
         double amount = (values[2] is double) ? Math.Min((double)values[2], 1.0) : 1;
         string sides = (values[3] as string) ?? "all";

         if ((points == null) || (points.Count < 3) || (width <= 0) || (amount <= 0))
         {
            return null;
         }

         bool all = sides.Contains("all");
         bool top = all || sides.Contains("top");
         bool bottom = all || sides.Contains("bottom");
         bool left = all || sides.Contains("left");
         bool right = all || sides.Contains("right");

         // The fading edges, as the game picks them
         int n = points.Count;
         double doubleArea = 0;
         for (int i = 0; i < n; i++)
         {
            Point a = points[i];
            Point b = points[(i + 1) % n];
            doubleArea += a.X * b.Y - b.X * a.Y;
         }

         var edgeA = new System.Collections.Generic.List<Point>();
         var edgeB = new System.Collections.Generic.List<Point>();
         double minX = double.MaxValue, minY = double.MaxValue, maxX = double.MinValue, maxY = double.MinValue;
         for (int i = 0; i < n; i++)
         {
            Point a = points[i];
            Point b = points[(i + 1) % n];
            minX = Math.Min(minX, a.X); maxX = Math.Max(maxX, a.X);
            minY = Math.Min(minY, a.Y); maxY = Math.Max(maxY, a.Y);

            Vector d = b - a;
            if (d.LengthSquared == 0)
            {
               continue;
            }
            Vector o = (doubleArea > 0) ? new Vector(d.Y, -d.X) : new Vector(-d.Y, d.X);
            bool use;
            if (-o.Y >= Math.Abs(o.X)) use = top;
            else if (o.Y >= Math.Abs(o.X)) use = bottom;
            else if (o.X < 0) use = left;
            else use = right;
            if (use)
            {
               edgeA.Add(a);
               edgeB.Add(b);
            }
         }

         if (edgeA.Count == 0)
         {
            return null;
         }

         // A pixel per quarter of the width, at most about 400 on a side
         double cell = Math.Max(width / 4.0, Math.Max(maxX - minX, maxY - minY) / 400.0);
         int w = Math.Max(2, (int)Math.Ceiling((maxX - minX) / cell) + 1);
         int h = Math.Max(2, (int)Math.Ceiling((maxY - minY) / cell) + 1);
         double x0 = minX - cell * 0.5;
         double y0 = minY - cell * 0.5;

         byte[] pixels = new byte[w * h * 4];
         for (int iy = 0; iy < h; iy++)
         {
            for (int ix = 0; ix < w; ix++)
            {
               Point p = new Point(x0 + (ix + 0.5) * cell, y0 + (iy + 0.5) * cell);
               double dist = double.MaxValue;
               for (int e = 0; e < edgeA.Count; e++)
               {
                  dist = Math.Min(dist, DistanceToSegment(p, edgeA[e], edgeB[e]));
               }
               double t = Math.Min(1.0, dist / width);
               double sm = t * t * (3 - 2 * t);
               byte a = (byte)Math.Round(255.0 * (1.0 - amount * (1.0 - sm)));
               int k = (iy * w + ix) * 4;
               pixels[k] = a; pixels[k + 1] = a; pixels[k + 2] = a; pixels[k + 3] = a;   // premultiplied
            }
         }

         BitmapSource bmp = BitmapSource.Create(w, h, 96, 96, PixelFormats.Pbgra32, null, pixels, w * 4);
         bmp.Freeze();

         ImageBrush brush = new ImageBrush(bmp);
         brush.ViewportUnits = BrushMappingMode.Absolute;
         brush.Viewport = new Rect(x0, y0, w * cell, h * cell);
         brush.Stretch = Stretch.Fill;
         brush.TileMode = TileMode.None;
         RenderOptions.SetBitmapScalingMode(brush, BitmapScalingMode.Linear);
         brush.Freeze();
         return brush;
      }

      private static double DistanceToSegment(Point p, Point a, Point b)
      {
         Vector ab = b - a;
         Vector ap = p - a;
         double len2 = ab.LengthSquared;
         double t = (len2 > 1e-12) ? (ap * ab) / len2 : 0;
         t = Math.Max(0, Math.Min(1, t));
         return (ap - ab * t).Length;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
      {
         return null;
      }
   }

   // Visible when the value is null (or, with parameter "notNull", when it isn't)
   class NullToVisibilityConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
      {
         bool wantNull = ((parameter as string) != "notNull");
         return ((value == null) == wantNull) ? Visibility.Visible : Visibility.Collapsed;
      }

      public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }
}
