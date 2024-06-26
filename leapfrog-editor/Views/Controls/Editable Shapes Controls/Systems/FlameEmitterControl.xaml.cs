﻿using System;
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
   /// Interaction logic for FlameEmitterControl.xaml
   /// </summary>
   public partial class FlameEmitterControl : UserControl
   {
      public FlameEmitterControl()
      {
         InitializeComponent();
      }

      private void AnchorA_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         if (parentControl != null)
         {
            if (DataContext is FlameEmitterPropertiesViewModel)
            {
               parentControl.GeneralMouse<FlameEmitterPropertiesViewModel>(true, MouseEventObjectType.flameEmitterSystem, sender, e);
               return;
            }
         }
      }

      private void AnchorA_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         if (parentControl != null)
         {
            if (DataContext is FlameEmitterPropertiesViewModel)
            {
               parentControl.GeneralMouse<FlameEmitterPropertiesViewModel>(false, MouseEventObjectType.flameEmitterSystem, sender, e);
               return;
            }
         }
      }

      private void AnchorA_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<FlameEmitterPropertiesViewModel>(MouseEventObjectType.flameEmitterSystem, sender, e);
      }
   }
}
