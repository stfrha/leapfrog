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
   /// Interaction logic for GunControl.xaml
   /// </summary>
   public partial class GunControl : UserControl
   {
      public GunControl()
      {
         InitializeComponent();
      }

      private void AnchorA_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         if (parentControl != null)
         {
            if (DataContext is GunPropertiesViewModel)
            {
               parentControl.GeneralMouse<GunPropertiesViewModel>(true, MouseEventObjectType.gunSystem, sender, e);
               return;
            }
         }
      }

      private void AnchorA_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         if (parentControl != null)
         {
            if (DataContext is GunPropertiesViewModel)
            {
               parentControl.GeneralMouse<GunPropertiesViewModel>(false, MouseEventObjectType.gunSystem, sender, e);
               return;
            }
         }
      }

      private void AnchorA_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<GunPropertiesViewModel>(MouseEventObjectType.gunSystem, sender, e);
      }
   }
}
