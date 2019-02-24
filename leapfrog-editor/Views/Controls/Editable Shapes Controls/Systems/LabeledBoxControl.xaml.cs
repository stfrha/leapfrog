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
   /// Interaction logic for LabeledBoxControl.xaml
   /// </summary>
   public partial class LabeledBoxControl : UserControl
   {
      public LabeledBoxControl()
      {
         InitializeComponent();
      }

      private void Shape_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         if (parentControl != null)
         {
            if (DataContext is ObjectFactoryPropertiesViewModel)
            {
               parentControl.GeneralMouse<ObjectFactoryPropertiesViewModel>(true, MouseEventObjectType.objectFactory, sender, e);
               return;
            }
         }
      }

      private void Shape_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         if (parentControl != null)
         {
            if (DataContext is ObjectFactoryPropertiesViewModel)
            {
               parentControl.GeneralMouse<ObjectFactoryPropertiesViewModel>(false, MouseEventObjectType.objectFactory, sender, e);
               return;
            }
         }
      }

      private void ShapeBorder_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<LfPointViewModel>(true, MouseEventObjectType.rotatableObjectDragableBorder, sender, e);
      }

      private void ShapeBorder_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<LfPointViewModel>(false, MouseEventObjectType.rotatableObjectDragableBorder, sender, e);
      }

      private void ShapeBorder_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<IPositionInterface>(MouseEventObjectType.rotatableObjectDragableBorder, sender, e);
      }
   }
}
