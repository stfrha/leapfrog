using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;
using System.Windows.Shapes;

namespace LeapfrogEditor
{
   class TriangleShape : Shape
   {
      public double Size
      {
         get { return (double)GetValue(SizeProperty); }
         set { SetValue(SizeProperty, value); }
      }

      // Using a DependencyProperty as the backing store for Size.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty SizeProperty =
          DependencyProperty.Register("Size", typeof(double), typeof(TriangleShape), new PropertyMetadata(default(double)));

      public TriangleShape()
      {
      }

      protected override Geometry DefiningGeometry
      {
         get
         {

            Point p1 = new Point(0.0d, this.Size);
            Point p2 = new Point(this.Size, this.Size);
            Point p3 = new Point(this.Size / 2, 0.0d);
           
            List<PathSegment> segments = new List<PathSegment>(3);
            segments.Add(new LineSegment(p1, true));
            segments.Add(new LineSegment(p2, true));
            segments.Add(new LineSegment(p3, true));

            List<PathFigure> figures = new List<PathFigure>(1);
            PathFigure pf = new PathFigure(p1, segments, true);
            figures.Add(pf);

            Geometry g = new PathGeometry(figures, FillRule.EvenOdd, null);

            return g;
         }
      }
   }
}
