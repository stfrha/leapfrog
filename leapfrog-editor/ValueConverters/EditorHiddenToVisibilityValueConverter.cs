using System;
using System.Globalization;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   // Shapes with "Hide in Editor" ticked (LfShapeViewModel.EditorHidden)
   // are collapsed on the edit area, so they are neither drawn nor
   // clicked. Anything else (joints, systems) stays visible.
   class EditorHiddenToVisibilityValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
      {
         if ((value is bool) && (bool)value)
         {
            return Visibility.Collapsed;
         }

         return Visibility.Visible;
      }

      public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }
}
