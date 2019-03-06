using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   class CenterPointAnglePositionValueConverter : IMultiValueConverter
   {
      // Values should be: Start point X, Start point Y, ENd point X, End Point Y,
      // Emitter angle, Body PosX, Body PosY, Body Angle
      // Parameter is "1x" or "1y", returns center point, x or y value.
      // Parameter is "2x" or "2y", returns direction point, x or y value.
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 8)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is double) &&
               (values[3] is double) && (values[4] is double) && (values[5] is double) &&
               (values[6] is double) && (values[7] is double))
            {
               Point startPoint = new Point((double)values[0], (double)values[1]);
               Point endPoint = new Point((double)values[2], (double)values[3]);
               Point pos = new Point((startPoint.X + endPoint.X) / 2, (startPoint.Y + endPoint.Y) / 2);

               double pointAngle = (double)values[4] + 180;

               // anglePoint is the point at the end of the vector from pos to 10 m in the 
               // pointAngle direction
               Vector angleVect = new Vector(3 * Math.Cos(pointAngle / 180 * Math.PI), 3 * Math.Sin(pointAngle / 180 * Math.PI));
               Point anglePoint = pos + angleVect;

               Point bodyPos = new Point((double)values[5], (double)values[6]);
               double bodyAngle = (double)values[7];
               Point rpc = CoordinateTransformations.RotatedPointFromLocal(pos, bodyAngle);
               Point rpd = CoordinateTransformations.RotatedPointFromLocal(anglePoint, bodyAngle);
               rpc.Offset(bodyPos.X, bodyPos.Y);
               rpd.Offset(bodyPos.X, bodyPos.Y);

               if (parameter as string == "1x")
               {
                  return rpc.X;
               }
               else if (parameter as string == "1y")
               {
                  return rpc.Y;
               }
               else if (parameter as string == "2x")
               {
                  return rpd.X;
               }
               else if (parameter as string == "2y")
               {
                  return rpd.Y;
               }
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
