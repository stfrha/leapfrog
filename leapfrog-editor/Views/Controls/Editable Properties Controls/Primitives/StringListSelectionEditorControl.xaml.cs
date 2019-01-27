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
   /// Interaction logic for StringListSelectionEditorControl.xaml
   /// </summary>
   public partial class StringListSelectionEditorControl : UserControl
   {
      public StringListSelectionEditorControl()
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
          DependencyProperty.Register("Headline", typeof(string), typeof(StringListSelectionEditorControl), new PropertyMetadata(default(string)));


      public int SelectionIndex
      {
         get { return (int)GetValue(SelectionIndexProperty); }
         set { SetValue(SelectionIndexProperty, value); }
      }

      // Using a DependencyProperty as the backing store for SelectionIndex.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty SelectionIndexProperty =
          DependencyProperty.Register("SelectionIndex", typeof(int), typeof(StringListSelectionEditorControl), new PropertyMetadata(default(int)));


      public List<string> Strings
      {
         get { return (List<string>)GetValue(StringsProperty); }
         set { SetValue(StringsProperty, value); }
      }

      // Using a DependencyProperty as the backing store for Strings.  This enables animation, styling, binding, etc...
      public static readonly DependencyProperty StringsProperty =
          DependencyProperty.Register("Strings", typeof(List<string>), typeof(StringListSelectionEditorControl), new PropertyMetadata(default(List<string>)));



   }
}
