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
   /// Interaction logic for ReentryFlameEmitterControl.xaml
   /// </summary>
   public partial class ReentryFlameEmitterControl : UserControl
   {
      public ReentryFlameEmitterControl()
      {
         InitializeComponent();
      }

      private void AnchorA_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<ReentryFlameEmitterPropertiesViewModel>(true, MouseEventObjectType.systemAnchorA, sender, e);
      }

      private void AnchorA_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<ReentryFlameEmitterPropertiesViewModel>(false, MouseEventObjectType.systemAnchorA, sender, e);
      }

      private void AnchorA_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<ReentryFlameEmitterPropertiesViewModel>(MouseEventObjectType.systemAnchorA, sender, e);
      }

      private void AnchorB_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<ReentryFlameEmitterPropertiesViewModel>(true, MouseEventObjectType.systemAnchorB, sender, e);
      }

      private void AnchorB_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<ReentryFlameEmitterPropertiesViewModel>(false, MouseEventObjectType.systemAnchorB, sender, e);
      }

      private void AnchorB_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<ReentryFlameEmitterPropertiesViewModel>(MouseEventObjectType.systemAnchorB, sender, e);
      }
   }

}
