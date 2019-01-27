using System;
using System.Collections.Generic;
using System.Collections.ObjectModel;
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
   /// Interaction logic for CollisionEntityEditorControl.xaml
   /// </summary>
   public partial class CollisionEntityEditorControl : UserControl
   {
      public CollisionEntityEditorControl()
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
          DependencyProperty.Register("Headline", typeof(string), typeof(CollisionEntityEditorControl), new PropertyMetadata(default(string)));



      public string MyValue
      {
         get { return (string)GetValue(MyValuesProperty); }
         set { SetValue(MyValuesProperty, value); }
      }

      // Using a DependencyProperty as the backing store for MyValue.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty MyValuesProperty =
          DependencyProperty.Register("MyValue", typeof(string), typeof(CollisionEntityEditorControl), new PropertyMetadata(default(string)));

      public CollisionEntity SelectedCollEnt
      {
         get { return (CollisionEntity)GetValue(SelectedCollEntProperty); }
         set { SetValue(SelectedCollEntProperty, value); }
      }

      // Using a DependencyProperty as the backing store for SelectedCollEnt.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty SelectedCollEntProperty =
          DependencyProperty.Register("SelectedCollEnt", typeof(CollisionEntity), typeof(CollisionEntityEditorControl), new PropertyMetadata(default(CollisionEntity)));

      private void HandleSelectionChanged(object sender, SelectionChangedEventArgs e)
      {
         if (sender is DataGrid)
         {
            DataGrid dg = (DataGrid)sender;

            if (dg.SelectedItem is CollisionEntity)
            {
               CollisionEntity ce = (CollisionEntity)dg.SelectedItem;

               MyValue = ce.Name;
            }
         }
      }
   }
}
