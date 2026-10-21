using System;
using System.Windows.Input;
using System.Windows.Controls;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

namespace LeapfrogEditor
{
   /// <summary>
   /// Interaction logic for App.xaml
   /// </summary>
   public partial class App : Application
   {
      protected override void OnStartup(StartupEventArgs e)
      {
         base.OnStartup(e);

         // A text box reached with Tab (or any other way from the keyboard)
         // gets its whole value selected, so typing replaces it. Clicking
         // into one still puts the cursor where it was clicked.
         EventManager.RegisterClassHandler(typeof(TextBox), UIElement.GotKeyboardFocusEvent,
            new KeyboardFocusChangedEventHandler(TextBox_GotKeyboardFocus));
      }

      private static void TextBox_GotKeyboardFocus(object sender, KeyboardFocusChangedEventArgs e)
      {
         TextBox tb = sender as TextBox;

         if ((tb != null) && (Mouse.LeftButton != MouseButtonState.Pressed))
         {
            tb.SelectAll();
         }
      }
   }
}
