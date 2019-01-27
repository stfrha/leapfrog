using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   class NegativeHalfValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is double)
         {
            double val = (double)value;

            return -val / 2;
         }

         return null;

      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }

   class NegativeValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is double)
         {
            double val = (double)value;

            return -val;
         }

         return null;

      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }



   class TextureOffsetXValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is IWidthHeightInterface)
         {
            IWidthHeightInterface ao = (IWidthHeightInterface)value;

            return -ao.Width / 2;
         }
         else if (value is LfStaticCircleViewModel)
         {
            LfStaticCircleViewModel ao = (LfStaticCircleViewModel)value;

            return -ao.Radius;
         }

         return null;

      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }

   class TextureOffsetYValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is IWidthHeightInterface)
         {
            IWidthHeightInterface ao = (IWidthHeightInterface)value;

            return -ao.Height / 2;
         }
         else if (value is LfStaticCircleViewModel)
         {
            LfStaticCircleViewModel ao = (LfStaticCircleViewModel)value;

            return -ao.Radius;
         }

         return null;

      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }

   class TextureCircleValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is ICircleInterface)
         {
            ICircleInterface ao = (ICircleInterface)value;

            return -ao.Radius;
         }

         return null;

      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }

}
