using System;
using System.Collections;
using System.Collections.Specialized;
using System.ComponentModel;
using System.Globalization;
using System.Windows;
using System.Windows.Input;
using System.Windows.Media;

namespace LeapfrogEditor
{
   // The ruler along the top of the canvas while a planet's surface is
   // edited (MainViewModel.PlanetView): degrees along the surface (x /
   // PlanetSurfaceProperties.Radius), finer ticks as you zoom in, a flag
   // for each landing site (drag it to move the site; Shift snaps to the
   // ruler's small ticks) and where the mouse is. Beyond +-180 degrees the
   // surface wraps round (shaded).
   class PlanetRuler : FrameworkElement
   {
      // The canvas whose (Origin + x) is the surface's x (MainWindow sets it)
      public CenteredCanvas Target { get; set; }

      // The surface's x under the mouse; NaN when not over the canvas
      private double _mouseX = double.NaN;
      public double MouseX
      {
         get { return _mouseX; }
         set
         {
            if (!value.Equals(_mouseX))
            {
               _mouseX = value;
               InvalidateVisual();
            }
         }
      }

      public static readonly DependencyProperty SitesProperty = DependencyProperty.Register(
         "Sites", typeof(IEnumerable), typeof(PlanetRuler),
         new PropertyMetadata(null, OnSitesChanged));

      // LandingSiteViewModels
      public IEnumerable Sites
      {
         get { return (IEnumerable)GetValue(SitesProperty); }
         set { SetValue(SitesProperty, value); }
      }

      private static readonly double[] c_steps = { 0.01, 0.02, 0.05, 0.1, 0.2, 0.5, 1, 2, 5, 10, 15, 30, 45, 90 };
      private const double c_minorPixels = 6.0;
      private const double c_majorPixels = 70.0;
      private const double c_flagGrabPixels = 8.0;

      private static readonly Brush c_background = new SolidColorBrush(Color.FromRgb(236, 236, 236));
      private static readonly Brush c_wrapped = new SolidColorBrush(Color.FromRgb(200, 200, 200));
      private static readonly Pen c_tickPen = new Pen(Brushes.DimGray, 1.0);
      private static readonly Pen c_borderPen = new Pen(Brushes.Gray, 1.0);
      private static readonly Brush c_flagBrush = new SolidColorBrush(Color.FromRgb(216, 90, 48));
      private static readonly Pen c_flagPen = new Pen(new SolidColorBrush(Color.FromRgb(153, 60, 29)), 1.0);
      private static readonly Pen c_mousePen = new Pen(Brushes.RoyalBlue, 1.0);
      private static readonly Typeface c_typeface = new Typeface("Segoe UI");

      private LandingSiteViewModel _dragged = null;
      private double _minorStep = 1.0;

      public PlanetRuler()
      {
         ClipToBounds = true;
         Focusable = false;
      }

      private static void OnSitesChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
      {
         PlanetRuler r = (PlanetRuler)d;

         INotifyCollectionChanged oldColl = e.OldValue as INotifyCollectionChanged;
         if (oldColl != null)
         {
            oldColl.CollectionChanged -= r.Sites_CollectionChanged;
         }
         r.Unwatch(e.OldValue as IEnumerable);

         INotifyCollectionChanged newColl = e.NewValue as INotifyCollectionChanged;
         if (newColl != null)
         {
            newColl.CollectionChanged += r.Sites_CollectionChanged;
         }
         r.Watch(e.NewValue as IEnumerable);

         r.InvalidateVisual();
      }

      private void Sites_CollectionChanged(object sender, NotifyCollectionChangedEventArgs e)
      {
         Unwatch(e.OldItems);
         Watch(e.NewItems);
         InvalidateVisual();
      }

      private void Watch(IEnumerable items)
      {
         if (items == null) return;
         foreach (object o in items)
         {
            INotifyPropertyChanged n = o as INotifyPropertyChanged;
            if (n != null) n.PropertyChanged += Site_PropertyChanged;
         }
      }

      private void Unwatch(IEnumerable items)
      {
         if (items == null) return;
         foreach (object o in items)
         {
            INotifyPropertyChanged n = o as INotifyPropertyChanged;
            if (n != null) n.PropertyChanged -= Site_PropertyChanged;
         }
      }

      private void Site_PropertyChanged(object sender, PropertyChangedEventArgs e)
      {
         InvalidateVisual();
      }

      // Screen x on the ruler of the surface's x 0, and pixels per unit
      private bool GetMapping(out double x0, out double k)
      {
         x0 = 0.0;
         k = 0.0;

         if ((Target == null) || !Target.IsVisible || (PresentationSource.FromVisual(this) == null) ||
            (PresentationSource.FromVisual(Target) == null))
         {
            return false;
         }

         try
         {
            Point o = Target.EffectiveOrigin;
            Point a = Target.TranslatePoint(new Point(o.X, 0.0), this);
            Point b = Target.TranslatePoint(new Point(o.X + 1000.0, 0.0), this);
            x0 = a.X;
            k = (b.X - a.X) / 1000.0;
         }
         catch (InvalidOperationException)
         {
            return false;
         }

         return (k > 0.0) && !double.IsNaN(k) && !double.IsInfinity(k);
      }

      private static string Degrees(double value, double step)
      {
         int decimals = 0;
         if (step < 0.1) decimals = 2;
         else if (step < 1.0) decimals = 1;
         return value.ToString("F" + decimals, CultureInfo.InvariantCulture) + "°";
      }

      private FormattedText Text(string s, double size, Brush brush)
      {
#pragma warning disable 618
         return new FormattedText(s, CultureInfo.InvariantCulture, FlowDirection.LeftToRight, c_typeface, size, brush);
#pragma warning restore 618
      }

      protected override void OnRender(DrawingContext dc)
      {
         double w = ActualWidth;
         double h = ActualHeight;

         dc.DrawRectangle(c_background, null, new Rect(0, 0, w, h));
         dc.DrawLine(c_borderPen, new Point(0, h - 0.5), new Point(w, h - 0.5));

         double x0, k;
         if (!GetMapping(out x0, out k))
         {
            return;
         }

         double unitsPerDegree = PlanetSurfaceProperties.AngleToX(1.0);
         double ppd = k * unitsPerDegree;      // pixels per degree

         // Beyond +-180 the surface wraps round
         double half = Math.PI * PlanetSurfaceProperties.Radius;
         double leftEdge = x0 - half * k;
         double rightEdge = x0 + half * k;
         if (leftEdge > 0) dc.DrawRectangle(c_wrapped, null, new Rect(0, 0, Math.Min(leftEdge, w), h));
         if (rightEdge < w) dc.DrawRectangle(c_wrapped, null, new Rect(Math.Max(rightEdge, 0), 0, w - Math.Max(rightEdge, 0), h));

         // Tick steps: small ones at least c_minorPixels apart, labelled
         // ones at least c_majorPixels apart and a whole number of small ones
         double minor = c_steps[c_steps.Length - 1];
         foreach (double s in c_steps)
         {
            if (s * ppd >= c_minorPixels) { minor = s; break; }
         }
         double major = c_steps[c_steps.Length - 1];
         foreach (double s in c_steps)
         {
            double ratio = s / minor;
            if ((s * ppd >= c_majorPixels) && (Math.Abs(ratio - Math.Round(ratio)) < 1e-6))
            {
               major = s;
               break;
            }
         }
         _minorStep = minor;

         double firstAngle = Math.Max(-180.0, (0.0 - x0) / ppd);
         double lastAngle = Math.Min(180.0, (w - x0) / ppd);
         long first = (long)Math.Floor(firstAngle / minor);
         long last = (long)Math.Ceiling(lastAngle / minor);

         if (last - first < 5000)
         {
            for (long i = first; i <= last; i++)
            {
               double angle = i * minor;
               double sx = x0 + angle * ppd;
               double ratio = angle / major;
               bool isMajor = Math.Abs(ratio - Math.Round(ratio)) < 1e-6;
               double len = isMajor ? 12.0 : 5.0;
               dc.DrawLine(c_tickPen, new Point(sx, h - len), new Point(sx, h));

               if (isMajor)
               {
                  FormattedText t = Text(Degrees(angle, major), 10, Brushes.Black);
                  dc.DrawText(t, new Point(sx + 2, h - 12 - t.Height + 2));
               }
            }
         }

         // The sites' flags
         if (Sites != null)
         {
            foreach (object o in Sites)
            {
               LandingSiteViewModel site = o as LandingSiteViewModel;
               if (site == null) continue;

               double sx = x0 + site.X * k;
               if ((sx < -200) || (sx > w + 200)) continue;

               StreamGeometry flag = new StreamGeometry();
               using (StreamGeometryContext g = flag.Open())
               {
                  g.BeginFigure(new Point(sx, h), true, true);
                  g.LineTo(new Point(sx, 2), true, false);
                  g.LineTo(new Point(sx + 10, 6), true, false);
                  g.LineTo(new Point(sx, 10), true, false);
               }
               flag.Freeze();
               dc.DrawGeometry(c_flagBrush, c_flagPen, flag);

               FormattedText t = Text(site.DisplayLabel + "  " + Degrees(site.Angle, 0.01), 10, c_flagPen.Brush);
               dc.DrawText(t, new Point(sx + 12, 1));
            }
         }

         // The mouse
         if (!double.IsNaN(_mouseX))
         {
            double sx = x0 + _mouseX * k;
            dc.DrawLine(c_mousePen, new Point(sx, 0), new Point(sx, h));
            double angle = _mouseX / unitsPerDegree;
            FormattedText t = Text(Degrees(angle, 0.01) + "  x " + Math.Round(_mouseX).ToString(CultureInfo.InvariantCulture),
               10, Brushes.RoyalBlue);
            double tx = (sx + 4 + t.Width < w) ? sx + 4 : sx - 4 - t.Width;
            dc.DrawText(t, new Point(tx, h - 12 - 2 * t.Height));
         }
      }

      private LandingSiteViewModel FlagAt(Point p)
      {
         double x0, k;
         if ((Sites == null) || !GetMapping(out x0, out k))
         {
            return null;
         }

         LandingSiteViewModel best = null;
         double bestDistance = c_flagGrabPixels;
         foreach (object o in Sites)
         {
            LandingSiteViewModel site = o as LandingSiteViewModel;
            if (site == null) continue;

            double d = Math.Abs(p.X - (x0 + site.X * k));
            if (d <= bestDistance)
            {
               bestDistance = d;
               best = site;
            }
         }
         return best;
      }

      protected override void OnMouseLeftButtonDown(MouseButtonEventArgs e)
      {
         _dragged = FlagAt(e.GetPosition(this));
         if (_dragged != null)
         {
            CaptureMouse();
            e.Handled = true;
         }
         base.OnMouseLeftButtonDown(e);
      }

      protected override void OnMouseMove(MouseEventArgs e)
      {
         Point p = e.GetPosition(this);

         if (_dragged != null)
         {
            double x0, k;
            if (GetMapping(out x0, out k))
            {
               double angle = PlanetSurfaceProperties.XToAngle((p.X - x0) / k);
               angle = Math.Max(-180.0, Math.Min(180.0, angle));
               if (Keyboard.IsKeyDown(Key.LeftShift) || Keyboard.IsKeyDown(Key.RightShift))
               {
                  angle = Math.Round(angle / _minorStep) * _minorStep;
               }
               _dragged.Angle = Math.Round(angle, 2);
            }
            e.Handled = true;
         }
         else
         {
            Cursor = (FlagAt(p) != null) ? Cursors.SizeWE : null;
         }

         base.OnMouseMove(e);
      }

      protected override void OnMouseLeftButtonUp(MouseButtonEventArgs e)
      {
         if (_dragged != null)
         {
            _dragged = null;
            ReleaseMouseCapture();
            e.Handled = true;
         }
         base.OnMouseLeftButtonUp(e);
      }
   }
}
