using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   // Converts a texture name to its image. (Used to return the file path and
   // let WPF load it, but WPF caches by path forever, so re-rendered textures
   // never updated. See TextureImages.)
   class TexturePathValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is string)
         {
            return TextureImages.Get((string)value);
         }

         return null;
      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }
}
