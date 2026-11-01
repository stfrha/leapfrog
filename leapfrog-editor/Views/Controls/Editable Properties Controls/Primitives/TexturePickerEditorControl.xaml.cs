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
   // One texture in the picker
   public class TextureChoice
   {
      public string Name { get; set; }
      public bool Available { get; set; }   // the game can load it where the edited file is used
      public string ToolTip { get; set; }
   }

   /// <summary>
   /// Interaction logic for TexturePickerEditorControl.xaml
   /// </summary>
   public partial class TexturePickerEditorControl : UserControl
   {
      public TexturePickerEditorControl()
      {
         InitializeComponent();
         Loaded += (s, e) => UpdateValueState();
      }

      // Whether the picker lists the textures that can't be used here
      // (greyed out). Shared by all pickers while the editor runs.
      private static bool _showUnavailable = true;

      // True while the list is (re)built, so its selection changes don't
      // set MyValue
      private bool _building = false;

      // The edited file's resource files (from MainViewModel), null if
      // there is no edited object
      private TextureContext CurrentContext()
      {
         MainViewModel mvm = MainVm();
         return (mvm != null) ? mvm.CurrentTextureContext() : null;
      }

      private MainViewModel MainVm()
      {
         Window w = Window.GetWindow(this);
         if ((w == null) && (Application.Current != null))
         {
            w = Application.Current.MainWindow;
         }
         return (w != null) ? w.DataContext as MainViewModel : null;
      }

      // "Can use: res.xml + res_mining.xml (from scene_resources.xml)"
      public string ContextText
      {
         get { return (string)GetValue(ContextTextProperty); }
         private set { SetValue(ContextTextProperty, value); }
      }

      public static readonly DependencyProperty ContextTextProperty =
          DependencyProperty.Register("ContextText", typeof(string), typeof(TexturePickerEditorControl), new PropertyMetadata(default(string)));

      // The chosen texture can't be loaded where the edited file is used
      public bool ValueUnavailable
      {
         get { return (bool)GetValue(ValueUnavailableProperty); }
         private set { SetValue(ValueUnavailableProperty, value); }
      }

      public static readonly DependencyProperty ValueUnavailableProperty =
          DependencyProperty.Register("ValueUnavailable", typeof(bool), typeof(TexturePickerEditorControl), new PropertyMetadata(false));

      // Why, for the field's tooltip (null when it's fine)
      public string ValueToolTip
      {
         get { return (string)GetValue(ValueToolTipProperty); }
         private set { SetValue(ValueToolTipProperty, value); }
      }

      public static readonly DependencyProperty ValueToolTipProperty =
          DependencyProperty.Register("ValueToolTip", typeof(string), typeof(TexturePickerEditorControl), new PropertyMetadata(default(string)));

      private bool ValueIsNone()
      {
         return string.IsNullOrEmpty(MyValue) ||
            (!string.IsNullOrEmpty(NoneValue) && (MyValue == NoneValue)) ||
            TextureAvailability.IsNoTexture(MyValue);
      }

      private void UpdateValueState()
      {
         if (System.ComponentModel.DesignerProperties.GetIsInDesignMode(this))
         {
            return;
         }

         TextureContext ctx = ValueIsNone() ? null : CurrentContext();
         string why = (ctx != null) ? ctx.WhyUnavailable(MyValue) : null;

         ValueUnavailable = (why != null);
         ValueToolTip = (why != null) ?
            "The game can't load " + MyValue + " here: " + why + ".\n" + ctx.FileName + " has " + ctx.Description + "." :
            null;
      }

      // Fills the list: the textures the edited file can use first, then
      // (if shown) the others greyed out, each with a tooltip saying why.
      private void BuildList()
      {
         MainViewModel mvm = MainVm();
         if (mvm == null)
         {
            return;
         }

         _building = true;
         try
         {
            TextureContext ctx = mvm.CurrentTextureContext();
            showUnavailable.IsChecked = _showUnavailable;

            List<TextureChoice> available = new List<TextureChoice>();
            List<TextureChoice> unavailable = new List<TextureChoice>();
            foreach (string name in mvm.Textures)
            {
               string why = (ctx != null) ? ctx.WhyUnavailable(name) : null;
               if (why == null)
               {
                  string inFiles = string.Join(", ", TextureDensity.ResourceFilesOf(name));
                  available.Add(new TextureChoice { Name = name, Available = true, ToolTip = (inFiles != "") ? name + " (" + inFiles + ")" : name });
               }
               else if (_showUnavailable || string.Equals(name, MyValue, StringComparison.OrdinalIgnoreCase))
               {
                  unavailable.Add(new TextureChoice { Name = name, Available = false, ToolTip = name + ": " + why });
               }
            }

            List<TextureChoice> choices = available.Concat(unavailable).ToList();

            ContextText = (ctx != null) ?
               ctx.FileName + " can use textures in " + ctx.Description + "." :
               "";

            list.ItemsSource = choices;
            list.SelectedItem = choices.FirstOrDefault(c => string.Equals(c.Name, MyValue, StringComparison.OrdinalIgnoreCase));
         }
         finally
         {
            _building = false;
         }

         UpdateValueState();
      }

      private void Popup_Opened(object sender, EventArgs e)
      {
         BuildList();
      }

      private void ShowUnavailable_Changed(object sender, RoutedEventArgs e)
      {
         if (_building)
         {
            return;
         }

         _showUnavailable = (showUnavailable.IsChecked == true);
         BuildList();
      }

      private void List_SelectionChanged(object sender, SelectionChangedEventArgs e)
      {
         if (_building)
         {
            return;
         }

         TextureChoice c = list.SelectedItem as TextureChoice;
         if ((c != null) && c.Available)
         {
            MyValue = c.Name;
         }
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
         c.UpdateValueState();
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
