using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

namespace LeapfrogEditor
{
   // Where the parallax view's window is, and the scene it looks at. One
   // immutable value, replaced whenever something changes (see
   // MainViewModel.ParallaxState).
   public class ParallaxViewState
   {
      public ParallaxViewState(bool enabled, Rect window, double stageWidth, double stageHeight)
      {
         Enabled = enabled;
         Window = window;
         StageWidth = stageWidth;
         StageHeight = stageHeight;
      }

      public bool Enabled { get; private set; }
      public Rect Window { get; private set; }
      public double StageWidth { get; private set; }
      public double StageHeight { get; private set; }

      // How the game places a parallax layer (SceneActor::doUpdate), for
      // the window at its current position: a point q on the layer (stage
      // coordinates) is drawn at offset + scale * q.
      //   view size V (stage units) = window size
      //   scale k = (V + (S - V)(1 - a)) / S per axis, and with
      //             parallaxKeepAspect (the default) the larger of the two,
      //             the bottom edge kept and the extra width split left/right
      //   offset  = window top left * a
      public void LayerTransform(double amount, out double scale, out double offsetX, out double offsetY)
      {
         double vx = Window.Width;
         double vy = Window.Height;
         double sx = Math.Max(StageWidth, 1e-6);
         double sy = Math.Max(StageHeight, 1e-6);

         double kx = (vx + (sx - vx) * (1.0 - amount)) / sx;
         double ky = (vy + (sy - vy) * (1.0 - amount)) / sy;

         offsetX = Window.X * amount;
         offsetY = Window.Y * amount;

         double k = Math.Max(kx, ky);
         offsetX += (kx - k) * sx * 0.5;
         offsetY += (ky - k) * sy;
         scale = k;
      }
   }

   // RenderTransform for a shape's ContentPresenter in the parallax view.
   // The presenter sits at the shape's AbsPosX/AbsPosY; the transform moves
   // and scales what it draws to where the game shows it. Shapes that
   // aren't on a parallax layer (or with the view off) get no transform.
   //
   // Values: [0] the shape view model, [1] AbsPosX, [2] AbsPosY,
   // [3] MainViewModel.ParallaxState, [4] ParallaxAmount (only there to
   // update when it changes; read from the view model).
   class ParallaxTransformConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, CultureInfo culture)
      {
         if ((values == null) || (values.Length < 4))
         {
            return Transform.Identity;
         }

         ParallaxViewState state = values[3] as ParallaxViewState;
         if ((state == null) || !state.Enabled)
         {
            return Transform.Identity;
         }

         double amount = ParallaxAmountOf(values[0]);
         if (amount <= 0.0)
         {
            return Transform.Identity;
         }

         LfShapeViewModel shape = values[0] as LfShapeViewModel;
         CompoundObjectViewModel scene = shape.ParentVm;

         // Position on the layer = position in the scene
         double qx = shape.AbsPosX - scene.AbsPosX;
         double qy = shape.AbsPosY - scene.AbsPosY;

         double k, ox, oy;
         state.LayerTransform(amount, out k, out ox, out oy);

         // A point d from the presenter's origin is at q + d in the scene
         // and must be drawn at offset + k (q + d), i.e. moved by
         // offset + (k - 1) q + (k - 1) d from where it is.
         return new MatrixTransform(new Matrix(k, 0, 0, k, ox + (k - 1.0) * qx, oy + (k - 1.0) * qy));
      }

      // The game puts sprite boxes and sprite polygons with parallaxAmount
      // > 0 on a parallax layer when they are directly in a scene.
      public static double ParallaxAmountOf(object o)
      {
         LfShapeViewModel shape = o as LfShapeViewModel;
         if ((shape == null) || (shape.ParentVm == null) || (shape.ParentVm.Behaviour == null) ||
            (shape.ParentVm.Behaviour.Type != "scene"))
         {
            return 0.0;
         }

         if (o is LfSpriteBoxViewModel)
         {
            return ((LfSpriteBoxViewModel)o).ParallaxAmount;
         }

         if (o is LfSpritePolygonViewModel)
         {
            return ((LfSpritePolygonViewModel)o).ParallaxAmount;
         }

         return 0.0;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, CultureInfo culture)
      {
         return null;
      }
   }

   // A length that is the given number of screen pixels whatever the zoom
   // (value: the ZoomAndPanControl's ContentScale, parameter: pixels).
   class ScreenPixelsValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
      {
         double pixels = 1.0;
         if (parameter != null)
         {
            double.TryParse(parameter.ToString(), NumberStyles.Float, CultureInfo.InvariantCulture, out pixels);
         }

         if ((value is double) && ((double)value > 0))
         {
            return pixels / (double)value;
         }

         return pixels;
      }

      public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }
}
