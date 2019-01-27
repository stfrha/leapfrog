using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace LeapfrogEditor
{
   /// <summary>
   /// Interaction logic for ScalableTexturePolygonControl.xaml
   /// </summary>
   public partial class ScalableTexturePolygonControl : UserControl
   {
      public ScalableTexturePolygonControl()
      {
         InitializeComponent();
      }

      private void Shape_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<LfShapeViewModel>(true, MouseEventObjectType.shape, sender, e);
      }

      private void Shape_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<LfShapeViewModel>(false, MouseEventObjectType.shape, sender, e);
      }

      private void PolygonBorder_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<LfDragablePointViewModel>(true, MouseEventObjectType.dragablePolygonBorder, sender, e);
      }

      private void PolygonBorder_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<LfDragablePointViewModel>(false, MouseEventObjectType.dragablePolygonBorder, sender, e);

      }

      private void PolygonBorder_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<IPositionInterface>(MouseEventObjectType.dragablePolygonBorder, sender, e);
      }

      private void DragablePoint_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<LfDragablePointViewModel>(true, MouseEventObjectType.dragablePoint, sender, e);
      }

      private void DragablePoint_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<LfDragablePointViewModel>(false, MouseEventObjectType.dragablePoint, sender, e);
      }

      private void DragablePoint_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<IPositionInterface>(MouseEventObjectType.dragablePoint, sender, e);

      }

      private void LineCursorKeyDown(object sender, KeyEventArgs e)
      {
         if (sender is FrameworkElement)
         {
            FrameworkElement el = (FrameworkElement)sender;

            if (Keyboard.IsKeyDown(Key.LeftCtrl) || Keyboard.IsKeyDown(Key.RightCtrl))
               el.Cursor = Cursors.Pen;

            e.Handled = true;
         }
      }

      private void LineCursorKeyUp(object sender, KeyEventArgs e)
      {
         if (sender is FrameworkElement)
         {
            FrameworkElement el = (FrameworkElement)sender;

            el.Cursor = null;

            e.Handled = true;
         }
      }
   }
}
