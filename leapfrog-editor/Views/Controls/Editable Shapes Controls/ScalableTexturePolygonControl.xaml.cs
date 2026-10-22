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

      // Dragging a shape by its inside moves everything that is selected
      private void Shape_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<LfShapeViewModel>(MouseEventObjectType.shape, sender, e);
      }

      // Drag Textures mode: the wheel over a selected polygon scales its
      // texture about the point under the mouse instead of zooming the view
      // (ctrl + wheel still rotates the selection). The position is taken in
      // the polygon's own coordinates (its rotation undone), which is where
      // the ImageBrush viewport lives.
      private void Shape_MouseWheel(object sender, MouseWheelEventArgs e)
      {
         if ((Keyboard.Modifiers & ModifierKeys.Control) != 0)
         {
            return;
         }

         FrameworkElement fwe = sender as FrameworkElement;
         LfScalableTexturePolygonViewModel poly = (fwe != null) ? fwe.DataContext as LfScalableTexturePolygonViewModel : null;
         if ((poly == null) || (poly.MainVm == null))
         {
            return;
         }

         if (poly.MainVm.ScaleTexture(poly, e.GetPosition(fwe), e.Delta))
         {
            e.Handled = true;
         }
      }


   }
}
