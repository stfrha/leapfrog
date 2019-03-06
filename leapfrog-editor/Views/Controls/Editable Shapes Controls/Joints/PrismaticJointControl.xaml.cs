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
   /// Interaction logic for PrismaticJointControl.xaml
   /// </summary>
   public partial class PrismaticJointControl : UserControl
   {
      public PrismaticJointControl()
      {
         InitializeComponent();
      }

      private void AnchorA_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<WeldJointViewModel>(true, MouseEventObjectType.jointAnchorA, sender, e);
      }

      private void AnchorA_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<WeldJointViewModel>(false, MouseEventObjectType.jointAnchorA, sender, e);
      }

      private void AnchorA_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<WeldJointViewModel>(MouseEventObjectType.jointAnchorA, sender, e);
      }

      private void AnchorB_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<WeldJointViewModel>(true, MouseEventObjectType.jointAnchorB, sender, e);
      }

      private void AnchorB_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<WeldJointViewModel>(false, MouseEventObjectType.jointAnchorB, sender, e);
      }

      private void AnchorB_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<WeldJointViewModel>(MouseEventObjectType.jointAnchorB, sender, e);
      }

      private void UpperLimit_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<PrismaticJointViewModel>(true, MouseEventObjectType.prismJointUpperLimit, sender, e);
      }

      private void UpperLimit_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<PrismaticJointViewModel>(false, MouseEventObjectType.prismJointUpperLimit, sender, e);
      }

      private void UpperLimit_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<PrismaticJointViewModel>(MouseEventObjectType.prismJointUpperLimit, sender, e);
      }

      private void LowerLimit_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<PrismaticJointViewModel>(true, MouseEventObjectType.prismJointLowerLimit, sender, e);
      }

      private void LowerLimit_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouse<PrismaticJointViewModel>(false, MouseEventObjectType.prismJointLowerLimit, sender, e);
      }

      private void LowerLimit_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<PrismaticJointViewModel>(MouseEventObjectType.prismJointLowerLimit, sender, e);
      }


   }
}
