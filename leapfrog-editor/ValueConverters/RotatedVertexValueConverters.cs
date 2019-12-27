using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;

namespace LeapfrogEditor
{
   class RotatableBodyPointMultiConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         // Values should be: PosX, PosY, Angle. Parameter is "x" or "y", 
         // Returns rotated point x or y value, depending on parameter.
         if (values.Count() == 3)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is double))
            {
               Point p = new Point((double)values[0], (double)values[1]);
               double angle = (double)values[2];

               Point rotPoint = CoordinateTransformations.RotatedPointFromLocal(p, angle);

               if (parameter as string == "x")
               {
                  return rotPoint.X;
               }
               return rotPoint.Y;
            }
         }

         return null;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }

   class PreviousRotatableBodyPointMultiConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         // Values should be: PosX, PosY, Angle and either a LfDragablePointViewModel 
         // or a LfPointViewModel with its parent a IBoxPointsInterface. 
         // Parameter is "x" or "y", returns rotated point x or y value, depending on parameter.
         // PosX and PosY is not really used, but they will force an update if they are
         // changed
         if (values.Count() == 4)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is double))
            {
               Point p = new Point(0,0);
               double angle = (double)values[2];

               if (values[3] is LfDragablePointViewModel)
               {
                  LfDragablePointViewModel vertex;

                  LfDragablePointViewModel origVertex = (LfDragablePointViewModel)values[3];
                  LfPolygonViewModel polygon = origVertex.Parent;

                  // Resolve the previous point, it is index - 1
                  // except if index == 0, in which case it is 
                  // count - 1
                  int i = polygon.PointVms.IndexOf(origVertex);

                  if (i == -1)
                  {
                     return null;
                  }


                  if (i > 0)
                  {
                     vertex = polygon.PointVms[i - 1];
                  }
                  else
                  {
                     vertex = polygon.PointVms[polygon.PointVms.Count() - 1];
                  }

                  p = new Point(vertex.PosX, vertex.PosY);

               }
               else if (values[3] is LfPointViewModel)
               {
                  LfPointViewModel vertex;

                  LfPointViewModel origVertex = (LfPointViewModel)values[3];
                  IBoxPointsInterface boxVm = origVertex.PointsParent;

                  int i = boxVm.PointVms.IndexOf(origVertex);

                  if (i == -1)
                  {
                     return null;
                  }

                  if (i > 0)
                  {
                     vertex = boxVm.PointVms[i - 1];
                  }
                  else
                  {
                     vertex = boxVm.PointVms[boxVm.PointVms.Count() - 1];
                  }

                  p = new Point(vertex.PosX, vertex.PosY);
               }
               else
               {
                  return null;
               }

               Point rp = CoordinateTransformations.RotatedPointFromLocal(p, angle);

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               return rp.Y;
            }
         }

         return null;
      }

      public object[] ConvertBack(object value, Type[] targetTypes, object parameter, System.Globalization.CultureInfo culture)
      {
         throw new NotImplementedException();
      }
   }

   class AttachedBodyPointValueConverter : IMultiValueConverter
   {
      // Values should be: PosX, PosY, Body PosX, Body PosY, Body Angle
      // Parameter is "x" or "y", returns rotated point x or y value, depending on parameter.
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 5)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is double) &&
               (values[3] is double) && (values[4] is double))
            {
               Point pos = new Point((double)values[0], (double)values[1]);
               Point bodyPos = new Point((double)values[2], (double)values[3]);
               double bodyAngle = (double)values[4];
               Point rp = CoordinateTransformations.RotatedPointFromLocal(pos, bodyAngle);
               rp.Offset(bodyPos.X, bodyPos.Y);

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               else
               {
                  return rp.Y;
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

   class PointAnglePositionValueConverter : IMultiValueConverter
   {
      // Values should be: PosX, PosY, Point angle, Body PosX, Body PosY, Body Angle
      // Parameter is "x" or "y", returns rotated point x or y value, depending on parameter.
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 6)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] is double) &&
               (values[3] is double) && (values[4] is double) && (values[5] is double))
            {
               Point pos = new Point((double)values[0], (double)values[1]);
               double pointAngle = (double)values[2];

               // anglePoint is the point at the end of the vector from pos to 10 m in the 
               // pointAngle direction
               Vector angleVect = new Vector(3 * Math.Cos(pointAngle / 180 * Math.PI), 3 * Math.Sin(pointAngle / 180 * Math.PI));
               Point anglePoint = pos + angleVect;

               Point bodyPos = new Point((double)values[3], (double)values[4]);
               double bodyAngle = (double)values[5];
               Point rp = CoordinateTransformations.RotatedPointFromLocal(anglePoint, bodyAngle);
               rp.Offset(bodyPos.X, bodyPos.Y);

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               else
               {
                  return rp.Y;
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


   class PreviousMultiRotatedTriangleVertexValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 3)
         {
            if ((values[0] is double) && (values[1] is LfDragablePointViewModel) && (values[2] is TriangleViewModel))
            {
               LfDragablePointViewModel origVertex = (LfDragablePointViewModel)values[1];
               LfPolygonViewModel polygon = origVertex.Parent;
               TriangleViewModel tvm = (TriangleViewModel)values[2];

               int i = tvm.PointVms.IndexOf(origVertex);

               if (i == -1)
               {
                  return null;
               }

               LfDragablePointViewModel vertex;

               if (i > 0)
               {
                  vertex = tvm.PointVms[i - 1];
               }
               else
               {
                  vertex = tvm.PointVms[tvm.PointVms.Count() - 1];
               }

               Point p = new Point(vertex.PosX, vertex.PosY);
               Point rp = CoordinateTransformations.RotatedPointFromLocal(p, polygon.Angle);

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               else
               {
                  return rp.Y;
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

   class ConditionalRotatedJointValueConverter : IMultiValueConverter
   {
      public object Convert(object[] values, Type targetType, object parameter, System.Globalization.CultureInfo culture)
      {
         if (values.Count() == 7)
         {
            if ((values[0] is double) && (values[1] is double) && (values[2] != null) && (values[2] is LfShapeViewModel))
            {
               // In this case Shape B exists and is used. The conversion is done as in MultiRotatedJointValueConverter
               Point pos = new Point((double)values[0], (double)values[1]);
               LfShapeViewModel shape = (LfShapeViewModel)values[2];
               Point rp = CoordinateTransformations.RotatedPointFromLocal(pos, shape.Angle);
               rp.Offset(shape.AbsPosX, shape.AbsPosY);

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               else
               {
                  return rp.Y;
               }
            }
            else if((values[3] is double) && (values[4] is double) && (values[5] is LfShapeViewModel) && (values[6] is double))
            {
               // In this case Shape B does not exists, the end point will be as For shape A but a distance Length below

               Point pos = new Point((double)values[3], (double)values[4]);
               LfShapeViewModel shape = (LfShapeViewModel)values[5];
               double length = (double)values[6];
               Point rp = CoordinateTransformations.RotatedPointFromLocal(pos, shape.Angle);
               rp.Offset(shape.AbsPosX, shape.AbsPosY);

               rp.Y += length; 

               if (parameter as string == "x")
               {
                  return rp.X;
               }
               else
               {
                  return rp.Y;
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
