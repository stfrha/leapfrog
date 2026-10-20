using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

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
