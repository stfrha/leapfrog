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
   /// Interaction logic for ShieldControl.xaml
   /// </summary>
   public partial class ShieldControl : UserControl
   {
      public ShieldControl()
      {
         InitializeComponent();
      }

      private void AnchorA_MouseDown(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         if (parentControl != null)
         {
            if (DataContext is ShieldPropertiesViewModel)
            {
               parentControl.GeneralMouse<ShieldPropertiesViewModel>(true, MouseEventObjectType.shieldSystem, sender, e);
               return;
            }
         }
      }

      private void AnchorA_MouseUp(object sender, MouseButtonEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         if (parentControl != null)
         {
            if (DataContext is ShieldPropertiesViewModel)
            {
               parentControl.GeneralMouse<ShieldPropertiesViewModel>(false, MouseEventObjectType.shieldSystem, sender, e);
               return;
            }
         }
      }

      private void AnchorA_MouseMove(object sender, MouseEventArgs e)
      {
         CompoundObjectUserControl parentControl = ParentalFinder.FindParent<CompoundObjectUserControl>(this);
         parentControl.GeneralMouseMove<ShieldPropertiesViewModel>(MouseEventObjectType.shieldSystem, sender, e);
      }
   }
}
