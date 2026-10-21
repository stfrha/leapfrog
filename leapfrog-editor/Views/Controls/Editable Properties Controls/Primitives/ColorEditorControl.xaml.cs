using System;
using System.Globalization;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Controls.Primitives;
using System.Windows.Input;
using System.Windows.Media;

namespace LeapfrogEditor
{
   // A colour field: the colour as text ("rrggbb" or "rrggbbaa", empty for
   // none), as in the files, plus a swatch that opens a picker. The picker
   // has a saturation/brightness square, a hue bar, an alpha slider and
   // preset colours. Changes are written to MyValue as they are made, so the
   // shape shows them straight away.
   //
   // DefaultAlpha is the alpha a colour without one has (255 for Color, 0
   // for Add Color); a colour with that alpha is written as rrggbb.
   // AllowAlpha false (the fade colour) hides the alpha slider and always
   // writes rrggbb.
   public partial class ColorEditorControl : UserControl
   {
      private static readonly string[] PresetColors =
      {
         "ffffff", "d0d0d0", "a0a0a0", "707070", "404040", "202020", "000000",
         "ff0000", "ff8000", "ffff00", "80ff00", "00ff00", "00ff80", "00ffff",
         "0080ff", "0000ff", "8000ff", "ff00ff", "ff0080", "e8dcc0", "c0a080",
         "907050", "604030", "b0c0d0", "6080a0", "304060", "182030", "402818"
      };

      // The colour being edited, as hue (0..360), saturation and
      // brightness (0..1) and alpha (0..255)
      private double _h = 0.0;
      private double _s = 0.0;
      private double _v = 1.0;
      private int _a = 255;

      // Set while the control itself writes MyValue (or sets the alpha
      // slider), so the change isn't read back
      private bool _updating = false;

      public ColorEditorControl()
      {
         InitializeComponent();

         foreach (string hex in PresetColors)
         {
            byte r, g, b, a;
            TextureTint.TryParse(hex, 255, out r, out g, out b, out a);

            Button button = new Button();
            button.Style = (Style)Resources["PresetButton"];
            button.Background = new SolidColorBrush(Color.FromRgb(r, g, b));
            button.Tag = hex;
            button.ToolTip = hex;
            button.Click += Preset_Click;
            presets.Children.Add(button);
         }

         UpdateFromValue();
      }

      #region Properties

      public string Headline
      {
         get { return (string)GetValue(HeadlineProperty); }
         set { SetValue(HeadlineProperty, value); }
      }

      public static readonly DependencyProperty HeadlineProperty =
          DependencyProperty.Register("Headline", typeof(string), typeof(ColorEditorControl), new PropertyMetadata(default(string)));

      public string MyValue
      {
         get { return (string)GetValue(MyValueProperty); }
         set { SetValue(MyValueProperty, value); }
      }

      public static readonly DependencyProperty MyValueProperty =
          DependencyProperty.Register("MyValue", typeof(string), typeof(ColorEditorControl),
             new FrameworkPropertyMetadata(default(string), FrameworkPropertyMetadataOptions.BindsTwoWayByDefault, OnMyValueChanged));

      public int DefaultAlpha
      {
         get { return (int)GetValue(DefaultAlphaProperty); }
         set { SetValue(DefaultAlphaProperty, value); }
      }

      public static readonly DependencyProperty DefaultAlphaProperty =
          DependencyProperty.Register("DefaultAlpha", typeof(int), typeof(ColorEditorControl), new PropertyMetadata(255, OnMyValueChanged));

      public bool AllowAlpha
      {
         get { return (bool)GetValue(AllowAlphaProperty); }
         set { SetValue(AllowAlphaProperty, value); }
      }

      public static readonly DependencyProperty AllowAlphaProperty =
          DependencyProperty.Register("AllowAlpha", typeof(bool), typeof(ColorEditorControl), new PropertyMetadata(true, OnMyValueChanged));

      // For the view: the colour, the pure hue for the square, and whether
      // the "none" slash and the alpha slider show

      public Brush SwatchBrush
      {
         get { return (Brush)GetValue(SwatchBrushProperty); }
         private set { SetValue(SwatchBrushProperty, value); }
      }

      public static readonly DependencyProperty SwatchBrushProperty =
          DependencyProperty.Register("SwatchBrush", typeof(Brush), typeof(ColorEditorControl), new PropertyMetadata(null));

      public Brush HueBrush
      {
         get { return (Brush)GetValue(HueBrushProperty); }
         private set { SetValue(HueBrushProperty, value); }
      }

      public static readonly DependencyProperty HueBrushProperty =
          DependencyProperty.Register("HueBrush", typeof(Brush), typeof(ColorEditorControl), new PropertyMetadata(Brushes.Red));

      public Visibility NoneVisibility
      {
         get { return (Visibility)GetValue(NoneVisibilityProperty); }
         private set { SetValue(NoneVisibilityProperty, value); }
      }

      public static readonly DependencyProperty NoneVisibilityProperty =
          DependencyProperty.Register("NoneVisibility", typeof(Visibility), typeof(ColorEditorControl), new PropertyMetadata(Visibility.Visible));

      public Visibility AlphaVisibility
      {
         get { return (Visibility)GetValue(AlphaVisibilityProperty); }
         private set { SetValue(AlphaVisibilityProperty, value); }
      }

      public static readonly DependencyProperty AlphaVisibilityProperty =
          DependencyProperty.Register("AlphaVisibility", typeof(Visibility), typeof(ColorEditorControl), new PropertyMetadata(Visibility.Visible));

      #endregion

      #region Reading the value

      private static void OnMyValueChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
      {
         ColorEditorControl c = d as ColorEditorControl;
         if ((c != null) && !c._updating)
         {
            c.UpdateFromValue();
         }
      }

      // Takes the colour from MyValue; with no (or an unreadable) colour the
      // picker starts from white (black for a colour whose default alpha is
      // 0, i.e. Add Color)
      private void UpdateFromValue()
      {
         int defaultAlpha = AllowAlpha ? Math.Max(0, Math.Min(255, DefaultAlpha)) : 255;
         byte r, g, b, a;
         bool has = TextureTint.TryParse(MyValue, (byte)defaultAlpha, out r, out g, out b, out a);

         if (!has)
         {
            byte start = (byte)((defaultAlpha == 0) ? 0 : 255);
            r = g = b = start;
            a = (byte)defaultAlpha;
         }

         if (!AllowAlpha)
         {
            a = 255;
         }

         // Keep the hue of greys and black as it was, so dragging down to
         // black and back doesn't lose it
         double h, s, v;
         RgbToHsv(r, g, b, out h, out s, out v);
         if ((s > 0.0) && (v > 0.0))
         {
            _h = h;
         }
         if (v > 0.0)
         {
            _s = s;
         }
         _v = v;
         _a = a;

         _updating = true;
         try
         {
            alphaSlider.Value = _a;
         }
         finally
         {
            _updating = false;
         }

         NoneVisibility = has ? Visibility.Collapsed : Visibility.Visible;
         AlphaVisibility = AllowAlpha ? Visibility.Visible : Visibility.Collapsed;
         UpdateVisuals(has);
      }

      #endregion

      #region Writing the value

      private void WriteValue()
      {
         byte r, g, b;
         HsvToRgb(_h, _s, _v, out r, out g, out b);

         string hex = string.Format(CultureInfo.InvariantCulture, "{0:x2}{1:x2}{2:x2}", r, g, b);
         if (AllowAlpha && (_a != DefaultAlpha))
         {
            hex += string.Format(CultureInfo.InvariantCulture, "{0:x2}", _a);
         }

         _updating = true;
         try
         {
            MyValue = hex;
         }
         finally
         {
            _updating = false;
         }

         // The property may have changed what was written (a colour that
         // changes nothing is stored as none)
         byte pr, pg, pb, pa;
         NoneVisibility = TextureTint.TryParse(MyValue, 255, out pr, out pg, out pb, out pa) ? Visibility.Collapsed : Visibility.Visible;
         UpdateVisuals(true);
      }

      private void UpdateVisuals(bool hasColor)
      {
         byte r, g, b;
         HsvToRgb(_h, _s, _v, out r, out g, out b);

         SolidColorBrush swatch = new SolidColorBrush(hasColor ?
            Color.FromArgb((byte)(AllowAlpha ? _a : 255), r, g, b) :
            Colors.Transparent);
         swatch.Freeze();
         SwatchBrush = swatch;

         byte hr, hg, hb;
         HsvToRgb(_h, 1.0, 1.0, out hr, out hg, out hb);
         SolidColorBrush hue = new SolidColorBrush(Color.FromRgb(hr, hg, hb));
         hue.Freeze();
         HueBrush = hue;

         Canvas.SetLeft(svMarker, _s * svArea.Width - svMarker.Width / 2);
         Canvas.SetTop(svMarker, (1.0 - _v) * svArea.Height - svMarker.Height / 2);
         svMarker.Stroke = (_v > 0.6) && (_s < 0.4) ? Brushes.Black : Brushes.White;

         Canvas.SetTop(hueMarker, (_h / 360.0) * hueArea.Height - hueMarker.Height / 2);
      }

      #endregion

      #region Picker events

      private void Popup_Opened(object sender, EventArgs e)
      {
         UpdateFromValue();
      }

      private void SvArea_MouseDown(object sender, MouseButtonEventArgs e)
      {
         svArea.CaptureMouse();
         SetSvFromMouse(e.GetPosition(svArea));
         e.Handled = true;
      }

      private void SvArea_MouseMove(object sender, MouseEventArgs e)
      {
         if (svArea.IsMouseCaptured)
         {
            SetSvFromMouse(e.GetPosition(svArea));
         }
      }

      private void HueArea_MouseDown(object sender, MouseButtonEventArgs e)
      {
         hueArea.CaptureMouse();
         SetHueFromMouse(e.GetPosition(hueArea));
         e.Handled = true;
      }

      private void HueArea_MouseMove(object sender, MouseEventArgs e)
      {
         if (hueArea.IsMouseCaptured)
         {
            SetHueFromMouse(e.GetPosition(hueArea));
         }
      }

      private void Area_MouseUp(object sender, MouseButtonEventArgs e)
      {
         UIElement area = sender as UIElement;
         if ((area != null) && area.IsMouseCaptured)
         {
            area.ReleaseMouseCapture();
         }
      }

      private void SetSvFromMouse(Point p)
      {
         _s = Clamp01(p.X / svArea.Width);
         _v = Clamp01(1.0 - p.Y / svArea.Height);
         WriteValue();
      }

      private void SetHueFromMouse(Point p)
      {
         _h = Clamp01(p.Y / hueArea.Height) * 360.0;
         if (_h >= 360.0)
         {
            _h = 0.0;
         }

         // Picking a hue for a grey or black colour should show it
         if (_s == 0.0)
         {
            _s = 1.0;
         }
         if (_v == 0.0)
         {
            _v = 1.0;
         }

         WriteValue();
      }

      private void AlphaSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
      {
         if (_updating || !IsLoaded)
         {
            return;
         }

         _a = (int)Math.Round(alphaSlider.Value);
         WriteValue();
      }

      private void Preset_Click(object sender, RoutedEventArgs e)
      {
         string hex = (string)((Button)sender).Tag;
         byte r, g, b, a;
         TextureTint.TryParse(hex, 255, out r, out g, out b, out a);

         double h, s, v;
         RgbToHsv(r, g, b, out h, out s, out v);
         if (s > 0.0)
         {
            _h = h;
         }
         _s = s;
         _v = v;

         WriteValue();
      }

      private void None_Click(object sender, RoutedEventArgs e)
      {
         MyValue = "";
         swatchButton.IsChecked = false;
      }

      private void Close_Click(object sender, RoutedEventArgs e)
      {
         swatchButton.IsChecked = false;
      }

      #endregion

      #region Colour conversion

      private static double Clamp01(double x)
      {
         return Math.Max(0.0, Math.Min(1.0, x));
      }

      private static void RgbToHsv(byte r, byte g, byte b, out double h, out double s, out double v)
      {
         double rf = r / 255.0, gf = g / 255.0, bf = b / 255.0;
         double max = Math.Max(rf, Math.Max(gf, bf));
         double min = Math.Min(rf, Math.Min(gf, bf));
         double d = max - min;

         v = max;
         s = (max > 0.0) ? d / max : 0.0;
         h = 0.0;

         if (d > 0.0)
         {
            if (max == rf)
            {
               h = 60.0 * (((gf - bf) / d) % 6.0);
            }
            else if (max == gf)
            {
               h = 60.0 * (((bf - rf) / d) + 2.0);
            }
            else
            {
               h = 60.0 * (((rf - gf) / d) + 4.0);
            }

            if (h < 0.0)
            {
               h += 360.0;
            }
         }
      }

      private static void HsvToRgb(double h, double s, double v, out byte r, out byte g, out byte b)
      {
         double c = v * s;
         double hp = (h % 360.0) / 60.0;
         double x = c * (1.0 - Math.Abs((hp % 2.0) - 1.0));
         double rf = 0.0, gf = 0.0, bf = 0.0;

         if (hp < 1.0) { rf = c; gf = x; }
         else if (hp < 2.0) { rf = x; gf = c; }
         else if (hp < 3.0) { gf = c; bf = x; }
         else if (hp < 4.0) { gf = x; bf = c; }
         else if (hp < 5.0) { rf = x; bf = c; }
         else { rf = c; bf = x; }

         double m = v - c;
         r = (byte)Math.Round((rf + m) * 255.0, MidpointRounding.AwayFromZero);
         g = (byte)Math.Round((gf + m) * 255.0, MidpointRounding.AwayFromZero);
         b = (byte)Math.Round((bf + m) * 255.0, MidpointRounding.AwayFromZero);
      }

      #endregion
   }
}
