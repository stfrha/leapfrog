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
   /// Interaction logic for TexturePickerEditorControl.xaml
   /// </summary>
   public partial class TexturePickerEditorControl : UserControl
   {
      public TexturePickerEditorControl()
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
          DependencyProperty.Register("Headline", typeof(string), typeof(TexturePickerEditorControl), new PropertyMetadata(default(string)));



      public string MyValue
      {
         get { return (string)GetValue(MyValuesProperty); }
         set { SetValue(MyValuesProperty, value); }
      }

      // Using a DependencyProperty as the backing store for MyValue.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty MyValuesProperty =
          DependencyProperty.Register("MyValue", typeof(string), typeof(TexturePickerEditorControl), new PropertyMetadata(default(string), OnValueOrNoneChanged));

      // The value that means "no texture" where a texture can be left out,
      // e.g. "notDefined" for polygon borders. When set, the picker has a
      // "No texture" button, and the field shows "(no texture)" for it.
      // Not set (the default): a texture must be chosen, as before.
      public string NoneValue
      {
         get { return (string)GetValue(NoneValueProperty); }
         set { SetValue(NoneValueProperty, value); }
      }

      public static readonly DependencyProperty NoneValueProperty =
          DependencyProperty.Register("NoneValue", typeof(string), typeof(TexturePickerEditorControl), new PropertyMetadata(null, OnValueOrNoneChanged));

      // What the field shows
      public string DisplayText
      {
         get { return (string)GetValue(DisplayTextProperty); }
         private set { SetValue(DisplayTextProperty, value); }
      }

      public static readonly DependencyProperty DisplayTextProperty =
          DependencyProperty.Register("DisplayText", typeof(string), typeof(TexturePickerEditorControl), new PropertyMetadata(default(string)));

      public Visibility NoneVisibility
      {
         get { return (Visibility)GetValue(NoneVisibilityProperty); }
         private set { SetValue(NoneVisibilityProperty, value); }
      }

      public static readonly DependencyProperty NoneVisibilityProperty =
          DependencyProperty.Register("NoneVisibility", typeof(Visibility), typeof(TexturePickerEditorControl), new PropertyMetadata(Visibility.Collapsed));

      private static void OnValueOrNoneChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
      {
         TexturePickerEditorControl c = d as TexturePickerEditorControl;
         if (c == null)
         {
            return;
         }

         bool canBeNone = !string.IsNullOrEmpty(c.NoneValue);
         bool isNone = canBeNone && (string.IsNullOrEmpty(c.MyValue) || (c.MyValue == c.NoneValue));

         c.NoneVisibility = canBeNone ? Visibility.Visible : Visibility.Collapsed;
         c.DisplayText = isNone ? "(no texture)" : c.MyValue;
      }

      private void NoTexture_Click(object sender, RoutedEventArgs e)
      {
         if (!string.IsNullOrEmpty(NoneValue))
         {
            MyValue = NoneValue;
         }

         button.IsChecked = false;
      }

      public CollisionEntity SelectedCollEnt
      {
         get { return (CollisionEntity)GetValue(SelectedCollEntProperty); }
         set { SetValue(SelectedCollEntProperty, value); }
      }

      // Using a DependencyProperty as the backing store for SelectedCollEnt.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty SelectedCollEntProperty =
          DependencyProperty.Register("SelectedCollEnt", typeof(CollisionEntity), typeof(TexturePickerEditorControl), new PropertyMetadata(default(CollisionEntity)));

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
