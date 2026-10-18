using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace LeapfrogEditor
{
   // A Canvas whose coordinate origin (0, 0) for its children is at Origin,
   // given in the canvas's own coordinates. By default (Origin not set) the
   // origin is the middle of the canvas. The editor moves the origin so that
   // a scene, which extends from (0, 0) to the right and down, fits on the
   // edit area.
   class CenteredCanvas : Canvas
   {
      public static readonly DependencyProperty OriginProperty = DependencyProperty.Register(
         "Origin", typeof(Point), typeof(CenteredCanvas),
         new FrameworkPropertyMetadata(new Point(double.NaN, double.NaN), FrameworkPropertyMetadataOptions.AffectsArrange));

      public Point Origin
      {
         get { return (Point)GetValue(OriginProperty); }
         set { SetValue(OriginProperty, value); }
      }

      // Where (0, 0) of the children is, in canvas coordinates
      public Point EffectiveOrigin
      {
         get
         {
            Point o = Origin;
            if (double.IsNaN(o.X) || double.IsNaN(o.Y))
            {
               double w = double.IsNaN(Width) ? ActualWidth : Width;
               double h = double.IsNaN(Height) ? ActualHeight : Height;
               return new Point(w / 2, h / 2);
            }
            return o;
         }
      }

      //public CenteredCanvas()
      //{
      //   LayoutTransform = new ScaleTransform() { ScaleX = 1, ScaleY = -1 };
      //}
      protected override Size ArrangeOverride(Size arrangeSize)
      {
         Point middle = Origin;
         if (double.IsNaN(middle.X) || double.IsNaN(middle.Y))
         {
            middle = new Point(arrangeSize.Width / 2, arrangeSize.Height / 2);
         }

         foreach (UIElement element in base.InternalChildren)
         {
            if (element == null)
            {
               continue;
            }
            double x = 0.0;
            double y = 0.0;
            double left = GetLeft(element);
            if (!double.IsNaN(left))
            {
               x = left;
            }

            double top = GetTop(element);
            if (!double.IsNaN(top))
            {
               y = top;
            }

            element.Arrange(new Rect(new Point(middle.X + x, middle.Y + y), element.DesiredSize));
         }
         return arrangeSize;
      }
   }
}
