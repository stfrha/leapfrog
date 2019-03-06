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
   /// Interaction logic for BodyRefEditorControl.xaml
   /// </summary>
   public partial class BodyRefEditorControl : UserControl
   {
      public BodyRefEditorControl()
      {
         InitializeComponent();
      }

      public string Headline
      {
         get { return (string)GetValue(HeadlineProperty); }
         set { SetValue(HeadlineProperty, value); }
      }

      // Using a DependencyProperty as the backing store for Headline.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty HeadlineProperty =
          DependencyProperty.Register("Headline", typeof(string), typeof(BodyRefEditorControl), new PropertyMetadata(default(string)));

      public string MyValue
      {
         get { return (string)GetValue(MyValueProperty); }
         set { SetValue(MyValueProperty, value); }
      }

      // Using a DependencyProperty as the backing store for MyValue.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty MyValueProperty =
          DependencyProperty.Register("MyValue", typeof(string), typeof(BodyRefEditorControl), new PropertyMetadata(default(string)));

   }
}