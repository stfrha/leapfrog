using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   class TextureRectMultiValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 4)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is double) && (values[3] is double))
            {
               double offsetX = (double)values[0];
               double offsetY = (double)values[1];
               double textureWidth = (double)values[2];
               double textureHeight = (double)values[3];

               Rect r = new Rect(offsetX, offsetY, textureWidth, textureHeight);

               return r;
            }
         }

         return null;
      }
      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }




   class TextureRectValueConverter : IValueConverter
   {
      public object Convert(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (value is LfScalableTexturePolygonViewModel)
         {
            LfScalableTexturePolygonViewModel stpvm = (LfScalableTexturePolygonViewModel)value;

            Rect r = new Rect(stpvm.TextureOffsetX, stpvm.TextureOffsetY, stpvm.TextureWidth, stpvm.TextureHeight);

            return r;
         }


         return null;

      }

      public object ConvertBack(object value, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         return DependencyProperty.UnsetValue;
      }
   }
}
