using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   class NegDoubleValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is double)
         {
            double bv = (double)value;

            return -bv;
         }

         return null;

      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }

   class HalfTurnDoubleValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is double)
         {
            double bv = (double)value - 180;

            if (bv < -360)
            {
               bv += 360;
            }

            return bv;
         }

         return null;

      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }

   class DirectionAngleValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         // Values should be: Gun angle, body angle, and angular offset
         if (values.Count() == 2)
         {
            double angularOffset = System.Convert.ToDouble(parameter);

            if ((values[0] is double) && (values[1] is double))
            {
               double gunAngle = (double)values[0];
               double bodyAngle = (double)values[1];
               return bodyAngle + gunAngle + angularOffset;
            }
         }

         return null;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }


}
