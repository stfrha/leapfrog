using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   class BitValueConverter : IValueConverter
   {
      private int target;


      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is double)
         {
            int val = System.Convert.ToInt32((double)value);
            int param = System.Convert.ToInt32((string)parameter);

            target = val;

            return ((val & 1 << param) != 0);
         }

         return null;
      }

      public object ConvertBack(object value, Type targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         int param = System.Convert.ToInt32((string)parameter);

         if ((bool)value)
         {
            target |= 1 <<  param;
         }
         else
         {
            target &= ~(1 << param);
         }

         return target;
      }
   }
}
