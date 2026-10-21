using System.ComponentModel;

namespace LeapfrogEditor
{
   // The edit area's zoom (the ZoomAndPanControl's ContentScale: screen
   // pixels per scene unit), for things that are drawn in scene units but
   // should look the same at every zoom: selection outlines, the rubber band
   // and the distance the mouse must move before a drag starts.
   //
   // MainWindow keeps Scale up to date. XAML binds to it with
   // {Binding SelectionLine, Source={x:Static local:EditorZoom.Instance}},
   // which works anywhere, also in controls that can't find the
   // ZoomAndPanControl among their ancestors.
   public class EditorZoom : INotifyPropertyChanged
   {
      // Width of the outline around selected shapes and child objects
      public const double SelectionLinePixels = 3.0;

      // How far (in screen pixels) the mouse must move with the button down
      // before a click becomes a drag
      public const double DragThresholdPixels = 3.0;

      private static readonly EditorZoom _instance = new EditorZoom();
      private double _scale = 1.0;

      public static EditorZoom Instance
      {
         get { return _instance; }
      }

      public event PropertyChangedEventHandler PropertyChanged;

      public double Scale
      {
         get { return _scale; }
         set
         {
            if ((value <= 0) || double.IsNaN(value) || double.IsInfinity(value) || (value == _scale))
            {
               return;
            }

            _scale = value;
            Changed("Scale");
            Changed("SelectionLine");
            Changed("HairLine");
         }
      }

      // Outline of selected objects, SelectionLinePixels wide on screen
      public double SelectionLine
      {
         get { return SelectionLinePixels / _scale; }
      }

      // One screen pixel
      public double HairLine
      {
         get { return 1.0 / _scale; }
      }

      // A distance in screen pixels, in scene units
      public double ToScene(double pixels)
      {
         return pixels / _scale;
      }

      private void Changed(string name)
      {
         PropertyChangedEventHandler h = PropertyChanged;
         if (h != null)
         {
            h(this, new PropertyChangedEventArgs(name));
         }
      }
   }
}
