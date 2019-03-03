using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   class FlameEmitterWidthValueConverter : IMultiValueConverter
   {
      // Values should be: PosX, PosY, emitter angle, emitter width, Body PosX, Body PosY, Body Angle
      // Parameter is "1x" or "1y", returns rotated point 1, x or y value.
      // Parameter is "2x" or "2y", returns rotated point 2, x or y value.
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 7)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is double) &&
               (values[3] is double) && (values[4] is double) && (values[5] is double) &&
               (values[6] is double))
            {
               Point pos = new Point((double)values[0], (double)values[1]);
               double emitterAngle = (double)values[2];
               double emitterWidth = (double)values[3];

               // anglePoint is the point at the end of the vector from pos to 10 m in the 
               // pointAngle direction
               Vector angleVect = new Vector(
                  emitterWidth / 2 * Math.Cos((emitterAngle - 90) / 180 * Math.PI), 
                  emitterWidth / 2 * Math.Sin((emitterAngle - 90) / 180 * Math.PI));
               Point anglePoint1 = pos + angleVect;

               angleVect = new Vector(
                  emitterWidth / 2 * Math.Cos((emitterAngle + 90) / 180 * Math.PI),
                  emitterWidth / 2 * Math.Sin((emitterAngle + 90) / 180 * Math.PI));
               Point anglePoint2 = pos + angleVect;

               Point bodyPos = new Point((double)values[4], (double)values[5]);
               double bodyAngle = (double)values[6];
               Point rp1 = CoordinateTransformations.RotatedPointFromLocal(anglePoint1, bodyAngle);
               Point rp2 = CoordinateTransformations.RotatedPointFromLocal(anglePoint2, bodyAngle);
               rp1.Offset(bodyPos.X, bodyPos.Y);
               rp2.Offset(bodyPos.X, bodyPos.Y);

               if (parameter as string == "1x")
               {
                  return rp1.X;
               }
               else if(parameter as string == "1y")
               {
                  return rp1.Y;
               }
               else if (parameter as string == "2x")
               {
                  return rp2.X;
               }
               else if (parameter as string == "2y")
               {
                  return rp2.Y;
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
